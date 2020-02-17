/*
 * rotary_encoder.h
 *
 * Created: 25.07.2019 08:38:24
 *  Author: fabia
 */ 

#ifndef ROTARY_ENCODER_H_
#define ROTARY_ENCODER_H_

#include <stdint.h>

void rotenc_init();
void rotenc_shutdown();
int16_t rotenc_get_inc();
// call this approximately every 1 ms
void rotenc_update();

#endif /* ROTARY_ENCODER_H_ */