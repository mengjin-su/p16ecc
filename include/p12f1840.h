#ifndef _P12F1840_H
#define _P12F1840_H

#define __DEVICE    "p12f1840"
#include <pic16e.h>

#define __FLASH_SIZE        4096
#define __SRAM_SIZE         256
#define __END_STACK_ADDR    (__SRAM_SIZE-16)

#define STATUS_SHAD			REG8(0x64 | _BK31)
#define WREG_SHAD			REG8(0x65 | _BK31)
#define BSR_SHAD			REG8(0x66 | _BK31)
#define PCLATH_SHAD			REG8(0x67 | _BK31)
#define FSR0L_SHAD			REG8(0x68 | _BK31)
#define FSR0H_SHAD			REG8(0x69 | _BK31)
#define FSR1L_SHAD			REG8(0x6a | _BK31)
#define FSR1H_SHAD			REG8(0x6b | _BK31)
#define STKPTR				REG8(0x6d | _BK31)
#define TOSL				REG8(0x6e | _BK31)
#define TOSH				REG8(0x6f | _BK31)

#define PORTA       		REG8(0x0c | _BK0)
#define PIR1				REG8(0x11 | _BK0)
#define PIR2				REG8(0x12 | _BK0)
#define TMR0				REG8(0x15 | _BK0)
#define TMR1L				REG8(0x16 | _BK0)
#define TMR1H				REG8(0x17 | _BK0)
#define TMR1				REG16(0x16 | _BK0)
#define T1CON				REG8(0x18 | _BK0)
#define T1GCON				REG8(0x19 | _BK0)
#define TMR2				REG8(0x1a | _BK0)
#define PR2					REG8(0x1b | _BK0)
#define T2CON				REG8(0x1c | _BK0)

#define TRISA       		REG8(0x0c | _BK1)
#define PIE1				REG8(0x11 | _BK1)
#define PIE2				REG8(0x12 | _BK1)
#define OPTION_REG			REG8(0x15 | _BK1)
#define PCON				REG8(0x16 | _BK1)
#define WDTCON				REG8(0x17 | _BK1)
#define OSCTUNE				REG8(0x18 | _BK1)
#define OSCCON				REG8(0x19 | _BK1)
#define OSCSTAT				REG8(0x1a | _BK1)
#define ADRESL				REG8(0x1b | _BK1)
#define ADRESH				REG8(0x1c | _BK1)
#define ADRES				REG16(0x1b | _BK1)
#define ADCON0				REG8(0x1d | _BK1)
#define ADCON1				REG8(0x1e | _BK1)

#define LATA	       		REG8(0x0c | _BK2)
#define CM1CON0				REG8(0x11 | _BK2)
#define CM1CON1				REG8(0x12 | _BK2)
#define CMOUT				REG8(0x15 | _BK2)
#define BORCON				REG8(0x16 | _BK2)
#define FVRCON				REG8(0x17 | _BK2)
#define DACCON0				REG8(0x18 | _BK2)
#define DACCON1				REG8(0x19 | _BK2)
#define SRCON0				REG8(0x1a | _BK2)
#define SRCON1				REG8(0x1b | _BK2)
#define APFCON				REG8(0x1d | _BK2)

#define ANSELA				REG8(0x0c | _BK3)
#define EEADRL				REG8(0x11 | _BK3)
#define EEADRH				REG8(0x12 | _BK3)
#define EEDATL				REG8(0x13 | _BK3)
#define EEDATH				REG8(0x14 | _BK3)
#define EECON1				REG8(0x15 | _BK3)
#define EECON2				REG8(0x16 | _BK3)
#define VREGCON				REG8(0x17 | _BK3)
#define RCREG				REG8(0x19 | _BK3)
#define TXREG				REG8(0x1a | _BK3)
#define SPBRGL				REG8(0x1b | _BK3)
#define SPBRGH				REG8(0x1c | _BK3)
#define RCSTA				REG8(0x1d | _BK3)
#define TXSTA				REG8(0x1e | _BK3)
#define BAUDCON				REG8(0x1f | _BK3)
sbit OERR				@	(&RCSTA << 3) | 1;
sbit CREN				@	(&RCSTA << 3) | 4;
sbit TRMT				@	(&TXSTA << 3) | 1;
sbit BRGH				@	(&TXSTA << 3) | 2;
sbit TX9				@	(&TXSTA << 3) | 6;
sbit TX9D				@	(&TXSTA << 3) | 0;

