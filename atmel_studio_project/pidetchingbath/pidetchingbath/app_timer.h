/*
 * app_timer.h
 *
 * Created: 25.07.2019 11:54:48
 *  Author: fabia
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

typedef ErrorCode (*appt_callback)();

void		appt_init();
void		appt_shutdown();
void		appt_start();
void		appt_stop();
void		appt_resume();
ErrorCode		appt_update();
float		appt_get_minutes_since_startup();
float		appt_get_seconds_since_startup();
float		appt_get_milli_seconds_since_startup();
float		appt_get_micro_seconds_since_startup();
void		appt_set_callback(float interval, appt_callback func, uint8_t index);
void		appt_clear_callback(uint8_t index);

#endif /* APP_TIMER_H_ */