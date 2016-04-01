
#include "header.h"
#include "pins.h"

#include "encoders.h"
#include "rfsensor.h"
#include "flickers.h"
#include "usart.h"
#include "twi.h"
#include "khc.h"
#include "kru.h"

#include <string.h>

uint8_t devAddr, devType;
volatile uint8_t ct = 0;

void initMainTimer()
{
	TCCR0 = _BV(CS02); // !!!!! 02 Режим Normal, делитель на 256
	TIMSK |= _BV(TOIE0); // Разрешить прерывание по переполнению Т0

	switch (devType)
	{
    case (0x01):
    {
			TCCR2 = _BV(CS22) | _BV(CS21) | _BV(CS20);
			break;
    }
    case (0x02):
        {
            TCCR1B = _BV(CS20) | _BV(CS00);
            break;
        }
	}
}

// Контроллер делает 8000000 тактов в секунду.
// Таймер с делителем на 256 делает 31250 тактов в секунду.
// Каждые 255 тактов таймера генерируется прерывание.
// За секунду генерируется 122 прерывания.

// дальномер: 60ms цикл
// 1000/60 = 17 опросов/сек
// 122/17 = 7 - опрос на каждое 7 прерывание

ISR(TIMER0_OVF_vect)
{
	cli();
    wheels.my_system_time++; // глобaльный таймер (2^32/122/60/60/24 = ~400 дней, т.е. нет смысла ловить ноль)
    if(current_time < 65535) current_time++; else current_time = 0;
    switch(devType)
    {
        case 0x01: // КХЧ и дальномеры
        {
            rfWork();
            if (mainTimerCounter == 122)
            {
                check_offline_state(FROM_TIMER); // проверяем оффлайн
                encodersWork();
                mainTimerCounter = 0;
            }
            else
                mainTimerCounter++;
            break;
        }
        case 0x02: // КРУ
        {
            flickersWork();
            rudderWork();

            /*if (mainTimerCounter == 122)
            {
                check_offline_state(FROM_TIMER); // проверяем оффлайн
                mainTimerCounter = 0;
            }
            else
                mainTimerCounter++;*/
            break;
        }
    }
	sei();
}

void getDeviceAddrType(uint8_t *addr, uint8_t *type)
{
	TYPE_DIR &= ~(_BV(TYPE0) | _BV(TYPE1) | _BV(TYPE2) | _BV(TYPE3));
	TYPE_PORT |= _BV(TYPE0) | _BV(TYPE1) | _BV(TYPE2) | _BV(TYPE3);
	ADDR_DIR &= ~(_BV(ADDR0) | _BV(ADDR1) | _BV(ADDR2) | _BV(ADDR3));
	ADDR_PORT |= _BV(ADDR0) | _BV(ADDR1) | _BV(ADDR2) | _BV(ADDR3);
	*type = ~TYPE_PIN & 0xF0;
	*addr = ~ADDR_PIN & 0x0F;
	*type >>= 4;
}

