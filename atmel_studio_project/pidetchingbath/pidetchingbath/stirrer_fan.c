/*
MIT License

Copyright (c) 2020 Fabian Friederichs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */ 

#include <avr/io.h>
#include "config.h"
#include "stirrer_fan.h"
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

#ifndef STIRRER_FAN_PWM_PRESCALE
	#error "STIRRER_FAN error: STIRRER_FAN_PWM_PRESCALE not defined."
#endif

#ifndef STIRRER_FAN_PWM_TOP
	#error "STIRRER_FAN error: STIRRER_FAN_PWM_TOP not defined."
#endif

#if STIRRER_FAN_PWM_PRESCALE == 1	// 1
	#define STIRRER_FAN_PWM_PRESCALE_BITS (1 << CS10)
#elif STIRRER_FAN_PWM_PRESCALE == 8 // 8
	#define STIRRER_FAN_PWM_PRESCALE_BITS (1 << CS11)
#elif STIRRER_FAN_PWM_PRESCALE == 64 // 64
	#define STIRRER_FAN_PWM_PRESCALE_BITS ((1 << CS10) | (1 << CS11))
#elif STIRRER_FAN_PWM_PRESCALE == 256 // 256
	#define STIRRER_FAN_PWM_PRESCALE_BITS (1 << CS12)
#elif STIRRER_FAN_PWM_PRESCALE == 1024 // 1024
	#define STIRRER_FAN_PWM_PRESCALE_BITS ((1 << CS10) | (1 << CS12))
#else
	#error "STIRRER_FAN error: Unexpected prescaler configuration value."
#endif

#ifdef STIRRER_PWM_INVERSE
	#define STIRRER_PWM_COMA_BITS ((1 << COM1A0) | (1 << COM1A1))
#else
	#define STIRRER_PWM_COMA_BITS (1 << COM1A1)
#endif

#ifdef FAN_PWM_INVERSE
#define FAN_PWM_COMB_BITS ((1 << COM1B0) | (1 << COM1B1))
#else
#define FAN_PWM_COMB_BITS (1 << COM1B1)
#endif

#define STIRRER_FAN_PWM_WGM_BITS_A 0x00
#define STIRRER_FAN_PWM_WGM_BITS_B (1 << WGM13) // phase and frequency correct pwm mode. top set by ICR1

void stirrer_fan_init()
{
	TIMSK1 = 0x00;
	// stop timer clock
	TCCR1B &= ~STIRRER_FAN_PWM_PRESCALE_BITS;
	// enable phase correct, frequency correct pwm mode
	TCCR1A |= STIRRER_FAN_PWM_WGM_BITS_A;
	TCCR1B |= STIRRER_FAN_PWM_WGM_BITS_B;
	// set top value
	ICR1 = STIRRER_FAN_PWM_TOP;
	// reset duty cycle val
	// stirrer
	#ifdef STIRRER_PWM_INVERSE
		OCR1A = STIRRER_FAN_PWM_TOP;
	#else
		OCR1A = 0x0000;
	#endif
	// fan
	#ifdef FAN_PWM_INVERSE
		OCR1B = STIRRER_FAN_PWM_TOP;
	#else
		OCR1B = 0x0000;
	#endif
	// reset timer
	TCNT1 = 0x0000;
	
	
	// enable (stirrer) output pin
	OC1A_DDR |= (1 << OC1A_BIT);
	// set pin state when pwm is off (becomes active when COMA bits are cleared)
	#ifdef STIRRER_PWM_INVERSE
		OC1A_PORT |= (1 << OC1A_BIT);
	#else
		OC1A_PORT &= ~(1 << OC1A_BIT);
	#endif
	// init stirrer duty cycle
	#ifdef STIRRER_PWM_INVERSE
		OCR1A = STIRRER_FAN_PWM_TOP;
	#else
		OCR1A = 0x0000;
	#endif
	
	// enable (fan) output pin
	OC1B_DDR |= (1 << OC1B_BIT);
	// set pin state when pwm is off (becomes active when COMB bits are cleared)
	#ifdef FAN_PWM_INVERSE
		OC1B_PORT |= (1 << OC1B_BIT);
	#else
		OC1B_PORT &= ~(1 << OC1B_BIT);
	#endif
	// init fan duty cycle
	#ifdef FAN_PWM_INVERSE
		OCR1B = STIRRER_FAN_PWM_TOP;
	#else
		OCR1B = 0x0000;
	#endif	
	
	// start timer
	TCCR1B |= STIRRER_FAN_PWM_PRESCALE_BITS;
}

void stirrer_fan_shutdown()
{
	// stop timer
	TCCR1B &= ~STIRRER_FAN_PWM_PRESCALE_BITS;
	// reset top value
	ICR1 = 0x0000;
	// reset duty cycle val
	TCNT1 = 0x0000;
	OCR1A = 0x0000;
	OCR1B = 0x0000;
	// back to normal mode
	TCCR1A &= ~(STIRRER_PWM_COMA_BITS | FAN_PWM_COMB_BITS | STIRRER_FAN_PWM_WGM_BITS_A);
	TCCR1B &= ~STIRRER_FAN_PWM_WGM_BITS_B;
	// disable output pin
	// stirrer
	OC1A_DDR &= ~(1 << OC1A_BIT);
	OC1A_PORT &= ~(1 << OC1A_BIT);
	// fan
	OC1B_DDR &= ~(1 << OC1B_BIT);
	OC1B_PORT &= ~(1 << OC1B_BIT);
}

void stirrer_set_duty_cycle(uint8_t dc)
{
	uint32_t compval = ((uint32_t)umax8(umin8(dc, 100), 0) * STIRRER_FAN_PWM_TOP) / 100;
	OCR1A = (uint16_t)compval;
}

void stirrer_on()
{
	TCNT1 = 0x0000;
	// enable stirrer waveform output
	TCCR1A |= STIRRER_PWM_COMA_BITS;
}

void stirrer_off()
{	
	// disable stirrer waveform output
	TCCR1A &= ~STIRRER_PWM_COMA_BITS;
	TCNT1 = 0x0000;
}

void fan_set_duty_cycle(uint8_t dc)
{
	uint32_t compval = ((uint32_t)umax8(umin8(dc, 100), 0) * STIRRER_FAN_PWM_TOP) / 100;
	OCR1B = (uint16_t)compval;
}

void fan_on()
{
	TCNT1 = 0x0000;	
	// enable fan waveform output
	TCCR1A |= FAN_PWM_COMB_BITS;
}

void fan_off()
{
	// disable fan waveform output
	TCCR1A &= ~FAN_PWM_COMB_BITS;
	TCNT1 = 0x0000;
}