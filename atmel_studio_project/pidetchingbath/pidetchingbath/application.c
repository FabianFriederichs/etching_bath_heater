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

#include "application.h"

// persistent state
eeprom_settings_t app_eeprom_settings EEMEM;

// application state
app_state_t app_state;

ErrorCode app_run()
{
	////////////////////////////////////// INITIALIZATION //////////////////////////////////////////
	// init error state
	app_state.current_error = EC_SUCCESS;
	
	// initialize app timer
	appt_init();
	
	// initialize display
	srd_init();
	
	// initialize input
	switch_init();
	rotenc_init();
	
	// initialize settings
	app_load_settings_from_eeprom();
	
	// initialize controller state
	app_state.stirrer_duty_cycle = 0;
	app_state.fan_duty_cycle = app_state.settings.fan_duty_cycle;
	app_state.heater_onoff = FALSE;
	app_state.stirrer_onoff = FALSE;
	app_state.fan_onoff = (app_state.fan_duty_cycle > 0 ? TRUE : FALSE);
	app_state.heater_rapid_heating = FALSE;
	
	
	// initialize control
	stirrer_fan_init();
	stirrer_off();
	fan_set_duty_cycle(app_state.fan_duty_cycle);
	if(app_state.fan_onoff)
		fan_on();
	else
		fan_off();	
	heater_init();
	heater_off();
	
	// initialize pid controller
	pid_init(&app_state.pid_state, app_state.settings.heater_pid_kp, app_state.settings.heater_pid_ti, app_state.settings.heater_pid_td, app_state.settings.heater_pid_i_clamp, app_state.settings.heater_pid_offset, app_state.settings.heater_pid_d_smoothing_factor, HEATER_CONTROL_MIN, HEATER_CONTROL_MAX);
	
	// initialize sensors
	tsens_init();
	
	// setup app timer callbacks
	// PID control loop
	appt_set_callback(APP_PID_LOOP_INTERVAL, app_control, 0);
	
	// input loop
	appt_set_callback(APP_USER_LOOP_INTERVAL, app_user_main, 1);
	
	// input polling
	appt_set_callback(APP_ROT_ENC_UPDATE_INTERVAL, app_rotenc_update, 2);
	appt_set_callback(APP_BUTTON_UPDATE_INTERVAL, app_button_update, 3);
	
	// initialize menu state
	app_clear_input();
	app_state.current_state_func = app_state_main;
	
	// start adc
	tsens_start_adc();
	
	// init sensor readings
	#ifdef TSENS_PROBE_0
		app_state.t0_current_temp = tsens_measure_probe0_temp(&app_state.current_error);
		if(app_state.current_error) { heater_shutdown(); stirrer_fan_shutdown(); tsens_shutdown(); rotenc_shutdown();	switch_shutdown(); app_error_display();	return app_state.current_error; }
		app_state.t0_resistance = tsens_measure0_resistance(&app_state.current_error);
		if(app_state.current_error) { heater_shutdown(); stirrer_fan_shutdown(); tsens_shutdown(); rotenc_shutdown();	switch_shutdown(); app_error_display();	return app_state.current_error; }
	#endif
	#ifdef TSENS_PROBE_1
		app_state.t1_current_temp = tsens_measure_probe1_temp(&app_state.current_error);
		if(app_state.current_error) { heater_shutdown(); stirrer_fan_shutdown(); tsens_shutdown(); rotenc_shutdown();	switch_shutdown(); app_error_display();	return app_state.current_error; }
		app_state.t1_resistance = tsens_measure1_resistance(&app_state.current_error);
		if(app_state.current_error) { heater_shutdown(); stirrer_fan_shutdown(); tsens_shutdown(); rotenc_shutdown();	switch_shutdown(); app_error_display();	return app_state.current_error; }
	#endif
	#ifdef TSENS_PROBE_2
		app_state.t2_current_temp = tsens_measure_probe2_temp(&app_state.current_error);
		if(app_state.current_error) { heater_shutdown(); stirrer_fan_shutdown(); tsens_shutdown(); rotenc_shutdown();	switch_shutdown(); app_error_display();	return app_state.current_error; }
		app_state.t2_resistance = tsens_measure2_resistance(&app_state.current_error);
		if(app_state.current_error) { heater_shutdown(); stirrer_fan_shutdown(); tsens_shutdown(); rotenc_shutdown();	switch_shutdown(); app_error_display();	return app_state.current_error; }
	#endif
	#ifdef TSENS_PROBE_3
		app_state.t3_current_temp = tsens_measure_probe3_temp(&app_state.current_error);
		if(app_state.current_error) { heater_shutdown(); stirrer_fan_shutdown(); tsens_shutdown(); rotenc_shutdown();	switch_shutdown(); app_error_display();	return app_state.current_error; }
		app_state.t3_resistance = tsens_measure3_resistance(&app_state.current_error);
		if(app_state.current_error) { heater_shutdown(); stirrer_fan_shutdown(); tsens_shutdown(); rotenc_shutdown();	switch_shutdown(); app_error_display();	return app_state.current_error; }
	#endif
		
	// start app timer
	appt_start();
		
	///////////////////////////////////// MAIN LOOP ////////////////////////////////////////////////
	// if should_stop gets set, the program will exit the main loop
	app_state.should_stop = FALSE;
	while(!app_state.should_stop)
	{
		app_state.current_error = appt_update();
		if(app_state.current_error)
			app_state.should_stop = TRUE;
	}
	if(app_state.current_error) // emergency shutdown. keep display alive for error display
	{
		heater_shutdown();
		stirrer_fan_shutdown();
		tsens_shutdown();
		rotenc_shutdown();
		switch_shutdown();
		app_error_display();
		return app_state.current_error;		
	}	
	// Else: graceful shutdown. (Maybe due to brown out detection or similar)
	///////////////////////////////////// SHUTDOWN / CLEANUP /////////////////////////////////////
	// shutdown app timer
	appt_shutdown();
	// shutdown subsystems
	stirrer_fan_shutdown();
	heater_shutdown();
	tsens_shutdown();
	rotenc_shutdown();
	switch_shutdown();
	srd_shutdown();
	return EC_SUCCESS;
}

