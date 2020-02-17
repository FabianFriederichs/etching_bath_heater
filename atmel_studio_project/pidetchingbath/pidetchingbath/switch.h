/*
 * switch.h
 *
 * Created: 28.07.2019 22:41:28
 *  Author: fabia
 */ 


#ifndef SWITCH_H_
#define SWITCH_H_

#include <stdint.h>

#define SWITCH_RELEASED 0
#define SWITCH_PRESSED 1

void switch_init();
void switch_shutdown();
uint8_t switch_press(uint8_t index);
uint8_t switch_longpress(uint8_t index);
uint8_t switch_release(uint8_t index);
uint8_t switch_get_state(uint8_t index);
// call this approximately every 5 to 10 ms
void switch_update();




#endif /* SWITCH_H_ */