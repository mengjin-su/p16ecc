#ifndef _P16F1788_H
#define _P16F1788_H

#define __DEVICE    "p16f1788"
#include <pic16e.h>

#define __FLASH_SIZE        1024*16
#define __SRAM_SIZE         2048
#define __STACK_INIT_ADDR 	(0x2000+__SRAM_SIZE-16)

#define PORTA       		REG8(0x0c | _BK0)
#define PORTB				REG8(0x0d | _BK0)
#define PORTC				REG8(0x0e | _BK0)
#define PORTE				REG8(0x10 | _BK0)
#define PIR1				REG8(0x11 | _BK0)
#define PIR2				REG8(0x12 | _BK0)
#define PIR3				REG8(0x13 | _BK0)
#define PIR4				REG8(0x14 | _BK0)
#define TMR0				REG8(0x15 | _BK0)
#define TMR1L				REG8(0x16 | _BK0)
#define TMR1H				REG8(0x17 | _BK0)
#define T1CON				REG8(0x18 | _BK0)
#define T1GCON				REG8(0x19 | _BK0)
#define TMR2				REG8(0x1a | _BK0)
#define PR2					REG8(0x1b | _BK0)
#define T2CON				REG8(0x1c | _BK0)

#define TRISA       		REG8(0x0c | _BK1)
#define TRISB				REG8(0x0d | _BK1)
#define TRISC				REG8(0x0e | _BK1)
#define TRISE				REG8(0x10 | _BK1)
#define PIE1				REG8(0x11 | _BK1)
#define PIE2				REG8(0x12 | _BK1)
#define PIE3				REG8(0x13 | _BK1)
#define PIE4				REG8(0x14 | _BK1)
#define OPTION_REG			REG8(0x15 | _BK1)
#define PCON				REG8(0x16 | _BK1)
#define WDTCON				REG8(0x17 | _BK1)
#define OSCTUNE				REG8(0x18 | _BK1)
#define OSCCON				REG8(0x19 | _BK1)
#define OSCSTAT				REG8(0x1a | _BK1)
#define ADRESL				REG8(0x1b | _BK1)
#define ADRESH				REG8(0x1c | _BK1)
#define ADCON0				REG8(0x1d | _BK1)
#define ADCON1				REG8(0x1e | _BK1)
#define ADCON2				REG8(0x1f | _BK1)
#define ADRES				REG16(0x1b| _BK1)

#define LATA	       		REG8(0x0c | _BK2)
#define LATB				REG8(0x0d | _BK2)
#define LATC				REG8(0x0e | _BK2)
#define LATE				REG8(0x10 | _BK2)
#define CM1CON0				REG8(0x11 | _BK2)
#define CM1CON1				REG8(0x12 | _BK2)
#define CM2CON0				REG8(0x13 | _BK2)
#define CM2CON1				REG8(0x14 | _BK2)
#define CMOUT				REG8(0x15 | _BK2)
#define BORCON				REG8(0x16 | _BK2)
#define FVRCON				REG8(0x17 | _BK2)
#define DAC1CON0			REG8(0x18 | _BK2)
#define DAC1CON1			REG8(0x19 | _BK2)
#define CM4CON0				REG8(0x1a | _BK2)
#define CM4CON1				REG8(0x1b | _BK2)
#define APFCON2				REG8(0x1c | _BK2)
#define APFCON1				REG8(0x1d | _BK2)
#define CM3CON0				REG8(0x1e | _BK2)
#define CM3CON1				REG8(0x1f | _BK2)

#define ANSELA				REG8(0x0c | _BK3)
#define ANSELB				REG8(0x0d | _BK3)
#define ANSELC				REG8(0x0e | _BK3)
#define EEADRL				REG8(0x11 | _BK3)
#define EEADRH				REG8(0x12 | _BK3)
#define EEDATL				REG8(0x13 | _BK3)
#define EEDATH				REG8(0x14 | _BK3)
#define EECON1				REG8(0x15 | _BK3)
#define EECON2				REG8(0x16 | _BK3)
#define VREGCON				REG8(0x17 | _BK3)
#define RC1REG				REG8(0x19 | _BK3)
#define TX1REG				REG8(0x1a | _BK3)
#define SP1BRGL				REG8(0x1b | _BK3)
#define SP1BRGH				REG8(0x1c | _BK3)
#define RC1STA				REG8(0x1d | _BK3)
#define TX1STA				REG8(0x1e | _BK3)
#define BAUD1CON			REG8(0x1f | _BK3)