void app_shutdown()
{
	app_state.should_stop = TRUE;
}

////////////////////////////////////////// INPUT CALLBACK /////////////////////////////////////////
ErrorCode app_user_main()
{
	// INPUT	
	app_clear_input();
	app_state.current_input.rotenc_delta = rotenc_get_inc();
	// Rotenc Button
	app_state.current_input.button_states |= (switch_get_state(BUTTON0) << BUTTON0);
	app_state.current_input.button_presses |= (switch_press(BUTTON0) << BUTTON0);
	app_state.current_input.button_long_presses |= (switch_longpress(BUTTON0) << BUTTON0);
	app_state.current_input.button_releases |= (switch_release(BUTTON0) << BUTTON0);
	// Secondary Button
	//---
	
	// query menu state machine
	return (*app_state.current_state_func)();
}

///////////////////////////////////////// PID CONTROL CALLBACK ////////////////////////////////////
ErrorCode app_control()
{
	// do measurements
	// if calibration menu is active, update corresponding resistance value
	#ifdef TSENS_PROBE_0
		// measure temperature and resistance, with open and short circuit protection
		app_state.t0_current_temp = tsens_measure_probe0_temp(&app_state.current_error);
		if(app_state.current_error)
		{
			return app_state.current_error;
		}
		if(app_state.current_state_func == app_state_menu_tprobe0_calib)
		{
			app_state.t0_resistance = tsens_measure0_resistance(&app_state.current_error);
			if(app_state.current_error)
			{
				return app_state.current_error;
			}
		}
		// min, max temp protection
		if(app_state.t0_current_temp < HEATER_TR_PROTECTION_MIN_TEMP)
			return EC_THERMISTOR_MIN_TEMP;
		else if(app_state.t0_current_temp > HEATER_TR_PROTECTION_MAX_TEMP)
			return EC_THERMISTOR_MAX_TEMP;
		
		// unresponsive thermistor protection
		if(app_state.settings.controlling_tprobe == 0 || HEATER_SAFETY_TPROBE == 0)
		{
			if(app_state.heater_rapid_heating
				&& (app_state.t0_current_temp - app_state.t0_tr_check_start_temp) < HEATER_PROBE0_TR_PROTECTION_EXPECTED_TEMP_CHANGE
				&& appt_cycles_to_seconds(appt_get_cycles_since_startup() - app_state.t0_tr_check_start_time) > HEATER_PROBE0_TR_PROTECTION_INTERVAL) // if temp change under full power not reached within interval
			{
				return EC_THERMISTOR_NOT_RESPONDING;
			}
			else if(app_state.heater_rapid_heating
				&& (app_state.t0_current_temp - app_state.t0_tr_check_start_temp) >= HEATER_PROBE0_TR_PROTECTION_EXPECTED_TEMP_CHANGE
				&& appt_cycles_to_seconds(appt_get_cycles_since_startup() - app_state.t0_tr_check_start_time) <= HEATER_PROBE0_TR_PROTECTION_INTERVAL)// reset start temp and time for next cycle
			{
				app_state.t0_tr_check_start_temp = app_state.t0_current_temp;
				app_state.t0_tr_check_start_time = appt_get_cycles_since_startup();
			}
		}
	#endif
	
	#ifdef TSENS_PROBE_1
		app_state.t1_current_temp = tsens_measure_probe1_temp(&app_state.current_error);
		if(app_state.current_error)
		{
			return app_state.current_error;
		}
		if(app_state.current_state_func == app_state_menu_tprobe1_calib)
		{
			app_state.t1_resistance = tsens_measure1_resistance(&app_state.current_error);
			if(app_state.current_error)
			{
				return app_state.current_error;
			}
		}
		// min, max temp protection
		if(app_state.t1_current_temp < HEATER_TR_PROTECTION_MIN_TEMP)
			return EC_THERMISTOR_MIN_TEMP;
		else if(app_state.t1_current_temp > HEATER_TR_PROTECTION_MAX_TEMP)
			return EC_THERMISTOR_MAX_TEMP;
		
		// unresponsive thermistor protection
		if(app_state.settings.controlling_tprobe == 1 || HEATER_SAFETY_TPROBE == 1)
		{
			if(app_state.heater_rapid_heating
				&& (app_state.t1_current_temp - app_state.t1_tr_check_start_temp) < HEATER_PROBE1_TR_PROTECTION_EXPECTED_TEMP_CHANGE
				&& appt_cycles_to_seconds(appt_get_cycles_since_startup() - app_state.t1_tr_check_start_time) > HEATER_PROBE1_TR_PROTECTION_INTERVAL) // if temp change under full power not reached within interval
			{
				return EC_THERMISTOR_NOT_RESPONDING;
			}
			else if(app_state.heater_rapid_heating
				&& (app_state.t1_current_temp - app_state.t1_tr_check_start_temp) >= HEATER_PROBE1_TR_PROTECTION_EXPECTED_TEMP_CHANGE
				&& appt_cycles_to_seconds(appt_get_cycles_since_startup() - app_state.t1_tr_check_start_time) <= HEATER_PROBE1_TR_PROTECTION_INTERVAL)// reset start temp and time for next cycle
			{
				app_state.t1_tr_check_start_temp = app_state.t1_current_temp;
				app_state.t1_tr_check_start_time = appt_get_cycles_since_startup();
			}
		}
	#endif
	
	#ifdef TSENS_PROBE_2
		app_state.t2_current_temp = tsens_measure_probe2_temp(&app_state.current_error);
		if(app_state.current_error)
		{
			return app_state.current_error;
		}
		if(app_state.current_state_func == app_state_menu_tprobe2_calib)
		{
			app_state.t2_resistance = tsens_measure2_resistance(&app_state.current_error);
			if(app_state.current_error)
			{
				return app_state.current_error;
			}
		}
		// min, max temp protection
		if(app_state.t2_current_temp < HEATER_TR_PROTECTION_MIN_TEMP)
			return EC_THERMISTOR_MIN_TEMP;
		else if(app_state.t2_current_temp > HEATER_TR_PROTECTION_MAX_TEMP)
			return EC_THERMISTOR_MAX_TEMP;
		
		// unresponsive thermistor protection
		if(app_state.settings.controlling_tprobe == 2 || HEATER_SAFETY_TPROBE == 2)
		{
			if(app_state.heater_rapid_heating
				&& (app_state.t2_current_temp - app_state.t2_tr_check_start_temp) < HEATER_PROBE2_TR_PROTECTION_EXPECTED_TEMP_CHANGE
				&& appt_cycles_to_seconds(appt_get_cycles_since_startup() - app_state.t2_tr_check_start_time) > HEATER_PROBE2_TR_PROTECTION_INTERVAL) // if temp change under full power not reached within interval
			{
				return EC_THERMISTOR_NOT_RESPONDING;
			}
			else if(app_state.heater_rapid_heating
				&& (app_state.t2_current_temp - app_state.t2_tr_check_start_temp) >= HEATER_PROBE2_TR_PROTECTION_EXPECTED_TEMP_CHANGE
				&& appt_cycles_to_seconds(appt_get_cycles_since_startup() - app_state.t2_tr_check_start_time) <= HEATER_PROBE2_TR_PROTECTION_INTERVAL)// reset start temp and time for next cycle
			{
				app_state.t2_tr_check_start_temp = app_state.t2_current_temp;
				app_state.t2_tr_check_start_time = appt_get_cycles_since_startup();
			}
		}
	#endif
	
	#ifdef TSENS_PROBE_3
		app_state.t3_current_temp = tsens_measure_probe3_temp(&app_state.current_error);
		if(app_state.current_error)
		{
			return app_state.current_error;
		}
		if(app_state.current_state_func == app_state_menu_tprobe3_calib)
		{
			app_state.t3_resistance = tsens_measure3_resistance(&app_state.current_error);
			if(app_state.current_error)
			{
				return app_state.current_error;
			}
		}
		// min, max temp protection
		if(app_state.t3_current_temp < HEATER_TR_PROTECTION_MIN_TEMP)
			return EC_THERMISTOR_MIN_TEMP;
		else if(app_state.t3_current_temp > HEATER_TR_PROTECTION_MAX_TEMP)
			return EC_THERMISTOR_MAX_TEMP;
		
		// unresponsive thermistor protection
		if(app_state.settings.controlling_tprobe == 3 || HEATER_SAFETY_TPROBE == 3)
		{
			if(app_state.heater_rapid_heating
				&& (app_state.t3_current_temp - app_state.t3_tr_check_start_temp) < HEATER_PROBE3_TR_PROTECTION_EXPECTED_TEMP_CHANGE
				&& appt_cycles_to_seconds(appt_get_cycles_since_startup() - app_state.t3_tr_check_start_time) > HEATER_PROBE3_TR_PROTECTION_INTERVAL) // if temp change under full power not reached within interval
			{
				return EC_THERMISTOR_NOT_RESPONDING;
			}
			else if(app_state.heater_rapid_heating
				&& (app_state.t3_current_temp - app_state.t3_tr_check_start_temp) >= HEATER_PROBE3_TR_PROTECTION_EXPECTED_TEMP_CHANGE
				&& appt_cycles_to_seconds(appt_get_cycles_since_startup() - app_state.t3_tr_check_start_time) <= HEATER_PROBE3_TR_PROTECTION_INTERVAL)// reset start temp and time for next cycle
			{
				app_state.t3_tr_check_start_temp = app_state.t3_current_temp;
				app_state.t3_tr_check_start_time = appt_get_cycles_since_startup();
			}
		}
	#endif
	
	// pid stuff
	if(app_state.heater_onoff)
	{
		float process_val;
		switch(app_state.settings.controlling_tprobe)
		{
			#ifdef TSENS_PROBE_0
			case 0:				
				process_val = app_state.t0_current_temp;				
				break;
				#endif
			#ifdef TSENS_PROBE_1
			case 1:				
				process_val = app_state.t1_current_temp;				
				break;
				#endif
			#ifdef TSENS_PROBE_2
			case 2:				
				process_val = app_state.t2_current_temp;				
				break;
				#endif
			#ifdef TSENS_PROBE_3
			case 3:				
				process_val = app_state.t3_current_temp;				
				break;
				#endif
			default:
				return EC_NO_CONTROLLING_TPROBE;
		}
		
		float pid_res = pid_step(&app_state.pid_state, process_val, app_state.settings.heater_target_temp);
		// if heater temp is > than safe maximum, default pwm duty cycle to 0
		uint8_t hdc;
		if(HEATER_SAFETY_TPROBE_CURRENT_TEMP > HEATER_MAX_OPERATING_TEMP) // HEATER_SAFETY_TPROBE_CURRENT_TEMP is the selected heater probe used to limit the maximum heater temperature
			hdc = 0;
		else
			hdc = (uint8_t)pid_res;
		
		if(hdc >= HEATER_TR_DUTY_CYCLE && !app_state.heater_rapid_heating) // beginning of rapid heating period.
		{
			app_state.heater_rapid_heating = TRUE;
			#ifdef TSENS_PROBE_0
				app_state.t0_tr_check_start_temp = app_state.t0_current_temp;
				app_state.t0_tr_check_start_time = appt_get_cycles_since_startup();
			#endif
			
			#ifdef TSENS_PROBE_1
				app_state.t1_tr_check_start_temp = app_state.t1_current_temp;
				app_state.t1_tr_check_start_time = appt_get_cycles_since_startup();
			#endif
			
			#ifdef TSENS_PROBE_2
				app_state.t2_tr_check_start_temp = app_state.t2_current_temp;
				app_state.t2_tr_check_start_time = appt_get_cycles_since_startup();
			#endif
			
			#ifdef TSENS_PROBE_3
				app_state.t3_tr_check_start_temp = app_state.t3_current_temp;
				app_state.t3_tr_check_start_time = appt_get_cycles_since_startup();
			#endif
		}
		else if(hdc < HEATER_TR_DUTY_CYCLE && app_state.heater_rapid_heating) // reset TRP state, end of rapid heating period
		{
			app_state.heater_rapid_heating = FALSE;
		}
		
		// set heater duty cycle
		heater_set_duty_cycle(hdc);
	}
	return EC_SUCCESS; // everything ok
}

