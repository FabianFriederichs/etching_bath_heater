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


#ifndef SRDISPLAY_H_
#define SRDISPLAY_H_

#include <stdint.h>

// available patterns
// individual segments
#define SRD_A		(0x1 << 0)
#define SRD_B		(0x1 << 1)
#define SRD_C		(0x1 << 2)
#define SRD_D		(0x1 << 3)
#define SRD_E		(0x1 << 4)
#define SRD_F		(0x1 << 5)
#define SRD_G		(0x1 << 6)
#define SRD_P		(0x1 << 7)

// digits
#define SRD_D0		SRD_A | SRD_B | SRD_C | SRD_D | SRD_E | SRD_F
#define SRD_D1		SRD_B | SRD_C
#define SRD_D2		SRD_A | SRD_B | SRD_D | SRD_E | SRD_G
#define SRD_D3		SRD_A | SRD_B | SRD_C | SRD_D | SRD_G
#define SRD_D4		SRD_B | SRD_C | SRD_F | SRD_G
#define SRD_D5		SRD_A | SRD_C | SRD_D | SRD_F | SRD_G
#define SRD_D6		SRD_A | SRD_C | SRD_D | SRD_E | SRD_F | SRD_G
#define SRD_D7		SRD_A | SRD_B | SRD_C
#define SRD_D8		SRD_A | SRD_B | SRD_C | SRD_D | SRD_E | SRD_F | SRD_G
#define SRD_D9		SRD_A | SRD_B | SRD_C | SRD_D | SRD_F | SRD_G

// characters
#define SRD_CA		SRD_A | SRD_B | SRD_C | SRD_E | SRD_F | SRD_G
#define SRD_CB		SRD_C | SRD_D | SRD_E | SRD_F | SRD_G
#define SRD_CC		SRD_A | SRD_D | SRD_E | SRD_F
#define SRD_CD		SRD_B | SRD_C | SRD_D | SRD_E | SRD_G
#define SRD_CE		SRD_A | SRD_D | SRD_E | SRD_F | SRD_G
#define SRD_CF		SRD_A | SRD_E | SRD_F | SRD_G
#define SRD_CG		SRD_A | SRD_B | SRD_C | SRD_D | SRD_F | SRD_G
#define SRD_CH		SRD_C | SRD_E | SRD_F | SRD_G
#define SRD_CI		SRD_B | SRD_C
#define SRD_CJ		SRD_B | SRD_C | SRD_D | SRD_E
#define SRD_CK		SRD_B | SRD_C | SRD_E | SRD_F | SRD_G
#define SRD_CL		SRD_D | SRD_E | SRD_F
#define SRD_CN		SRD_C | SRD_E | SRD_G
#define SRD_CO		SRD_C | SRD_D | SRD_E | SRD_G
#define SRD_CP		SRD_A | SRD_B | SRD_E | SRD_F | SRD_G
#define SRD_CQ		SRD_A | SRD_B | SRD_C | SRD_F | SRD_G
#define SRD_CR		SRD_E | SRD_G
#define SRD_CS		SRD_A | SRD_C | SRD_D | SRD_F | SRD_G
#define SRD_CT		SRD_D | SRD_E | SRD_F | SRD_G
#define SRD_CU		SRD_B | SRD_C | SRD_D | SRD_E | SRD_F
#define SRD_CV		SRD_C | SRD_D | SRD_E
#define SRD_CY		SRD_B | SRD_E | SRD_F | SRD_G
#define SRD_CZ		SRD_A | SRD_B | SRD_D | SRD_E | SRD_G

// specials
#define SRD_EMPTY	0x00
#define SRD_DOT		SRD_P
#define SRD_MINUS	SRD_G

void srd_init();
void srd_shutdown();

uint8_t srd_set(uint8_t index, uint8_t pattern);
uint8_t srd_setm(uint8_t index, uint8_t patterns[], uint8_t length);
uint8_t srd_setfloat(float num, uint8_t index, uint8_t decimal_places, uint8_t length);
uint8_t srd_setint16(int16_t num, uint8_t index, uint8_t length);
void srd_display();
void srd_clear();

#endif /* SRDISPLAY_H_ */