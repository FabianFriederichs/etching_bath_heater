/*
 * shiftreg.c
 *
 * Created: 25.07.2019 08:41:22
 *  Author: fabia
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