int main(void)
{
	/*
	Получение адреса и типа устройства
	*/
	getDeviceAddrType(&devAddr, &devType);

	initUSART(51);

	/*
	Настройка силовых выходов
	*/
	COM1_3_DIR |= _BV(COM1) | _BV(COM2) | _BV(COM3);
	COM4_DIR |= _BV(COM4);

	/*
	Настройка в зависимости
	от типа устройства
	*/
    ct = 0;
	switch (devType)
	{
		case 0x01:
		{
			// КХЧ
			// Настроить выходы управления акселератором
			ACCEL_DIR |= _BV(ACCEL_EN) | _BV(ACCEL_DWN) | _BV(ACCEL_CLK);
			changeAccelPosition(0xff,true);

			/*************** дальномеры ***************/
			// дальномеры: выходы триггеров 1, 2 и 3
			RANGE_ECHO_TRIG_1_3_DIR |= _BV(RANGE_TRIG1) | _BV(RANGE_TRIG2) | _BV(RANGE_TRIG3);
			// дальномеры: выход 4
			RANGE_TRIG_4_DIR |= _BV(RANGE_TRIG4);
			// дальномеры: вход echo без подтяжки
			RANGE_ECHO_TRIG_1_3_DIR &= ~_BV(RANGE_ECHO);

			GICR |= _BV(INT2); //Разрешить прерывание INT2 (PB2) (echo дальномера)
			MCUCSR |= _BV(ISC2); //INT2 дергается по переднему фронту

			/**************** Энкодеры ****************/


			init_encoders();

			// PC1, PC2 - INT0, Задние колеса
			// PC0, PD6 - INT1, Передние колеса

			DDRD &= ~_BV(PD6);
			PORTD |= _BV(PD6);
			DDRC &= ~_BV(PC2) & ~_BV(PC1) & ~_BV(PC0);
			PORTC |= _BV(PC2) | _BV(PC1) | _BV(PC0);

			GICR |= _BV(INT0) | _BV(INT1); //Разрешить прерывание INT0 (pd2) и INT1 (PD3)
			MCUCR |= _BV(ISC01) | _BV(ISC11); //INT0 и INT1 дергаются по спадающему фронту

			break;
		}
		case 0x02:
		{
			// КРУ
			LIM_DIR &= ~_BV(LIM_LEFT) & ~_BV(LIM_RIGHT);
			LIM_PORT |= _BV(LIM_LEFT) | _BV(LIM_RIGHT);

			initKRUVars();
			KRU_encoderInit();
			break;
		}
		case 0x03:
		{
			// КСП
			break;
		}
		case 0x04:
		{
			// КЭН
			break;
		}
	}

	/*
		Инициализация главного таймера
	*/
	initMainTimer();

	sei();

    while(1)
    {
		//TODO:: Please write your application code
    }
}

//const uchar plen = 7 + 4;
//uchar *statePacket = (uchar *)malloc(sizeof(uchar) * plen);
//statePacket[PACKET_SYNCHRO_OFFSET] = 0xAEAE;
//statePacket[PACKET_TARGET_ADDRESS_OFFSET] = 0x00;
//statePacket[PACKET_SOURCE_ADDRESS_OFFSET] = devAddr;
//statePacket[PACKET_TYPE_OFFSET] = 0x03; //FIXME
//statePacket[PACKET_LENGTH_OFFSET] = plen;
//statePacket[PACKET_CRC_OFFSET] = 0x00;
//statePacket[PACKET_DATA_OFFSET + 0] = rfDistances[RF_FRONT];
//statePacket[PACKET_DATA_OFFSET + 1] = rfDistances[RF_LEFT];
//statePacket[PACKET_DATA_OFFSET + 2] = rfDistances[RF_FRONT];
//statePacket[PACKET_DATA_OFFSET + 3] = rfDistances[RF_REAR];
//
//// Скопировать пакет в буфер передачи
//memcpy(txBuffer, statePacket, plen);
//txBufferDataLen = plen;
//txBufferDataPos = 0;
//
//// Инициировать отправку
//funcUSART_enableTransmit(true);
//// Освободить выделенную под пакет память
//free(statePacket);
//

// Получено "Верни положение"
// 						uchar *statePacket = (uchar *)malloc(sizeof(uchar)*8);			// Создаем пакет длиной 8 байт
// 						statePacket[PACKET_SYNCHRO_OFFSET] = 0xAEAE;
// 						/*answerPacket[PACKET_SYNCHRO_OFFSET+1] = 0xAE;*/
// 						statePacket[PACKET_TARGET_ADDRESS_OFFSET] = 0x00;
// 						statePacket[PACKET_SOURCE_ADDRESS_OFFSET] = devAddr;
// 						statePacket[PACKET_TYPE_OFFSET] = 0x03;							// Тип ответного пакета - 0x03
// 						statePacket[PACKET_LENGTH_OFFSET] = 8;
// 						statePacket[PACKET_DATA_OFFSET] = (uchar)currentRudderPos;		// Вернуть текущее положение руля
// 						statePacket[PACKET_CRC_OFFSET] = 0x00;
//
// 						// Скопировать пакет в буфер передачи
// 						memcpy(txBuffer, statePacket, 8);
// 						txBufferDataLen = 8;
// 						txBufferDataPos = 0;
//
// 						// Инициировать отправку
// 						funcUSART_enableTransmit(true);
// 						// Освободить выделенную под пакет память
// 						free(statePacket);