#define WPUA				REG8(0x0c | _BK4)
#define WPUB				REG8(0x0d | _BK4)
#define WPUC				REG8(0x0e | _BK4)
#define WPUE				REG8(0x10 | _BK4)
#define SSPBUF				REG8(0x11 | _BK4)
#define SSPADD				REG8(0x12 | _BK4)
#define SSPMSK				REG8(0x13 | _BK4)
#define SSPSTAT				REG8(0x14 | _BK4)
#define SSPCON1				REG8(0x15 | _BK4)
#define SSPCON2				REG8(0x16 | _BK4)
#define SSPCON3				REG8(0x17 | _BK4)

#define ODCONA				REG8(0x0c | _BK5)
#define ODCONB				REG8(0x0d | _BK5)
#define ODCONC				REG8(0x0e | _BK5)
#define CCPR1L				REG8(0x11 | _BK5)
#define CCPR1H				REG8(0x12 | _BK5)
#define CCP1CON				REG8(0x13 | _BK5)
#define CCPR2L				REG8(0x18 | _BK5)
#define CCPR2H				REG8(0x19 | _BK5)
#define CCP2CON				REG8(0x1a | _BK5)

#define SLRCONA				REG8(0x0c | _BK6)
#define SLRCONB				REG8(0x0d | _BK6)
#define SLRCONC				REG8(0x0e | _BK6)
#define CCPR3L				REG8(0x11 | _BK6)
#define CCPR3H				REG8(0x12 | _BK6)
#define CCP3CON				REG8(0x13 | _BK6)

#define INLVLA				REG8(0x0c | _BK7)
#define INLVLB				REG8(0x0d | _BK7)
#define INLVLC				REG8(0x0e | _BK7)
#define INLVLE				REG8(0x10 | _BK7)
#define IOCAP				REG8(0x11 | _BK7)
#define IOCAN				REG8(0x12 | _BK7)
#define IOCAF				REG8(0x13 | _BK7)
#define IOCBP				REG8(0x14 | _BK7)
#define IOCBN				REG8(0x15 | _BK7)
#define IOCBF				REG8(0x16 | _BK7)
#define IOCCP				REG8(0x17 | _BK7)
#define IOCCN				REG8(0x18 | _BK7)
#define IOCCF				REG8(0x19 | _BK7)

#define OPA1CON				REG8(0x11 | _BK10)
#define OPA2CON				REG8(0x13 | _BK10)
#define OPA3CON				REG8(0x15 | _BK10)
#define CLKRCON				REG8(0x1a | _BK10)

#define DAC2CON0			REG8(0x11 | _BK11)
#define DAC2CON1			REG8(0x12 | _BK11)
#define DAC3CON0			REG8(0x13 | _BK11)
#define DAC3CON1			REG8(0x14 | _BK11)
#define DAC4CON0			REG8(0x15 | _BK11)
#define DAC4CON1			REG8(0x16 | _BK11)


sbit TMR1GIF			@	(&PIR1 << 3) | 7;
sbit ADIF				@	(&PIR1 << 3) | 6;
sbit RCIF				@	(&PIR1 << 3) | 5;
sbit TXIF				@	(&PIR1 << 3) | 4;
sbit SSPIF				@	(&PIR1 << 3) | 3;
sbit CCP1IF				@	(&PIR1 << 3) | 2;
sbit TMR2IF				@	(&PIR1 << 3) | 1;
sbit TMR1IF				@	(&PIR1 << 3) | 0;

sbit OSFIF				@	(&PIR2 << 3) | 7;
sbit C2IF				@	(&PIR2 << 3) | 6;
sbit C1IF				@	(&PIR2 << 3) | 5;
sbit EEIF				@	(&PIR2 << 3) | 4;
sbit BCLIF				@	(&PIR2 << 3) | 3;
sbit C4IF				@	(&PIR2 << 3) | 2;
sbit C3IF				@	(&PIR2 << 3) | 1;
sbit CCP2IF				@	(&PIR2 << 3) | 0;