#define WPUA				REG8(0x0c | _BK4)
#define SSP1BUF				REG8(0x11 | _BK4)
#define SSP1ADD				REG8(0x12 | _BK4)
#define SSP1MSK				REG8(0x13 | _BK4)
#define SSP1STAT			REG8(0x14 | _BK4)
#define SSP1CON1			REG8(0x15 | _BK4)
#define SSP1CON2			REG8(0x16 | _BK4)
#define SSP1CON3			REG8(0x17 | _BK4)

#define CCPR1L				REG8(0x11 | _BK5)
#define CCPR1H				REG8(0x12 | _BK5)
#define CCP1CON				REG8(0x13 | _BK5)
#define PWM1CON				REG8(0x14 | _BK5)
#define CCP1AS				REG8(0x15 | _BK5)
#define PSRT1CON			REG8(0x16 | _BK5)

#define IOCAP				REG8(0x11 | _BK7)
#define IOCAN				REG8(0x12 | _BK7)
#define IOCAF				REG8(0x13 | _BK7)
#define CLKRCON				REG8(0x1a | _BK7)
#define MDCON				REG8(0x1c | _BK7)
#define MDSRC				REG8(0x1d | _BK7)
#define MDCARL				REG8(0x1e | _BK7)
#define MDCARH				REG8(0x1f | _BK7)

sbit TMR1GIF			@	(&PIR1 << 3) | 7;
sbit ADIF				@	(&PIR1 << 3) | 6;
sbit RCIF				@	(&PIR1 << 3) | 5;
sbit TXIF				@	(&PIR1 << 3) | 4;
sbit SSP1IF				@	(&PIR1 << 3) | 3;
sbit CCP1IF				@	(&PIR1 << 3) | 2;
sbit TMR2IF				@	(&PIR1 << 3) | 1;
sbit TMR1IF				@	(&PIR1 << 3) | 0;

sbit OSFIF				@	(&PIR2 << 3) | 7;
sbit C1IF				@	(&PIR2 << 3) | 5;
sbit EEIF				@	(&PIR2 << 3) | 4;
sbit BCLIF				@	(&PIR2 << 3) | 3;

sbit TMR1CS1			@	(&T1CON << 3) | 7;
sbit TMR1CS0			@	(&T1CON << 3) | 6;
sbit T1CKPS1			@	(&T1CON << 3) | 5;
sbit T1CKPS0			@	(&T1CON << 3) | 4;
sbit T1OSCEN			@	(&T1CON << 3) | 3;
sbit T1SYNC				@	(&T1CON << 3) | 2;
sbit TMR1ON				@	(&T1CON << 3) | 0;

sbit T2OUTPS3			@	(&T2CON << 3) | 6;
sbit T2OUTPS2			@	(&T2CON << 3) | 5;
sbit T2OUTPS1			@	(&T2CON << 3) | 4;
sbit T2OUTPS0			@	(&T2CON << 3) | 3;
sbit TMR2ON				@	(&T2CON << 3) | 2;
sbit T2CKPS1			@	(&T2CON << 3) | 1;
sbit T2CKPS0			@	(&T2CON << 3) | 0;

sbit IOCIF				@ 	(&INTCON << 3) | 0;
sbit INTF				@ 	(&INTCON << 3) | 1;
sbit TMR0IF				@ 	(&INTCON << 3) | 2;
sbit IOCIE				@ 	(&INTCON << 3) | 3;
sbit INTE				@ 	(&INTCON << 3) | 4;
sbit TMR0IE				@ 	(&INTCON << 3) | 5;
sbit PEIE				@ 	(&INTCON << 3) | 6;
sbit GIE				@ 	(&INTCON << 3) | 7;

