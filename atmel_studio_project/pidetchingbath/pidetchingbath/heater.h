/*
 * heater.h
 *
 * Created: 11.02.2020 23:20:15
 *  Author: fabia
 */ 


#ifndef HEATER_H_
#define HEATER_H_
#include <stdint.h>

void heater_init();
void heater_shutdown();

void heater_set_duty_cycle(uint8_t dc);
void heater_on();
void heater_off();
#endif /* HEATER_H_ */