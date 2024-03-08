#include "includes.h"

// 0.91" OLED size = 132 x 32
#define OLED_HEIGHT		 32
#define OLED_WIDTH		128

static const unsigned char oledInitCmdList[] = {
	0xae, 0x00, 0x10, 0x40, 0xb0, 0x81, 0xff, 0xa1, 0xa6, 0xa8, 0x1f, 0xc8,
	0xd3, 0x00, 0xd5, 0x80, 0xd9, 0xf1, 0xda, 0x02, 0xdb, 0x40, 0x8d, 0x14,
	0x20, 0x02, 0xa4, 0xa6, 0x2e,
	0xaf
};

void OLED_command(unsigned char cmd);
void OLED_data(unsigned char *dat, unsigned char length);

void OLED_setPos(unsigned char x, unsigned char y)
{
	OLED_command(0xb0+y);
	OLED_command((x >> 4)|0x10);
	OLED_command((x&0x0f)|0x01);
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
	unsigned char y, x, c = 0;
	for (y = 0; y < OLED_HEIGHT/8; y++)
	{
		OLED_command(0xb0+y);
		OLED_command(0x01);
		OLED_command(0x10);

		for (x = 0; x < OLED_WIDTH; x++)
			OLED_data(&c, 1);
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
	unsigned char i;

	TMR0_delayMs(100);

   	for (i = 0; i < sizeof(oledInitCmdList); i++)
		OLED_command(oledInitCmdList[i]);

	TMR0_delayMs(100);

	OLED_clr();
//	OLED_setPos(0,0);
}

void OLED_command(unsigned char cmd)
{
	I2C_start(SSD1306_IIC_ADDR);
	I2C_writeByte(SSD1306_IIC_CMD_MASK);
	I2C_writeByte(cmd);
	I2C_end();
}

void OLED_data(unsigned char *dat, unsigned char length)
{
	I2C_start(SSD1306_IIC_ADDR);
	I2C_writeByte(SSD1306_IIC_DAT_MASK);
	while ( length-- ) I2C_writeByte(*dat++);
	I2C_end();
}
#if 0
/**************************************************
* ????:OLED_Wakeup()
* ????:?OLED??????
* ????:
* ????:
* ????:?????
**************************************************/
void OLED_wakeup(void)
{
	OLED_command(0X8D);  //?????
	OLED_command(0X14);  //?????
	OLED_command(0XAF);  //OLED??
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
	OLED_command(0X8D);  //?????
	OLED_command(0X10);  //?????
	OLED_command(0XAE);  //OLED??
}

void OLED_setContrast(char c)
{
	OLED_command(0x81);	//--set contrast control register
	OLED_command(c); 	// Set SEG Output Current Brightness ?????
}

void OLED_displayChar_8x6(unsigned char x, unsigned char y, unsigned char *fp)
{
	OLED_setPos(x, y);
	OLED_data(fp, 6);
}
#endif
void OLED_displayChar_8x6_str(unsigned char x, unsigned char y, unsigned char *sp)
{
	while ( *sp )
	{
        OLED_setPos(x, y);
	    OLED_data(FONT_6x8[*sp++ - 32], 6);
	    x += 6;
	}    
}

void OLED_displayChar_32x19(unsigned char x, unsigned char *fp)
{
	unsigned char i, y;
	for (y = 0; y < 4;  y++)
	{
		OLED_setPos(x, y);
		OLED_data(fp, 19);
		fp += 19;
	}
}

void OLED_displayChar_32x10(unsigned char x, unsigned char *fp)
{
	unsigned char i, y;
	for (y = 0; y < 4;  y++)
	{
		OLED_setPos(x, y);
		OLED_data(fp, 10);
		fp += 10;
	}
}

void OLED_displayChar_16(unsigned char x, unsigned char y, unsigned char *fp, unsigned char width)
{
	unsigned char row;
	for (row = 0; row < 2; row++)
	{
		OLED_setPos(x, y+row);
		OLED_data(fp, width);
		fp += width;
	}
}
