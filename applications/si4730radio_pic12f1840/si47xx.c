#include "includes.h"
#define SI47xx_I2C_WR_ADDR		(SI473X_ADDR_SEN_HIGH << 1)
#define SI47xx_I2C_RD_ADDR		(SI47xx_I2C_WR_ADDR   +  1)

// STATUS bits - Used by all methods
#define STCINT  0x01
#define ASQINT  0x02
#define RDSINT  0x04
#define RSQINT  0x08
#define ERR     0x40
#define CTS     0x80
// POWER_UP
//?#define POWER_UP                      0x01
#define POWER_UP_IN_FUNC_FMRX         0x00
#define POWER_UP_IN_FUNC_AMRX         0x01
#define POWER_UP_IN_FUNC_FMTX         0x02
#define POWER_UP_IN_FUNC_WBRX         0x03
#define POWER_UP_IN_FUNC_QUERY        0x0F
#define POWER_UP_IN_PATCH             0x20
#define POWER_UP_IN_GPO2OEN           0x40
#define POWER_UP_IN_CTSIEN            0x80
#define POWER_UP_IN_OPMODE_RX_ANALOG  0x05
#define POWER_UP_IN_OPMODE_TX_ANALOG  0x50

//////////////////////////////////////////////////////////////////////////////
static void SI47xx_write(char *buf, char length)
{
	I2C_start(SI47xx_I2C_WR_ADDR);
	while ( length-- ) { I2C_writeByte(*buf); buf++; }
	I2C_end();
}

void SI47xx_read(char *buf, char length)
{
	I2C_start(SI47xx_I2C_RD_ADDR);
	while ( length-- ) { *buf = I2C_readByte(length); buf++; }
	I2C_end();
}

static void SI47xx_waitCTS(void)
{
	char counter = 100, status;
	while ( counter-- )
	{
		SI47xx_read(&status, 1);
		if ( (status & CTS) ) break;
		TMR0_delayMs(1);
	}
}

static void SI47xx_command(char *wbuf, char wlength)
{
	SI47xx_waitCTS();
	SI47xx_write(wbuf, wlength);
	SI47xx_waitCTS();
}

static void SI47xx_property(unsigned int prop_num, unsigned int prop_val)
{
	char cmd[6];
	cmd[0] = SET_PROPERTY;
	cmd[1] = 0;
	cmd[2] = prop_num >> 8;
	cmd[3] = prop_num;
	cmd[4] = prop_val >> 8;
	cmd[5] = prop_val;
	SI47xx_command(cmd, 6);
}


//////////////////////////////////////////////////////////////////////////////
const char IS47xx_powerUpFM[] = {POWER_UP, POWER_UP_FM|0xd0, POWER_UP_IN_OPMODE_RX_ANALOG};
const char IS47xx_powerUpAM[] = {POWER_UP, POWER_UP_AM|0xd0, POWER_UP_IN_OPMODE_RX_ANALOG};

void SI47xx_init(void)
{
	if ( RADIO_mode == FM_MODE )
		SI47xx_command(IS47xx_powerUpFM, sizeof(IS47xx_powerUpFM));
	else
		SI47xx_command(IS47xx_powerUpAM, sizeof(IS47xx_powerUpFM));
}

void SI47xx_setVolume(unsigned char v)
{
	SI47xx_property(RX_HARD_MUTE, 0);
	SI47xx_property(RX_VOLUME,    v);
}

void SI47xx_setFreq(unsigned int freq)
{
	char cmd[6];

	cmd[1] = 0;
	cmd[2] = freq >> 8;
	cmd[3] = freq;
	cmd[4] = 0;

	if ( RADIO_mode == FM_MODE )
	{
		cmd[0] = FM_TUNE_FREQ;
		SI47xx_command(cmd, 5);
	}
	else
	{
		cmd[0] = AM_TUNE_FREQ;
		cmd[5] = 0;
		SI47xx_command(cmd, 6);
	}
}

