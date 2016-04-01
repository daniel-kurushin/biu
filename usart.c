/*
 * usart.c
 *
 * Created: 01.10.2015 15:29:12
 *  Author: User
 */

#include "usart.h"
#include "pins.h"
#include "khc.h"
#include "kru.h"
#include "rfsensor.h"
#include "flickers.h"
#include "encoders.h"

#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

RCVR_STATE_t receiverState;
unsigned char *rxBuffer;
uint8_t rxBufferDataLen;

unsigned char *txBuffer;
uint8_t txBufferDataLen;
uint8_t txBufferDataPos;

OFFLINE_CHECK_t state, prev_state = INIT;
// таймер проверки оффлайна
volatile uint16_t last_command_time, current_time = 0;
volatile bool isCommRcv = false;

void processPacket(uchar *packet, uint8_t packetLen)
{
	check_offline_state(FROM_CMD_PROCESS);
	// Вычислить контрольную сумму пакета
	//uint8_t sum = funcGetCRC(packet, packetLen);
	// Считать заявленную контрольную сумму
	//uint8_t crc = packet[PACKET_CRC_OFFSET];
	//if (sum == crc)
	//{
	if (packet[PACKET_TYPE_OFFSET] == 0x00)
	{
		// Обработка для всех устройств одинаковая
		// Получен запрос типа устройства.
		uchar *answerPacket = (uchar *)malloc(8); // Создаем пакет длиной 8 байт
		answerPacket[PACKET_SYNCHRO_OFFSET] = 0xAE;
		answerPacket[PACKET_SYNCHRO_OFFSET+1] = 0xAE;
		answerPacket[PACKET_TARGET_ADDRESS_OFFSET] = 0x00;
		answerPacket[PACKET_SOURCE_ADDRESS_OFFSET] = devAddr;
		answerPacket[PACKET_TYPE_OFFSET] = 0x00;
		answerPacket[PACKET_LENGTH_OFFSET] = 8;
		answerPacket[PACKET_DATA_OFFSET] = devType;
		answerPacket[PACKET_CRC_OFFSET] = getCRC(answerPacket, 8);

		// Скопировать пакет в буфер передачи
		memcpy(txBuffer, answerPacket, 8);
		txBufferDataLen = 8;
		txBufferDataPos = 0;

		// Инициировать отправку
		enableUSART_Transmit(true);
		// Освободить выделенную под пакет память
		free(answerPacket);
	}

	// Дальше работаем в обычном режиме
	switch (devType)
	{
		case 0x01:
		{
			// Команды для КХЧ
			switch (packet[PACKET_TYPE_OFFSET])
			{
				case 0x01://aeae0100f10800NN увеличить обороты и подтвердить результат
				{
					changeAccelPos(packet, false);
					transmitSynchro(1);
					transmitAccelState(); break;
				}
				case 0x02://aeae0100f20800NN уменьшить обороты и подтвердить результат
				{
					changeAccelPos(packet, true);
					transmitSynchro(1);
					transmitAccelState(); break;
				}
				case 0x03://aeae0100030700
				{
					reverse(SWITCH);
					transmitSynchro(1);
					transmitReverseState(); break;
				}
				case 0x04://aeae0100040700
				{
					brake_frw(SWITCH);
					transmitSynchro(1);
					transmitBrakeState(FRW); break;
				}
				case 0x05://aeae0100050700
				{
					brake_lft(SWITCH);
					transmitSynchro(1);
					transmitBrakeState(LFT); break;
				}
				case 0x06://aeae0100060700
				{
					brake_rgt(SWITCH);
					transmitSynchro(1);
					transmitBrakeState(RGT); break;
				}
				case 0x07: //aeae0100070700 остановка
				{
					full_stop();
					transmitSynchro(1);
					transmitAccelState(); break;
				}
				case 0x08: //aeae0100080700 дальномеры
				{
					transmitSynchro(4);
				    transmitRFData();
				    break;
                }
				case 0x09: //aeae0100090700 энкодеры колес
				{
					transmitSynchro(20);
				    transmitSpeedData( );
				    break;
                }
				case 0x0A: //aeae01000A0800xx вкл-выкл реверса
				{
					uint8_t rev = packet[PACKET_DATA_OFFSET];
					if (rev) reverse(ON); else reverse(OFF);
					transmitSynchro(1);
					transmitReverseState();
					break;
				}
				case 0x11: //aeae0100110a00xxyyzz тормоза
				//              | | +--- правый 0 - выкл, 1 - вкл
				//              | +----- левый
				//              +------- передний
				{
					uint8_t frw = packet[PACKET_DATA_OFFSET];
					uint8_t lft = packet[PACKET_DATA_OFFSET+1];
					uint8_t rgt = packet[PACKET_DATA_OFFSET+2];
					if (frw) brake_frw(ON); else brake_frw(OFF);
					if (lft) brake_lft(ON); else brake_lft(OFF);
					if (rgt) brake_rgt(ON); else brake_rgt(OFF);
					transmitSynchro(3);
					transmitBrakeState(ALL);
					break;
				}
				case 0xff:  //aeae0100ff0700 вернуть состояние КХЧ
							// 0 - currentAccelPos - обороты
							// 1 - ff - вкл передний тормоз 00 - выкл
							// 2 - ff - вкл левый тормоз    00 - выкл
							// 3 - ff - вкл правый тормоз   00 - выкл
							// 4 - ff - вкл реверс          00 - выкл
							// 5 - срабатываний энкодера в сек
							// 6 - срабатываний энкодера в мин
				{
				    transmitSynchro(7);
					doUSART_Transmit(currentAccelPos);
					transmitBrakeReverseState();
					doUSART_Transmit(get_AvgSecSpeed());
					doUSART_Transmit(get_AvgMinSpeed());
					break;
				}
				default:
				{
					doUSART_Transmit(packet[PACKET_TYPE_OFFSET]);
				}
			}
			break;
		}
		case 0x02:
		{
			// Команды для КРУ
			switch (packet[PACKET_TYPE_OFFSET])
			{
				case 0x01: //aeae02000108xx00 повернуть в поз хх
				{
					targetRudderPos = packet[PACKET_DATA_OFFSET];
					isCommRcv = true; // Команда на поворот получена
					break;
				}
				case 0x02: //aeae0200020700 вернуть текущее положение
				{
				    transmitSynchro(1);
					doUSART_Transmit(currentRudderPos);
					isCommRcv = false;
					break;
				}
				case 0x03: //aeae0200030900xxxx                                 // сторона: 0х00 левый
				{																//          0х01 правый
					// Получено "Включить указатели поворота"					//			0х02 оба
					uint8_t side = packet[PACKET_DATA_OFFSET];					// режим:   0х00 поворот       --__--__--
					uint8_t mode = packet[PACKET_DATA_OFFSET+1];				//          0х01 затмевание    ---_---_--
                                                                                //          0х02 проблеск      -___-___-
					setFlickers(side, mode);
					transmitSynchro(4);
					transmitFlickersState();
					break;
				}
				case 0x04: //aeae0200040700 отключить указатели поворота
				{
					setFlickersOFF();
					transmitSynchro(4);
					transmitFlickersState();
					break;
				}
				case 0x05: //aeae0200050700 перезапустить машину состояний
				{
					initKRUVars();
					transmitSynchro(1);
					doUSART_Transmit(0xff);
					break;
				}
				case 0xFF: //aeae0200050700 вернуть состояние КРУ
				{
					transmitSynchro(12);
					transmitKRUState();
					break;
				}
			}
			break;
		}
	}

	// Освободить выделенную под пакет память
	free(packet);
	//}
}