/////////////////////////////////////// ROT_ENC UPDATE CALLBACK ///////////////////////////////////
ErrorCode app_rotenc_update()
{
	rotenc_update();
	return EC_SUCCESS;
}

/////////////////////////////////////// BUTTONS UPDATE CALLBACK ///////////////////////////////////
ErrorCode app_button_update()
{
	switch_update();
	return EC_SUCCESS;
}

/////////////////////////////////////// STATE MACHINE IMPLEMENTATION //////////////////////////////
// all the state functions
ErrorCode app_state_main()
{
	if(app_state.current_input.rotenc_delta > 0)
		app_state.selected_menu_item_index = imax8(imin8(app_state.selected_menu_item_index + 1, 1), 0);
	else if(app_state.current_input.rotenc_delta < 0)
		app_state.selected_menu_item_index = imax8(imin8(app_state.selected_menu_item_index - 1, 1), 0);
	// display current temp
	srd_clear();
	switch(app_state.selected_menu_item_index)
	{
		#ifdef TSENS_PROBE_0
		case 0:
			mr_main(app_state.t0_current_temp, 0);
			break;
			#endif
		#ifdef TSENS_PROBE_1
		case 1:
			mr_main(app_state.t1_current_temp, 1);
			break;
			#endif
		#ifdef TSENS_PROBE_2
		case 2:
			mr_main(app_state.t2_current_temp, 2);
			break;
			#endif
		#ifdef TSENS_PROBE_3
		case 3:
			mr_main(app_state.t3_current_temp, 3);
			break;
			#endif
		default:
			return EC_NO_CONTROLLING_TPROBE;
			break;
	}
	srd_display();
	
	// state change
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		app_state.selected_menu_item_index = 0;
		app_state.current_state_func = app_state_menu_main;
	}
	
	return EC_SUCCESS; // everything ok	
}

