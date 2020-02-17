/*
 * temp_sensors.h
 *
 * Created: 14.02.2020 09:34:33
 *  Author: fabia
 */ 


#ifndef TEMP_SENSORS_H_
#define TEMP_SENSORS_H_
#include "config.h"

void tsens_init();
void tsens_shutdown();
void tsens_start_adc();
void tsens_stop_adc();

#ifdef TSENS_PROBE_0
	uint16_t tsens_measure0_raw(ErrorCode* ec);
	float tsens_measure_probe0_temp(ErrorCode* ec);
	float tsens_measure0_resistance(ErrorCode* ec);
#endif

#ifdef TSENS_PROBE_1
	uint16_t tsens_measure1_raw(ErrorCode* ec);
	float tsens_measure_probe1_temp(ErrorCode* ec);
	float tsens_measure1_resistance(ErrorCode* ec);
#endif

#ifdef TSENS_PROBE_2
	uint16_t tsens_measure2_raw(ErrorCode* ec);
	float tsens_measure_probe2_temp(ErrorCode* ec);
	float tsens_measure2_resistance(ErrorCode* ec);
#endif

#ifdef TSENS_PROBE_3
	uint16_t tsens_measure3_raw(ErrorCode* ec);
	float tsens_measure_probe3_temp(ErrorCode* ec);
	float tsens_measure3_resistance(ErrorCode* ec);
#endif

#endif /* TEMP_SENSORS_H_ */