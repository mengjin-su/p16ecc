#include <p16f18857.h>

unsigned char tmr0Count;

void TMR0_init(void)
{
	T0CON0 = 0x80 | 0x2f;		// 8-bit timer mode
	T0CON1 = (3 << 5) | 0x2;
	TMR0IF = 0;
	TMR0IE = 1;
}

interrupt tmr0_isr() // interrupt every 1ms
{
	if ( TMR0IF )
	{
		TMR0L  = 68;
		TMR0IF = 0;
        tmr0Count++;
    }
}

void TMR0_delayMs(unsigned char ms)
{
	unsigned char t = tmr0Count;
	while ( (unsigned char)(tmr0Count - t) < ms );
}