ErrorCode app_state_menu_main()
{
	if(app_state.current_input.rotenc_delta > 0)
		app_state.selected_menu_item_index = imax8(imin8(app_state.selected_menu_item_index + 1, 6), 0);
	else if(app_state.current_input.rotenc_delta < 0)
		app_state.selected_menu_item_index = imax8(imin8(app_state.selected_menu_item_index - 1, 6), 0);
	// display selected menu item
	srd_clear();
	mr_main_menu(app_state.selected_menu_item_index);
	srd_display();
	
	// state change
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		switch(app_state.selected_menu_item_index)
		{
			case 0:	// back to main screen
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_main;
				break;
			case 1: // heater menu
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_heater;
				break;
			case 2:	// stirrer menu
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_stirrer;
				break;
			case 3:	// fan menu
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_fan;
				break;
			case 4:	// thermistor calibration menu
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_tprobe;
				break;
			case 5:	// load settings from eeprom
				//app_state.selected_menu_item_index = 0;
				//app_state.current_state_func = app_state_menu_load_eeprom_settings;
				app_load_settings_from_eeprom();
				break;
			case 6:	// store settings to eeprom
				//app_state.selected_menu_item_index = 0;
				//app_state.current_state_func = app_state_menu_store_eeprom_settings;
				app_store_settings_to_eeprom();
				break;
		}
	}
	return EC_SUCCESS;
}

ErrorCode app_state_menu_heater()
{
	if(app_state.current_input.rotenc_delta > 0)
		app_state.selected_menu_item_index = imax8(imin8(app_state.selected_menu_item_index + 1, 4), 0);
	else if(app_state.current_input.rotenc_delta < 0)
		app_state.selected_menu_item_index = imax8(imin8(app_state.selected_menu_item_index - 1, 4), 0);
	// display selected menu item
	srd_clear();
	mr_heater_menu(app_state.selected_menu_item_index);
	srd_display();
	
	// state change
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		switch(app_state.selected_menu_item_index)
		{
			case 0:	// back to main menu
				app_state.selected_menu_item_index = 1;
				app_state.current_state_func = app_state_menu_main;
				break;
			case 1: // heater on / off
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_heater_onoff;
				break;
			case 2:	// heater target temp
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_heater_target_temp;
				break;
			case 3:	// thermistor select
				app_state.selected_menu_item_index = app_state.settings.controlling_tprobe;
				app_state.current_state_func = app_state_menu_heater_controlling_tprobe;
				break;
			case 4:	// heater pid menu
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_heater_pid;
				break;
		}
	}
	return EC_SUCCESS;
}

