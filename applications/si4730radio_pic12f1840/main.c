#include <string.h>
#include "includes.h"

#pragma isr_no_stack
#pragma acc_save 		1

#pragma FUSE0	_FOSC_INTOSC & _WDT_DIS
#pragma FUSE1	0xffff	// _LVP_HV_ONLY | 0x100

void main()
{
    OSCCON = 0x70;
    OSCTUNE = 0x00;
    BORCON = 0x00;
    while( !(OSCSTAT & (1 << 6)) );
    
	GIE = 1;           // enable interrupt
	ANSELA = 0;        // all inputs are digital

	TMR0_init();
	I2C_init();
	OLED_init();
	KEY_init();
	RADIO_init();

	for (;;)
	{
		RADIO_poll();
	}
}