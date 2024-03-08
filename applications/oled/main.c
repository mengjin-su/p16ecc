#include <p16f18857.h>
#include "timer0.h"
#include "i2c.h"
#include "font.h"
#include "oled.h"

#pragma FUSE1	0x3f9c
#pragma FUSE2	0x3fff
#pragma FUSE3	WDTE_00
#pragma FUSE4	0x3fff
#pragma FUSE5	0x3fff

#pragma isr_no_stack
#pragma acc_save 		0

void main()
{
    char c;

    OSCCON1 = 0x70;
    OSCTUNE = 0x00;
//	BORCON = 0x00;
    while( !(OSCSTAT & (1 << 6)) );

	GIE = 1;           // enable interrupt
	PEIE = 1;
	ANSELA = 0;        // all inputs are digital

	TMR0_init();
	I2C_init();

	OLED_init();

	OLED_ShowStr(0, 0, "Hello, world!", 		FONT_SIZE_16x8);
    OLED_ShowStr(0, 7, "diycompiler@gmail.com", FONT_SIZE_8x6);
	for (c = 0;;)
	{
		OLED_travelImg(c);
		if ( ++c > 120 ) c = -128;
	}
}