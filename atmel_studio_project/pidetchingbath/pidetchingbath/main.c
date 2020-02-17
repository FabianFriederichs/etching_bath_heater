/*
 * TinyTest.c
 *
 * Created: 25.07.2019 08:36:50
 * Author : fabia
 */ 
#include "application.h"

int main(void)
{
	ErrorCode res = app_run();
	if(res) // if error occured, block forever, keeping the error message on the display
	{
		while(TRUE) {};
	}
}