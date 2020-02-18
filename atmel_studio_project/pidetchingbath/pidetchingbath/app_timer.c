/*
 * app_timer.c
 *
 * Created: 25.07.2019 11:55:06
 *  Author: fabia
 */ 

#include "app_timer.h"
#include <assert.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#if APP_TIMER_PRESCALE == APP_TIMER_PRESCALE_1	// 1
	#define APP_TIMER_PRESCALE_BITS (1 << CS00)
	#define APP_TIMER_TIME_MUL 1
#elif APP_TIMER_PRESCALE == APP_TIMER_PRESCALE_8 // 8
	#define APP_TIMER_PRESCALE_BITS (1 << CS01)
	#define APP_TIMER_TIME_MUL 8
#elif APP_TIMER_PRESCALE == APP_TIMER_PRESCALE_64 // 64
	#define APP_TIMER_PRESCALE_BITS ((1 << CS00) | (1 << CS01))
	#define APP_TIMER_TIME_MUL 64
#elif APP_TIMER_PRESCALE == APP_TIMER_PRESCALE_256 // 256
	#define APP_TIMER_PRESCALE_BITS (1 << CS02)
	#define APP_TIMER_TIME_MUL 256
#elif APP_TIMER_PRESCALE == APP_TIMER_PRESCALE_1024 // 1024
	#define APP_TIMER_PRESCALE_BITS ((1 << CS00) | (1 << CS02))
	#define APP_TIMER_TIME_MUL 1024
#else
	#error "APP TIMER: Unexpected prescaler selection."
#endif

#define APP_TIMER_TICK_DURATION ((1.0 / F_CPU) * APP_TIMER_TIME_MUL * APP_TIMER_BASE_CLOCK)

#define APP_TIMER_CLOCK_STOP_BITS ((1 << CS00) | (1 << CS01) | (1 << CS02))

#define APPT_CYCLE_ZERO 0

// global counter
volatile appt_cycle_t appt_cycles;
appt_cycle_t appt_cycles_old;

// callback type
typedef struct
{
	appt_cycle_t interval;
	appt_cycle_t accumulator;
	appt_callback func;	
} appt_callback_entry;

// callback array
appt_callback_entry appt_callbacks[APP_TIMER_MAX_CALLBACKS];

appt_cycle_t appt_seconds_to_cycles(float seconds)
{
	return (appt_cycle_t)(seconds / APP_TIMER_TICK_DURATION + 0.5);
}

float appt_cycles_to_seconds(appt_cycle_t cycles)
{
	return cycles * APP_TIMER_TICK_DURATION;
}

float appt_cycles_to_milli_seconds(appt_cycle_t cycles)
{
	return cycles * APP_TIMER_TICK_DURATION * 10e3;
}

float appt_cycles_to_micro_seconds(appt_cycle_t cycles)
{
	return cycles * APP_TIMER_TICK_DURATION * 10e6;
}

float appt_cycles_to_minutes(appt_cycle_t cycles)
{
	return (cycles * APP_TIMER_TICK_DURATION) / 60.0;
}

float appt_cycles_to_hours(appt_cycle_t cycles)
{
	return (cycles * APP_TIMER_TICK_DURATION) / 3600.0;
}

void appt_init()
{
	appt_cycles = APPT_CYCLE_ZERO;
	appt_cycles_old = APPT_CYCLE_ZERO;
	for(uint8_t i = 0; i < APP_TIMER_MAX_CALLBACKS; ++i)
		appt_callbacks[i] = (appt_callback_entry){APPT_CYCLE_ZERO, APPT_CYCLE_ZERO, 0};
	// disable compare match interrupt and overflow interrupt
	TIMSK0 &= ~((1 << OCIE0A) | (1 << TOIE0));
	// stop timer clock
	TCCR0B &= ~APP_TIMER_CLOCK_STOP_BITS;
	// set output compare register and enable CTC mode
	#ifdef APP_TIMER_OUTPUT_CLOCK
		TCCR0A |= (1 << COM0A0) | (1 << WGM01);
		DDRB |= (1 << PORTB2);
	#else
		TCCR0A |= (1 << WGM01);
	#endif
	// set top
	OCR0A = APP_TIMER_BASE_CLOCK - 1;
	// enable interrupts
	TIMSK0 |= (1 << OCIE0A);
	sei();
}

