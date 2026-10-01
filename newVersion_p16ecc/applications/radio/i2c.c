#include "includes_c.h"

#define SCL_PIN		1
#define SDA_PIN		2

#define SDA_HI		TRISA |=  (1 << SDA_PIN)
#define SDA_LO		TRISA &= ~(1 << SDA_PIN)
#define SCL_HI		TRISA |=  (1 << SCL_PIN)
#define SCL_LO		TRISA &= ~(1 << SCL_PIN)
#define I2C_READ	(PORTA & (1 << SDA_PIN))


static char I2C_writeByte(char data);
static char I2C_readByte(void);


void I2C_init(void)
{
	LATA    &= ~((1 << SCL_PIN) | (1 << SDA_PIN));
/*
    SSP1STAT = 0x00;
    SSP1CON1 = 0x08;
    SSP1CON2 = 0x00;
    SSP1ADD  = 0x4F; */
//  SSP1CON1bits.SSPEN = 0;
}

static I2C_delay(void)
{
	WREG = 20;
	do { WREG--; } while ( _Z == 0 );
}

static void I2C_start(char addr)
{
	SCL_LO;	I2C_delay();
	SDA_LO;	I2C_delay();
	I2C_writeByte(addr);
}

static void I2C_end(void)
{
	SDA_LO;	I2C_delay();
	SCL_HI;	I2C_delay();
	SDA_HI;
}

static char I2C_writeByte(char data)
{
	FSR0L = 8;
	do {
		SDA_LO;
		if ( data & 0x80 ) SDA_HI;
		I2C_delay();	SCL_HI;
		I2C_delay();	SCL_LO;
		data <<= 1;
	} while ( --FSR0L );
	SDA_HI;	I2C_delay();
	if ( !I2C_READ ) FSR0L++;
	SDA_LO;	I2C_delay();
	return FSR0L;
}

static char I2C_readByte(void)
{
	SDA_HI;
	FSR0L = 8;
	do {
		FSR0H <<= 1;
		SCL_HI;	I2C_delay();
		if ( I2C_READ ) FSR0H++;
		SCL_LO;	I2C_delay();
	} while ( --FSR0L );
	SDA_LO;
	SCL_HI;	I2C_delay();
	SCL_LO;	I2C_delay();
	SDA_HI;
	return FSR0H;
}

void I2C_writeOp(unsigned char addr, unsigned char data)
{
	I2C_start(addr);
	I2C_writeByte(data);
	I2C_end();
}

char I2C_readOp(unsigned char addr)
{
	I2C_start(addr|1);
	FSR0L = I2C_readByte();
	I2C_end();
	return FSR0L;
}

void I2C_command(unsigned char cmd)	// only for OLED
{
	I2C_writeOp(OLED_CMD_ADDR, cmd);
}

void I2C_data(unsigned char dat)
{
	I2C_writeOp(OLED_DAT_ADDR, dat);
}
