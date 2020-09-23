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

#include "menu_rendering.h"
#include "srdisplay.h"
#include "my_util.h"
//#include <avr/pgmspace.h>

//// text patterns stored in progmem
//static const uint8_t pg_menu_patterns[] PROGMEM = {
	//SRD_CB, SRD_CA, SRD_CC, SRD_CT							// "BACK", 4
//};

void mr_main(float current_temp, uint8_t tprobe_index)
{
	switch(tprobe_index)
	{
		case 0:
			srd_set(0, SRD_D0 | SRD_DOT);
			break;
		case 1:
			srd_set(0, SRD_D1 | SRD_DOT);
			break;
		case 2:
			srd_set(0, SRD_D2 | SRD_DOT);
			break;
		case 3:
			srd_set(0, SRD_D3 | SRD_DOT);
			break;
	}
	srd_setfloat(current_temp, 1, 1, 5);
}

void mr_main_menu(uint8_t item_index)
{
	switch (item_index)
	{
		case 0: // "-"
			srd_set(0, SRD_MINUS);
			break;
		case 1: // "HEAT"
			srd_set(0, SRD_CH); srd_set(1, SRD_CE); srd_set(2, SRD_CA); srd_set(3, SRD_CT);
			break;
		case 2: // "STIR"
			srd_set(0, SRD_CS); srd_set(1, SRD_CT); srd_set(2, SRD_CI); srd_set(3, SRD_CR);
			break;
		case 3: // "FAN"
			srd_set(0, SRD_CF); srd_set(1, SRD_CA); srd_set(2, SRD_CN);
			break;
		case 4: // "T.CALIB"
			srd_set(0, SRD_CT | SRD_DOT); srd_set(1, SRD_CC); srd_set(2, SRD_CA); srd_set(3, SRD_CL); srd_set(4, SRD_CI); srd_set(5, SRD_CB);
			break;
		case 5: // "LOAD.S."
			srd_set(0, SRD_CL); srd_set(1, SRD_CO); srd_set(2, SRD_CA); srd_set(3, SRD_CD | SRD_DOT); srd_set(4, SRD_CS | SRD_DOT);
			break;
		case 6: // "STORE.S."
			srd_set(0, SRD_CS); srd_set(1, SRD_CT); srd_set(2, SRD_CO); srd_set(3, SRD_CR); srd_set(4, SRD_CE | SRD_DOT); srd_set(5, SRD_CS | SRD_DOT);
			break;
	}
}

void mr_heater_menu(uint8_t item_index)
{
	switch (item_index)
	{
		case 0: // "--"
			srd_set(0, SRD_MINUS); srd_set(1, SRD_MINUS);
			break;
		case 1: // "ONOFF"
			srd_set(0, SRD_CO); srd_set(1, SRD_CN); srd_set(2, SRD_CO); srd_set(3, SRD_CF); srd_set(4, SRD_CF);
			break;
		case 2: // "TG.TPTR"
			srd_set(0, SRD_CT); srd_set(1, SRD_CG | SRD_DOT); srd_set(2, SRD_CT); srd_set(3, SRD_CP);
			break;
		case 3: // "TSEL"
			srd_set(0, SRD_CT | SRD_DOT); srd_set(1, SRD_CS); srd_set(2, SRD_CE); srd_set(3, SRD_CL);
			break;
		case 4: // "PID"
			srd_set(0, SRD_CP); srd_set(1, SRD_CI); srd_set(2, SRD_CD);
			break;
		case 5: // "OFFSET"
			srd_set(0, SRD_CO); srd_set(1, SRD_CF); srd_set(2, SRD_CF); srd_set(3, SRD_CS);
			break;
	}
}

void mr_stirrer_menu(uint8_t item_index)
{
	switch (item_index)
	{
		case 0: // "--"
			srd_set(0, SRD_MINUS); srd_set(1, SRD_MINUS);
			break;
		case 1: // "SPEED"
			srd_set(0, SRD_CS); srd_set(1, SRD_CP); srd_set(2, SRD_CE); srd_set(3, SRD_CE); srd_set(4, SRD_CD);
			break;
	}
}

