#include <p16f18857.h>
#include "i2c.h"

/*
	128 x 64 OLED driver (I2C interface) ...
*/

#define I2C_DDR_PORT	TRISA
#define I2C_ODR_PORT	LATA
#define I2C_SCL_PIN     1		// PA1
#define I2C_SDA_PIN     0		// PA0
#define SDA_HI			I2C_DDR_PORT |=  (1 << I2C_SDA_PIN)
#define SDA_LO			I2C_DDR_PORT &= ~(1 << I2C_SDA_PIN)
#define SCL_HI			I2C_DDR_PORT |=  (1 << I2C_SCL_PIN)
#define SCL_LO			I2C_DDR_PORT &= ~(1 << I2C_SCL_PIN)

#define I2C_ADDR		0x78

void I2C_init(void)
{
//	I2C_DDR_PORT &= ~((1 << I2C_SCL_PIN) | (1 << I2C_SDA_PIN));
	I2C_ODR_PORT &= ~((1 << I2C_SCL_PIN) | (1 << I2C_SDA_PIN));
	I2C_DDR_PORT |=  ((1 << I2C_SCL_PIN) | (1 << I2C_SDA_PIN));
}

void I2C_delay(void)
{
//	asm("nop"); //asm("nop");
}

void I2C_start(void)
{
	SDA_LO;
	I2C_delay();
	SCL_LO;
	I2C_write(I2C_ADDR);
}

void I2C_end(void)
{
	SDA_LO;
	I2C_delay();
	SCL_HI;
	I2C_delay();
	SDA_HI;
}

void I2C_command(char c)
{
	I2C_start();
	I2C_write(0x00);
	I2C_write(c);
	I2C_end();
}

void I2C_data(char c)
{
	I2C_start();
	I2C_write(0x40);
	I2C_write(c);
	I2C_end();
}

void I2C_dataLength(char *s, unsigned char length)
{
	I2C_start();
	I2C_write(0x40);
	while ( length-- ) I2C_write(*s++);
	I2C_end();
}

void I2C_write(unsigned char c)
{
	char count = 8;
	do {
		if ( c & 0x80 ) SDA_HI;
		else SDA_LO;
		I2C_delay();
		SCL_HI;
		I2C_delay();
		c <<= 1;
		SCL_LO;
	} while ( --count );

	SDA_HI;
	I2C_delay();
	SCL_HI;
	I2C_delay();
	SCL_LO;
}

void I2C_str(char *s, char length)
{
	while ( length ) {
		I2C_data(*s++);
		length--;
	}
}
