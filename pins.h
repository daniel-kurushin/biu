#ifndef PINS_H_
#define PINS_H_

#include <avr/io.h>

#define ADDR0			PA0
#define	ADDR1			PA1
#define ADDR2			PA2
#define ADDR3			PA3
#define ADDR_DIR		DDRA
#define ADDR_PORT		PORTA
#define ADDR_PIN		PINA

#define TYPE0			PA4
#define TYPE1			PA5
#define TYPE2			PA6
#define TYPE3			PA7
#define TYPE_DIR		DDRA
#define TYPE_PORT		PORTA
#define TYPE_PIN		PINA

#define COM1			PD7
#define	COM2			PD5
#define COM3			PD4
#define COM4			PB3
#define COM1_3_DIR		DDRD
#define COM1_3_PORT		PORTD
#define COM1_3_PIN		PIND
#define COM4_DIR		DDRB
#define COM4_PORT		PORTB
#define COM4_PIN		PINB

#define RANGE_ECHO	PB2
#define RANGE_TRIG1	PB4
#define RANGE_TRIG2	PB1
#define RANGE_TRIG3 PB0
#define RANGE_TRIG4	PC7

#define RANGE_ECHO_TRIG_1_3_DIR DDRB
#define RANGE_TRIG_4_DIR DDRC
#define RANGE_ECHO_TRIG_1_3_PORT PORTB
#define RANGE_TRIG_4_PORT PORTC
#define RANGE_ECHO_PIN PINB

#define ACCEL_EN		PB5
#define ACCEL_DWN		PB6
#define ACCEL_CLK		PB7
#define ACCEL_DIR		DDRB
#define ACCEL_PORT		PORTB
#define ACCEL_PIN		PINB

#define TWI_SDA			PC1
#define TWI_SCL			PC0
#define TWI_PORT		PORTC
#define TWI_DIR			DDRC
#define	TWI_PIN			PINC

#define LIM_LEFT		PB5
#define LIM_RIGHT		PB6
#define LIM_PORT		PORTB
#define LIM_DIR			DDRB
#define LIM_PIN			PINB

#endif /* PINS_H_ */