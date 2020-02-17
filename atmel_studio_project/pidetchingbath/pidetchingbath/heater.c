/*
 * heater.c
 *
 * Created: 11.02.2020 23:20:34
 *  Author: fabia
 */ 
#include <avr/io.h>
#include "config.h"
#include "heater.h"
#include "my_util.h"

#ifndef HEATER_PWM_PRESCALE
	#error "HEATER error: HEATER_PWM_PRESCALE not defined."
#endif

#if HEATER_PWM_PRESCALE == 1	// 1
	#define HEATER_PWM_PRESCALE_BITS (1 << CS20)
#elif HEATER_PWM_PRESCALE == 8 // 8
	#define HEATER_PWM_PRESCALE_BITS (1 << CS21)
#elif HEATER_PWM_PRESCALE == 32// 32
	#define HEATER_PWM_PRESCALE_BITS ((1 << CS20) | (1 << CS21))
#elif HEATER_PWM_PRESCALE == 64 // 64
	#define HEATER_PWM_PRESCALE_BITS (1 << CS22)
#elif HEATER_PWM_PRESCALE == 128 // 128
	#define HEATER_PWM_PRESCALE_BITS ((1 << CS22) | (1 << CS20))
#elif HEATER_PWM_PRESCALE == 256 // 256
	#define HEATER_PWM_PRESCALE_BITS ((1 << CS22) | (1 << CS21))
#elif HEATER_PWM_PRESCALE == 1024 // 1024
	#define HEATER_PWM_PRESCALE_BITS ((1 << CS22) | (1 << CS21) | (1 << CS20))
#else
	#error "HEATER error: Unexpected prescaler configuration value."
#endif

#ifdef HEATER_PWM_INVERSE
	#define HEATER_PWM_COMA_BITS ((1 << COM2A1) | (1 << COM2A0))
#else
	#define HEATER_PWM_COMA_BITS (1 << COM2A1)
#endif

#define HEATER_PWM_WGM_BITS_A (1 << WGM20) // phase correct pwm

void heater_init()
{
	TIMSK2 = 0x00;
	// stop timer clock
	TCCR2B &= ~HEATER_PWM_PRESCALE_BITS;
	// enable phase correct, frequency correct pwm mode
	TCCR2A |= HEATER_PWM_WGM_BITS_A;
	// reset duty cycle val
	#ifdef HEATER_PWM_INVERSE
		OCR2A = 0xFF;
	#else
		OCR2A = 0x00;
	#endif
	// reset timer
	TCNT2 = 0x00;
	
	// enable output pin
	OC2A_DDR |= (1 << OC2A_BIT);
	// set pin state when pwm is off (becomes active when COMA bits are cleared)
	#ifdef HEATER_PWM_INVERSE
		OC2A_PORT |= (1 << OC2A_BIT);
	#else
		OC2A_PORT &= ~(1 << OC2A_BIT);
	#endif
}

void heater_shutdown()
{
	// stop timer clock
	TCCR2B &= ~HEATER_PWM_PRESCALE_BITS;
	// reset duty cycle val
	TCNT2 = 0x00;
	OCR2A = 0x00;
	// back to normal mode
	TCCR2A &= ~(HEATER_PWM_COMA_BITS | HEATER_PWM_WGM_BITS_A);
	// disable output pin
	OC2A_DDR &= ~(1 << OC2A_BIT);
	OC2A_PORT &= ~(1 << OC2A_BIT);
}

void heater_set_duty_cycle(uint8_t dc)
{
	uint16_t compval = ((uint16_t)umax8(umin8(dc, 100), 0) * 0xFF) / 100;
	OCR2A = (uint8_t)compval;
}

void heater_on()
{
	TCNT2 = 0x00;
	#ifdef HEATER_PWM_INVERSE
		OCR2A = 0xFF;
	#else
		OCR2A = 0x00;
	#endif
	TCCR2A |= HEATER_PWM_COMA_BITS;
	TCCR2B |= HEATER_PWM_PRESCALE_BITS;
}

void heater_off()
{	
	TCCR2A &= ~HEATER_PWM_COMA_BITS;
	TCCR2B &= ~HEATER_PWM_PRESCALE_BITS;	
	TCNT2 = 0x00;
}
