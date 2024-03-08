#include "includes.h"
#include <string.h>

char RADIO_mode;
unsigned char RADIO_volume;
unsigned int  RADIO_fmFreq = 9990;	// 10KHz -> 99.9MHz
unsigned int  RADIO_amFreq = 550;	// 1KHz  -> 550KHz

void RADIO_dispFreq(void);


void RADIO_init(void)
{
	LATA  &= ~(1 << 5);
	TRISA &= ~(1 << 5);     // RST = 0
    TMR0_delayMs(100);
    TRISA |= (1 << 5);      // RST = 1
    TMR0_delayMs(100);

	startFM();
	startFM();
}

void startFM(void)
{
    while ( MODE_KEY_HOLD ) TMR0_delayMs(5);
	KEY_read();		// clear up key buffer

	RADIO_mode = FM_MODE;
	SI47xx_init();
	SI47xx_setVolume(RADIO_volume = 63);
	SI47xx_setFreq(RADIO_fmFreq);

    RADIO_dispFreq();

	OLED_displayChar_16( 7-2, 0, verdana_16x11ptBitmaps_F, 11);
	OLED_displayChar_16(20-2, 0, verdana_16x16ptBitmaps_M, 16);

    OLED_displayChar_8x6_str(12-2, 3, "MHz");
}

void startAM(void)
{
    while ( MODE_KEY_HOLD ) TMR0_delayMs(5);
	KEY_read();		// clear up key buffer

	RADIO_mode = AM_MODE;
	SI47xx_init();
	SI47xx_setVolume(RADIO_volume = 63);
	SI47xx_setFreq(RADIO_amFreq);

	RADIO_dispFreq();

	OLED_displayChar_16( 2-2, 0, verdana_16x16ptBitmaps_A, 16);
	OLED_displayChar_16(20-2, 0, verdana_16x16ptBitmaps_M, 16);

    OLED_displayChar_8x6_str(12-2, 3, "KHz");
}

void RADIO_poll(void)
{
	unsigned char key = KEY_read();
	unsigned char delta;

	switch ( key )
	{
		case MODE_KEY:	// FM/AM switch
		    OLED_clr();
			if ( RADIO_mode == FM_MODE )
				startAM();
			else
				startFM();
			break;

		case INC_KEY:
		case FAST_INC:
			if ( RADIO_mode == FM_MODE )
			{
				delta = RADIO_fmFreq % 50;
				if ( key == FAST_INC )
					RADIO_fmFreq += 50 - delta;	// inc 0.5 MHz
				else
					RADIO_fmFreq += 10;			// inc .1 MHz

				if ( RADIO_fmFreq > 10850 )
					RADIO_fmFreq = 10850;

				SI47xx_setFreq(RADIO_fmFreq);
				RADIO_dispFreq();
			}
			else
			{
				delta = RADIO_amFreq % 10;
				if ( key == FAST_INC )
				{
					RADIO_amFreq += 10 - delta;	// inc 10KHz
				}
				else
					RADIO_amFreq++;		    	// inc 1 KHz

				if ( RADIO_amFreq > 1650 )
					RADIO_amFreq = 1650;

				SI47xx_setFreq(RADIO_amFreq);
				RADIO_dispFreq();
			}
			break;
		case DEC_KEY:
		case FAST_DEC:
			if ( RADIO_mode == FM_MODE )
			{
				delta = RADIO_fmFreq % 50;

				if ( key == FAST_DEC )
					RADIO_fmFreq -= delta? delta: 50;	// dec 0.5 MHz
				else
					RADIO_fmFreq -= 10;			// dec 0.1 MHz

				if ( RADIO_fmFreq < 8750 )
					RADIO_fmFreq = 8750;

				SI47xx_setFreq(RADIO_fmFreq);
				RADIO_dispFreq();
			}
			else
			{
				delta = RADIO_amFreq % 10;

				if ( key == FAST_DEC )
					RADIO_amFreq -= delta? delta: 10;	// dec 10 KHz
				else
					RADIO_amFreq--;				// dec 1 KHz

                if ( RADIO_amFreq < 535 )
                	RADIO_amFreq = 535;

				SI47xx_setFreq(RADIO_amFreq);
				RADIO_dispFreq();
			}
			break;
	}
}

void RADIO_dispFreq(void)
{
    if ( RADIO_mode == FM_MODE )
    {
        unsigned int  freq  = RADIO_fmFreq;
        unsigned char freq0 = freq/10000;   freq %= 10000;
        unsigned char freq1 = freq/1000;    freq %= 1000;
        unsigned char freq2 = freq/100;     freq %= 100;
        unsigned char freq3 = freq/10;

		OLED_displayChar_16(106, 2, &arial_16x11ptBitmaps[10*22],    11);
		OLED_displayChar_16(117, 2, &arial_16x11ptBitmaps[freq3*22], 11);

		if ( freq0 )
			OLED_displayChar_32x19(46, &arialNarrow_32x19_Bitmaps[(4*19)]);
		else
			OLED_displayChar_32x19(46, arialNarrow_32x19_Blank);

		OLED_displayChar_32x19(66, &arialNarrow_32x19_Bitmaps[freq1*(4*19)]);
		OLED_displayChar_32x19(86, &arialNarrow_32x19_Bitmaps[freq2*(4*19)]);
    }
    else
    {
        unsigned int  freq  = RADIO_amFreq;
        unsigned char freq0 = freq/1000;   freq %= 1000;
        unsigned char freq1 = freq/100;    freq %= 100;
        unsigned char freq2 = freq/10;     freq %= 10;

        if ( freq0 )
            OLED_displayChar_32x19(46, &arialNarrow_32x19_Bitmaps[(4*19)]);
        else
            OLED_displayChar_32x19(46, arialNarrow_32x19_Blank);

        OLED_displayChar_32x19( 66, &arialNarrow_32x19_Bitmaps[freq1*(4*19)]);
        OLED_displayChar_32x19( 86, &arialNarrow_32x19_Bitmaps[freq2*(4*19)]);
        OLED_displayChar_32x19(106, &arialNarrow_32x19_Bitmaps[freq *(4*19)]);
    }
}