sbit TMR1IE				@	(&PIE1 << 3) | 0;
sbit TMR2IE				@	(&PIE1 << 3) | 1;
sbit CCP1IE				@	(&PIE1 << 3) | 2;
sbit SSP1IE				@	(&PIE1 << 3) | 3;
sbit TXIE				@	(&PIE1 << 3) | 4;
sbit RCIE				@	(&PIE1 << 3) | 5;
sbit ADIE				@	(&PIE1 << 3) | 6;
sbit TMR1GIE			@	(&PIE1 << 3) | 7;

sbit BCLIE				@	(&PIE2 << 3) | 3;
sbit EEIE				@	(&PIE2 << 3) | 4;
sbit C1IE				@	(&PIE2 << 3) | 5;
sbit OSFIE				@	(&PIE2 << 3) | 7;

sbit PS0				@	(&OPTION_REG << 3) | 0;
sbit PS1				@	(&OPTION_REG << 3) | 1;
sbit PS2				@	(&OPTION_REG << 3) | 2;
sbit PSA				@	(&OPTION_REG << 3) | 3;
sbit TMR0SE				@	(&OPTION_REG << 3) | 4;
sbit TMR0CS				@	(&OPTION_REG << 3) | 5;
sbit INTEDG				@	(&OPTION_REG << 3) | 6;
sbit WPUEN				@	(&OPTION_REG << 3) | 7;

sbit BOR				@	(&PCON << 3) | 0;
sbit POR				@	(&PCON << 3) | 1;
sbit RI					@	(&PCON << 3) | 2;
sbit RMCLR				@	(&PCON << 3) | 3;
sbit RWDT				@	(&PCON << 3) | 4;
sbit STKUNF				@	(&PCON << 3) | 6;
sbit STKOVF				@	(&PCON << 3) | 7;

sbit SWDTEN				@	(&WDTCON << 3) | 0;
sbit WDTPS0				@	(&WDTCON << 3) | 1;
sbit WDTPS1				@	(&WDTCON << 3) | 2;
sbit WDTPS2				@	(&WDTCON << 3) | 3;
sbit WDTPS3				@	(&WDTCON << 3) | 4;
sbit WDTPS4				@	(&WDTCON << 3) | 5;

sbit SCS0				@	(&OSCCON << 3) | 0;
sbit SCS1				@	(&OSCCON << 3) | 1;
sbit IRCF0				@	(&OSCCON << 3) | 2;
sbit IRCF1				@	(&OSCCON << 3) | 3;
sbit IRCF2				@	(&OSCCON << 3) | 4;
sbit IRCF3				@	(&OSCCON << 3) | 5;
sbit SPLLMULT			@	(&OSCCON << 3) | 6;
sbit SPLLEN				@	(&OSCCON << 3) | 7;

sbit HFIOFS				@	(&OSCSTAT << 3) | 0;
sbit LFIOFR				@	(&OSCSTAT << 3) | 1;
sbit MFIOFR				@	(&OSCSTAT << 3) | 2;
sbit HFIOFL				@	(&OSCSTAT << 3) | 3;
sbit HFIOFR				@	(&OSCSTAT << 3) | 4;
sbit OSTS				@	(&OSCSTAT << 3) | 5;
sbit PLLRDY				@	(&OSCSTAT << 3) | 6;
sbit T1OSCR				@	(&OSCSTAT << 3) | 7;

sbit ADON				@	(&ADCON0 << 3) | 0;
sbit GO					@	(&ADCON0 << 3) | 1;
sbit DONE				@	(&ADCON0 << 3) | 1;
sbit CHS0				@	(&ADCON0 << 3) | 2;
sbit CHS1				@	(&ADCON0 << 3) | 3;
sbit CHS2				@	(&ADCON0 << 3) | 4;
sbit CHS3				@	(&ADCON0 << 3) | 5;
sbit CHS4				@	(&ADCON0 << 3) | 6;

