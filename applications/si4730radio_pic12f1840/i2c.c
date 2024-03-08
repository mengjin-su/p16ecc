#include "includes.h"

void I2C_init(void)
{
	TRISA |= ((1 << SCL_PIN) | (1 << SDA_PIN));
	LATA &= ~((1 << SCL_PIN) | (1 << SDA_PIN));
}

static void I2C_delay(void)
{
	char n = 2;
	while ( --n );
}

void I2C_start(unsigned char addr)
{
	SDA_LO;	asm(" nop"); asm(" nop"); asm(" nop"); asm(" nop");
	SCL_LO;	I2C_delay();
	I2C_writeByte(addr);
}

void I2C_end(void)
{
	SDA_LO; I2C_delay();
	SCL_HI;	asm(" nop"); asm(" nop"); asm(" nop"); asm(" nop");
	SDA_HI;	I2C_delay();
}

char I2C_writeByte(unsigned char data)
{
	unsigned char cnt = 8;
	do {
		if ( data & 0x80 ) SDA_HI;
		else			   SDA_LO;
		I2C_delay();	SCL_HI;
		I2C_delay();	SCL_LO;
		data <<= 1;
	} while ( --cnt );

	SDA_HI;	I2C_delay();
	SCL_HI;	I2C_delay();
	if ( I2C_READ ) cnt++;
	SCL_LO;
	return cnt;
}

char I2C_readByte(char more_bytes)
{
	unsigned char cnt = 8;
	unsigned char data;
	do {
		I2C_delay();
		SCL_HI;
		I2C_delay();
		data <<= 1;
		if ( I2C_READ ) data++;
		SCL_LO;
	} while ( --cnt );

	if ( more_bytes )
	{
		SDA_LO;	I2C_delay();
		SCL_HI;	I2C_delay();
		SCL_LO;	SDA_HI;
	}
	return data;
}
