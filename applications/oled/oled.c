#include "i2c.h"
#include "oled.h"
#include "font.h"
#include "on_img.h"
#include "timer0.h"

#define OLED_HEIGHT		 64
#define OLED_WIDTH		128
#define OLED_BRIGHTNESS	0x1

void OLED_setPos(unsigned char x, unsigned char y)
{
	I2C_command(0xb0+y);
	I2C_command(((x&0xf0)>>4)|0x10);
	I2C_command((x&0x0f)|0x01);
}

void OLED_fill(unsigned char bmp_dat)
{
	unsigned char y, x;

	for (y = 0; y < OLED_HEIGHT/8; y++)
	{
		I2C_command(0xb0+y);
		I2C_command(0x01);
		I2C_command(0x10);

		for (x = 0; x < OLED_WIDTH; x++)
			I2C_data(bmp_dat);
	}
}

/**************************************************
* ????:OLED_CLS()
* ????:OLED??
* ????:
* ????:
* ????:?????
**************************************************/
void OLED_clr(void)
{
	unsigned char y, x;
	for (y = 0; y < OLED_HEIGHT/8; y++)
	{
		I2C_command(0xb0+y);
		I2C_command(0x01);
		I2C_command(0x10);
		for (x = 0; x < OLED_WIDTH; x++)
			I2C_data(0x00);
	}
}


/**************************************************
* ????:OLED_Init()
* ????:OLED???
* ????:
* ????:
* ????:?????
**************************************************/
void OLED_init(void)
{
//	Delay(500);//???????????!
	TMR0_delayMs(200);
	I2C_command(0xAE);//--turn off oled panel ????
	I2C_command(0x00);//---set low column address
	I2C_command(0x10);//---set high column address
	I2C_command(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	I2C_command(0x81);//--set contrast control register
	I2C_command(OLED_BRIGHTNESS); // Set SEG Output Current Brightness ?????
	I2C_command(0xA1);//--Set SEG/Column Mapping     0xA0???? 0xA1??(?????)
	I2C_command(0xC8);//Set COM/Row Scan Direction   0xC0???? 0xC8??
	I2C_command(0xA6);//--set normal display
	I2C_command(0xA8);//--set multiplex ratio(1 to 64)
	I2C_command(0x3F);//--1/64 duty
	I2C_command(0xD3);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	I2C_command(0x00);//-not offset
	I2C_command(0xD5);//--set display clock divide ratio/oscillator frequency
	I2C_command(0x80);//--set divide ratio, Set Clock as 100 Frames/Sec
	I2C_command(0xD9);//--set pre-charge period
	I2C_command(0xF1);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	I2C_command(0xDA);//--set com pins hardware configuration
	I2C_command(0x12);
	I2C_command(0xDB);//--set vcomh
	I2C_command(0x40);//Set VCOM Deselect Level
	I2C_command(0x20);//-Set Page Addressing Mode (0x00/0x01/0x02)
	I2C_command(0x02);//
	I2C_command(0x8D);//--set Charge Pump enable/disable
	I2C_command(0x14);//--set(0x10) disable
	I2C_command(0xA4);// Disable Entire Display On  0xA4?? 0xA5????
	I2C_command(0xA6);// Disable Inverse Display On 0xA6?? 0xA7??
	I2C_command(0xAF);//--turn on oled panel ????

	OLED_fill(0x00); //????
//	OLED_setPos(0,0);
}

/**************************************************
* ????:OLED_Wakeup()
* ????:?OLED??????
* ????:
* ????:
* ????:?????
**************************************************/
void OLED_wakeup(void)
{
	I2C_command(0X8D);  //?????
	I2C_command(0X14);  //?????
	I2C_command(0XAF);  //OLED??
}

/**************************************************
* ????:OLED_Wakeup()
* ????:OLED??,????10uA
* ????:
* ????:
* ????:?????
**************************************************/
void OLED_sleep(void)
{
	I2C_command(0X8D);  //?????
	I2C_command(0X10);  //?????
	I2C_command(0XAE);  //OLED??
}

void OLED_setContrast(char c)
{
	I2C_command(0x81);//--set contrast control register
	I2C_command(c); // Set SEG Output Current Brightness ?????
}

/**************************************************************************
* ????:OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
* ????:OLED??codetab.h??ASCII??,?6*8?8*16???
* ????:x,y -- ?????(x:0~127, y:0~7);
           ch[] -- ???????;
           TextSize -- ????(1:6*8 ; 2:8*16)
* ????:
* ????:?????
**************************************************************************/
void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char *ch, FONT_SIZE size)
{
	while ( *ch )
	{
		unsigned char c = *ch++ - 32;
		unsigned int  pos;
		switch ( size )
		{
			case FONT_SIZE_8x6:
				if ( x > 126 )
				{
					x = 0;
					y++;
				}
				OLED_setPos(x, y);
				I2C_dataLength(FONT_6x8[c], 6);
				x += 6;
				break;

			case FONT_SIZE_16x8:
				if ( x > 120 )
				{
					x = 0;
					y++;
				}
				OLED_setPos(x,y);
				pos = c * 16;

				I2C_dataLength(&FONT_8x16[pos], 8);
				OLED_setPos(x,y+1);
				I2C_dataLength(&FONT_8x16[pos+8], 8);
				x += 8;
				break;
		}
	}
}
#if 0
/**************************************************************************
* ????:OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
* ????:??codetab.h????,16*16??
* ????:x,y -- ?????(x:0~127, y:0~7);
           tab16[] -- ?????
           N -- ???codetab.h????
* ????:
* ????:?????
**************************************************************************/
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char tab16[], unsigned char N)
{
	unsigned char wm = 0;
	unsigned char adder = 32*N;
	OLED_setPos(x, y);
	for(wm = 0;wm < 16;wm++)
	{
		I2C_data(tab16[adder]);
		adder += 1;
	}
	OLED_setPos(x,y+1);
	for(wm = 0;wm < 16;wm++)
	{
		I2C_data(tab16[adder]);
		adder += 1;
	}
}

