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

#include "shiftreg.h"
#include "config.h"
#include <util/delay.h>

#if SH_REG_DELAY_US
#define SH_REG_DELAY _delay_us(SH_REG_DELAY_US);
#else
#define SH_REG_DELAY
#endif

void shreg_init()
{
	// set data dir
	SH_REG_DDR |= (1 << SH_REG_SCLR) | (1 << SH_REG_SCK) | (1 << SH_REG_RCK) | (1 << SH_REG_SI);
	// clear SI, SCK and RCK
	SH_REG_PORT &= ~((1 << SH_REG_SI) | (1 << SH_REG_SCK) | (1 << SH_REG_RCK));
	// clear (inverted) SCLR
	SH_REG_PORT |= (1 << SH_REG_SCLR);
	
	// reset shift register and its output
	shreg_clear();
	shreg_out();
}

void shreg_shutdown()
{
	shreg_clear();
	shreg_out();
}

void shreg_write_bit(uint8_t bit)
{
	SH_REG_PORT = (SH_REG_PORT & ~(1 << SH_REG_SI)) | (bit << SH_REG_SI);
	SH_REG_DELAY
	shreg_shift();
}

void shreg_write_byte(uint8_t byte)
{
	for(uint8_t i = 0; i < 8; ++i)
	{
		shreg_write_bit((byte >> i) & 0x1);
	}
}

void shreg_write_bytes(uint8_t bytes[], uint8_t length)
{
	for(uint8_t i = 0; i < length; ++i)
	{
		shreg_write_byte(bytes[(length - 1) - i]);
	}
}

void shreg_clear()
{
	SH_REG_PORT &= ~(1 << SH_REG_SCLR);
	SH_REG_DELAY
	SH_REG_PORT |= (1 << SH_REG_SCLR);
	SH_REG_DELAY
}

void shreg_shift()
{
	SH_REG_PORT |= (1 << SH_REG_SCK);
	SH_REG_DELAY
	SH_REG_PORT &= ~(1 << SH_REG_SCK);
	SH_REG_DELAY
}

void shreg_mshift(uint8_t amount)
{
	for(uint8_t i = 0; i < amount; ++i)
	{
		SH_REG_PORT |= (1 << SH_REG_SCK);
		SH_REG_DELAY
		SH_REG_PORT &= ~(1 << SH_REG_SCK);
		SH_REG_DELAY
	}
}

void shreg_out()
{
	SH_REG_PORT |= (1 << SH_REG_RCK);
	SH_REG_DELAY
	SH_REG_PORT &= ~(1 << SH_REG_RCK);
	SH_REG_DELAY
}