/*
 * stirrer.c
 *
 * Created: 11.02.2020 23:21:31
 *  Author: fabia
 */ 
#include <avr/io.h>
#include "config.h"
#include "stirrer.h"
#include "my_util.h"

/*
	PWM TOP calculation for phase correct and frequency correct PWM, 25khz
	8000000 / (2 * N * TOP) = 25000    // * (2 * N * TOP)
	8000000 = (2 * N * TOP) * 25000
	8000000 = N * TOP * 50000          // / (50000 * N)
	8000000 / (50000 * N) = TOP

	N		TOP
	1		160
	8		20
	64		2.5
	256		0.625
	1024		0.15625
	
	=> 160 TOP with a prescaler of 1 gives us 7 bits resolution
*/

#ifndef STIRRER_PWM_PRESCALE
	#error "STIRRER error: STIRRER_PWM_PRESCALE not defined."
#endif

#ifndef STIRRER_PWM_TOP
	#error "STIRRER error: STIRRER_PWM_TOP not defined."
#endif

#if STIRRER_PWM_PRESCALE == 1	// 1
	#define STIRRER_PWM_PRESCALE_BITS (1 << CS10)
#elif STIRRER_PWM_PRESCALE == 8 // 8
	#define STIRRER_PWM_PRESCALE_BITS (1 << CS11)
#elif STIRRER_PWM_PRESCALE == 64 // 64
	#define STIRRER_PWM_PRESCALE_BITS ((1 << CS10) | (1 << CS11))
#elif STIRRER_PWM_PRESCALE == 256 // 256
	#define STIRRER_PWM_PRESCALE_BITS (1 << CS12)
#elif STIRRER_PWM_PRESCALE == 1024 // 1024
	#define STIRRER_PWM_PRESCALE_BITS ((1 << CS10) | (1 << CS12))
#else
	#error "STIRRER error: Unexpected prescaler configuration value."
#endif

#ifdef STIRRER_PWM_INVERSE
	#define STIRRER_PWM_COMA_BITS ((1 << COM1A0) | (1 << COM1A1))
#else
	#define STIRRER_PWM_COMA_BITS (1 << COM1A1)
#endif

#define STIRRER_PWM_WGM_BITS_A 0x00
#define STIRRER_PWM_WGM_BITS_B (1 << WGM13) // phase and frequency correct pwm mode. top set by ICR1

void stirrer_init()
{
	TIMSK1 = 0x00;
	// stop timer clock
	TCCR1B &= ~STIRRER_PWM_PRESCALE_BITS;
	// enable phase correct, frequency correct pwm mode
	TCCR1A |= STIRRER_PWM_WGM_BITS_A;
	TCCR1B |= STIRRER_PWM_WGM_BITS_B;
	// set top value
	ICR1 = STIRRER_PWM_TOP;
	// reset duty cycle val
	#ifdef STIRRER_PWM_INVERSE
		OCR1A = STIRRER_PWM_TOP;
	#else
		OCR1A = 0x0000;
	#endif
	// reset timer
	TCNT1 = 0x0000;
	
	// enable output pin
	OC1A_DDR |= (1 << OC1A_BIT);
	// set pin state when pwm is off (becomes active when COMA bits are cleared)
	#ifdef STIRRER_PWM_INVERSE
		OC1A_PORT |= (1 << OC1A_BIT);
	#else
		OC1A_PORT &= ~(1 << OC1A_BIT);
	#endif
}

void stirrer_shutdown()
{
	// stop timer clock
	TCCR1B &= ~STIRRER_PWM_PRESCALE_BITS;
	// reset top value
	ICR1 = 0x0000;
	// reset duty cycle val
	TCNT1 = 0x0000;
	OCR1A = 0x0000;
	// back to normal mode
	TCCR1A &= ~(STIRRER_PWM_COMA_BITS | STIRRER_PWM_WGM_BITS_A);
	TCCR1B &= ~STIRRER_PWM_WGM_BITS_B;
	// disable output pin
	OC1A_DDR &= ~(1 << OC1A_BIT);
	OC1A_PORT &= ~(1 << OC1A_BIT);
}

void stirrer_set_duty_cycle(uint8_t dc)
{
	uint32_t compval = ((uint32_t)umax8(umin8(dc, 100), 0) * STIRRER_PWM_TOP) / 100;
	OCR1A = (uint16_t)compval;
}

void stirrer_on()
{
	TCNT1 = 0x0000;
	#ifdef STIRRER_PWM_INVERSE
		OCR1A = STIRRER_PWM_TOP;
	#else
		OCR1A = 0x0000;
	#endif
	TCCR1A |= STIRRER_PWM_COMA_BITS;
	TCCR1B |= STIRRER_PWM_PRESCALE_BITS;
}

void stirrer_off()
{	
	TCCR1A &= ~STIRRER_PWM_COMA_BITS;
	TCCR1B &= ~STIRRER_PWM_PRESCALE_BITS;	
	TCNT1 = 0x0000;
}