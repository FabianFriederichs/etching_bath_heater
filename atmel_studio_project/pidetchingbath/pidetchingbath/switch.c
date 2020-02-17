/*
 * switch.c
 *
 * Created: 28.07.2019 22:41:44
 *  Author: fabia
 */ 

#include "config.h"
#include "switch.h"

#define SWITCH_STATE_PRESS 0
#define SWITCH_STATE_RELEASE 1
#define SWITCH_STATE_LONGPRESS 2
#define SWITCH_STATE_PRESSED 3

#define SWITCH_PIN_MASK ((1 << SWITCH_NUMBER) - 1)


// ---------------------------- PRIVATE -----------------------------------------
// high level state
static volatile uint8_t switch_state_press;
static volatile uint8_t switch_state_release;
static volatile uint8_t switch_state_longpress;
static volatile uint8_t switch_press_counter[SWITCH_NUMBER];

// low level state
static volatile uint8_t switch_state;
static volatile uint8_t switch_debounce_ct0;
static volatile uint8_t switch_debounce_ct1;

// ---------------------------- PUBLIC ------------------------------------------

void switch_init()
{
	// set data dir
	SWITCH_DDR &= ~SWITCH_PIN_MASK;
	// enable internal pullups
	SWITCH_PORT |= SWITCH_PIN_MASK;
	
	// clear all state variables
	switch_state_press = 0x00;
	switch_state_release = 0x00;
	switch_state_longpress = 0x00;
	switch_state = 0x00;
	
	for (uint8_t i = 0; i < SWITCH_NUMBER; ++i)
		switch_press_counter[i] = 0;
	
	switch_debounce_ct0 = 0xFF & SWITCH_PIN_MASK;	
	switch_debounce_ct1 = 0xFF & SWITCH_PIN_MASK;	
}

void switch_shutdown()
{
	// disable internal pullups
	SWITCH_PORT &= ~SWITCH_PIN_MASK;
	
	// clear all state variables
	switch_state_press = 0x00;
	switch_state_release = 0x00;
	switch_state_longpress = 0x00;
	switch_state = 0x00;

	for (uint8_t i = 0; i < SWITCH_NUMBER; ++i)
		switch_press_counter[i] = 0;
	
	switch_debounce_ct0 = 0xFF & SWITCH_PIN_MASK;
	switch_debounce_ct1 = 0xFF & SWITCH_PIN_MASK;
}

uint8_t switch_press(uint8_t index)
{
	if(switch_state_press & (1 << index))
	{
		switch_state_press &= ~(1 << index);
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t switch_release(uint8_t index)
{
	if(switch_state_release & (1 << index))
	{
		switch_state_release &= ~(1 << index);
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t switch_longpress(uint8_t index)
{
	if(switch_state_longpress & (1 << index))
	{
		switch_state_longpress &= ~(1 << index);
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t switch_get_state(uint8_t index)
{
	if(switch_state & (1 << index))
	{
		return SWITCH_PRESSED;
	}
	else
	{
		return SWITCH_RELEASED;
	}
}

/* ------------ HOW THAT STUFF WORKS ------------
ct1 ct0 i	ct1 ct0		ct0 ct1
// switch changed from the last debounced state
1   1	1	1   0		~(1 & 1) => 0		// C1 3 -> 2
			1   0		0 ^ (1 & 1) => 1
1   0   1	1   1		~(0 & 1) => 1		// C2 2 -> 1
			0   1		1 ^ (1 & 1) => 0
0   1   1   0   0   	~(1 & 1) => 0		// C3 1 -> 0
			0   0		0 ^ (0 & 1) => 0
0   0   1	0   1		~(0 & 1) => 1		// C4 0 -> 3
			1	1		1 ^ (0 & 1) => 1
// switch changes before stopped bouncing
// or no change at all
1   1	0	1   1		~(1 & 0) => 1
			1   1		1 ^ (1 & 0) => 1
1   0   0	1   1		~(1 & 0) => 1
			1   1		1 ^ (1 & 0) => 1
0   1   0	1   1		~(1 & 0) => 1
			1   1		1 ^ (0 & 0) => 1
0   0   0	1   1		~(0 & 0) => 1
			1   1		1 ^ (0 & 1) => 1
*/

void switch_update()
{
	uint8_t pinchange = (switch_state ^ ~SWITCH_PIN);
	// bit 0 of counter. Toggle if change detected, otherwise reset to 1
	switch_debounce_ct0 = ~(switch_debounce_ct0 & pinchange);
	// bit 1 of counter.
	switch_debounce_ct1 = switch_debounce_ct0 ^ (switch_debounce_ct1 & pinchange);
	// if counter rolled back to 3 while change was present, write 1 into pinstate to record a debounced change event
	pinchange &= switch_debounce_ct0 & switch_debounce_ct1 & SWITCH_PIN_MASK;
	// change key state
	switch_state ^= pinchange;
	
	// evaluate changes												// 00010100 new state; inverted: 11101011
	uint8_t key_presses = pinchange & switch_state;		// 00011100 pin changes
	uint8_t key_releases = pinchange & ~switch_state;		// presses => 00010100; releases =>  00001000
	
	for (uint8_t i = 0; i < SWITCH_NUMBER; ++i)
	{
		if (key_presses & (1 << i))
		{
			switch_state_press |= (1 << i);
			switch_press_counter[i] = 0;
		}
		else if (key_releases & (1 << i))
		{
			switch_state_release |= (1 << i);			
		}
	
		// switch is held down
		if((switch_state & (1 << i)) && switch_press_counter[i] < SWITCH_LONGPRESS_TICKS)
		{
			if(++switch_press_counter[i] == SWITCH_LONGPRESS_TICKS)
			{
				switch_state_longpress |= (1 << i);
			}
		}
	}
}