sbit CCP3IF				@	(&PIR3 << 3) | 4;

sbit PSMC4TIF			@	(&PIR4 << 3) | 7;
sbit PSMC3TIF			@	(&PIR4 << 3) | 6;
sbit PSMC2TIF			@	(&PIR4 << 3) | 5;
sbit PSMC1TIF			@	(&PIR4 << 3) | 4;
sbit PSMC4SIF			@	(&PIR4 << 3) | 3;
sbit PSMC3SIF			@	(&PIR4 << 3) | 2;
sbit PSMC2SIF			@	(&PIR4 << 3) | 1;
sbit PSMC1SIF			@	(&PIR4 << 3) | 0;

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

sbit RBIF				@ 	(&INTCON << 3) | 0;
sbit INTF				@ 	(&INTCON << 3) | 1;
sbit TMR0IF				@ 	(&INTCON << 3) | 2;
sbit RBIE				@ 	(&INTCON << 3) | 3;
sbit INTE				@ 	(&INTCON << 3) | 4;
sbit TMR0IE				@ 	(&INTCON << 3) | 5;
sbit PEIE				@ 	(&INTCON << 3) | 6;
sbit GIE				@ 	(&INTCON << 3) | 7;

sbit TMR1IE				@	(&PIE1 << 3) | 0;
sbit TMR2IE				@	(&PIE1 << 3) | 1;
sbit CCP1IE				@	(&PIE1 << 3) | 2;
sbit SSPIE				@	(&PIE1 << 3) | 3;
sbit TXIE				@	(&PIE1 << 3) | 4;
sbit RCIE				@	(&PIE1 << 3) | 5;
sbit ADIE				@	(&PIE1 << 3) | 6;
sbit TMR1GIE			@	(&PIE1 << 3) | 7;

sbit CCP2IE				@	(&PIE2 << 3) | 0;
sbit LCDIE				@	(&PIE2 << 3) | 2;
sbit BCLIE				@	(&PIE2 << 3) | 3;
sbit EEIE				@	(&PIE2 << 3) | 4;
sbit C1IE				@	(&PIE2 << 3) | 5;
sbit C2IE				@	(&PIE2 << 3) | 6;
sbit OSFIE				@	(&PIE2 << 3) | 7;

sbit TMR4IE				@	(&PIE3 << 3) | 1;
sbit TMR6IE				@	(&PIE3 << 3) | 3;
sbit CCP3IE				@	(&PIE3 << 3) | 4;
sbit CCP4IE				@	(&PIE3 << 3) | 5;
sbit CCP5IE				@	(&PIE3 << 3) | 6;

sbit PS0				@	(&OPTION_REG << 3) | 0;
sbit PS1				@	(&OPTION_REG << 3) | 1;
sbit PS2				@	(&OPTION_REG << 3) | 2;
sbit PSA				@	(&OPTION_REG << 3) | 3;
sbit T0SE				@	(&OPTION_REG << 3) | 4;
sbit T0CS				@	(&OPTION_REG << 3) | 5;
sbit INTEDG				@	(&OPTION_REG << 3) | 6;
sbit WPUEN				@	(&OPTION_REG << 3) | 7;

sbit BOR				@	(&PCON << 3) | 0;
sbit POR				@	(&PCON << 3) | 1;
sbit RI					@	(&PCON << 3) | 2;
sbit RMCLR				@	(&PCON << 3) | 3;
sbit STKUNF				@	(&PCON << 3) | 6;
sbit STKOVF				@	(&PCON << 3) | 7;

sbit SWDTEN				@	(&WDTCON << 3) | 0;
sbit WDTPS0				@	(&WDTCON << 3) | 1;
sbit WDTPS1				@	(&WDTCON << 3) | 2;
sbit WDTPS2				@	(&WDTCON << 3) | 3;
sbit WDTPS3				@	(&WDTCON << 3) | 4;
sbit WDTPS4				@	(&WDTCON << 3) | 5;

