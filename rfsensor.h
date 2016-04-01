/*
 * rfsensor.h
 *
 * Created: 01.10.2015 16:01:13
 *  Author: User
 */ 

#ifndef RFSENSOR_H_
#define RFSENSOR_H_

#include <avr/common.h>

extern volatile uint8_t rfDistances[4];
	
void rfWork();

#endif /* RFSENSOR_H_ */