/**************************************************************************
* ????:OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
* ????:??codetab.h?????,16*16??
* ????:x,y -- ?????(x:0~127, y:0~7);
           tab16[] -- ?????
           len -- ?????
* ????:
* ????:?????
**************************************************************************/
void OLED_ShowCNStr(unsigned char x, unsigned char y, unsigned char tab16[], unsigned char len)
{
  	unsigned char i;
  	OLED_setPos(x, y);
	for (i = 0; i < len; i++)
  	{
  		OLED_ShowCN(22+i*16, y, (unsigned char *)tab16, i);//??????
  	}
}

/**************************************************************************
* ????:OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[])
* ????:??BMP??
* ????:x0,y0 -- ?????(x0:0~127, y0:0~7);
           x1,y1 -- ?????(???)???(x1:1~128,y1:1~8)
           BMP[] -- ????
* ????:
* ????:?????
**************************************************************************/
void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[])
{
	unsigned int j = 0;
	unsigned char x, y;

  	if ( y1%8 == 0 )
		y = y1/8;
  	else
		y = y1/8 + 1;

	for (y = y0; y < y1; y++)
	{
		OLED_setPos(x0, y);
    	for (x = x0; x < x1; x++)
			I2C_data(BMP[j++]);
	}
}

/**************************************************************************
* ????:OLED_DrawAll(unsigned char BMP[])
* ????:????BMP??,??128*64??
* ????:BMP[] -- ????
* ????:
* ????:?????
**************************************************************************/
void OLED_DrawAll(unsigned char BMP[])
{
	unsigned int j=0;
	unsigned char x,y;

	for (y = 0; y < OLED_HEIGHT/8; y++)
  	{
    	OLED_setPos(0, y);
    	for (x = 0; x < OLED_WIDTH; x++)
      		I2C_data(BMP[j++]);
	}
}
#endif

const unsigned char mask8[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

void OLED_printPwrOnImg(void)
{
	unsigned char row, col, i, d, c;
	for (row = 0; row < 32/8; row++)
	{
		OLED_setPos(4, row+2);
		for (col = 0; col < 15*8; col++)
		{
			for (i = 0; i < 8; i++) {
				c = powerOnImage[row*120 + i*15 + col/8];
				d >>= 1;
				if ( c & mask8[col&7] ) d |= 0x80;
			}
			I2C_data(d);
		}
	}
}

void OLED_travelImg(char start_col)
{
	char col;
	unsigned char row, i, d, c;
	for (row = 0; row < 32/8; row++)
	{
		OLED_setPos(4, row+2);
		for (col = 0; col < 15*8; col++)
		{
			d = 0;
			char cc = col + start_col;
			if ( cc >= 0 )
			{
				if ( cc < 120 )
					for (i = 0; i < 8; i++) {
						c = powerOnImage[row*120 + i*15 + cc/8];
						d >>= 1;
						if ( c & mask8[cc&7] ) d |= 0x80;
					}
			}
			I2C_data(d);
		}
	}
}

