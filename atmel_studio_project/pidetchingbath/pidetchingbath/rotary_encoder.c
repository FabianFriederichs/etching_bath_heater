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

#include "rotary_encoder.h"
#include "config.h"
#include "my_util.h"
#include <util/atomic.h>
#include <avr/pgmspace.h>

#define ROT_ENC_PIN_MASK ((1 << ROT_ENC_A) | (1 << ROT_ENC_B))
#define ROT_ENC_READ_A (~ROT_ENC_PIN & (1 << ROT_ENC_A))
#define ROT_ENC_READ_B (~ROT_ENC_PIN & (1 << ROT_ENC_B))

#ifdef ROT_ENC_REVERSE_DIR
	#define ROT_ENC_SIGN_MUL -1
#else
	#define ROT_ENC_SIGN_MUL 1
#endif

// ------------------------------------ PRIVATE -----------------------------------------

/*  OLD			NEW			DELTA
	
	A	B		A	B
	
0	0	0		0	0		0
1	0	0		0	1		0
2	0	0		1	0		0
3	0	0		1	1		0
4	0	1		0	0		0
5	0	1		0	1		0
6	0	1		1	0		0
7	0	1		1	1		1			1
8	1	0		0	0		0
9	1	0		0	1		0
10	1	0		1	0		0
11	1	0		1	1		0
12	1	1		0	0		0
13	1	1		0	1		-1			-1
14	1	1		1	0		0
15	1	1		1	1		0
*/

static const int16_t rotenc_decoder_lut[16] PROGMEM = {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -1, 0, 0};

static volatile uint8_t rotenc_last;
static volatile int16_t rotenc_delta;

// ------------------------------------ PUBLIC -------------------------------------------

void rotenc_init()
{
	// set data dir
	ROT_ENC_DDR &= ~ROT_ENC_PIN_MASK;
	// enable internal pullups
	ROT_ENC_PORT |= ROT_ENC_PIN_MASK;
	
	rotenc_delta = 0;
	if(ROT_ENC_READ_A) rotenc_last = 2;
	if(ROT_ENC_READ_B) rotenc_last |= 1;
}

void rotenc_shutdown()
{
	rotenc_delta = 0;
	// disable internal pullups
	ROT_ENC_PORT &= ~ROT_ENC_PIN_MASK;
}

int16_t rotenc_get_inc()
{
	int16_t res;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		res = rotenc_delta;
		rotenc_delta = 0;
	}
	return res;
}

void rotenc_update()
{
	// combine old and new state into one byte and use as address for the LUT.
	// shift old state
	rotenc_last = (rotenc_last << 2) & 0x0F;
	// write new state
	if(ROT_ENC_READ_A) rotenc_last |= 2;
	if(ROT_ENC_READ_B) rotenc_last |= 1;
	
	// retrieve increment value from LUT
	rotenc_delta += (int16_t)pgm_read_word(&rotenc_decoder_lut[rotenc_last]) * ROT_ENC_SIGN_MUL;
	
	// clamp delta to prevent overflows
	rotenc_delta = imax16(ROT_ENC_MIN_DELTA, imin16(ROT_ENC_MAX_DELTA, rotenc_delta));
}
