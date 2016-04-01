#pragma once

#include <avr/common.h>

#include "header.h"

extern volatile FLKR_STATE_t flickerState;
extern volatile FLKR_MODE_t flickersMode;
extern volatile uint8_t flk_onTime;
extern volatile uint8_t flk_offTime;

void flickersWork();

void setFlickers(uint8_t side, uint8_t mode);
void setFlickersOFF();

void flickers_left();
void flickers_right();
void flickers_alert();