sbit TUN0				@	(&OSCTUNE << 3) | 0;
sbit TUN1				@	(&OSCTUNE << 3) | 1;
sbit TUN2				@	(&OSCTUNE << 3) | 2;
sbit TUN3				@	(&OSCTUNE << 3) | 3;
sbit TUN4				@	(&OSCTUNE << 3) | 4;
sbit TUN5				@	(&OSCTUNE << 3) | 5;

sbit SCS0				@	(&OSCCON << 3) | 0;
sbit SCS1				@	(&OSCCON << 3) | 1;
sbit IRCF0				@	(&OSCCON << 3) | 3;
sbit IRCF1				@	(&OSCCON << 3) | 4;
sbit IRCF2				@	(&OSCCON << 3) | 5;
sbit IRCF3				@	(&OSCCON << 3) | 6;
sbit SPLLEN				@	(&OSCCON << 3) | 7;

sbit HFIOFS				@	(&OSCSTAT << 3) | 0;
sbit LFIOFR				@	(&OSCSTAT << 3) | 1;
sbit MFIOFR				@	(&OSCSTAT << 3) | 2;
sbit HFIOFL				@	(&OSCSTAT << 3) | 3;
sbit HFIOFR				@	(&OSCSTAT << 3) | 4;
sbit OSTS				@	(&OSCSTAT << 3) | 5;
sbit PLLR				@	(&OSCSTAT << 3) | 6;
sbit T1OSCR				@	(&OSCSTAT << 3) | 7;

sbit ADON				@	(&ADCON0 << 3) | 0;
sbit GO					@	(&ADCON0 << 3) | 1;
sbit DONE				@	(&ADCON0 << 3) | 1;
sbit CHS0				@	(&ADCON0 << 3) | 2;
sbit CHS1				@	(&ADCON0 << 3) | 3;
sbit CHS2				@	(&ADCON0 << 3) | 4;
sbit CHS3				@	(&ADCON0 << 3) | 5;
sbit CHS4				@	(&ADCON0 << 3) | 6;
sbit ADRMD				@	(&ADCON0 << 3) | 7;

sbit ADPREF0			@	(&ADCON1 << 3) | 0;
sbit ADPREF1			@	(&ADCON1 << 3) | 1;
sbit ADNREF				@	(&ADCON1 << 3) | 2;
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

sbit C2SYNC				@	(&CM2CON0 << 3);
sbit C2HYS				@	(&CM2CON0 << 3) | 1;
sbit C2SP				@	(&CM2CON0 << 3) | 2;
sbit C2POL				@	(&CM2CON0 << 3) | 4;
sbit C2OE				@	(&CM2CON0 << 3) | 5;
sbit C2OUT				@	(&CM2CON0 << 3) | 6;
sbit C2ON				@	(&CM2CON0 << 3) | 7;

sbit C2NCH0				@	(&CM2CON1 << 3);
sbit C2NCH1				@	(&CM2CON1 << 3) | 1;
sbit C2PCH0				@	(&CM2CON1 << 3) | 4;
sbit C2PCH1				@	(&CM2CON1 << 3) | 5;
sbit C2INTN				@	(&CM2CON1 << 3) | 6;
sbit C2INTP				@	(&CM2CON1 << 3) | 7;

sbit MC1OUT				@	(&CMOUT << 3);
sbit MC2OUT				@	(&CMOUT << 3) | 1;

sbit BORRDY				@	(&BORCON << 3);
sbit SBOREN				@ 	(&BORCON << 3) | 7;

sbit ADFVR0				@	(&FVRCON << 3) | 0;
sbit ADFVR1				@	(&FVRCON << 3) | 1;
sbit CDAFVR0			@	(&FVRCON << 3) | 2;
sbit CDAFVR1			@	(&FVRCON << 3) | 3;
sbit TSRNG				@	(&FVRCON << 3) | 4;
sbit TSEN				@	(&FVRCON << 3) | 5;
sbit FVRRDY				@	(&FVRCON << 3) | 6;
sbit FVREN				@	(&FVRCON << 3) | 7;

sbit DACNSS				@	(&DACCON0 << 3);
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

