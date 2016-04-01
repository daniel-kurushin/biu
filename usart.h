/*
 * usart.h
 *
 * Created: 01.10.2015 15:29:02
 *  Author: User
 */

#ifndef USART_H_
#define USART_H_

#include "header.h"

#include <avr/common.h>
#include <stdbool.h>

void initUSART(uint8_t baudrate);
void enableUSART_Transmit(bool enable);
void doUSART_Transmit(uint8_t ch);

void transmitAccelState();
void transmitReverseState();
void transmitFlickersState();
void transmitBrakeReverseState();
void transmitKRUState();
void transmitSpeedData();
void transmitRFData();
void transmitBrakeState(BRAKES_t param1);
void transmitSynchro(uint8_t len);

void setOnlineOffline(OFFLINE_CHECK_t a_state);
void check_offline_state(OFFLINE_CHECK_t mode);
uint8_t getCRC(uchar *packet, uint8_t packetLen);
void processPacket(uchar *packet, uint8_t packetLen);

#endif /* USART_H_ */
