/*
 * PID.c
 *
 * Created: 16.02.2020 15:30:53
 *  Author: fabia
 */ 

#include "PID.h"
#include "my_util.h"

void pid_init(pid_state_t* state, float pid_p, float pid_i, float pid_d, float pid_i_clamp, float control_min, float control_max)
{
	state->kp = pid_p;
	state->ki = pid_i;
	state->kd = pid_d;
	state->i_clamp = pid_i_clamp;
	state->control_max = control_max;
	state->control_min = control_min;
	
	state->old_process_value = 0.0;
	state->integrator = 0.0;
}

void pid_set_params(pid_state_t* state, float pid_p, float pid_i, float pid_d, float pid_i_clamp, float control_min, float control_max)
{
	state->kp = pid_p;
	state->ki = pid_i;
	state->kd = pid_d;
	state->i_clamp = pid_i_clamp;
	state->control_max = control_max;
	state->control_min = control_min;
	
	state->old_process_value = 0.0;
	state->integrator = 0.0;
}

float pid_step(pid_state_t* state, float process_value, float set_value)
{
	// error
	float error = set_value - process_value;
	// proportional term
	float output = state->kp * error;
	// derivative term (instead of d/de use -d/dPV to get rid of set point spikes)
	output -= state->kd * (process_value - state->old_process_value);
	state->old_process_value = process_value;
	// integral term
	// integrate and clamp error signal; dynamic clamping! (and additionally scale the usable integrator range with i_clamp e[0, 1] to lessen the integrator overshoot for large delays)
	float i_max = fmax(state->control_max - output, 0.0) * state->i_clamp;
	float i_min = fmin(state->control_min - output, 0.0) * state->i_clamp;
	state->integrator = fmax(fmin(state->integrator + state->ki * error, i_max), i_min);
	output += state->integrator; // = p + i + d
	// clamp to control signal range and return
	return fmax(fmin(output, state->control_max), state->control_min);	
}

void pid_reset(pid_state_t* state)
{
	state->old_process_value = 0.0;
	state->integrator = 0.0;
}