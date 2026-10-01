#include "includes_c.h"

enum {KEY_IDLE, KEY_WAIT, KEY_DONE};

#define KEY1_IN(r)	!(r & (1 << 0))		// PA0
#define KEY2_IN(r)	!(r & (1 << 1))		// PA1
#define KEY3_IN(r)	!(r & (1 << 5))		// PA5
#define KEY_MASK	((1 << 0) | (1 << 1) | (1 << 5))
#define KEY_READ	(PORTA & KEY_MASK)
#define ANY_KEY		(KEY_READ != KEY_MASK)

static unsigned char keyValue;
static unsigned char keyTimer;
static unsigned char keyState;
static unsigned char keyRead;
static unsigned char keyTemp;

void KEY_init(void)
{
	TRISA   |= KEY_MASK;	// TRISA0/1/5 = input
	WPUA    |= KEY_MASK;	// weak pull-up enable
	keyValue = 0;
	keyState = KEY_IDLE;
}

void KEY_scan(void)
{
	switch ( keyState )
	{
		case KEY_IDLE:
			if ( ANY_KEY )
			{
				keyRead = KEY_READ;
				if ( KEY1_IN(keyRead) ) keyTemp = KEY1;
				if ( KEY2_IN(keyRead) ) keyTemp = KEY2;
				if ( KEY3_IN(keyRead) ) keyTemp = KEY3;
				keyState = KEY_WAIT;
				keyTimer = tmr0Count;
			}
			break;

		case KEY_WAIT:	// deboncing
			if ( keyRead != KEY_READ )
				keyState = KEY_IDLE;
			else if ( (unsigned char)(tmr0Count - keyTimer) > 4 )
			{
				keyValue = keyTemp;
				keyState = KEY_DONE;
			}
			keyTimer = tmr0Count;
			break;

		case KEY_DONE:	// key depresing
			if ( ANY_KEY )
				keyTimer = tmr0Count;
			else if ( (unsigned char)(tmr0Count - keyTimer) > 4 )
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