sbit ADPREF0			@	(&ADCON1 << 3) | 0;
sbit ADPREF1			@	(&ADCON1 << 3) | 1;
sbit ADCS0				@	(&ADCON1 << 3) | 4;
sbit ADCS1				@	(&ADCON1 << 3) | 5;
sbit ADCS2				@	(&ADCON1 << 3) | 6;
sbit ADFM				@	(&ADCON1 << 3) | 7;

sbit C1SYNC				@	(&CM1CON0 << 3);
sbit C1HYS				@	(&CM1CON0 << 3) | 1;
sbit C1SP				@	(&CM1CON0 << 3) | 2;
sbit C1POL				@	(&CM1CON0 << 3) | 4;
sbit C1OE				@	(&CM1CON0 << 3) | 5;
sbit C1OUT				@	(&CM1CON0 << 3) | 6;
sbit C1ON				@	(&CM1CON0 << 3) | 7;

sbit C1NCH0				@	(&CM1CON1 << 3);
sbit C1NCH1				@	(&CM1CON1 << 3) | 1;
sbit C1PCH0				@	(&CM1CON1 << 3) | 4;
sbit C1PCH1				@	(&CM1CON1 << 3) | 5;
sbit C1INTN				@	(&CM1CON1 << 3) | 6;
sbit C1INTP				@	(&CM1CON1 << 3) | 7;

sbit MC1OUT				@	(&CMOUT << 3);

sbit BORRDY				@	(&BORCON << 3);
sbit BORFS				@ 	(&BORCON << 3) | 6;
sbit SBOREN				@ 	(&BORCON << 3) | 7;

sbit ADFVR0				@	(&FVRCON << 3) | 0;
sbit ADFVR1				@	(&FVRCON << 3) | 1;
sbit CDAFVR0			@	(&FVRCON << 3) | 2;
sbit CDAFVR1			@	(&FVRCON << 3) | 3;
sbit TSRNG				@	(&FVRCON << 3) | 4;
sbit TSEN				@	(&FVRCON << 3) | 5;
sbit FVRRDY				@	(&FVRCON << 3) | 6;
sbit FVREN				@	(&FVRCON << 3) | 7;

sbit DACPSS0			@	(&DACCON0 << 3) | 2;
sbit DACPSS1			@	(&DACCON0 << 3) | 3;
sbit DACOE				@	(&DACCON0 << 3) | 5;
sbit DACLPS				@	(&DACCON0 << 3) | 6;
sbit DACEN				@	(&DACCON0 << 3) | 7;

sbit DACR0				@	(&DACCON1 << 3) | 0;
sbit DACR1				@	(&DACCON1 << 3) | 1;
sbit DACR2				@	(&DACCON1 << 3) | 2;
sbit DACR3				@	(&DACCON1 << 3) | 3;
sbit DACR4				@	(&DACCON1 << 3) | 4;

sbit WCOL				@	(&SSP1CON1 << 3) | 7;
sbit SSPOV				@	(&SSP1CON1 << 3) | 6;
sbit SSPEN				@	(&SSP1CON1 << 3) | 5;
sbit CKP				@	(&SSP1CON1 << 3) | 4;

sbit GCEN				@	(&SSP1CON2 << 3) | 7;
sbit ACKSTAT			@	(&SSP1CON2 << 3) | 6;
sbit ACKDT				@	(&SSP1CON2 << 3) | 5;
sbit ACKEN				@	(&SSP1CON2 << 3) | 4;
sbit RCEN				@	(&SSP1CON2 << 3) | 3;
sbit PEN				@	(&SSP1CON2 << 3) | 2;
sbit RSEN				@	(&SSP1CON2 << 3) | 1;
sbit SEN				@	(&SSP1CON2 << 3) | 0;

