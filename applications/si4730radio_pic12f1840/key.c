#include "includes.h"

enum {KEY_IDLE, KEY_WAIT, KEY_DONE};

static unsigned char keyValue;
static unsigned char keyTimer;
static unsigned char keyState;
static unsigned char keyRead;
static unsigned char keyLast;
static unsigned int  keyElapse;

void KEY_init(void)
{
	WPUEN    = 0;			// global weak pull-up enable
	WPUA    |= KEY_MASK;	// weak pull-up enable for PA0/4/5
	TRISA   |= KEY_MASK;	// TRISA0/4/5 = input
	keyValue = 0;			// clear Key read
	keyState = KEY_IDLE;
}

void KEY_scan(void)			// called from TMR0 interrupt
{
	char key = KEY_READ;
	switch ( keyState )
	{
		case KEY_IDLE:
			if ( ANY_KEY(key) )
			{
				keyRead  = key;
				keyState = KEY_WAIT;
				keyTimer = tmr0Count;
			}
			break;

		case KEY_WAIT:	// deboncing
			if ( keyRead != key )
				keyState = KEY_IDLE;
			else if ( (unsigned char)(tmr0Count - keyTimer) >= 20 )
			{
				if ( KEY1_IN(keyRead) ) keyValue = KEY1;
				if ( KEY2_IN(keyRead) ) keyValue = KEY2;
				if ( KEY3_IN(keyRead) ) keyValue = KEY3;
				keyState = KEY_DONE;
				keyTimer = tmr0Count;
				keyElapse= 600;
				keyLast  = keyValue;
			}
			break;

		case KEY_DONE:	// key depressing
			if ( ANY_KEY(key) )
			{
				keyTimer = tmr0Count;
				if ( keyElapse ) keyElapse--;
				if ( keyElapse == 0 && keyLast != KEY1 )
				{
//				    keyValue = keyLast;
				    keyElapse= 100;

					if ( keyLast == KEY2 )
						keyValue = FAST_INC;
					else
						keyValue = FAST_DEC;
				}
            }
			else if ( (unsigned char)(tmr0Count - keyTimer) >= 20 )
				keyState = KEY_IDLE;
			break;
	}
}

char KEY_read(void)
{
	WREG = keyValue;
	keyValue = 0;
	return WREG;
}