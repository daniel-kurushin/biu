#pragma once

#include <stdbool.h>

#include "header.h"

#define STOP_ENGINE changeAccelPosition(100, true)

extern volatile uint8_t currentAccelPos;

void changeAccelPosition(uint8_t delta, bool down);
void reverse(REVERSE_BRAKE_STATE_t r_state);
void brake_frw(REVERSE_BRAKE_STATE_t b_state);
void brake_lft(REVERSE_BRAKE_STATE_t b_state);
void brake_rgt(REVERSE_BRAKE_STATE_t b_state);
void changeAccelPos(uchar * packet, bool down);

void full_stop();

