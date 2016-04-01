#pragma once

#include "pins.h"

typedef struct                     
{	
	uint8_t whl_sec[4], spd_sec[4], k[4], enc_work, spd_minute[4], error[4], error_2sec, sum_error, spd_zadnie_prev_sec[2], srednee_min[4];
	uint32_t whl_minute[4], my_system_time, whl_error[4];
} twheels;            

twheels wheels;

void init_encoders();
uint16_t get_AvgSecSpeed();
uint16_t get_AvgMinSpeed();
void encodersWork();


