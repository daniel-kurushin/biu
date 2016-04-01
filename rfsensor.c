/*
 * rfsensor.c
 *
 * Created: 01.10.2015 16:01:53
 *  Author: User
 */

#include "header.h"
#include "rfsensor.h"
#include "pins.h"
#include "kru.h"
#include <util/delay.h>

uint8_t rfCounter = 0;
volatile RANGEFINDER_SIDE_t rfCurrent = RF_FRONT;
volatile uint8_t rfDistances[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

//прерывание от echo дальномеров
ISR(INT2_vect)
{
	switch (devType)
	{
		case 0x01:
		{
			cli();

			if (RANGE_ECHO_PIN & _BV(RANGE_ECHO)) //передний фронт
			{
				//сброс значения таймера
				TCNT2 = 0x00;

				//переключаем INT2 на срабатывание по заднему фронту
				MCUCSR &= ~_BV(ISC2);
			}
			else //задний фронт
			{
				//кол-во тиков таймера для текущего дальномера
				rfDistances[rfCurrent] = TCNT2;

				//переключаем INT2 на срабатывание по переднему фронту
				MCUCSR |= _BV(ISC2);
			}

			sei();

			break;
		}
		case 0x02:
		{
		    cli();
		    if (PINB & _BV(PB2))
            {
                MCUCSR &= ~_BV(ISC2);
                TCNT1 = 0x00;
            }
            else
            {
                MCUCSR |= _BV(ISC2);
                currentRudderPos = TCNT1/512 - mdlRudderPos;
            }
            sei();
            break;
		}
	}

	return;

	// Глобальный запрет прерываний


	// Глобально разрешить прерывания
}

void rfWork()
{
    /*
    COM1_3_PORT ^= _BV(COM1);
    COM1_3_PORT ^= _BV(COM2);
    COM1_3_PORT ^= _BV(COM3);
    */
//    RANGE_ECHO_TRIG_1_3_PORT ^= _BV(RANGE_TRIG1);
//    _delay_us(20);
//    RANGE_ECHO_TRIG_1_3_PORT ^= _BV(RANGE_TRIG1);
//    _delay_us(20);
//    RANGE_ECHO_TRIG_1_3_PORT ^= _BV(RANGE_TRIG2);
//    _delay_us(20);
//    RANGE_ECHO_TRIG_1_3_PORT ^= _BV(RANGE_TRIG2);
//    _delay_us(20);
//    RANGE_ECHO_TRIG_1_3_PORT ^= _BV(RANGE_TRIG3);
//    _delay_us(20);
//    RANGE_ECHO_TRIG_1_3_PORT ^= _BV(RANGE_TRIG3);
//    _delay_us(20);
	if(++rfCounter > 7)
	{
		cli();

		rfCounter = 0;
		rfCurrent = (rfCurrent + 1) % RF_LAST;

		uint8_t trig_pins[] = { RANGE_TRIG1, RANGE_TRIG2, RANGE_TRIG3, RANGE_TRIG4 };

		volatile uint8_t *cur_port = (rfCurrent == RF_LAST - 1) ? &RANGE_TRIG_4_PORT : &RANGE_ECHO_TRIG_1_3_PORT;
		uint8_t cur_trig = trig_pins[rfCurrent];

		//импульс на trig
		*cur_port |= _BV(cur_trig);
		_delay_us(10);
		*cur_port &= ~_BV(cur_trig);



		sei();
	}
}
