#ifndef _P16F15214_H
#define _P16F15214_H

#define __DEVICE    "p16f15214"
#include <pic16e.h>

#define __FLASH_SIZE        4096
#define __SRAM_SIZE         512
#define __END_STACK_ADDR    (__SRAM_SIZE-16)

#define PORTA       		REG8(0x0c | _BK0)
#define PORTB       		REG8(0x0d | _BK0)
#define PORTC				REG8(0x0e | _BK0)
#define TRISA       		REG8(0x12 | _BK0)
#define TRISB       		REG8(0x13 | _BK0)
#define TRISC				REG8(0x14 | _BK0)
#define LATA	       		REG8(0x18 | _BK0)
#define LATB	       		REG8(0x19 | _BK0)
#define LATC				REG8(0x1a | _BK0)

#define CPCON	       		REG8(0x1a | _BK1)
#define ADRESL				REG8(0x1b | _BK1)
#define ADRESH				REG8(0x1c | _BK1)
#define ADRES				REG16(0x1b| _BK1)
#define ADCON0				REG8(0x1d | _BK1)
#define ADCON1				REG8(0x1e | _BK1)
#define ADACT				REG8(0x1f | _BK1)

#define RB4I2C				REG8(0x0c | _BK2)
#define RB6I2C				REG8(0x0d | _BK2)
#define RC0I2C				REG8(0x0e | _BK2)
#define RC1I2C				REG8(0x0f | _BK2)
#define RC1REG				REG8(0x19 | _BK2)
#define TX1REG				REG8(0x1a | _BK2)
#define SP1BRGL				REG8(0x1b | _BK2)
#define SP1BRGH				REG8(0x1c | _BK2)
#define SP1BRG				REG16(0x1b| _BK2)
#define RC1STA				REG8(0x1d | _BK2)
#define TX1STA				REG8(0x1e | _BK2)
#define BAUD1CON			REG8(0x1f | _BK2)
sbit OERR				@	(&RC1STA << 3) | 1;
sbit CREN				@	(&RC1STA << 3) | 4;
sbit TX9D				@	(&TX1STA << 3) | 0;
sbit TRMT				@	(&TX1STA << 3) | 1;
sbit TX9				@	(&TX1STA << 3) | 6;

#define SSP1BUF				REG8(0x0c | _BK3)
#define SSP1ADD				REG8(0x0d | _BK3)
#define SSP1MSK				REG8(0x0e | _BK3)
#define SSP1STAT			REG8(0x0f | _BK3)
#define SSP1CON1			REG8(0x10 | _BK3)
#define SSP1CON2			REG8(0x11 | _BK3)
#define SSP1CON3			REG8(0x12 | _BK3)

#define TMR1L				REG8(0x0c | _BK4)
#define TMR1H				REG8(0x0d | _BK4)
#define TMR1				REG16(0x0c| _BK4)
#define T1CON				REG8(0x0e | _BK4)
#define T1GCON				REG8(0x0f | _BK4)
#define T1GATE				REG8(0x10 | _BK4)
#define T1CLK				REG8(0x11 | _BK4)

#define T2TMR				REG8(0x0c | _BK5)
#define T2PR				REG8(0x0d | _BK5)
#define T2CON				REG8(0x0e | _BK5)
#define T2HLT				REG8(0x0f | _BK5)
#define T2CLKCON			REG8(0x10 | _BK5)
#define T2RST				REG8(0x11 | _BK5)

#define CCPR1L				REG8(0x0c | _BK6)
#define CCPR1H				REG8(0x0d | _BK6)
#define CCPR1				REG16(0x0c| _BK6)
#define CCP1CON				REG8(0x0e | _BK6)
#define CCP1CAP				REG8(0x0f | _BK6)
#define CCPR2L				REG8(0x10 | _BK6)
#define CCPR2H				REG8(0x11 | _BK6)
#define CCPR2				REG16(0x10| _BK6)
#define CCP2CON				REG8(0x12 | _BK6)
#define CCP2CAP				REG8(0x13 | _BK6)
#define PWM3DCL				REG8(0x14 | _BK6)
#define PWM3DCH				REG8(0x15 | _BK6)
#define PWM3DC				REG16(0x14| _BK6)
#define PWM3CON				REG8(0x16 | _BK6)
#define PWM4DCL				REG8(0x18 | _BK6)
#define PWM4DCH				REG8(0x19 | _BK6)
#define PWM4DC				REG16(0x18| _BK6)
#define PWM4CON				REG8(0x1a | _BK6)

