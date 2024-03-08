#include "includes.h"

unsigned char tmr0Count = 0;

void TMR0_init(void)
{
	OPTION_REG = (OPTION_REG & 0xC0) | (0xD4 & 0x3F);	// 32MHz

	TMR0IF = 0;			// clear timer0 interrupt
    TMR0IE = 1;			// enable timer0 interrupt
}

void TMR0_delayMs(unsigned char ms)
{
	unsigned char t = tmr0Count;
	while ( (unsigned char)(tmr0Count - t) < ms );
}

interrupt tmr0_isr()
{
	if ( TMR0IF )
	{
		TMR0 = 6;
		TMR0IF = 0;
        tmr0Count++;
	    KEY_scan();
    }
}