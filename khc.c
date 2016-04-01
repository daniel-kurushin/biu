/*
 * khc.c
 *
 * Created: 01.10.2015 15:54:32
 *  Author: User
 */ 

#include "header.h"
#include "khc.h"
#include "pins.h"

#include <util/delay.h>

volatile uint8_t currentAccelPos = 0;

void changeAccelPosition(uint8_t delta, bool down)
{
	ACCEL_PORT |= _BV(ACCEL_EN);
	_delay_ms(1);
	if (down)
	{
		ACCEL_PORT |=_BV(ACCEL_DWN);
		_delay_ms(1);
	} 
	for (uint8_t i = 0; i < delta; i++)
	{
		ACCEL_PORT |= _BV(ACCEL_CLK);
		_delay_ms(1);
		ACCEL_PORT &= ~_BV(ACCEL_CLK);
		_delay_ms(1);
		if (!down && currentAccelPos < 100) currentAccelPos++;
		if ( down && currentAccelPos >   0) currentAccelPos--;
	}
	ACCEL_PORT &= _BV(ACCEL_DWN);
	ACCEL_PORT &= _BV(ACCEL_EN);
}

void reverse(REVERSE_BRAKE_STATE_t r_state)
{
	STOP_ENGINE;
	switch (r_state)
	{
		case ON:    { COM1_3_PORT |= _BV(COM1); break; }
		case OFF:   { COM1_3_PORT &= ~_BV(COM1);break; }
		case SWITCH:{ COM1_3_PORT ^= _BV(COM1); break; }
	}
}

void brake_frw(REVERSE_BRAKE_STATE_t b_state)
{
	if (b_state != OFF && currentAccelPos > 0) STOP_ENGINE;
	switch (b_state)
	{
		case ON:    { COM1_3_PORT |= _BV(COM2); break; }
		case OFF:   { COM1_3_PORT &= ~_BV(COM2);break; }
		case SWITCH:{ COM1_3_PORT ^= _BV(COM2); break; }
	}
}

void brake_lft(REVERSE_BRAKE_STATE_t b_state)
{
	switch (b_state)
	{
		case ON:    { COM1_3_PORT |= _BV(COM3); break; }
		case OFF:   { COM1_3_PORT &= ~_BV(COM3);break; }
		case SWITCH:{ COM1_3_PORT ^= _BV(COM3); break; }
	}
}

void brake_rgt(REVERSE_BRAKE_STATE_t b_state)
{
	switch (b_state)
	{
		case ON:    { COM4_PORT |= _BV(COM4); break; }
		case OFF:   { COM4_PORT &= ~_BV(COM4);break; }
		case SWITCH:{ COM4_PORT ^= _BV(COM4); break; }
	}
}

void changeAccelPos(uchar * packet, bool down)
{
	uint8_t delta = packet[PACKET_DATA_OFFSET];
	changeAccelPosition(delta, down);
}

void full_stop()
{
	STOP_ENGINE;
}
