/*
 * header.h
 *
 * Created: 19.03.2015 11:34:54
 *  Author: x-8973
 */


#ifndef HEADER_H_
#define HEADER_H_

#include <stdbool.h>
#include <avr/common.h>

#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define uchar unsigned char

/* Машины состояний ----------------------------------------------------------*/
typedef enum
{
	FLKR_STATE_INACTIVE,
	FLKR_STATE_OFF,
	FLKR_STATE_ON
} FLKR_STATE_t;

typedef enum
{
	FLKR_LEFT,
	FLKR_RIGHT,
	FLKR_ALL
} FLKR_MODE_t;

typedef enum
{
    RCVR_READY,
    RCVR_SYNCHRO_FIRSTBYTE_READY,
    RCVR_SYNCHRO_READY,
    RCVR_ADDR_READY,
    RCVR_LENGTH_READY
} RCVR_STATE_t;

typedef enum
{
	OFF,
	ON,
	SWITCH
} REVERSE_BRAKE_STATE_t;

void initMainTimer();

/* USART ---------------------------------------------------------------------*/
void initUSART(uint8_t baudrate);
void enableUSART_Transmit(bool enable);
void doUSART_Transmit(uint8_t ch);

#define SYNCHRO_BYTE					0xAE

#define PACKET_SYNCHRO_OFFSET			0
#define PACKET_TARGET_ADDRESS_OFFSET	2
#define PACKET_SOURCE_ADDRESS_OFFSET	3
#define PACKET_TYPE_OFFSET				4
#define PACKET_LENGTH_OFFSET			5
#define PACKET_CRC_OFFSET				6
#define PACKET_DATA_OFFSET				7

#define PACKET_HEADER_LENGTH			7
#define PACKET_MAX_LENGTH				50

typedef enum
{
	ALL,
	FRW,
	LFT,
	RGT
} BRAKES_t;

typedef enum
{
	INIT,
	ONLINE,
	OFFLINE,
	ERROR,
	FROM_CMD_PROCESS,
	FROM_TIMER
} OFFLINE_CHECK_t;

/* Дальномеры ------------------------------------ */

typedef enum {
	RF_FRONT = 0,
	RF_LEFT  = 1,
	RF_REAR  = 2,
	RF_RIGHT = 3,

	RF_LAST
} RANGEFINDER_SIDE_t;

/* ---------------------------------------------- */
uint8_t devAddr, devType, doUSARTLog;

int8_t getSign(int8_t);

//void changeAccelPosition(uint8_t delta, bool down);
uint8_t getCRC(uchar *packet, uint8_t packetLen);
void processPacket(uchar *packet, uint8_t packetLen);
void getDeviceAddrType(uint8_t *addr, uint8_t *type);

bool isZeroReached(int8_t current);

void initTWIMaster();
void TWISetStart();

void flickersWork();

void encodersWork();

//void changeAccelPos(uchar * packet, bool down);
void reverse(REVERSE_BRAKE_STATE_t r_state);
void brake_frw(REVERSE_BRAKE_STATE_t b_state);
void brake_lft(REVERSE_BRAKE_STATE_t b_state);
void brake_rgt(REVERSE_BRAKE_STATE_t b_state);
void full_stop();

void setFlickersOFF();
void setFlickers(uint8_t side, uint8_t mode);

void transmitBrakeReverseState();
void transmitReverseState();
void transmitBrakeState(BRAKES_t param1);
void transmitKRUState();
void transmitSpeedData();
void transmitRFData();

extern OFFLINE_CHECK_t state, prev_state;
// таймер проверки оффлайна
extern volatile uint16_t last_command_time, current_time;
// энкодеры

uint8_t mainTimerCounter;		// Переменная для отсчета 1 секунды

#endif /* HEADER_H_ */

