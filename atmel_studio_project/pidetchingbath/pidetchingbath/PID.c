/*
 * PID.c
 *
 * Created: 16.02.2020 15:30:53
 *  Author: fabia
 */ 

#include "PID.h"
#include "my_util.h"
#include "config.h"

void pid_init(pid_state_t* state, float pid_Kp, float pid_Ti, float pid_Td, float pid_i_clamp, float pid_offset, float smoothing_factor, float control_min, float control_max)
{
	state->Kp = pid_Kp;
	state->Ti = pid_Ti;
	state->Td = pid_Td;
	state->i_clamp = pid_i_clamp;
	state->control_max = control_max;
	state->control_min = control_min;
	state->offset = pid_offset;
	state->smoothing_factor = smoothing_factor;
	
	state->old_process_value = 0.0;
	state->integrator = 0.0;
	state->smd1 = 0.0;
	state->smd2 = 0.0;
}

void pid_set_params(pid_state_t* state, float pid_Kp, float pid_Ti, float pid_Td, float pid_i_clamp, float pid_offset, float smoothing_factor, float control_min, float control_max)
{
	state->Kp = pid_Kp;
	state->Ti = pid_Ti;
	state->Td = pid_Td;
	state->i_clamp = pid_i_clamp;
	state->control_max = control_max;
	state->control_min = control_min;
	state->offset = pid_offset;
	state->smoothing_factor = smoothing_factor;
}

float pid_step(pid_state_t* state, float process_value, float set_value)
{
	// error
	float error = set_value - process_value;
	// proportional term
	float output = state->offset + state->Kp * error;
	// derivative term (instead of d/de use -d/dPV to get rid of set point spikes)
	float d_dPV = ((process_value - state->old_process_value) / PID_DELTA_T);
	// second order exponential smoothing
	state->smd1 = (1.0 - state->smoothing_factor) * d_dPV + state->smoothing_factor * state->smd1;
	state->smd2 = (1.0 - state->smoothing_factor) * state->smd1 + (1.0 - state->smoothing_factor) * state->smd2;
	// calculate d term contribution
	output -= state->Kp * state->Td * state->smd2;
	state->old_process_value = process_value;
	// integral term
	// integrate and clamp error signal; dynamic clamping! (and additionally scale the usable integrator range with i_clamp e[0, 1] to lessen the integrator overshoot for large delays)
	float i_max = fmax(state->control_max - output, 0.0) * state->i_clamp;
	float i_min = fmin(state->control_min - output, 0.0) * state->i_clamp;
	state->integrator = fmax(fmin(state->integrator + (state->Kp / fmax(state->Ti, 1.0 / F_CPU)) * error * PID_DELTA_T, i_max), i_min);
	output += state->integrator; // = p + i + d
	// clamp to control signal range and return
	return fmax(fmin(output, state->control_max), state->control_min);	
}

void pid_reset(pid_state_t* state)
{
	state->old_process_value = 0.0;
	state->integrator = 0.0;
	state->smd1 = 0.0;
	state->smd2 = 0.0;
}