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