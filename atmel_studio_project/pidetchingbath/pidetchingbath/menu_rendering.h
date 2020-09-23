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


#ifndef MENU_RENDERING_H_
#define MENU_RENDERING_H_
#include <stdint.h>
#include "config.h"

// display rendering functions
void mr_main(float current_temp, uint8_t tprobe_index);

void mr_main_menu(uint8_t item_index);
void mr_heater_menu(uint8_t item_index);
void mr_stirrer_menu(uint8_t item_index);
void mr_fan_menu(uint8_t item_index);
void mr_heater_menu_pid(uint8_t item_index);

void mr_heater_menu_onoff(uint8_t onoff);
void mr_heater_menu_target_temp(float temp);
void mr_heater_menu_controlling_probe_select(uint8_t tprobe_index, uint8_t selection_valid);
void mr_heater_menu_pid_p(float pid_p);
void mr_heater_menu_pid_i(float pid_i);
void mr_heater_menu_pid_d(float pid_d);
void mr_heater_menu_pid_i_clamp(float pid_i_clamp);
void mr_heater_menu_pid_offset(float offset);
void mr_heater_menu_pid_dsmooth(float dsmooth);

void mr_stirrer_menu_dc(uint8_t dutycycle);
void mr_fan_menu_dc(uint8_t dutycycle);

void mr_tprobe_menu(uint8_t menu_index);
void mr_tprobe_calib_menu(float resistance);
void mr_tprobe_calib_menu_nc();

void mr_thermistor_error(ErrorCode error);

#endif /* MENU_RENDERING_H_ */