/*
 * config.h
 *
 * Created: 25.07.2019 08:37:18
 *  Author: fabia
 *
 * This file contains all configuration parameters.
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_

#include <avr/io.h>

// --------------------- convenience stuff ------------------

#define TRUE 1
#define FALSE 0

// --------------------- general uC settings ------------------------------------------

// controller base clock
#define UVCC 5.0
#ifndef F_CPU
#define F_CPU 8000000UL
#endif

//---------------------- application settings ------------------------------------------

// settings limits
#define MIN_HEATER_TARGET_TEMP 0.0
#define MAX_HEATER_TARGET_TEMP 100.0

#define MIN_HEATER_PID_P 0.0
#define MAX_HEATER_PID_P 999.99
#define MIN_HEATER_PID_I 0.0
#define MAX_HEATER_PID_I 999.99
#define MIN_HEATER_PID_D 0.0
#define MAX_HEATER_PID_D 999.99
// static clamping on top of dynamic c. Wen don't need I for speed in this application
#define MAX_HEATER_PID_I_CLAMP 1.0
#define MIN_HEATER_PID_I_CLAMP 0.0
#define MIN_HEATER_OFFSET -50.0
#define MAX_HEATER_OFFSET 50.0
#define MIN_HEATER_PID_D_SMOOTHING_FACTOR 0.0
#define MAX_HEATER_PID_D_SMOOTHING_FACTOR 1.0

// --------------------- temp sensor -----------------------------------------
#define TSENS_ADC_PRESCALER 64 // 2, 4, 8, 16, 32, 64, 128. F_CPU / PRESCALER should lie between 50kHz and 200kHz

#define TSENS_PROBE_0
#define TSENS_PROBE_0_RESISTANCE 100000
#define TSENS_PROBE_0_CHANNEL 0
// steinhart-hart coefficients
#define TSENS_PROBE_0_A0 -0.0011518398410375126
#define TSENS_PROBE_0_A1 0.00046229828474071683
#define TSENS_PROBE_0_A2 -5.209446350514327e-07

#define TSENS_PROBE_1
#define TSENS_PROBE_1_RESISTANCE 10000
#define TSENS_PROBE_1_CHANNEL 1
// steinhart-hart coefficients
#define TSENS_PROBE_1_A0 0.0013191887792613783
#define TSENS_PROBE_1_A1 0.00020290126749676136
#define TSENS_PROBE_1_A2 2.351818173839399e-07

// box filter with 4-sample-width
#define TSENS_NUM_MEASUREMENTS 4

// -------------------- heater --------------------------------------------------------------------------
// 256 gives ~60hz PWM frequency
#define HEATER_PWM_PRESCALE 256 // must be one out of {1, 8, 32, 64, 128, 256, 1024}
#define HEATER_CONTROL_MIN 0 // maximum duty cycle
#define HEATER_CONTROL_MAX 100 // minimum duty cycle

#define HEATER_MAX_OPERATING_TEMP 140.0 // max 100% duty cycle operating temp of heater mat
	
// heater thermal protection stuff
#define HEATER_SAFETY_TPROBE 0 // heater-attached safety probe index {0, 1, 2, 3}. Selected probe must be present and configured. Used to limit temperature of the heating element itself.
#define HEATER_TR_PROTECTION_MAX_TEMP 200.0 // if a temperature greater than this value is read from any probe, MAX_TEMP_ERROR is triggered
#define HEATER_TR_PROTECTION_MIN_TEMP 0.0 // if a temperature smaller than this value is read from any probe, MIN_TEMP_ERROR is triggered

// if heater is operating at full duty cycle, some temperature change is expected after some time interval.
// if the temperature change after that time interval is smaller than the expected change, THERMAL_RUNAWAY_ERROR is triggered.
#define HEATER_PROBE0_TR_PROTECTION_EXPECTED_TEMP_CHANGE 5.0 // expected temp change for probe 0
#define HEATER_PROBE1_TR_PROTECTION_EXPECTED_TEMP_CHANGE 1.0 // expected temp change for probe 1
//#define HEATER_PROBE2_EXPECTED_TEMP_CHANGE 1.0 //
//#define HEATER_PROBE3_EXPECTED_TEMP_CHANGE 1.0 //

// time intervals for thermal runway protection (time in seconds)
#define HEATER_PROBE0_TR_PROTECTION_INTERVAL 60 // at full heater dc, the heater mat thermistor should read at least 5 degrees temp change within 60 seconds
#define HEATER_PROBE1_TR_PROTECTION_INTERVAL (5 * 60) // at full heater dc, the bath thermistor should read at least 1 degrees temp change within 5 minutes
//#define HEATER_PROBE2_TR_PROTECTION_INTERVAL 20
//#define HEATER_PROBE3_TR_PROTECTION_INTERVAL 20

// defines the min duty cycle of the heater at which the TRP starts counting (has to be less than or equal to HEATER_CONTROL_MAX)
#define HEATER_TR_DUTY_CYCLE HEATER_CONTROL_MAX - 1

// -------------------- stirrer -------------------------------------------------------------------------
// 25khz pwm
#define STIRRER_PWM_PRESCALE 1 // must be one out of {1, 8, 64, 256, 1024}
#define STIRRER_PWM_TOP 160 // 16 bit uint, determines PWM resolution

// --------------------- display / shift register -------------------------------------------------------

#define SRD_DIGITS		6
#define SH_REG_PORT		PORTB
#define SH_REG_DDR		DDRB
#define SH_REG_PIN		PINB
#define SH_REG_SCLR		PORTB4
#define SH_REG_SCK		PORTB5
#define SH_REG_RCK		PORTB6
#define SH_REG_SI		PORTB7
#define SH_REG_WIDTH	8 * SRD_DIGITS
#define SH_REG_DELAY_US 0

// --------------------- rotary encoder -----------------------------------------------------------------

#define ROT_ENC_PORT	PORTB
#define ROT_ENC_DDR		DDRB
#define ROT_ENC_PIN		PINB
#define ROT_ENC_A		PORTD2
#define ROT_ENC_B		PORTD3
#define ROT_ENC_MAX_DELTA 1000
#define ROT_ENC_MIN_DELTA -1000
//#define ROT_ENC_REVERSE_DIR

// rotary encoder intervals
#define TEMP_CHANGE_PER_ROTENC_STEP 0.5
#define PID_COARSE_CHANGE_PER_ROTENC_STEP 0.1
#define PID_FINE_CHANGE_PER_ROTENC_STEP 0.005
#define STIRRER_DC_CHANGE_PER_STEP 1

// -------------------- switch --------------------------------------------------------------------------

// All switches have to be connected at one port. Up to #portpins switches are supported.
#define SWITCH_PORT		PORTB
#define SWITCH_DDR		DDRB
#define SWITCH_PIN		PINB
#define SWITCH_LONGPRESS_TICKS 100 // * 10ms
// switches have to start at bit 0 of a port and cover consecutive bits.
#define SWITCH_NUMBER	2
// for convenience
#define BUTTON0 0
#define BUTTON1 1

// -------------------- app timer ------------------------------------------------------------------------------------------------
// one main tick every 100us
#define APP_TIMER_MAX_CALLBACKS 4
#define APP_TIMER_BASE_CLOCK 99
#define APP_TIMER_PRESCALE APP_TIMER_PRESCALE_8
#define APP_TIMER_RESOLUTION APP_TIMER_RES_64_BIT

// callback intervals in seconds
#define APP_PID_LOOP_INTERVAL 0.02 // ~50hz
#define APP_USER_LOOP_INTERVAL 0.04 // ~25hz
#define APP_ROT_ENC_UPDATE_INTERVAL 0.001 // every 1 ms
#define APP_BUTTON_UPDATE_INTERVAL 0.005 // every 5 ms

// -------------------- default user-adjustable settings -------------------------------------------------------------------------

// default values
#define SETTINGS_DEFAULT_HEATER_TARGET_TEMP 25.0
#define SETTINGS_DEFAULT_HEATER_PID_KP 1.0
#define SETTINGS_DEFAULT_HEATER_PID_TI 30.0
#define SETTINGS_DEFAULT_HEATER_PID_TD 0.0
#define SETTINGS_DEFAULT_HEATER_PID_OFFSET 0.0
#define SETTINGS_DEFAULT_HEATER_PID_I_CLAMP 1.0
#define SETTINGS_DEFAULT_HEATER_PID_D_SMOOTHING_FACTOR 0.0
#define SETTINGS_DEFAULT_CONTROLLING_TPROBE HEATER_SAFETY_TPROBE

//////////////////////////////////////////////////////// HELPER STUFF //////////////////////////////////////////////////////
// num thermistors
#ifdef TSENS_PROBE_0
#define TSENS_PROBE_0_PRESENT 1
#else
#define TSENS_PROBE_0_PRESENT 0
#endif

#ifdef TSENS_PROBE_1
#define TSENS_PROBE_1_PRESENT 1
#else
#define TSENS_PROBE_1_PRESENT 0
#endif

#ifdef TSENS_PROBE_2
#define TSENS_PROBE_2_PRESENT 1
#else
#define TSENS_PROBE_2_PRESENT 0
#endif

#ifdef TSENS_PROBE_3
#define TSENS_PROBE_3_PRESENT 1
#else
#define TSENS_PROBE_3_PRESENT 0
#endif

typedef enum {
	EC_SUCCESS = 0,
	EC_THERMISTOR_OPEN_CIRCUIT = 1,
	EC_THERMISTOR_SHORT_CIRCUIT = 2,
	EC_THERMISTOR_NOT_RESPONDING = 3,
	EC_NO_CONTROLLING_TPROBE = 4,
	EC_THERMISTOR_MAX_TEMP = 5,
	EC_THERMISTOR_MIN_TEMP = 6
} ErrorCode;

// --------------------- PID -------------------------------------------------
#define PID_DELTA_T APP_PID_LOOP_INTERVAL

#endif /* CONFIG_H_ */