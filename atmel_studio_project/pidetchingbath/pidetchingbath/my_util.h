/*
 * my_util.h
 *
 * Created: 27.07.2019 16:14:39
 *  Author: fabia
 */ 


#ifndef MY_UTIL_H_
#define MY_UTIL_H_
#include <stdint.h>
// math stuff
int16_t decpowi16(int16_t e);
int16_t iabs16(int16_t num);
int8_t iround8(float num);
int16_t iround16(float num);
int32_t iround32(float num);
int64_t iround64(float num);
int8_t imax8(int8_t num1, int8_t num2);
int16_t imax16(int16_t num1, int16_t num2);
int32_t imax32(int32_t num1, int32_t num2);
int64_t imax64(int64_t num1, int64_t num2);
int8_t imin8(int8_t num1, int8_t num2);
int16_t imin16(int16_t num1, int16_t num2);
int32_t imin32(int32_t num1, int32_t num2);
int64_t imin64(int64_t num1, int64_t num2);

uint8_t umax8(uint8_t num1, uint8_t num2);
uint16_t umax16(uint16_t num1, uint16_t num2);
uint32_t umax32(uint32_t num1, uint32_t num2);
uint64_t umax64(uint64_t num1, uint64_t num2);
uint8_t umin8(uint8_t num1, uint8_t num2);
uint16_t umin16(uint16_t num1, uint16_t num2);
uint32_t umin32(uint32_t num1, uint32_t num2);
uint64_t umin64(uint64_t num1, uint64_t num2);

float fmax(float num1, float num2);
float fmin(float num1, float num2);
float fabs(float num);
float fround(float num, uint8_t decimal_places);

uint8_t crc7_byte(uint8_t byte);
uint8_t crc7_bytes(const uint8_t byte[], uint16_t length);
uint8_t crc7_append(uint8_t byte, uint8_t old_crc);


#endif /* MY_UTIL_H_ */