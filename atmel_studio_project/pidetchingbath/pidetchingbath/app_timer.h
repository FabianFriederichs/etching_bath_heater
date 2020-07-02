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


#ifndef APP_TIMER_H_
#define APP_TIMER_H_

#include <stdint.h>
#include "config.h"

// calculation of top and prescaler:
//  TOP = ((s_inverval * F_CPU) / N) - 1

#define APP_TIMER_PRESCALE_1 1
#define APP_TIMER_PRESCALE_8 8
#define APP_TIMER_PRESCALE_64 64
#define APP_TIMER_PRESCALE_256 256
#define APP_TIMER_PRESCALE_1024 1024

#define APP_TIMER_RES_64_BIT 0
#define APP_TIMER_RES_32_BIT 1
#define APP_TIMER_RES_16_BIT 2
#define APP_TIMER_RES_8_BIT 3

#if APP_TIMER_RESOLUTION == APP_TIMER_RES_64_BIT // 64 bit
	typedef uint64_t appt_cycle_t;
#elif APP_TIMER_RESOLUTION == APP_TIMER_RES_32_BIT // 32 bit
	typedef uint32_t appt_cycle_t;
#elif APP_TIMER_RESOLUTION == APP_TIMER_RES_16_BIT // 16 bit
	typedef uint16_t appt_cycle_t;
#elif APP_TIMER_RESOLUTION == APP_TIMER_RES_8_BIT // 8 bit
	typedef uint8_t appt_cycle_t;
#else
	typedef uint8_t appt_cycle_t;
#endif

typedef ErrorCode (*appt_callback)();

void			appt_init();
void			appt_shutdown();
void			appt_start();
void			appt_stop();
void			appt_resume();
ErrorCode		appt_update();
float			appt_get_hours_since_startup();
float			appt_get_minutes_since_startup();
float			appt_get_seconds_since_startup();
float			appt_get_milli_seconds_since_startup();
float			appt_get_micro_seconds_since_startup();
appt_cycle_t	appt_get_cycles_since_startup();
void			appt_set_callback(float interval, appt_callback func, uint8_t index);
void			appt_clear_callback(uint8_t index);
appt_cycle_t	appt_seconds_to_cycles(float seconds);
float			appt_cycles_to_seconds(appt_cycle_t cycles);
float			appt_cycles_to_milli_seconds(appt_cycle_t cycles);
float			appt_cycles_to_micro_seconds(appt_cycle_t cycles);
float			appt_cycles_to_minutes(appt_cycle_t cycles);
float			appt_cycles_to_hours(appt_cycle_t cycles);

#endif /* APP_TIMER_H_ */