#define TMR0				REG16(0x1c| _BK11)
#define TMR0L				REG8(0x1c | _BK11)
#define TMR0H				REG8(0x1d | _BK11)
#define T0CON0				REG8(0x1e | _BK11)
#define T0CON1				REG8(0x1f | _BK11)

#define PIR0				REG8(0x0c | _BK14)
#define PIR1				REG8(0x0d | _BK14)
#define PIR2				REG8(0x0e | _BK14)
#define PIE0				REG8(0x16 | _BK14)
#define PIE1				REG8(0x17 | _BK14)
#define PIE2				REG8(0x18 | _BK14)

#define WDTCON				REG8(0x0c | _BK16)
#define BORCON				REG8(0x11 | _BK16)
#define PCON0				REG8(0x13 | _BK16)
#define PCON1				REG8(0x14 | _BK16)
#define NVMADR				REG16(0x1a| _BK16)
#define NVMADRL				REG8(0x1a | _BK16)
#define NVMADRH				REG8(0x1b | _BK16)
#define NVMDAT				REG16(0x1c| _BK16)
#define NVMDATL				REG8(0x1c | _BK16)
#define NVMDATH				REG8(0x1d | _BK16)
#define NVMCON1				REG8(0x1e | _BK16)
#define NVMCON2				REG8(0x1f | _BK16)

#define OSCCON				REG8(0x0e | _BK17)
#define OSCSTAT				REG8(0x10 | _BK17)
#define OSCEN				REG8(0x11 | _BK17)
#define OSCTUNE				REG8(0x12 | _BK17)
#define OSCFRQ				REG8(0x13 | _BK17)

#define FVRCON				REG8(0x0c | _BK18)

#define INTPPS				REG8(0x10 | _BK61)
#define T0CKIPPS			REG8(0x11 | _BK61)
#define T1CKIPPS			REG8(0x12 | _BK61)
#define T1GPPS				REG8(0x13 | _BK61)

#define T2INPPS				REG8(0x1c | _BK61)
#define CCP1PPS				REG8(0x21 | _BK61)
#define CCP2PPS				REG8(0x22 | _BK61)

#define ANSELA				REG8(0x38 | _BK62)
#define WPUA				REG8(0x39 | _BK62)
#define ODCONA				REG8(0x3a | _BK62)
#define SLRCONA				REG8(0x3b | _BK62)
#define INVLA				REG8(0x3c | _BK62)

sbit TMR0IF				@ 	(&PIR0 << 3) | 5;
sbit INTF				@ 	(&PIR0 << 3) | 0;
sbit TMR0IE				@	(&PIE0 << 3) | 5;
sbit INTE				@	(&PIE0 << 3) | 0;

sbit GIE				@ 	(&INTCON << 3) | 7;
sbit PEIE				@	(&INTCON << 3) | 6;
sbit INTEDG				@	(&INTCON << 3) | 0;


#define __FUSE_UNIT_SIZE	14
#define FUSE1				0x8007
	#define _VDDAR_LOW		0x2fff	// bit12	- Vdd = 1.8~3.6v
	#define _CLKOUT_ENA		0x3eff	// bit8		- CLKOUT enabled
	#define _HFINTOSC_1MHZ	0x3fef	// bit5-4	- Watchdog timer enable
	#define _LFINTOSC		0x3fdf	// bit5-4
	#define _HFINTOSC_32MHZ	0x3fc7	// bit5-4
	#define _ECH			0x3fff	// bit1-0
	#define _ECL			0x3ffe	// bit1-0
	#define _OSC_DIS		0x3ffd	// bit1-0
#define FUSE2				0x8008
	#define _DEBUG_ENA		0x1fff	// bit13
	#define _STACK_OVFL_RST	0x2fff	// bit12
	#define _PSS_CHANGEABLE	0x37ff	// bit11
	#define _BORV_2_85		0x3dff	// bit9
	#define _PLL_DISABLE	0x3eff	// bit8
	#define _PLL_4X			0x3f7f	// bit7
	#define _USB_CLK_DIV4	0x3fbf	// bit6
	#define _CPU_DIV_3		0x3fef	// bit5-4
	#define _CPU_DIV_2		0x3fdf	// bit5-4
	#define _CPU_DIV_NONE	0x3fcf	// bit5-4
	#define _FLASH_WP_01FF	0x3ffe	// bit1-0
	#define _FLASH_WP_0FFF	0x3ffd	// bit1-0
	#define _FLASH_WP_1FFF	0x3ffc	// bit1-0
#define FUSE3				0x8009
#define FUSE4				0x800a
#define FUSE5				0x800b


#endif
