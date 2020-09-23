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

// FORMULA FOR CALCULATING RESISTANCE OF THERMISTOR
// RT = (R1 * UT) / (Uges - UT)


#include <avr/io.h>
#include "temp_sensors.h"
#include <math.h>
#include <util/atomic.h>

#if TSENS_ADC_PRESCALER == 2 // 2
	#define TSENS_ADC_PRESCALER_BITS (1 << ADPS0)
#elif TSENS_ADC_PRESCALER == 4 // 4
	#define TSENS_ADC_PRESCALER_BITS (1 << ADPS1)
#elif TSENS_ADC_PRESCALER == 8 // 8
	#define TSENS_ADC_PRESCALER_BITS ((1 << ADPS0) | (1 << ADPS1))
#elif TSENS_ADC_PRESCALER == 16 // 16
	#define TSENS_ADC_PRESCALER_BITS (1 << ADPS2)
#elif TSENS_ADC_PRESCALER == 32 // 32
	#define TSENS_ADC_PRESCALER_BITS ((1 << ADPS2) | (1 << ADPS0))
#elif TSENS_ADC_PRESCALER == 64 // 64
	#define TSENS_ADC_PRESCALER_BITS ((1 << ADPS2) | (1 << ADPS1))
#elif TSENS_ADC_PRESCALER == 128 // 64
	#define TSENS_ADC_PRESCALER_BITS ((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0))
#else
	#error "TEMP SENSORS: Unexpected prescaler selection."
#endif


// define unused adc channels.
#define TSENS_ADC_DEFAULT_DISABLE_BITS ((1 << ADC4_BIT) | (1 << ADC5_BIT) | (1 << ADC6_BIT) | (1 << ADC7_BIT))

#ifdef TSENS_PROBE_0
	#define TSENS_ADC_DISABLE_PROBE0_BIT (1 << ADC0_BIT)
#else
	#define TSENS_ADC_DISABLE_PROBE0_BIT 0x00
#endif

#ifdef TSENS_PROBE_1
	#define TSENS_ADC_DISABLE_PROBE1_BIT (1 << ADC1_BIT)
#else
	#define TSENS_ADC_DISABLE_PROBE1_BIT 0x00
#endif

#ifdef TSENS_PROBE_2
	#define TSENS_ADC_DISABLE_PROBE2_BIT (1 << ADC2_BIT)
#else
	#define TSENS_ADC_DISABLE_PROBE2_BIT 0x00
#endif

#ifdef TSENS_PROBE_3
	#define TSENS_ADC_DISABLE_PROBE3_BIT (1 << ADC3_BIT)
#else
	#define TSENS_ADC_DISABLE_PROBE3_BIT 0x00
#endif

#define TSENS_ADC_DISABLE_BITS (TSENS_ADC_DEFAULT_DISABLE_BITS | TSENS_ADC_DISABLE_PROBE0_BIT | TSENS_ADC_DISABLE_PROBE1_BIT | TSENS_ADC_DISABLE_PROBE2_BIT | TSENS_ADC_DISABLE_PROBE3_BIT)

#define TSENS_ADC_MUX_MASK ((1 << MUX4) | (1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0))

#define TSENS_ADC_PROBE0_MUX_BITS 0x00
#define TSENS_ADC_PROBE1_MUX_BITS (1 << MUX0)
#define TSENS_ADC_PROBE2_MUX_BITS (1 << MUX1)
#define TSENS_ADC_PROBE3_MUX_BITS ((1 << MUX0) | (1 << MUX1))

void tsens_init()
{
	// initialize adc
	// turn off adc
	ADCSRA &= ~((1 << ADEN) | (1 << ADIE) | (1 << ADATE));
	//clear ADMUX
	ADMUX = 0x00;
	// set voltage reference to AVCC
	ADMUX |= (1 << REFS0);
	// set prescaler
	ADCSRA |= TSENS_ADC_PRESCALER_BITS;
	// deactivate unused adc channels so save power
	DIDR0 = TSENS_ADC_DEFAULT_DISABLE_BITS;
}

void tsens_shutdown()
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		// shutdown adc
		// turn off adc
		ADCSRA &= ~((1 << ADEN) | (1 << ADIE) | (1 << ADATE));
		// clear ADMUX
		ADMUX = 0x00;
		// reset prescaler
		ADCSRA &= ~((1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2));
		// reset all deactivated pins
		DIDR0 = 0xFF;
	}	
}

void tsens_start_adc()
{
	ADCSRA |= (1 << ADEN);
	// do dummy reading
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC)) {};
	(void) ADCW;
}

void tsens_stop_adc()
{
	ADCSRA &= ~(1 << ADEN);
}

#ifdef TSENS_PROBE_0
uint16_t tsens_measure0_raw(ErrorCode* ec)
{
	// set channel
	ADMUX &= ~TSENS_ADC_MUX_MASK;
	ADMUX |= TSENS_ADC_PROBE0_MUX_BITS;
	// trigger reading
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC)) {};
	// return result
	uint16_t res = ADCW;
	
	// short circuit / open circuit protection
	if(res == 0)
		*ec = EC_THERMISTOR_SHORT_CIRCUIT;
	else if(res >= 1023)
		*ec = EC_THERMISTOR_OPEN_CIRCUIT;
	else
		*ec = EC_SUCCESS;
		
	return res;
}