void mr_fan_menu(uint8_t item_index)
{
	switch (item_index)
	{
		case 0: // "--"
		srd_set(0, SRD_MINUS); srd_set(1, SRD_MINUS);
		break;
		case 1: // "SPEED"
		srd_set(0, SRD_CS); srd_set(1, SRD_CP); srd_set(2, SRD_CE); srd_set(3, SRD_CE); srd_set(4, SRD_CD);
		break;
	}
}

void mr_heater_menu_controlling_probe_select(uint8_t tprobe_index, uint8_t selection_valid)
{
	if(selection_valid)
	{
		switch (tprobe_index)
		{
			case 0:
				srd_set(4, SRD_CT); srd_set(5, SRD_D0);
				break;
			case 1:
				srd_set(4, SRD_CT); srd_set(5, SRD_D1);
				break;
			case 2:
				srd_set(4, SRD_CT); srd_set(5, SRD_D2);
				break;
			case 3:
				srd_set(4, SRD_CT); srd_set(5, SRD_D3);
				break;
			default:
				srd_set(0,SRD_CN); srd_set(1,SRD_CC);
		}
	}
	else
	{
		srd_set(0,SRD_CN); srd_set(1,SRD_CC);
	}	
}

void mr_heater_menu_pid(uint8_t item_index)
{
	switch (item_index)
	{
		case 0: // "---"
			srd_set(0, SRD_MINUS); srd_set(1, SRD_MINUS); srd_set(2, SRD_MINUS);
			break;
		case 1: // "KP"
			srd_set(0, SRD_CP);
			break;
		case 2: // "TI"
			srd_set(0, SRD_CT); srd_set(1, SRD_CI);
			break;
		case 3: // "TD"
			srd_set(0, SRD_CT); srd_set(1, SRD_CD);
			break;
		case 4: // "I-CLP"
			srd_set(0, SRD_CI); srd_set(1, SRD_MINUS); srd_set(2, SRD_CC); srd_set(3, SRD_CL); srd_set(4, SRD_CP);
			break;
		case 5: // "OFFSET"
			srd_set(0, SRD_CO); srd_set(1, SRD_CF); srd_set(2, SRD_CF); srd_set(3, SRD_CS); srd_set(4, SRD_CE); srd_set(5, SRD_CT);
			break;
		case 6: // "DSF"
			srd_set(0, SRD_CD); srd_set(1, SRD_CS); srd_set(2, SRD_CF);
			break;
	}
}

void mr_heater_menu_onoff(uint8_t onoff)
{
	if(onoff)
	{
		srd_set(0, SRD_E | SRD_F); srd_set(1, SRD_CO); srd_set(2, SRD_CN);
	}
	else
	{
		srd_set(0, SRD_E | SRD_F); srd_set(1, SRD_CO); srd_set(2, SRD_CF); srd_set(3, SRD_CF);
	}
}

void mr_heater_menu_target_temp(float temp)
{
	srd_set(0, SRD_E | SRD_F);
	srd_setfloat(temp, 1, 1, 5);
}

void mr_heater_menu_pid_p(float pid_p)
{
	srd_set(0, SRD_E | SRD_F);
	srd_setfloat(pid_p, 1, 2, 5);
}

void mr_heater_menu_pid_i(float pid_i)
{
	srd_set(0, SRD_E | SRD_F);;
	srd_setfloat(pid_i, 1, 2, 5);
}

void mr_heater_menu_pid_d(float pid_d)
{
	srd_set(0, SRD_E | SRD_F);
	srd_setfloat(pid_d, 1, 2, 5);
}

void mr_heater_menu_pid_i_clamp(float pid_i_clamp)
{
	srd_set(0, SRD_E | SRD_F);
	srd_setfloat(pid_i_clamp, 1, 2, 5);
}

void mr_heater_menu_pid_offset(float offset)
{
	srd_set(0, SRD_E | SRD_F);
	srd_setfloat(offset, 1, 2, 5);
}

