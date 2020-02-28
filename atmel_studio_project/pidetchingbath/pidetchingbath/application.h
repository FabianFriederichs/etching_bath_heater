/*
 * application.h
 *
 * Created: 12.02.2020 14:17:00
 *  Author: fabia
 */ 


#ifndef APPLICATION_H_
#define APPLICATION_H_
#include <stdint.h>
#include "config.h"

// sys-headers
#include <avr/io.h>
#include <util/atomic.h>
#include <avr/eeprom.h>

// subsystem headers
#include "app_timer.h"
#include "srdisplay.h"
#include "switch.h"
#include "rotary_encoder.h"
#include "temp_sensors.h"
#include "stirrer.h"
#include "heater.h"
#include "PID.h"

// menu stuff
#include "menu_rendering.h"

// helpers
#include "my_util.h"

//////////////////////////////////// DATA STRUCTURES / DEFINITIONS /////////////////////////////////
typedef struct
{
	float heater_target_temp;
	float heater_pid_kp;
	float heater_pid_ti;
	float heater_pid_td;
	float heater_pid_i_clamp;
	float heater_pid_offset;
	uint8_t controlling_tprobe;
} app_settings_t;

typedef struct
{
	int16_t rotenc_delta;			// last rotenc delta
	uint8_t button_presses;			// 1 for every button press event
	uint8_t button_long_presses;	// 1 for every button long press event
	uint8_t button_releases;		// 1 for every button release event
	uint8_t button_states;			// current state of buttons
} Input;

typedef uint8_t (*state_function)();

typedef struct  
{
	uint8_t magic_number;
	app_settings_t settings;
} eeprom_settings_t;
#define EEPROM_SETTINGS_MAGIC_NUMBER 7

// define safety temp varname
#ifdef HEATER_SAFETY_TPROBE
#if HEATER_SAFETY_TPROBE == 0 && TSENS_PROBE_0_PRESENT
#define HEATER_SAFETY_TPROBE_CURRENT_TEMP app_state.t0_current_temp
#elif HEATER_SAFETY_TPROBE == 1 && TSENS_PROBE_1_PRESENT
#define HEATER_SAFETY_TPROBE_CURRENT_TEMP app_state.t1_current_temp
#elif HEATER_SAFETY_TPROBE == 2 && TSENS_PROBE_2_PRESENT
#define HEATER_SAFETY_TPROBE_CURRENT_TEMP app_state.t2_current_temp
#elif HEATER_SAFETY_TPROBE == 3 && TSENS_PROBE_3_PRESENT
#define HEATER_SAFETY_TPROBE_CURRENT_TEMP app_state.t3_current_temp
#else
#error "Unknown safety tprobe index."
#endif
#else
#error "No safety tprobe index defined."
#endif

//////////////////////////////////// APP STATE /////////////////////////////////////////////////////


typedef struct {
	// --- persistent state ---
	app_settings_t settings;
	
	// --- runtime state ---
	volatile uint8_t should_stop; // stop condition for whole application
	
	// input
	Input current_input;
	
	// state machine
	state_function current_state_func;
	int8_t selected_menu_item_index;
	ErrorCode current_error;
	
	// controller state
	pid_state_t pid_state;	
	uint8_t stirrer_duty_cycle;
	uint8_t heater_rapid_heating;
	uint8_t heater_onoff;
	uint8_t stirrer_onoff;
	
	// sensor state
	#ifdef TSENS_PROBE_0
	float t0_current_temp;					// value of last t0 temp measurement
	float t0_resistance;					// value of last t0 resistance meaturement
	float t0_tr_check_start_temp;			// thermal runaway check start temperature
	appt_cycle_t t0_tr_check_start_time;	// thermal runaway check start time
	#endif
	
	#ifdef TSENS_PROBE_1
	float t1_current_temp;					// value of last t1 temp measurement
	float t1_resistance;					// value of last t1 resistance meaturement
	float t1_tr_check_start_temp;			// thermal runaway check start temperature
	appt_cycle_t t1_tr_check_start_time;	// thermal runaway check start time
	#endif
	
	#ifdef TSENS_PROBE_2
	float t2_current_temp;					// value of last t2 temp measurement
	float t2_resistance;					// value of last t2 resistance meaturement
	float t2_tr_check_start_temp;			// thermal runaway check start temperature
	appt_cycle_t t2_tr_check_start_time;	// thermal runaway check start time
	#endif
	
	#ifdef TSENS_PROBE_3
	float t3_current_temp;					// value of last t3 temp measurement
	float t3_resistance;					// value of last t3 resistance meaturement
	float t3_tr_check_start_temp;			// thermal runaway check start temperature
	appt_cycle_t t3_tr_check_start_time;	// thermal runaway check start time
	#endif
} app_state_t;

ErrorCode app_run();
void app_shutdown();

ErrorCode app_user_main();
ErrorCode app_control();
ErrorCode app_rotenc_update();
ErrorCode app_button_update();

// state functions
ErrorCode app_state_main();
ErrorCode app_state_menu_main();
	ErrorCode app_state_menu_heater();
		ErrorCode app_state_menu_heater_onoff();
		ErrorCode app_state_menu_heater_target_temp();
		ErrorCode app_state_menu_heater_controlling_tprobe();
		ErrorCode app_state_menu_heater_pid();
			ErrorCode app_state_menu_heater_pid_p();
			ErrorCode app_state_menu_heater_pid_i();
			ErrorCode app_state_menu_heater_pid_d();
			ErrorCode app_state_menu_heater_pid_i_clamp();
			ErrorCode app_state_menu_heater_pid_offset();
	ErrorCode app_state_menu_stirrer();
		ErrorCode app_state_menu_stirrer_duty_cycle();
	ErrorCode app_state_menu_tprobe();
		ErrorCode app_state_menu_tprobe0_calib();
		ErrorCode app_state_menu_tprobe1_calib();
		ErrorCode app_state_menu_tprobe2_calib();
		ErrorCode app_state_menu_tprobe3_calib();
	//ErrorCode app_state_menu_load_eeprom_settings();
	//ErrorCode app_state_menu_store_eeprom_settings();
		
// error display
void app_error_display();			

// helpers
void app_clear_input();
void app_load_default_settings();
void app_load_settings_from_eeprom();
void app_store_settings_to_eeprom();

#endif /* APPLICATION_H_ */