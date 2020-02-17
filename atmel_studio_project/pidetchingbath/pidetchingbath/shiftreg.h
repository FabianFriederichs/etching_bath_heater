/*
 * shiftreg.h
 *
 * Created: 25.07.2019 08:37:39
 *  Author: fabia
 */ 


#ifndef SHIFTREG_H_
#define SHIFTREG_H_

#include <stdint.h>

void shreg_init();
void shreg_shutdown();
void shreg_write_bit(uint8_t bit);
void shreg_write_byte(uint8_t byte);
void shreg_write_bytes(uint8_t bytes[], uint8_t length);
void shreg_clear();
void shreg_shift();
void shreg_mshift(uint8_t amount);
void shreg_out();

#endif /* SHIFTREG_H_ */