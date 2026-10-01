#include "includes_c.h"

unsigned char tmr0Count = 0;

void TMR0_init(void)
{
	TMR0CS = 0;
	PSA    = 0;
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
		TMR0IF = 0;
        tmr0Count++;
        KEY_scan();
    }
}