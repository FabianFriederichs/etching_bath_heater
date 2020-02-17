/*
 * srdisplay.c
 *
 * Created: 25.07.2019 08:41:38
 *  Author: fabia
 */ 

#include "srdisplay.h"
#include "shiftreg.h"
#include "config.h"
#include "my_util.h"

// ------------------------- PRIVATE ----------------------------------------------------------------

#if SRD_DIGITS == 0
#error "SRD: No digits available"
#endif

// buffer for display data
static uint8_t srd_buffer[SRD_DIGITS];

static uint8_t srd_dig_to_pattern(uint8_t dig)
{
	switch (dig)
	{
		case 0: return SRD_D0; break;
		case 1:	return SRD_D1; break;
		case 2:	return SRD_D2; break;
		case 3:	return SRD_D3; break;
		case 4:	return SRD_D4; break;
		case 5:	return SRD_D5; break;
		case 6:	return SRD_D6; break;
		case 7:	return SRD_D7; break;
		case 8:	return SRD_D8; break;
		case 9:	return SRD_D9; break;
		default: return SRD_D0; break;
	}
}

static uint8_t srd_ctd(int16_t n)
{
	if(n == 0)
		return 1;
	uint8_t count = 0;
	if(n < 0)
		++count;
	while (n != 0)
	{
		n = n / 10;
		++count;
	}
	return count;
}

static uint8_t srd_signed_ctd(int16_t n, uint8_t isneg)
{
	if(n == 0)
	return 1 + (isneg ? 1 : 0);
	uint8_t count = 0;
	if(isneg)
		++count;
	while (n != 0)
	{
		n = n / 10;
		++count;
	}
	return count;
}

static uint8_t srd_int16_to_patterns(int16_t num, uint8_t * buf, uint8_t buflength)
{
	uint8_t ct = 0;
	uint8_t isneg = num < 0;
	if(isneg)
	{
		num = -num;
	}
	if(num == 0)
	{
		buf[((int16_t)buflength - 1)] = SRD_D0;
		++ct;
	}
	else
	{
		while(num > 0 && ct < (buflength - (isneg ? 1 : 0)))
		{
			buf[(buflength - 1) - ct++] = srd_dig_to_pattern(num % 10);
			num /= 10;
		}
	}	
	if(isneg)
	{
		buf[((int16_t)buflength - 1) - (int16_t)ct] = SRD_MINUS;
	}
	return TRUE;
}

static uint8_t srd_signed_int16_to_patterns(int16_t num, uint8_t isneg, uint8_t * buf, uint8_t buflength)
{
	uint8_t ct = 0;
	if(isneg)
	{
		num = -num;
	}
	if(num == 0)
	{
		buf[((int16_t)buflength - 1)] = SRD_D0;
		++ct;
	}
	else
	{
		while(num > 0 && ct < (buflength - (isneg ? 1 : 0)))
		{
			buf[(buflength - 1) - ct++] = srd_dig_to_pattern(num % 10);
			num /= 10;
		}
	}
	if(isneg)
	{
		buf[((int16_t)buflength - 1) - (int16_t)ct] = SRD_MINUS;
	}
	return TRUE;
}

static uint8_t srd_float_to_patterns(float num, uint8_t decimal_places, uint8_t * buf, uint8_t buflength)
{
	uint8_t isneg = num < 0.0;
	int16_t wholePart = (int16_t)num;
	int16_t fracPart = (int16_t)(fabs(num - (float)wholePart) * (float)decpowi16(decimal_places));
	uint8_t whplen = srd_signed_ctd(wholePart, isneg);
	uint8_t frplen = srd_ctd(fracPart);
	if((whplen + decimal_places) > buflength)
		return FALSE; 
	// srd_int16_to_patterns produces left aligned numbers
	// write fractional part into decimal block
	srd_signed_int16_to_patterns(fracPart, FALSE, &buf[buflength - decimal_places], decimal_places);
	// fill space between decimal point and fractional part with zeros
	for(int16_t i = 0; i < decimal_places - frplen; ++i)
	{
		buf[buflength - decimal_places + i] = SRD_D0;
	}	
	// write whole part
	srd_signed_int16_to_patterns(wholePart, isneg, &buf[buflength - decimal_places - whplen], whplen);
	buf[buflength - decimal_places - 1] |= SRD_P;
	return TRUE;	
}

// ---------------------------------- PUBLIC --------------------------------------------------------

void srd_init()
{
	shreg_init();
	srd_clear();
	srd_display();	
}

void srd_shutdown()
{
	shreg_shutdown();
}

uint8_t srd_set(uint8_t index, uint8_t pattern)
{
	if(index >= SRD_DIGITS) return FALSE;
	srd_buffer[index] = pattern;
	return TRUE;
}

uint8_t srd_setm(uint8_t index, uint8_t patterns[], uint8_t length)
{
	if(index + length > SRD_DIGITS) return FALSE;
	for(uint8_t i = 0; i < length; ++i)
	{
		srd_set(i + index, patterns[i]);
	}
	return TRUE;
}

uint8_t srd_setfloat(float num, uint8_t index, uint8_t decimal_places, uint8_t length)
{
	if(index + length > SRD_DIGITS) return FALSE;
	return srd_float_to_patterns(num, decimal_places, &srd_buffer[index], length);
}

uint8_t srd_setint16(int16_t num, uint8_t index, uint8_t length)
{
	if(index + length > SRD_DIGITS) return FALSE;
	return srd_int16_to_patterns(num, &srd_buffer[index], length);
}

void srd_display()
{
	shreg_clear();
	shreg_write_bytes(srd_buffer, SRD_DIGITS);
	shreg_out();
}

void srd_clear()
{
	for(uint8_t i = 0; i < SRD_DIGITS; ++i)
	{
		srd_buffer[i] = SRD_EMPTY;
	}
}