uint8_t getCRC(uchar *packet, uint8_t packetLen)
{
	uint8_t i = 0;
	uint8_t sum = 0;
	for (i = 0; i < packetLen; i++)
	{
		sum += packet[i];
	}
	sum -= packet[PACKET_CRC_OFFSET];
	return sum;
}

void check_offline_state(OFFLINE_CHECK_t mode)
{
	if (mode == FROM_TIMER)
	{
		if(current_time > last_command_time)
		{
			switch (devType)
			{
				case 0x01:
				{
					if (current_time - last_command_time > 244) state = OFFLINE;
					break;
				}
				case 0x02:
				{
					if (current_time - last_command_time > 2440) state = OFFLINE;
					break;
				}
				case 0x03: { break;	}
				case 0x04: { break;	}
			}
		}
	}
	else
	{
		state = ONLINE;
		last_command_time = current_time;
	}
	//if (state == OFFLINE && prev_state == INIT   ) setOnlineOffline(OFFLINE);
	//if (state == ONLINE  && prev_state == INIT   ) setOnlineOffline(ONLINE);
	if (state == OFFLINE && prev_state == ONLINE ) setOnlineOffline(OFFLINE);
	if (state == ONLINE  && prev_state == OFFLINE) setOnlineOffline(ONLINE );
}

void setOnlineOffline(OFFLINE_CHECK_t a_state)
{
	prev_state = a_state;
	if (a_state == OFFLINE)
	{
		switch (devType)
		{
			case 0x01:
			{
				if (currentAccelPos > 0) {
					changeAccelPosition(currentAccelPos, 1);
				}

				brake_frw(ON);
				reverse(OFF);
				break;
			}
			case 0x02: {
				setFlickers(0x02, 0);
				break;
			}
			case 0x03: { break; }
			case 0x04: { break; }
		}
	}
	if (a_state == ONLINE)
	{
		switch (devType)
		{
			case 0x01: {
				brake_frw(OFF);
				brake_rgt(OFF);
				brake_lft(OFF);
			}
			case 0x02: {
				setFlickersOFF();
				break;
			}
			case 0x03: { break; }
			case 0x04: { break; }
		}
	}
}

