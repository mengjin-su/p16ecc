#include <string.h>
#include "includes_c.h"

#pragma isr_no_stack
#pragma acc_save 		1

void main()
{
	OPTION_REG = (OPTION_REG & 0xC0) | (0xD7 & 0x3F);	// 32MHz

	TMR0_init();
	I2C_init();
	OLED_init();
	KEY_init();
	PLAY_init();

	GIE = 1;		// enable interrupt
	for (;;)
	{
		PLAY_poll();
	}
}