sbit SRPR				@	(&SRCON0 << 3) | 0;
sbit SRPS				@	(&SRCON0 << 3) | 1;
sbit SRNQEN				@	(&SRCON0 << 3) | 2;
sbit SRQEN				@	(&SRCON0 << 3) | 3;
sbit SRCLK0				@	(&SRCON0 << 3) | 4;
sbit SRCLK1				@	(&SRCON0 << 3) | 5;
sbit SRCLK2				@	(&SRCON0 << 3) | 6;
sbit SRLEN				@	(&SRCON0 << 3) | 7;

sbit SRRC1E				@	(&SRCON1 << 3) | 0;
sbit SRRC2E				@	(&SRCON1 << 3) | 1;
sbit SRRCKE				@	(&SRCON1 << 3) | 2;
sbit SRRPE				@	(&SRCON1 << 3) | 3;
sbit SRSC1E				@	(&SRCON1 << 3) | 4;
sbit SRSC2E				@	(&SRCON1 << 3) | 5;
sbit SRSCKE				@	(&SRCON1 << 3) | 6;
sbit SRSPE				@	(&SRCON1 << 3) | 7;

sbit CCP2SEL			@	(&APFCON << 3) | 0;
sbit SSSEL				@	(&APFCON << 3) | 1;
sbit C2OUTSEL			@	(&APFCON << 3) | 2;
sbit SRNQSEL			@	(&APFCON << 3) | 3;
sbit P2BSEL				@	(&APFCON << 3) | 4;
sbit T1GSEL				@	(&APFCON << 3) | 5;
sbit CCP3SEL			@	(&APFCON << 3) | 6;	// end of BANK2

sbit RD					@	(&EECON1 << 3) | 0;
sbit WR					@	(&EECON1 << 3) | 1;
sbit WREN				@	(&EECON1 << 3) | 2;
sbit WRERR				@	(&EECON1 << 3) | 3;
sbit FREE				@	(&EECON1 << 3) | 4;
sbit LWLO				@	(&EECON1 << 3) | 5;
sbit CFGS				@	(&EECON1 << 3) | 6;
sbit EEPGD				@	(&EECON1 << 3) | 7;

sbit SPEN				@	(&RCSTA << 3) | 7;
sbit RX9				@	(&RCSTA << 3) | 6;
sbit SREN				@	(&RCSTA << 3) | 5;
sbit CREN				@	(&RCSTA << 3) | 4;
sbit ADDEN				@	(&RCSTA << 3) | 3;
sbit FERR				@	(&RCSTA << 3) | 2;
sbit OERR				@	(&RCSTA << 3) | 1;
sbit RX9D				@	(&RCSTA << 3) | 0;

sbit CSRC				@	(&TXSTA << 3) | 7;
sbit TX9				@	(&TXSTA << 3) | 6;
sbit TXEN				@	(&TXSTA << 3) | 5;
sbit SYNC				@	(&TXSTA << 3) | 4;
sbit SENDB				@	(&TXSTA << 3) | 3;
sbit BRGH				@	(&TXSTA << 3) | 2;
sbit TRMT				@	(&TXSTA << 3) | 1;
sbit TX9D				@	(&TXSTA << 3) | 0;

sbit SMP				@	(&SSPSTAT << 3)|7;
sbit CKE				@	(&SSPSTAT << 3)|6;
sbit D_A				@	(&SSPSTAT << 3)|5;
sbit _P					@	(&SSPSTAT << 3)|4;
sbit _S					@	(&SSPSTAT << 3)|3;
sbit R_W				@	(&SSPSTAT << 3)|2;
sbit UA					@	(&SSPSTAT << 3)|1;
sbit BF					@	(&SSPSTAT << 3)|0;

sbit WCOL				@	(&SSPCON1 << 3)|7;
sbit SSPOV				@	(&SSPCON1 << 3)|6;
sbit SSPEN				@	(&SSPCON1 << 3)|5;
sbit CKP				@	(&SSPCON1 << 3)|4;

#define CONFIG0         REG16(0x8007)
#define CONFIG1         REG16(0x8008)
#define FUSE0           0x8007
#define FUSE1           0x8008

#endif