// Прерывание по окончанию приема байта модулем USART
ISR(USART_RXC_vect)
{
	// Глобальный запрет прерываний
	cli();

	unsigned char ch = UDR;

	switch (receiverState)
	{
		case RCVR_READY:
		// Приемник готов. Ждем первый синхро-байт.
		if (ch == SYNCHRO_BYTE)
		{
			// Если байт похож на синхро-байт, то добавляем его в буфер
			// и изменяем состояние автомата
			rxBuffer[rxBufferDataLen] = ch;
			rxBufferDataLen++;
			receiverState = RCVR_SYNCHRO_FIRSTBYTE_READY;
		}
		break;
		case RCVR_SYNCHRO_FIRSTBYTE_READY:
		// Первый синхро-байт принят. Ждем второй синхро-байт.
		if (ch == SYNCHRO_BYTE)
		{
			// Если байт похож на синхро-байт, то добавляем его в буфер
			// и изменяем состояние автомата
			rxBuffer[rxBufferDataLen] = ch;
			rxBufferDataLen++;
			receiverState = RCVR_SYNCHRO_READY;
		}
		else
		{
			// иначе очищаем буфер и ставим состояние "Готов".
			rxBufferDataLen = 0;
			receiverState = RCVR_READY;
		}
		break;
		case RCVR_SYNCHRO_READY:
		// Оба синхро-байта приняты. Ждем адрес пакета.
		if (ch == devAddr)
		{
			// Если адрес пакета равен адресу устройства, то добавляем байт в буфер
			// и изменяем состояние автомата
			rxBuffer[rxBufferDataLen] = ch;
			rxBufferDataLen++;
			receiverState = RCVR_ADDR_READY;
		}
		else
		{
			// иначе очищаем буфер и ставим состояние "Готов".
			rxBufferDataLen = 0;
			receiverState = RCVR_READY;
		}
		break;
		case RCVR_ADDR_READY:
		// Адрес принят и подтвержден. Ждем длину пакета.
		rxBuffer[rxBufferDataLen] = ch;
		rxBufferDataLen++;
		if (rxBufferDataLen > PACKET_LENGTH_OFFSET)
		{
			uint8_t packetLength = rxBuffer[PACKET_LENGTH_OFFSET];
			// Если длина пакета принята, то проверяем ее валидность
			if ((packetLength >= PACKET_HEADER_LENGTH)
			& (packetLength <= PACKET_MAX_LENGTH))
			{
				// Если длина валидна, то изменяем состояние автомата
				receiverState = RCVR_LENGTH_READY;
			}
			else
			{
				// иначе очищаем буфер и ставим состояние "Готов".
				rxBufferDataLen = 0;
				receiverState = RCVR_READY;
			}
		}
		break;
		case RCVR_LENGTH_READY:
		// Длина пакета принята. Ждем приема всех байт пакета.
		rxBuffer[rxBufferDataLen] = ch;
		rxBufferDataLen++;
		if (rxBufferDataLen == rxBuffer[PACKET_LENGTH_OFFSET])
		{
			// Если пакет принят целиком, то передаем его на обработку,
			// очищаем буфер и ставим состояние "Готов"
			unsigned char *buffer = (unsigned char *)malloc(rxBufferDataLen);
			memcpy(buffer, rxBuffer, rxBufferDataLen);
			processPacket(buffer, rxBufferDataLen);
			rxBufferDataLen = 0;
			receiverState = RCVR_READY;
		}
		break;
	}

	// Глобально разрешить прерывания
	sei();
}

// Прерывание по опустошению буфера передачи USART
ISR(USART_UDRE_vect)
{
	// Глобальный запрет прерываний
	cli();

	// Если еще есть данные для передачи, то
	if (txBufferDataPos < txBufferDataLen)
	{
		// Скармливаем очередной байт буферу передачи
		UDR = txBuffer[txBufferDataPos];
		// Увеличиваем текущую позицию в буфере
		txBufferDataPos++;
	}
	else
	// Иначе гасим прерывания на опустошение буфера передачи
	enableUSART_Transmit(false);

	// Глобально разрешить прерывания
	sei();
}

