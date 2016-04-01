#pragma once

#include <avr/common.h>
#include <stdbool.h>

#define IS_NOT_RIGHT_LIMIT (LIM_PIN & _BV(LIM_RIGHT))
#define IS_RIGHT_LIMIT   (!(LIM_PIN & _BV(LIM_RIGHT)))
#define IS_NOT_LEFT_LIMIT  (LIM_PIN & _BV(LIM_LEFT ))
#define IS_LEFT_LIMIT    (!(LIM_PIN & _BV(LIM_LEFT )))
#define IS_BOTH_LIMIT    (!(LIM_PIN & _BV(LIM_LEFT)) && !(LIM_PIN & _BV(LIM_RIGHT)))

#define GO_LEFT  (COM1_3_PORT |= _BV(COM1))
#define GO_RIGHT (COM1_3_PORT |= _BV(COM2))
#define STOP_RUDDER_ENGINE (COM1_3_PORT &= ~_BV(COM2) & ~_BV(COM1))

volatile int8_t rudderPauseCnt,
				targetRudderPos;

volatile int8_t minRudderPos,
				 maxRudderPos,
				 mdlRudderPos,
				 currentRudderPos;

volatile uint16_t KRUsteerTimeout,
				KRUtimeCounter,
				KRUinitCounter;

uint32_t        kru_ot;

uint8_t D;

//volatile uint16_t KRUzeroCounter;

typedef enum
{
	STATE_GOTO_ZERO  = 10,
	STATE_IDLE		 = 11,
	STATE_SPIN_RIGHT = 12,
	STATE_SPIN_LEFT  = 13,
	STATE_SPIN_OK    = 14,
	STATE_SPIN_FAIL  = 15,
	STATE_INIT       = 16
} RUDDER_STATE_MACHINE_t;

volatile RUDDER_STATE_MACHINE_t rudderState;
volatile RUDDER_STATE_MACHINE_t oldRudderState;
volatile bool isCommRcv;

void rudderFindZero();
void rudderWork();
bool isZeroReached(int8_t current);
void KRU_encoderInit();
void initKRUVars();
void setRudderState(RUDDER_STATE_MACHINE_t new_state);
void measureKRUSHIM();

typedef enum
{
	FIND_LEFT = 100,
	LEFT_OK   = 101,
	FIND_RGHT = 102,
	RGHT_OK   = 103,
	FIND_MDDL = 104,
	MDDL_OK   = 105
} RUDDER_INIT_STATE;

RUDDER_INIT_STATE rudderInitState, oldInitState;