ErrorCode app_state_menu_heater_onoff()
{
	if(app_state.current_input.rotenc_delta != 0)
	{
		app_state.heater_onoff = !app_state.heater_onoff;
		if(app_state.heater_onoff)
		{
			heater_on();
		}
		else
		{
			heater_off();
			app_state.heater_rapid_heating = FALSE;
		}
		pid_reset(&app_state.pid_state);
	}
	
	// display current value
	srd_clear();
	mr_heater_menu_onoff(app_state.heater_onoff);
	srd_display();
	
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		app_state.selected_menu_item_index = 1;
		app_state.current_state_func = app_state_menu_heater;
	}
	return EC_SUCCESS;
}

ErrorCode app_state_menu_heater_target_temp()
{
	if(app_state.current_input.rotenc_delta != 0)
		app_state.settings.heater_target_temp = fmax(fmin(app_state.settings.heater_target_temp + app_state.current_input.rotenc_delta * TEMP_CHANGE_PER_ROTENC_STEP, MAX_HEATER_TARGET_TEMP), MIN_HEATER_TARGET_TEMP);
	
	// display current value
	srd_clear();
	mr_heater_menu_target_temp(app_state.settings.heater_target_temp);
	srd_display();
	
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		app_state.selected_menu_item_index = 2;
		app_state.current_state_func = app_state_menu_heater;
	}
	return EC_SUCCESS;
}

ErrorCode app_state_menu_heater_controlling_tprobe()
{
	if(app_state.current_input.rotenc_delta > 0)
		app_state.selected_menu_item_index = imax8(imin8(app_state.selected_menu_item_index + 1, 3), 0);
	else if(app_state.current_input.rotenc_delta < 0)
		app_state.selected_menu_item_index = imax8(imin8(app_state.selected_menu_item_index - 1, 3), 0);
	
	uint8_t selection_valid;
	switch(app_state.selected_menu_item_index)
	{
		case 0:
			#ifdef TSENS_PROBE_0
				selection_valid = TRUE;
			#else
				selection_valid = FALSE;
			#endif
			break;
		case 1:
			#ifdef TSENS_PROBE_1
				selection_valid = TRUE;
			#else
				selection_valid = FALSE;
			#endif
			break;
		case 2:
			#ifdef TSENS_PROBE_2
				selection_valid = TRUE;
			#else
				selection_valid = FALSE;
			#endif
			break;
		case 3:
			#ifdef TSENS_PROBE_3
				selection_valid = TRUE;
			#else
				selection_valid = FALSE;
			#endif
			break;
		default:
			selection_valid = FALSE;
			break;
	}
	
	// display current selection
	srd_clear();
	mr_heater_menu_controlling_probe_select(app_state.selected_menu_item_index, selection_valid);
	srd_display();
	
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		if(selection_valid)
		{
			app_state.settings.controlling_tprobe = (uint8_t)app_state.selected_menu_item_index;
			app_state.selected_menu_item_index = 3;
			app_state.current_state_func = app_state_menu_heater;
		}		
	}
	return EC_SUCCESS;
}

ErrorCode app_state_menu_heater_pid()
{
	if(app_state.current_input.rotenc_delta > 0)
		app_state.selected_menu_item_index = imax8(imin8(app_state.selected_menu_item_index + 1, 6), 0);
	else if(app_state.current_input.rotenc_delta < 0)
		app_state.selected_menu_item_index = imax8(imin8(app_state.selected_menu_item_index - 1, 6), 0);
	// display selected menu item
	srd_clear();
	mr_heater_menu_pid(app_state.selected_menu_item_index);
	srd_display();
	
	// state change
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		switch(app_state.selected_menu_item_index)
		{
			case 0:	// back to main menu
				app_state.selected_menu_item_index = 4;
				app_state.current_state_func = app_state_menu_heater;
				break;
			case 1: // P
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_heater_pid_p;
				break;
			case 2:	// I
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_heater_pid_i;
				break;
			case 3:	// D
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_heater_pid_d;
				break;
			case 4:	// I-CLAMP
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_heater_pid_i_clamp;
				break;
			case 5:	// OFFSET
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_heater_pid_offset;
				break;
			case 6:	// DSMOOTH
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_heater_pid_d_smoothing_factor;
				break;
		}
	}
	return EC_SUCCESS;
}

ErrorCode app_state_menu_heater_pid_p()
{
	if(app_state.current_input.rotenc_delta != 0)
	{
		app_state.settings.heater_pid_kp = fmax(fmin(app_state.settings.heater_pid_kp + app_state.current_input.rotenc_delta * PID_COARSE_CHANGE_PER_ROTENC_STEP, MAX_HEATER_PID_P), MIN_HEATER_PID_P);
		pid_set_params(&app_state.pid_state, app_state.settings.heater_pid_kp, app_state.settings.heater_pid_ti, app_state.settings.heater_pid_td, app_state.settings.heater_pid_i_clamp, app_state.settings.heater_pid_offset, app_state.settings.heater_pid_d_smoothing_factor,  HEATER_CONTROL_MIN, HEATER_CONTROL_MAX);
	}
	
	// display current value
	srd_clear();
	mr_heater_menu_pid_p(app_state.settings.heater_pid_kp);
	srd_display();
	
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		app_state.selected_menu_item_index = 1;
		app_state.current_state_func = app_state_menu_heater_pid;
	}
	return EC_SUCCESS;
}

