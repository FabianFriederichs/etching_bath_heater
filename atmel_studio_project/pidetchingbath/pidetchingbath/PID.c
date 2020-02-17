/*
 * PID.c
 *
 * Created: 16.02.2020 15:30:53
 *  Author: fabia
 */ 

#include "PID.h"
#include "my_util.h"

void pid_init(pid_state_t* state, float pid_p, float pid_i, float pid_d, float control_min, float control_max)
{
	state->kp = pid_p;
	state->ki = pid_i;
	state->kd = pid_d;
	state->control_max = control_max;
	state->control_min = control_min;
	
	state->old_process_value = 0.0;
	state->sum_error = 0.0;
}

void pid_set_params(pid_state_t* state, float pid_p, float pid_i, float pid_d, float control_min, float control_max)
{
	state->kp = pid_p;
	state->ki = pid_i;
	state->kd = pid_d;
	state->control_max = control_max;
	state->control_min = control_min;
	
	state->old_process_value = 0.0;
	state->sum_error = 0.0;
}

float pid_step(pid_state_t* state, float process_value, float set_value)
{
	// error
	float error = set_value - process_value;
	// proportional term
	float p_term = state->kp * error;
	// integral term
	float i_term = state->ki * (state->sum_error + error);
	// derivative term
	float d_term = state->kd * (process_value - state->old_process_value);
	state->old_process_value = process_value;
	
	// clamping the integrator
	float control = p_term + i_term + d_term;
	if(control >= state->control_min && control <= state->control_max)
	{
		state->sum_error += error;
		return control;
	}
	else
	{
		return fmax(fmin(p_term + state->ki * state->sum_error + d_term, state->control_max), state->control_min);
	}	
}

void pid_reset(pid_state_t* state)
{
	state->old_process_value = 0.0;
	state->sum_error = 0.0;
}