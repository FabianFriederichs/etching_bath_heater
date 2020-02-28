/*
 * menu_rendering.h
 *
 * Created: 13.02.2020 12:16:11
 *  Author: fabia
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

void mr_tprobe_menu(uint8_t menu_index);
void mr_tprobe_calib_menu(float resistance);
void mr_tprobe_calib_menu_nc();

void mr_thermistor_error(ErrorCode error);

#endif /* MENU_RENDERING_H_ */