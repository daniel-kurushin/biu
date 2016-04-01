/*
 * kru.c
 *
 * Created: 01.10.2015 15:53:28
 *  Author: User
 */

#include "header.h"
#include "kru.h"
#include "pins.h"
#include "flickers.h"
#include "usart.h"

#include <stdlib.h>
#include <avr/common.h>

void initKRUVars()
{
	rudderPauseCnt   =    0;
	minRudderPos     = -100;
	mdlRudderPos     =    0;
	maxRudderPos     =  100;
	currentRudderPos =    0;
//	oldRudderPos	 =    0;
	targetRudderPos  =    0;
	KRUsteerTimeout  =  488;
	KRUtimeCounter   =    0;
	KRUinitCounter   =    0;
	rudderInitState  = FIND_LEFT;
	rudderState		 = STATE_INIT;
	oldRudderState   = STATE_INIT;
	isCommRcv		 = false;
}

void KRU_encoderInit()
{
	//// Инициализировать пины
	//DDRD &= ~_BV(PD3) & ~_BV(PD2); // Ноги прерываний INT1
	//PORTD |= _BV(PD3) | _BV(PD2);
	//
	//// Настроить прерывания
	//GICR |= _BV(INT1) | _BV(INT0); //Разрешить прерывание INT1 (pd3)
	//MCUCR |= _BV(ISC11) | _BV(ISC01); //INT1 дергаются по спадающему фронту
	DDRB   &= ~_BV(PB2);
	PORTB  |= _BV(PB2);
	GICR   |= _BV(INT2); //Разрешить прерывание INT2 (PB2) (echo дальномера)
	MCUCSR |= _BV(ISC2); //INT2 дергается по переднему фронту
}

void rudderFindZero()
{
	rudderState = STATE_INIT;
}

void check_KRU_timeout()
{
	if (KRUtimeCounter > KRUsteerTimeout)
	{
		KRUtimeCounter = 0;
		setRudderState(STATE_SPIN_FAIL);
	}
	else
	{
		KRUtimeCounter++;
	}
}

void initRudderMachine()
{
	switch (rudderInitState)
	{
		case FIND_LEFT:
		{
			check_KRU_timeout();
			if (IS_NOT_LEFT_LIMIT)
			{
				GO_LEFT;
			}
			else
			{
				rudderInitState = LEFT_OK;
				KRUtimeCounter = 0;
				rudderPauseCnt = 0;
			}
			break;
		}
		case LEFT_OK:
		{
			if (rudderPauseCnt++ < 62)
			{
				STOP_RUDDER_ENGINE;
			}
			else
			{
				rudderInitState = FIND_RGHT;
				maxRudderPos = currentRudderPos;
			}
			break;
		}
		case FIND_RGHT:
		{
			check_KRU_timeout();
			if (IS_NOT_RIGHT_LIMIT)
			{
				GO_RIGHT;
			}
			else
			{
				rudderInitState = RGHT_OK;
				rudderPauseCnt = 0;
				KRUtimeCounter = 0;
			}
			break;
		}
		case RGHT_OK:
		{
			if (rudderPauseCnt++ < 62)
			{
				STOP_RUDDER_ENGINE;
				if (rudderPauseCnt == 10)
                {
                    minRudderPos = currentRudderPos;
                    mdlRudderPos = (maxRudderPos + minRudderPos) / 2;
                }
				if (rudderPauseCnt == 20)
                {
                    minRudderPos = currentRudderPos;
                    maxRudderPos = - minRudderPos;
                }
			}
			else
			{
				rudderInitState = FIND_MDDL;
			}
			break;
		}
		case FIND_MDDL:
		{
			check_KRU_timeout();
			if (IS_NOT_LEFT_LIMIT && (currentRudderPos < 0))
			{
				GO_LEFT;
			}
			else if (IS_LEFT_LIMIT)
			{
				STOP_RUDDER_ENGINE;
				setRudderState(STATE_SPIN_FAIL);
				KRUtimeCounter = 0;
			}
			else
			{
				STOP_RUDDER_ENGINE;
				rudderInitState = MDDL_OK;
				rudderPauseCnt = 0;
				KRUtimeCounter = 0;
			}
			break;
		}
		case MDDL_OK:
		{
			setRudderState(STATE_IDLE);
			KRUtimeCounter = 0;
			break;
		}
		default:
		{
			//rudderState = STATE_SPIN_FAIL;
		}
	}
}