ErrorCode app_state_menu_heater_pid_i()
{
	if(app_state.current_input.rotenc_delta != 0)
	{
		app_state.settings.heater_pid_ti = fmax(fmin(app_state.settings.heater_pid_ti + app_state.current_input.rotenc_delta * PID_COARSE_CHANGE_PER_ROTENC_STEP, MAX_HEATER_PID_I), MIN_HEATER_PID_I);
		pid_set_params(&app_state.pid_state, app_state.settings.heater_pid_kp, app_state.settings.heater_pid_ti, app_state.settings.heater_pid_td, app_state.settings.heater_pid_i_clamp, app_state.settings.heater_pid_offset, app_state.settings.heater_pid_d_smoothing_factor, HEATER_CONTROL_MIN, HEATER_CONTROL_MAX);
	}
	
	// display current value
	srd_clear();
	mr_heater_menu_pid_i(app_state.settings.heater_pid_ti);
	srd_display();
	
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		app_state.selected_menu_item_index = 2;
		app_state.current_state_func = app_state_menu_heater_pid;
	}
	return EC_SUCCESS;
}

ErrorCode app_state_menu_heater_pid_d()
{
	if(app_state.current_input.rotenc_delta != 0)
	{
		app_state.settings.heater_pid_td = fmax(fmin(app_state.settings.heater_pid_td + app_state.current_input.rotenc_delta * PID_COARSE_CHANGE_PER_ROTENC_STEP, MAX_HEATER_PID_D), MIN_HEATER_PID_D);
		pid_set_params(&app_state.pid_state, app_state.settings.heater_pid_kp, app_state.settings.heater_pid_ti, app_state.settings.heater_pid_td, app_state.settings.heater_pid_i_clamp, app_state.settings.heater_pid_offset, app_state.settings.heater_pid_d_smoothing_factor, HEATER_CONTROL_MIN, HEATER_CONTROL_MAX);
	}
	
	// display current value
	srd_clear();
	mr_heater_menu_pid_d(app_state.settings.heater_pid_td);
	srd_display();
	
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		app_state.selected_menu_item_index = 3;
		app_state.current_state_func = app_state_menu_heater_pid;
	}
	return EC_SUCCESS;
}

ErrorCode app_state_menu_heater_pid_i_clamp()
{
	if(app_state.current_input.rotenc_delta != 0)
	{
		app_state.settings.heater_pid_i_clamp = fmax(fmin(app_state.settings.heater_pid_i_clamp + app_state.current_input.rotenc_delta * PID_COARSE_CHANGE_PER_ROTENC_STEP, MAX_HEATER_PID_I_CLAMP), MIN_HEATER_PID_I_CLAMP);
		pid_set_params(&app_state.pid_state, app_state.settings.heater_pid_kp, app_state.settings.heater_pid_ti, app_state.settings.heater_pid_td, app_state.settings.heater_pid_i_clamp, app_state.settings.heater_pid_offset, app_state.settings.heater_pid_d_smoothing_factor, HEATER_CONTROL_MIN, HEATER_CONTROL_MAX);
	}
	
	// display current value
	srd_clear();
	mr_heater_menu_pid_i_clamp(app_state.settings.heater_pid_i_clamp);
	srd_display();
	
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		app_state.selected_menu_item_index = 4;
		app_state.current_state_func = app_state_menu_heater_pid;
	}
	return EC_SUCCESS;
}

ErrorCode app_state_menu_heater_pid_offset()
{
	if(app_state.current_input.rotenc_delta != 0)
	{
		app_state.settings.heater_pid_offset = fmax(fmin(app_state.settings.heater_pid_offset + app_state.current_input.rotenc_delta * TEMP_CHANGE_PER_ROTENC_STEP, MAX_HEATER_OFFSET), MIN_HEATER_OFFSET);
		pid_set_params(&app_state.pid_state, app_state.settings.heater_pid_kp, app_state.settings.heater_pid_ti, app_state.settings.heater_pid_td, app_state.settings.heater_pid_i_clamp, app_state.settings.heater_pid_offset, app_state.settings.heater_pid_d_smoothing_factor, HEATER_CONTROL_MIN, HEATER_CONTROL_MAX);
	}
	
	// display current value
	srd_clear();
	mr_heater_menu_pid_offset(app_state.settings.heater_pid_offset);
	srd_display();
	
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		app_state.selected_menu_item_index = 5;
		app_state.current_state_func = app_state_menu_heater_pid;
	}
	return EC_SUCCESS;
}

ErrorCode app_state_menu_heater_pid_d_smoothing_factor()
{
	if(app_state.current_input.rotenc_delta != 0)
	{
		app_state.settings.heater_pid_d_smoothing_factor = fmax(fmin(app_state.settings.heater_pid_d_smoothing_factor + app_state.current_input.rotenc_delta * PID_FINE_CHANGE_PER_ROTENC_STEP, MAX_HEATER_PID_D_SMOOTHING_FACTOR), MIN_HEATER_PID_D_SMOOTHING_FACTOR);
		pid_set_params(&app_state.pid_state, app_state.settings.heater_pid_kp, app_state.settings.heater_pid_ti, app_state.settings.heater_pid_td, app_state.settings.heater_pid_i_clamp, app_state.settings.heater_pid_offset, app_state.settings.heater_pid_d_smoothing_factor, HEATER_CONTROL_MIN, HEATER_CONTROL_MAX);
	}
	
	// display current value
	srd_clear();
	mr_heater_menu_pid_dsmooth(app_state.settings.heater_pid_d_smoothing_factor);
	srd_display();
	
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		app_state.selected_menu_item_index = 6;
		app_state.current_state_func = app_state_menu_heater_pid;
	}
	return EC_SUCCESS;
}

