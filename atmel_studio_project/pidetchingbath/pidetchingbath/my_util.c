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

#include "my_util.h"

#include <stdint.h>
#include <assert.h>

int16_t decpowi16(int16_t e)
{
	assert(e >= 0);
	int16_t result = 1;
	for(uint8_t i = 0; i < e; ++i)
	{
		result *= 10;
	}
	return result;
}

int16_t iabs16(int16_t num)
{
	if(num < 0)
		return -num;
	else
		return num;
}

int8_t iround8(float num)
{
	return (int8_t)(num + (num < 0.0 ? -0.5 : 0.5));
}

int16_t iround16(float num)
{
	return (int16_t)(num + (num < 0.0 ? -0.5 : 0.5));
}

int32_t iround32(float num)
{
	return (int32_t)(num + (num < 0.0 ? -0.5 : 0.5));
}

int64_t iround64(float num)
{
	return (int64_t)(num + (num < 0.0 ? -0.5 : 0.5));
}

int8_t imax8(int8_t num1, int8_t num2)
{
	return num1 > num2 ? num1 : num2;
}
int16_t imax16(int16_t num1, int16_t num2)
{
	return num1 > num2 ? num1 : num2;
}
int32_t imax32(int32_t num1, int32_t num2)
{
	return num1 > num2 ? num1 : num2;
}
int64_t imax64(int64_t num1, int64_t num2)
{
	return num1 > num2 ? num1 : num2;
}
int8_t imin8(int8_t num1, int8_t num2)
{
	return num1 < num2 ? num1 : num2;
}
int16_t imin16(int16_t num1, int16_t num2)
{
	return num1 < num2 ? num1 : num2;
}
int32_t imin32(int32_t num1, int32_t num2)
{
	return num1 < num2 ? num1 : num2;
}
int64_t imin64(int64_t num1, int64_t num2)
{
	return num1 < num2 ? num1 : num2;
}
uint8_t umax8(uint8_t num1, uint8_t num2)
{
	return num1 > num2 ? num1 : num2;
}
uint16_t umax16(uint16_t num1, uint16_t num2)
{
	return num1 > num2 ? num1 : num2;
}
uint32_t umax32(uint32_t num1, uint32_t num2)
{
	return num1 > num2 ? num1 : num2;
}
uint64_t umax64(uint64_t num1, uint64_t num2)
{
	return num1 > num2 ? num1 : num2;
}
uint8_t umin8(uint8_t num1, uint8_t num2)
{
	return num1 < num2 ? num1 : num2;
}
uint16_t umin16(uint16_t num1, uint16_t num2)
{
	return num1 < num2 ? num1 : num2;
}
uint32_t umin32(uint32_t num1, uint32_t num2)
{
	return num1 < num2 ? num1 : num2;
}
uint64_t umin64(uint64_t num1, uint64_t num2)
{
	return num1 < num2 ? num1 : num2;
}
float fmax(float num1, float num2)
{
	return num1 > num2 ? num1 : num2;
}
float fmin(float num1, float num2)
{
	return num1 < num2 ? num1 : num2;
}

float fabs(float num)
{
	if(num < 0.0)
		return -num;
	else
		return num;
}

float fround(float num, uint8_t decimal_places)
{
	int16_t sf = decpowi16((int16_t)decimal_places);
	return (float)((int16_t)(num * sf + (num < 0.0 ? -0.5 : 0.5))) / sf;
}

uint8_t crc7_byte(uint8_t byte)
{
	uint8_t generator = 0b10001001;
	uint8_t crc = byte;

	for (uint8_t i = 0; i < 8; ++i)
	{
		if ((crc & 0x80) != 0)
		{
			// if a 1 would pop out at the next shift, do the division
			crc = ((crc << 1) ^ (generator << 1));	// the msb is not needed for calculation.
		}											// We only consider the stuff in our "division register"
		else
		{
			crc <<= 1; // shift until a 1 pops out
		}
	}
	return crc >> 1; // shift registers msb is ignored so we have to shift one to the right
}

uint8_t crc7_bytes(const uint8_t byte[], uint16_t length)
{
	uint8_t generator = 0b10001001;
	uint8_t crc = 0x00;

	for(uint16_t b = 0; b < length; ++b)
	{
		crc ^= byte[b];
		for (uint8_t i = 0; i < 8; ++i)
		{
			if ((crc & 0x80) != 0)
			{
				crc = ((crc << 1) ^ (generator << 1));
			}
			else
			{
				crc <<= 1;
			}
		}
	}
	return crc >> 1; // shift registers msb is ignored so we have to shift one to the right
}

uint8_t crc7_append(uint8_t byte, uint8_t old_crc)
{
	uint8_t generator = 0b10001001;
	uint8_t crc = (old_crc << 1) ^ byte;

	for (uint8_t i = 0; i < 8; ++i)
	{
		if ((crc & 0x80) != 0)
		{
			crc = ((crc << 1) ^ (generator << 1));
		}
		else
		{
			crc <<= 1;
		}
	}
	return crc >> 1; // shift registers msb is ignored so we have to shift one to the right
}