float tsens_measure0_resistance(ErrorCode* ec)
{
	int32_t temp = 0;
	for(uint8_t i = 0; i < TSENS_NUM_MEASUREMENTS; ++i)
	{
		temp += tsens_measure0_raw(ec);
	}
	float tempf = ((float)temp / (TSENS_NUM_MEASUREMENTS * 1024)) * UVCC;
	
	// for now return resistance for calibration
	return (TSENS_PROBE_0_RESISTANCE * tempf) / (UVCC - tempf);
}

float tsens_measure_probe0_temp(ErrorCode* ec)
{
	float logR = log(tsens_measure0_resistance(ec));
	return (1.0 / (TSENS_PROBE_0_A0 + TSENS_PROBE_0_A1 * logR + TSENS_PROBE_0_A2 * logR * logR * logR)) - 273.15;
}
#endif

#ifdef TSENS_PROBE_1
uint16_t tsens_measure1_raw(ErrorCode* ec)
{
	// set channel
	ADMUX &= ~TSENS_ADC_MUX_MASK;
	ADMUX |= TSENS_ADC_PROBE1_MUX_BITS;
	// trigger reading
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC)) {};
	// return result
	uint16_t res = ADCW;
	
	// short circuit / open circuit protection
	if(res == 0)
		*ec = EC_THERMISTOR_SHORT_CIRCUIT;
	else if(res >= 1023)
		*ec = EC_THERMISTOR_OPEN_CIRCUIT;
	else
		*ec = EC_SUCCESS;
	
	return res;
}

float tsens_measure1_resistance(ErrorCode* ec)
{
	int32_t temp = 0;
	for(uint8_t i = 0; i < TSENS_NUM_MEASUREMENTS; ++i)
	{
		temp += tsens_measure1_raw(ec);
	}
	float tempf = ((float)temp / (TSENS_NUM_MEASUREMENTS * 1024)) * UVCC;
	
	// for now return resistance for calibration
	return (TSENS_PROBE_1_RESISTANCE * tempf) / (UVCC - tempf);
}

float tsens_measure_probe1_temp(ErrorCode* ec)
{
	float logR = log(tsens_measure1_resistance(ec));
	return (1.0 / (TSENS_PROBE_1_A0 + TSENS_PROBE_1_A1 * logR + TSENS_PROBE_1_A2 * logR * logR * logR)) - 273.15;
}
#endif

#ifdef TSENS_PROBE_2
uint16_t tsens_measure2_raw(ErrorCode* ec)
{
	// set channel
	ADMUX &= ~TSENS_ADC_MUX_MASK;
	ADMUX |= TSENS_ADC_PROBE2_MUX_BITS;
	// trigger reading
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC)) {};
	// return result
	uint16_t res = ADCW;
	
	// short circuit / open circuit protection
	if(res == 0)
		*ec = EC_THERMISTOR_SHORT_CIRCUIT;
	else if(res >= 1023)
		*ec = EC_THERMISTOR_OPEN_CIRCUIT;
	else
		*ec = EC_SUCCESS;
	
	return res;
}

float tsens_measure2_resistance(ErrorCode* ec)
{
	int32_t temp = 0;
	for(uint8_t i = 0; i < TSENS_NUM_MEASUREMENTS; ++i)
	{
		temp += tsens_measure2_raw(ec);
	}
	float tempf = ((float)temp / (TSENS_NUM_MEASUREMENTS * 1024)) * UVCC;
	
	// for now return resistance for calibration
	return (TSENS_PROBE_2_RESISTANCE * tempf) / (UVCC - tempf);
}

float tsens_measure_probe2_temp(ErrorCode* ec)
{
	float logR = log(tsens_measure2_resistance(ec));
	return (1.0 / (TSENS_PROBE_2_A0 + TSENS_PROBE_2_A1 * logR + TSENS_PROBE_2_A2 * logR * logR * logR)) - 273.15;
}
#endif

#ifdef TSENS_PROBE_3
uint16_t tsens_measure3_raw(ErrorCode* ec)
{
	// set channel
	ADMUX &= ~TSENS_ADC_MUX_MASK;
	ADMUX |= TSENS_ADC_PROBE3_MUX_BITS;
	// trigger reading
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADSC)) {};
	// return result
	uint16_t res = ADCW;
	
	// short circuit / open circuit protection
	if(res == 0)
		*ec = EC_THERMISTOR_SHORT_CIRCUIT;
	else if(res >= 1023)
		*ec = EC_THERMISTOR_OPEN_CIRCUIT;
	else
		*ec = EC_SUCCESS;
	
	return res;
}

float tsens_measure3_resistance(ErrorCode* ec)
{
	int32_t temp = 0;
	for(uint8_t i = 0; i < TSENS_NUM_MEASUREMENTS; ++i)
	{
		temp += tsens_measure3_raw(ec);
	}
	float tempf = ((float)temp / (TSENS_NUM_MEASUREMENTS * 1024)) * UVCC;
	
	// for now return resistance for calibration
	return (TSENS_PROBE_3_RESISTANCE * tempf) / (UVCC - tempf);
}

float tsens_measure_probe3_temp(ErrorCode* ec)
{
	float logR = log(tsens_measure3_resistance(ec));
	return (1.0 / (TSENS_PROBE_3_A0 + TSENS_PROBE_3_A1 * logR + TSENS_PROBE_3_A2 * logR * logR * logR)) - 273.15;
}
#endif
