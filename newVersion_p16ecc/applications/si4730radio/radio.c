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
	switch ( KEY_read() )
	{
		case MODE_KEY:	// FM/AM switch
		    OLED_clr();
			if ( RADIO_mode == FM_MODE )
				startAM();
			else
				startFM();
			break;

		case INC_KEY:
			if ( RADIO_mode == FM_MODE )
			{
				if ( RADIO_fmFreq < 10850 )
				{
					RADIO_fmFreq += 10;				// inc .1 MHz
					SI47xx_setFreq(RADIO_fmFreq);
					RADIO_dispFreq();
				}
			}
			else
			{
				if ( RADIO_amFreq < 1650 )
				{
					RADIO_amFreq++;				    // inc 1 KHz
					SI47xx_setFreq(RADIO_amFreq);
					RADIO_dispFreq();
				}
			}
			break;
		case DEC_KEY:
			if ( RADIO_mode == FM_MODE )
			{
				if ( RADIO_fmFreq > 8750 )
				{
					RADIO_fmFreq -= 10;				// dec 0.1 MHz
					SI47xx_setFreq(RADIO_fmFreq);
					RADIO_dispFreq();
				}
			}
			else
			{
                if ( RADIO_amFreq > 535 )
				{
					RADIO_amFreq--;                 // dec 1 KHz
					SI47xx_setFreq(RADIO_amFreq);
					RADIO_dispFreq();
				}
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