void transmitBrakeState(BRAKES_t param1)
{
	switch (param1)
	{
		case ALL:
		{
			transmitBrakeState(FRW);
			transmitBrakeState(LFT);
			transmitBrakeState(RGT);
			break;
		}
		case FRW:
		{
			if (COM1_3_PORT & _BV(COM2)) doUSART_Transmit(0xff);
			else doUSART_Transmit(0x00);
			break;
		}
		case LFT:
		{
			if (COM1_3_PORT & _BV(COM3)) doUSART_Transmit(0xff);
			else doUSART_Transmit(0x00);
			break;
		}
		case RGT:
		{
			if (COM4_PORT & _BV(COM4)) doUSART_Transmit(0xff);
			else doUSART_Transmit(0x00);
			break;
		}
	}
}

void transmitRFData()
{
	doUSART_Transmit(rfDistances[RF_FRONT]);
	doUSART_Transmit(rfDistances[RF_LEFT]);
	doUSART_Transmit(rfDistances[RF_RIGHT]);
	doUSART_Transmit(rfDistances[RF_REAR]);
}

void transmitSynchro(uint8_t len)
{
    doUSART_Transmit(0xab);
    doUSART_Transmit(0xab);
    doUSART_Transmit(len);
}

void transmitSpeedData()
{
	for (uint8_t i = 0; i < 4; i++)
	{
		doUSART_Transmit(wheels.spd_sec[i]);
		doUSART_Transmit(wheels.whl_sec[i]);
		doUSART_Transmit(wheels.spd_minute[i]);
		doUSART_Transmit(wheels.whl_minute[i]);
		doUSART_Transmit(wheels.error[i]); // 0 - ошибка(нет сигнала с энкодера), 1 - верно (нет ошибки), 2 - пробуксовка колеса
	}
}

void transmitKRUState()
{
	                                    // Номинальные значения
	doUSART_Transmit(currentRudderPos); //128   0
	doUSART_Transmit(targetRudderPos);  //  0   0
	doUSART_Transmit(minRudderPos);     //-100   0
	doUSART_Transmit(maxRudderPos);     //100   0
	doUSART_Transmit(mdlRudderPos);     //  0   0
	doUSART_Transmit(rudderState);      //  1   1
	doUSART_Transmit(!(LIM_PIN & _BV(LIM_RIGHT))); // 0   0
	doUSART_Transmit(!(LIM_PIN & _BV(LIM_LEFT)));  // 0   0
	doUSART_Transmit(flickersMode);     //  2   2
	doUSART_Transmit(flickerState);     //  0   0
	doUSART_Transmit(flk_onTime);       // 61  61
	doUSART_Transmit(flk_offTime);      // 61  61
}

void transmitBrakeReverseState()
{
	transmitBrakeState(ALL);
	transmitReverseState();
}

void transmitFlickersState()
{
	doUSART_Transmit(flickersMode);
	doUSART_Transmit(flickerState);
	doUSART_Transmit(flk_onTime);
	doUSART_Transmit(flk_offTime);
}

void transmitReverseState()
{
	if (COM1_3_PORT & _BV(COM1)) doUSART_Transmit(0xff);
	else doUSART_Transmit(0x00);
}

void transmitAccelState()
{
	doUSART_Transmit(currentAccelPos);
}

void doUSART_Transmit(uint8_t ch)
{
	while (!(UCSRA & _BV(UDRE))); //Ожидание опустошения буфера передачи
	UDR = ch; //Начало передачи данных
}

void enableUSART_Transmit(bool enable)
{
	if (enable)
		UCSRB |= _BV(UDRIE);
	else
		UCSRB &= ~_BV(UDRIE);
}

void initUSART(uint8_t baudrate) /* Инициализация USART Скорость: 9600 бит/сек Четность: нет Бит данных: 8 Стоп-бит: 1 */
{
	/*
	Инициализация USART
	Скорость:		9600 бит/сек
	Четность:		нет
	Бит данных:		8
	Стоп-бит:		1
	*/
	UBRRH = (baudrate >> 8);
	UBRRL = (baudrate & 0xFF);
	// Удвоение скорости отключено
	UCSRA = 0;
	//Разрешение на прием и на передачу через USART, разрешение прерываний RX*
	UCSRB = _BV(RXEN) | _BV(TXEN) | _BV(RXCIE);
	//Формат кадра: 8 бит данных, 1 стоп-бит.
	UCSRC = _BV(URSEL) | _BV(UCSZ0) | _BV(UCSZ1);

	rxBuffer = (unsigned char *) malloc(PACKET_MAX_LENGTH*sizeof(unsigned char));
	rxBufferDataLen = 0;
	receiverState = RCVR_READY;

	txBuffer = (unsigned char *) malloc(PACKET_MAX_LENGTH*sizeof(unsigned char));
	txBufferDataLen = 0;
	txBufferDataPos = 0;
}
