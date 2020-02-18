/*
 * PID.h
 *
 * Created: 16.02.2020 15:30:37
 *  Author: fabia
 */
#ifndef PID_H_
#define PID_H_
#include <stdint.h>

typedef struct
{
	float old_error;
	float integrator;
	float kp;
	float ki;
	float kd;
	float i_clamp;
	float control_min;
	float control_max;
} pid_state_t;

void pid_init(pid_state_t* state, float pid_p, float pid_i, float pid_d, float pid_i_clamp, float control_min, float control_max);
void pid_set_params(pid_state_t* state, float pid_p, float pid_i, float pid_d, float pid_i_clamp, float control_min, float control_max);
float pid_step(pid_state_t* state, float process_value, float set_value);
void pid_reset(pid_state_t* state);

#endif /* PID_H_ */