/*
 * stirrer.h
 *
 * Created: 11.02.2020 23:21:12
 *  Author: fabia
 */ 


#ifndef STIRRER_H_
#define STIRRER_H_
#include <stdint.h>

void stirrer_init();
void stirrer_shutdown();

void stirrer_set_duty_cycle(uint8_t dc);
void stirrer_on();
void stirrer_off();

#endif /* STIRRER_H_ */