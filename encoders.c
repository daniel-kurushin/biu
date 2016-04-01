/*
 * encoders.c
 *
 * Created: 01.10.2015 16:04:27
 *  Author: User
 */

#include <avr/interrupt.h>

#include "encoders.h"
#include "pins.h"
#include "header.h"
#include "kru.h"

void init_encoders()
{

	for (uint8_t i = 0; i < 4; i++)
	{
	 wheels.whl_minute[i] = 0;
	 wheels.k[i] = 1;
	 wheels.whl_sec[i] = 0;
	 wheels.whl_minute[i] = 0;
	 wheels.whl_error[i] = 0;
	 wheels.srednee_min[i] = 0;
    }


	wheels.enc_work = 0;

	wheels.spd_zadnie_prev_sec[0] = 0; // скорости задних колес в предыдущую секунду
	wheels.spd_zadnie_prev_sec[1] = 0;

	wheels.my_system_time = 0; // системный таймер
	wheels.error_2sec = 0; // таймер-счетчик (2сек.)
	wheels.sum_error = 0; // сумма ошибок
}

uint16_t get_AvgSecSpeed()
{
	uint16_t s = 0;
	for(uint8_t i=0; i<4;i++) s += wheels.spd_sec[i];
	return s/4;
}

uint16_t get_AvgMinSpeed()
{
	uint16_t s = 0;
	for(uint8_t i=0; i<4;i++) s += wheels.spd_minute[i];
	return s/4;
}

void encodersWork()
{
	for (uint8_t i = 0; i < 4; i++) // раз в секунду
	{
//		wheels.whl_minute[i] += wheels.whl_sec[i];
//		wheels.whl_error[i] += wheels.whl_sec[i];
		wheels.spd_sec[i] = wheels.whl_sec[i] / wheels.k[i];
		wheels.whl_sec[i] = 0;
	}
    return;
//
//	if (wheels.error_2sec > 1) //раз в 2 сек.
//	{
//		wheels.error_2sec = 0; // обнуляем таймер(2 сек.)
//		for (uint8_t i = 0; i < 4; i++)
//			if (wheels.my_system_time - wheels.whl_error[i] > 243) // если не получали данные больше 2 сек.
//				{
//					wheels.error[i] = 0; // 0 - как ошибка
//					wheels.sum_error++; // кол-во ошибок
//				}
//				else wheels.error[i] = 1; // 1 - как не ошибка (верно)
//		if (wheels.sum_error == 4) // если четыре ошибки (все колеса не дают данные, то ошибок нет ( робот стоит))
//			for (uint8_t i = 0; i < 4; i++)
//				wheels.error[i] = 1; // 1 - как не ошибка (верно)
//
//		if (wheels.sum_error == 3) // если три ошибки (1 ошибка по умолчанию, т.к. нет одного энкоера)
//			for (uint8_t i = 0; i < 4; i++)
//				if (wheels.error[i] == 1) wheels.error[i] = 0;//значит, один энкодер показывает значения, что робот едет, т.е. он ошибается и мы...
//					else wheels.error [i] = 1; // ... считаем, что энкодеры, которые показывают, что робот стоит, верными
//		wheels.sum_error = 0;
//		if ((wheels.error[2] == 0) && (wheels.error[3] == 0) && ((wheels.error[0] != 0) && (wheels.error[1] != 0))
//							&& ((wheels.spd_zadnie_prev_sec[0]*1.5 <= wheels.spd_sec[0]) || (wheels.spd_zadnie_prev_sec[1]*1.5 <= wheels.spd_sec[1])))
//		//передние стоят, а задние двигаются на протяжении 2 сек, так же скорость задних увеличилась за секунду более чем в 1.5 раза
//			{
//				wheels.error[0] = 2; //заднее левое пробуксовывает
//				wheels.error[1] = 2; //заднее правое пробуксовывает
//			}
//		// если две ошибки, то ничего не делаем, т.к. одна по умолчанию (нет энкодера), а вторая - действительно ошибка
//	}
//	else wheels.error_2sec++; // инкремент таймер (2 сек.)
//
//	if (wheels.enc_work > 59) // раз в минуту
//	{
//		wheels.enc_work = 0; // обнуляем таймер (1 мин.)
//		for (uint8_t i = 0; i < 4; i++)
//		{
//			wheels.spd_minute[i] = wheels.whl_minute[i] / wheels.k[i]*60; // скорости колес в срабатываниях за минуту
//			wheels.whl_minute[i] = 0; // обнуляем срабатывания за минуту
//			wheels.srednee_min[i] = 0; //обнулние среднего значения
//		}
//	}
//	else {
//		    wheels.enc_work++; // инкремент таймера (1 мин.)
//			for(uint8_t i = 0; i < 4; i++)
//				wheels.srednee_min[i] = (wheels.srednee_min[i] + wheels.spd_sec[i])/wheels.enc_work; //прибавляем к предыдущему значению новую скрость в секунду и усредняем на текущий момент времени
//		 }
//
//	for (uint8_t i = 0; i < 4; i++) // раз в секунду
//	{
//		wheels.whl_minute[i] += wheels.whl_sec[i] - wheels.srednee_min[i]; // прибавляем срабатывания в секунду к срабатываниям в минуту (delta speed/sec)
//		wheels.whl_error[i] += wheels.whl_sec[i]; // прибавляем срабатывания в секунду к срабатываниям в 2 сек (для ошибок)
//
//		wheels.spd_sec[i] = wheels.whl_sec[i] / wheels.k[i]; // скорости колес в срабатываниях в секунду
//		wheels.whl_sec[i] = 0; // обнуляем срабатывания в секунду
//
//		/*
//		char buf[64];
//		sprintf(buf, "spd[%d]=%d\n\r\n;   ", i , wheels.spd_sec[i]);
//		char*c = buf; while(*c++ != 0)
//		funcUSART_Transmit(*(c-1));
//		*/
//	}
//	wheels.spd_zadnie_prev_sec[0] = wheels.spd_sec[0];
//	wheels.spd_zadnie_prev_sec[1] = wheels.spd_sec[1];
}


// PC1, PC2 - INT0, Передние колеса
// PC0, PD6 - INT1, Задние колеса

ISR(INT0_vect)
{
	cli();
	switch (devType)
	{
		case 0x01:
		{
			if (!(PINC & _BV(PC1)))
			{
				wheels.whl_sec[2]++; //перед
			}
			if (!(PINC & _BV(PC2)))
			{
				wheels.whl_sec[3]++; //перед
			}
		}
		case 0x02:
		{
		}
	}
	sei();
}

ISR(INT1_vect)
{
	cli();
	switch (devType)
	{
		case 0x01:
		{
            if (!(PINC & _BV(PC0)))
            {
                wheels.whl_sec[0]++; //лев зад
            }
            if (!(PIND & _BV(PD6)))
            {
                wheels.whl_sec[1]++; // прав зад
            }
		}
		case 0x02:
		{
		}
	}
	sei();
}