void mr_heater_menu_pid_dsmooth(float dsmooth)
{
	srd_set(0, SRD_E | SRD_F);
	srd_setfloat(dsmooth, 1, 3, 5);
}

void mr_stirrer_menu_dc(uint8_t dutycycle)
{
	srd_set(0, SRD_E | SRD_F);
	if(dutycycle > 0)
	{		
		srd_setint16((int16_t)dutycycle, 1, 5);
	}
	else
	{
		srd_set(1, SRD_CO); srd_set(2, SRD_CF); srd_set(3, SRD_CF); 
	}	
}

void mr_fan_menu_dc(uint8_t dutycycle)
{
	srd_set(0, SRD_E | SRD_F);
	if(dutycycle > 0)
	{
		srd_setint16((int16_t)dutycycle, 1, 5);
	}
	else
	{
		srd_set(1, SRD_CO); srd_set(2, SRD_CF); srd_set(3, SRD_CF);
	}
}

void mr_tprobe_menu(uint8_t menu_index)
{
	switch(menu_index)
	{
		case 0: // "--"
			srd_set(0, SRD_MINUS); srd_set(1, SRD_MINUS);
			break;
		case 1: // thermistor 0
			srd_set(0, SRD_CT); srd_set(1, SRD_D0);
			break;
		case 2: // thermistor 1
			srd_set(0, SRD_CT); srd_set(1, SRD_D1);
			break;
		case 3: // thermistor 2
			srd_set(0, SRD_CT); srd_set(1, SRD_D2);
			break;
		case 4: // thermistor 3
			srd_set(0, SRD_CT); srd_set(1, SRD_D3);
			break;
	}
}

void mr_tprobe_calib_menu(float resistance)
{
	srd_setfloat(fabs(resistance) / 1000, 0, 3, 6);
}

void mr_tprobe_calib_menu_nc()
{
	srd_set(0,SRD_CN); srd_set(1,SRD_CC);
}

void mr_thermistor_error(ErrorCode error)
{
	
	switch(error)
	{
		case EC_THERMISTOR_SHORT_CIRCUIT:
			srd_set(0, SRD_CT); srd_set(1, SRD_CH | SRD_DOT); srd_set(2, SRD_CS); srd_set(3, SRD_CH); srd_set(4, SRD_CR); srd_set(5, SRD_CT);
			break;
		case EC_THERMISTOR_OPEN_CIRCUIT:
			srd_set(0, SRD_CT); srd_set(1, SRD_CH | SRD_DOT); srd_set(2, SRD_CO); srd_set(3, SRD_CP); srd_set(4, SRD_CE); srd_set(5, SRD_CN);
			break;
		case EC_THERMISTOR_NOT_RESPONDING:
			srd_set(0, SRD_CT); srd_set(1, SRD_CH | SRD_DOT); srd_set(2, SRD_CN); srd_set(3, SRD_CR); srd_set(4, SRD_CE); srd_set(5, SRD_CS);
			break;
		case EC_NO_CONTROLLING_TPROBE:
			srd_set(0, SRD_CT); srd_set(1, SRD_CH | SRD_DOT); srd_set(2, SRD_CN); srd_set(3, SRD_CO); srd_set(4, SRD_CP); srd_set(5, SRD_CE);
			break;
		case EC_THERMISTOR_MIN_TEMP:
			srd_set(0, SRD_CT); srd_set(1, SRD_CH | SRD_DOT); srd_set(2, SRD_CL); srd_set(3, SRD_CT); srd_set(4, SRD_CP);
			break;
		case EC_THERMISTOR_MAX_TEMP:
			srd_set(0, SRD_CT); srd_set(1, SRD_CH | SRD_DOT); srd_set(2, SRD_CH); srd_set(3, SRD_CT); srd_set(4, SRD_CP);
			break;
		default:
			srd_set(0, SRD_CE); srd_set(1, SRD_CR); srd_set(2, SRD_CR); srd_set(3, SRD_CO); srd_set(4, SRD_CR);
			break;
	}
}