ErrorCode app_state_menu_stirrer()
{
	if(app_state.current_input.rotenc_delta > 0)
		app_state.selected_menu_item_index = imax8(imin8(app_state.selected_menu_item_index + 1, 1), 0);
	else if(app_state.current_input.rotenc_delta < 0)
		app_state.selected_menu_item_index = imax8(imin8(app_state.selected_menu_item_index - 1, 1), 0);
	// display selected menu item
	srd_clear();
	mr_stirrer_menu(app_state.selected_menu_item_index);
	srd_display();
	
	// state change
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		switch(app_state.selected_menu_item_index)
		{
			case 0:	// back to main menu
				app_state.selected_menu_item_index = 2;
				app_state.current_state_func = app_state_menu_main;
				break;
			case 1: // heater duty cycle
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_stirrer_duty_cycle;
				break;
		}
	}
	return EC_SUCCESS;
}

ErrorCode app_state_menu_stirrer_duty_cycle()
{
	if(app_state.current_input.rotenc_delta != 0)
	{
		app_state.stirrer_duty_cycle = (uint8_t)imax16(imin16((int16_t)app_state.stirrer_duty_cycle + app_state.current_input.rotenc_delta * STIRRER_DC_CHANGE_PER_STEP, 100), 0);
		
		if(!app_state.stirrer_onoff && app_state.stirrer_duty_cycle > 0) // stirrer was switched on
		{
			stirrer_on();
		}
		else if(app_state.stirrer_onoff && (app_state.stirrer_duty_cycle == 0)) // stirrer was switched off
		{
			stirrer_off();
		}
			
		app_state.stirrer_onoff = app_state.stirrer_duty_cycle > 0;
		
		// set stirrer duty cycle
		stirrer_set_duty_cycle(app_state.stirrer_duty_cycle);
	}
	
	// display current value
	srd_clear();
	mr_stirrer_menu_dc(app_state.stirrer_duty_cycle);
	srd_display();
	
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		app_state.selected_menu_item_index = 1;
		app_state.current_state_func = app_state_menu_stirrer;
	}
	return EC_SUCCESS;
}

ErrorCode app_state_menu_fan()
{
	if(app_state.current_input.rotenc_delta > 0)
		app_state.selected_menu_item_index = imax8(imin8(app_state.selected_menu_item_index + 1, 1), 0);
	else if(app_state.current_input.rotenc_delta < 0)
		app_state.selected_menu_item_index = imax8(imin8(app_state.selected_menu_item_index - 1, 1), 0);
	// display selected menu item
	srd_clear();
	mr_fan_menu(app_state.selected_menu_item_index);
	srd_display();
	
	// state change
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		switch(app_state.selected_menu_item_index)
		{
			case 0:	// back to main menu
				app_state.selected_menu_item_index = 3;
				app_state.current_state_func = app_state_menu_main;
				break;
			case 1: // heater duty cycle
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_fan_duty_cycle;
				break;
		}
	}
	return EC_SUCCESS;
}

ErrorCode app_state_menu_fan_duty_cycle()
{
	if(app_state.current_input.rotenc_delta != 0)
	{
		app_state.fan_duty_cycle = (uint8_t)imax16(imin16((int16_t)app_state.fan_duty_cycle + app_state.current_input.rotenc_delta * STIRRER_DC_CHANGE_PER_STEP, 100), 0);
		app_state.settings.fan_duty_cycle = app_state.fan_duty_cycle;
		
		if(!app_state.fan_onoff && app_state.fan_duty_cycle > 0) // fan was switched on
		{
			fan_on();
		}
		else if(app_state.fan_onoff && (app_state.fan_duty_cycle == 0)) // fan was switched off
		{
			fan_off();
		}
		
		app_state.fan_onoff = app_state.fan_duty_cycle > 0;
		
		// set fan duty cycle
		fan_set_duty_cycle(app_state.fan_duty_cycle);
	}
	
	// display current value
	srd_clear();
	mr_fan_menu_dc(app_state.fan_duty_cycle);
	srd_display();
	
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		app_state.selected_menu_item_index = 1;
		app_state.current_state_func = app_state_menu_fan;
	}
	return EC_SUCCESS;
}

ErrorCode app_state_menu_tprobe()
{
	if(app_state.current_input.rotenc_delta > 0)
		app_state.selected_menu_item_index = imax8(imin8(app_state.selected_menu_item_index + 1, 4), 0);
	else if(app_state.current_input.rotenc_delta < 0)
		app_state.selected_menu_item_index = imax8(imin8(app_state.selected_menu_item_index - 1, 4), 0);
	// display selected menu item
	srd_clear();
	mr_tprobe_menu(app_state.selected_menu_item_index);
	srd_display();
	
	// state change
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		switch(app_state.selected_menu_item_index)
		{
			case 0:	// back to main menu
				app_state.selected_menu_item_index = 4;
				app_state.current_state_func = app_state_menu_main;
				break;
			case 1: // thermistor 0 resistance
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_tprobe0_calib;
				break;
			case 2: // thermistor 1 resistance
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_tprobe1_calib;
				break;
			case 3: // thermistor 2 resistance
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_tprobe2_calib;
				break;
			case 4: // thermistor 3 resistance
				app_state.selected_menu_item_index = 0;
				app_state.current_state_func = app_state_menu_tprobe3_calib;
				break;
		}
	}
	return EC_SUCCESS;
}