void appt_shutdown()
{
	appt_stop();
	TIMSK0 &= ~(1 << OCIE0A);
	TCCR0A &= ~((1 << COM0A0) | (1 << WGM01));
	TCNT0 = 0;
	OCR0A = 0;
	appt_cycles = APPT_CYCLE_ZERO;
	appt_cycles_old = APPT_CYCLE_ZERO;
	for(uint8_t i = 0; i < APP_TIMER_MAX_CALLBACKS; ++i)
		appt_callbacks[i] = (appt_callback_entry){APPT_CYCLE_ZERO, APPT_CYCLE_ZERO, 0};
}

void appt_start()
{	
	// reset cycle count
	appt_cycles = APPT_CYCLE_ZERO;
	appt_cycles_old = APPT_CYCLE_ZERO;
	// reset counter
	TCNT0 = 0;
	// start timer clock
	TCCR0B |= APP_TIMER_PRESCALE_BITS;	
}

void appt_stop()
{
	// stop timer clock
	TCCR0B &= ~APP_TIMER_CLOCK_STOP_BITS;	
}

void appt_resume()
{
	// start timer clock
	TCCR0B |= APP_TIMER_PRESCALE_BITS;
}

ErrorCode appt_update()
{
	appt_cycle_t dt;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		dt = appt_cycles - appt_cycles_old;
		appt_cycles_old = appt_cycles;
	}	
	
	for(uint8_t i = 0; i < APP_TIMER_MAX_CALLBACKS; ++i)
	{
		if(appt_callbacks[i].func)
		{
			appt_callbacks[i].accumulator += dt;
			if(appt_callbacks[i].accumulator >= appt_callbacks[i].interval)
			{
				appt_callbacks[i].accumulator -= appt_callbacks[i].interval;
				if(appt_callbacks[i].accumulator >= appt_callbacks[i].interval)
					appt_callbacks[i].accumulator = APPT_CYCLE_ZERO;
				ErrorCode ec = (*(appt_callbacks[i].func))();
				if(ec) // in case of any callback wants to stop the application, it should return TRUE or an error code != 0
					return ec;
			}
		}
	}
	return FALSE;
}

float appt_get_hours_since_startup()
{
	return appt_cycles_to_hours(appt_cycles_old);
}

float appt_get_minutes_since_startup()
{
	return appt_cycles_to_minutes(appt_cycles_old);
}

float appt_get_seconds_since_startup()
{
	return appt_cycles_to_seconds(appt_cycles_old);
}

float appt_get_milli_seconds_since_startup()
{
	return appt_cycles_to_milli_seconds(appt_cycles_old);
}

float appt_get_micro_seconds_since_startup()
{
	return appt_cycles_to_micro_seconds(appt_cycles_old);
}

appt_cycle_t appt_get_cycles_since_startup()
{
	return appt_cycles_old;
}

void appt_set_callback(float interval, appt_callback func, uint8_t index)
{
	assert(index < APP_TIMER_MAX_CALLBACKS);
	appt_callbacks[index] = (appt_callback_entry){appt_seconds_to_cycles(interval), APPT_CYCLE_ZERO, func};
}

void appt_clear_callback(uint8_t index)
{
	assert(index < APP_TIMER_MAX_CALLBACKS);
	appt_callbacks[index] = (appt_callback_entry){APPT_CYCLE_ZERO, APPT_CYCLE_ZERO, 0};
}


// ------------------------------ ISR ------------------------------------
ISR(TIMER0_COMPA_vect)
{
	++appt_cycles;
}