sbit ACKTIM				@	(&SSP1CON3 << 3) | 7;
sbit PCIE				@	(&SSP1CON3 << 3) | 6;
sbit SCIE				@	(&SSP1CON3 << 3) | 5;
sbit BOEN				@	(&SSP1CON3 << 3) | 4;
sbit SDAHT				@	(&SSP1CON3 << 3) | 3;
sbit SBCDE				@	(&SSP1CON3 << 3) | 2;
sbit AHEN				@	(&SSP1CON3 << 3) | 1;
sbit DHEN				@	(&SSP1CON3 << 3) | 0;

sbit SMP				@	(&SSP1STAT << 3) | 7;
sbit CKE				@	(&SSP1STAT << 3) | 6;
sbit D_A				@	(&SSP1STAT << 3) | 5;
sbit _P					@	(&SSP1STAT << 3) | 4;
sbit _S					@	(&SSP1STAT << 3) | 3;
sbit R_W				@	(&SSP1STAT << 3) | 2;
sbit UA					@	(&SSP1STAT << 3) | 1;
sbit BF					@	(&SSP1STAT << 3) | 0;

sbit PWM1EN				@	(&PWM1CON << 3) | 7;
sbit PWM1OE				@	(&PWM1CON << 3) | 6;
sbit PWM1OUT			@	(&PWM1CON << 3) | 5;
sbit PWM1POL			@	(&PWM1CON << 3) | 4;

sbit P2SEL				@	(&APFCON << 3) | 2;
sbit T1GSEL				@	(&APFCON << 3) | 3;
sbit SSSEL				@	(&APFCON << 3) | 5;
sbit SDOSEL				@	(&APFCON << 3) | 6;
sbit CLKRSEL			@	(&APFCON << 3) | 7;

#define FUSE0          (0x8007)
	#define _FCMEN_ENA		0x3fff	// bit13	- Fail-safe clock monitor enable
	#define _FCMEN_DIS		0x1fff	// bit13	- Fail-safe clock monitor disable
	#define _IESO_ENA		0x3fff	// bit12	- Int/Ext swichover enable
	#define _IESO_DIS		0x2fff	// bit12	- Int/Ext swichover disable
	#define _CLKOUTEN_ENA	0x37ff	// bit11 	- Clock out enable
	#define _BOR_DIS		0x39ff	// bit10-9	- Brown-out reset enable
	#define _BOR_SBOREN		0x3bff	// bit10-9
	#define _BOR_SLEEP		0x3dff	// bit10-9
	#define _CP_ENA			0x3f7f	// bit7		- Code Protection
	#define _MCLR_DIS		0x3fbf	// bit6		- MCLR/VPP pin function select
	#define _PWRT_ENA		0x3fdf	// bit5		- Power-up timer enable
	#define _WDT_ENA_RUN	0x3ff7	// bit4-3	- Watchdog timer enable
	#define _WDT_SWDTEN		0x3fef	// bit4-3
	#define _WDT_DIS		0x3fe7	// bit4-3
	#define _FOSC_ECH		0x3fff	// bit2-0	- Oscillator selection
	#define _FOSC_ECM		0x3ffe	// bit2-0
	#define _FOSC_ECL		0x3ffd	// bit2-0
	#define _FOSC_INTOSC	0x3ffc	// bit2-0
	#define _FOSC_EXTRC		0x3ffb	// bit2-0
	#define _FOSC_HS		0x3ffa	// bit2-0
	#define _FOSC_XT		0x3ff9	// bit2-0
	#define _FOSC_LP		0x3ff8	// bit2-0
#define FUSE1          (0x8008)
	#define _LVP_HV_ONLY	0x1fff	// bit13
	#define _DEBUG_ENA		0x2fff	// bit12
	#define _LPBOR_ENA		0x37ff	// bit11
	#define _BORV_H_TRIP	0x3bff	// bit10
	#define _STVREN_ENA		0x3dff	// bit9
	#define _STVREN_DIS		0x3fff	// bit9
	#define _PLL_DISABLE	0x3eff	// bit8
	#define _FLASH_WP_01FF	0x3ffe	// bit1-0
	#define _FLASH_WP_0FFF	0x3ffd	// bit1-0
	#define _FLASH_WP_1FFF	0x3ffc	// bit1-0

#endif