void setRudderState(RUDDER_STATE_MACHINE_t new_state)
{
	rudderState = new_state;
}

void rudderWork()
{
	switch (rudderState)
	{
		case STATE_INIT:
		{
			if(++KRUinitCounter > 244) // пауза
			{
				KRUinitCounter = 0;
				KRUtimeCounter = 0;
				setRudderState(STATE_GOTO_ZERO);
			}
			break;
		}
		case STATE_GOTO_ZERO:
		{
			initRudderMachine();
			break;
		}
		case STATE_SPIN_RIGHT:     //   +----------------- Еще не доехали
		{                          //   |
			check_KRU_timeout();   //   v
			if (IS_NOT_RIGHT_LIMIT && targetRudderPos < currentRudderPos)
			{
				GO_RIGHT;
			}
			else
			{
				setRudderState(STATE_SPIN_OK);
			}
			break;
		}
		case STATE_SPIN_LEFT:      //   +----------------- Еще не доехали
		{                          //   |
			check_KRU_timeout();   //   v
			if (IS_NOT_LEFT_LIMIT  && targetRudderPos > currentRudderPos)
			{
				GO_LEFT;
			}
			else
			{
				setRudderState(STATE_SPIN_OK);
			}
			break;
		}
		case STATE_SPIN_OK:
		{
			KRUtimeCounter = 0;
			STOP_RUDDER_ENGINE;
			transmitSynchro(2);
			doUSART_Transmit(0xff);
			doUSART_Transmit(currentRudderPos);
			setRudderState(STATE_IDLE);
			break;
		}
		case STATE_SPIN_FAIL:
		{
			KRUtimeCounter = 0;
			STOP_RUDDER_ENGINE;
			if (oldRudderState != STATE_SPIN_FAIL)
			{
                transmitSynchro(2);
				doUSART_Transmit(0xaa);
				doUSART_Transmit(currentRudderPos);
			}
			break;
		}
		case STATE_IDLE:
		{
			KRUtimeCounter = 0;
			if (isCommRcv)
			{
				isCommRcv = false;
				if (targetRudderPos > currentRudderPos) setRudderState(STATE_SPIN_LEFT);
				if (targetRudderPos < currentRudderPos) setRudderState(STATE_SPIN_RIGHT);
				if (targetRudderPos == currentRudderPos) setRudderState(STATE_SPIN_OK);
			}
			break;
		}
	}
	oldRudderState = rudderState;
}
/*
bool isZeroReached(int8_t current)
{
	if (current == 0) return true;
	else
	{
		if (KRUzeroCounter < 200)
		{
			KRUzeroCounter++;
			if (current < D)
			{
				D = current;
			}
			return false;
		}
		else
		{
			return current < D + 1;
		}
	}
}

			//oldRudderState = STATE_SPIN_RIGHT;
			//check_KRU_timeout();
			//if (targetRudderPos <= currentRudderPos)
			//{
			//rudderState = STATE_SPIN_OK;
			//doUSART_Transmit(0xff);
			//doUSART_Transmit(currentRudderPos);
			//}
			//else
			//if (IS_NOT_RIGHT_LIMIT)
			//{
			//rudderState = STATE_SPIN_OK;
			//doUSART_Transmit(0x01);
			//doUSART_Transmit(currentRudderPos);
			//}
			//else COM1_3_PORT |= _BV(COM2);

			//oldRudderState = STATE_SPIN_LEFT;
			//check_KRU_timeout();
			//if (targetRudderPos >= currentRudderPos)
			//{
			//rudderState = STATE_SPIN_OK;
			//doUSART_Transmit(0xff);
			//doUSART_Transmit(currentRudderPos);
			//}
			//else
			//if (!(LIM_PIN & _BV(LIM_LEFT)))
			//{
			//rudderState = STATE_SPIN_OK;
			//doUSART_Transmit(0x02);
			//doUSART_Transmit(currentRudderPos);
			//}
			//else COM1_3_PORT |= _BV(COM1);
			break;
*/

