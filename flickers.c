/*
 * flickers.c
 *
 * Created: 01.10.2015 15:59:22
 *  Author: User
 */ 

#include "header.h"
#include "flickers.h"
#include "pins.h"

volatile FLKR_STATE_t flickerState = FLKR_STATE_INACTIVE;
volatile FLKR_MODE_t flickersMode = FLKR_ALL;
volatile uint8_t flk_onTime = 61;
volatile uint8_t flk_offTime = 61;

uint8_t flk_counter = 0;

void flickersWork()
{
	if (flickerState != FLKR_STATE_INACTIVE)
	{
		if (--flk_counter <= 0)
		{
			if ((flickersMode == FLKR_LEFT) || (flickersMode == FLKR_ALL))
				COM1_3_PORT ^= _BV(COM3);

			if ((flickersMode == FLKR_RIGHT) || (flickersMode == FLKR_ALL))
				COM4_PORT ^= _BV(COM4);
			
			if (flickerState == FLKR_STATE_ON)
			{
				flk_counter = flk_offTime;
				flickerState = FLKR_STATE_OFF;
			}
			else
			{
				flk_counter = flk_onTime;
				flickerState = FLKR_STATE_ON;
			}
		}		
	}
}

void setFlickers(uint8_t side, uint8_t mode)
{
	switch (side)
	{
		case 0x00: flickersMode = FLKR_LEFT; break;
		case 0x01: flickersMode = FLKR_RIGHT; break;
		case 0x02: flickersMode = FLKR_ALL; break;
	}
	switch (mode)
	{
		case 0x00: flk_onTime = 60; flk_offTime = 60; break; // Поворот
		case 0x01: flk_onTime = 20; flk_offTime = 100; break; // Вспышки
		case 0x02: flk_onTime = 100; flk_offTime = 20; break; // Погасание
	}
	
	flk_counter = flk_onTime;
	flickerState = FLKR_STATE_ON;
}

void setFlickersOFF()
{
	flickerState = FLKR_STATE_INACTIVE;
	COM1_3_PORT &= ~_BV(COM3);
	COM4_PORT &= ~_BV(COM4);
}

void flickers_left()
{
	setFlickers(0,0);
}

void flickers_right()
{
	setFlickers(1,0);
}

void flickers_alert()
{
	setFlickers(2,0);
}