ErrorCode app_state_menu_tprobe0_calib()
{
	// display current resistance
	#ifdef TSENS_PROBE_0
	srd_clear();
	mr_tprobe_calib_menu(app_state.t0_resistance);
	srd_display();
	#else
	srd_clear();
	mr_tprobe_calib_menu_nc();
	srd_display();
	#endif
	
	// state change
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		app_state.selected_menu_item_index = 1;
		app_state.current_state_func = app_state_menu_tprobe;
	}
	
	return EC_SUCCESS; // everything ok
}

ErrorCode app_state_menu_tprobe1_calib()
{
	// display current resistance
	#ifdef TSENS_PROBE_1
	srd_clear();
	mr_tprobe_calib_menu(app_state.t1_resistance);
	srd_display();
	#else
	srd_clear();
	mr_tprobe_calib_menu_nc();
	srd_display();
	#endif
	
	// state change
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		app_state.selected_menu_item_index = 2;
		app_state.current_state_func = app_state_menu_tprobe;
	}
	
	return EC_SUCCESS; // everything ok
}

ErrorCode app_state_menu_tprobe2_calib()
{
	// display current resistance
	#ifdef TSENS_PROBE_2
	srd_clear();
	mr_tprobe_calib_menu(app_state.t2_resistance);
	srd_display();
	#else
	srd_clear();
	mr_tprobe_calib_menu_nc();
	srd_display();
	#endif
	
	// state change
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		app_state.selected_menu_item_index = 3;
		app_state.current_state_func = app_state_menu_tprobe;
	}
	
	return EC_SUCCESS; // everything ok
}

ErrorCode app_state_menu_tprobe3_calib()
{
	// display current resistance
	#ifdef TSENS_PROBE_3
	srd_clear();
	mr_tprobe_calib_menu(app_state.t3_resistance);
	srd_display();
	#else
	srd_clear();
	mr_tprobe_calib_menu_nc();
	srd_display();
	#endif
	
	// state change
	if(app_state.current_input.button_presses & (1 << BUTTON0))
	{
		app_state.selected_menu_item_index = 4;
		app_state.current_state_func = app_state_menu_tprobe;
	}
	
	return EC_SUCCESS; // everything ok
}

//ErrorCode app_state_menu_load_eeprom_settings()
//{
	//app_load_settings_from_eeprom();	
	//return EC_SUCCESS;
//}
//
//ErrorCode app_state_menu_store_eeprom_settings()
//{
	//app_store_settings_to_eeprom();
	//return EC_SUCCESS;
//}

////////////////////////////////////// HELPERS ////////////////////////////////////////////////////
void app_clear_input()
{
	app_state.current_input.rotenc_delta = 0;
	app_state.current_input.button_presses = 0;
	app_state.current_input.button_long_presses = 0;
	app_state.current_input.button_releases = 0;
	app_state.current_input.button_states = 0;
}

void app_error_display()
{
	switch(app_state.current_error)
	{
		case EC_SUCCESS:
			break;
		default:
			srd_clear();
			mr_thermistor_error(app_state.current_error);
			srd_display();
			break;
	}
}

void app_load_default_settings()
{
	app_state.settings.heater_target_temp = SETTINGS_DEFAULT_HEATER_TARGET_TEMP;
	app_state.settings.heater_pid_kp = SETTINGS_DEFAULT_HEATER_PID_KP;
	app_state.settings.heater_pid_ti = SETTINGS_DEFAULT_HEATER_PID_TI;
	app_state.settings.heater_pid_td = SETTINGS_DEFAULT_HEATER_PID_TD;
	app_state.settings.heater_pid_i_clamp = SETTINGS_DEFAULT_HEATER_PID_I_CLAMP;
	app_state.settings.heater_pid_offset = SETTINGS_DEFAULT_HEATER_PID_OFFSET;
	app_state.settings.heater_pid_d_smoothing_factor = SETTINGS_DEFAULT_HEATER_PID_D_SMOOTHING_FACTOR;
	app_state.settings.controlling_tprobe = SETTINGS_DEFAULT_CONTROLLING_TPROBE;
	app_state.settings.fan_duty_cycle = SETTINGS_DEFAULT_FAN_DUTY_CYCLE;
}

void app_load_settings_from_eeprom()
{
	eeprom_settings_t load_settings;
	eeprom_read_block(&load_settings, &app_eeprom_settings, sizeof(eeprom_settings_t));
	// if no valid data was found in eeprom, initialize it with default settings
	if(load_settings.magic_number != EEPROM_SETTINGS_MAGIC_NUMBER)
	{
		app_load_default_settings();
		app_store_settings_to_eeprom();
	}
	else
	{
		app_state.settings = load_settings.settings;
	}
	pid_set_params(&app_state.pid_state, app_state.settings.heater_pid_kp, app_state.settings.heater_pid_ti, app_state.settings.heater_pid_td, app_state.settings.heater_pid_i_clamp, app_state.settings.heater_pid_offset, app_state.settings.heater_pid_d_smoothing_factor, HEATER_CONTROL_MIN, HEATER_CONTROL_MAX);
}

void app_store_settings_to_eeprom()
{
	eeprom_settings_t store_settings = {EEPROM_SETTINGS_MAGIC_NUMBER, app_state.settings};
	eeprom_update_block(&store_settings, &app_eeprom_settings, sizeof(eeprom_settings_t));
}