#include "includes_c.h"

enum {FM_MODE=0, AM_MODE};

char playMode;
int  amFreq;
int  fmFreq;

void PLAY_init(void)
{
	playMode = FM_MODE;
	OLED_displayChar_8x16(0, 0, CHAR_F_8x16);
}

void PLAY_poll(void)
{
	switch ( KEY_read() )
	{
		case MODE_KEY:
			playMode ^= 1;
			break;

		case INC_KEY:
			break;

		case DEC_KEY:
			break;
	}
}