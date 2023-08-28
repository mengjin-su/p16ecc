#ifndef _PIC16F1825_H
#define _PIC16F1825_H

#define __DEVICE    "p16f1825"
#include <pic16e.h>

#define __FLASH_SIZE        1024*8
#define __SRAM_SIZE         256*4
#define __END_STACK_ADDR    (__SRAM_SIZE-16)

#define PORTA       		REG8(0x0c | _BK0)
#define PORTC				REG8(0x0e | _BK0)
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
#define CPSCON0				REG8(0x1e | _BK0)
#define CPSCON1				REG8(0x1f | _BK0)

#define TRISA       		REG8(0x0c | _BK1)
#define TRISC				REG8(0x0e | _BK1)
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
#define LATC				REG8(0x0e | _BK2)
#define CM1CON0				REG8(0x11 | _BK2)
#define CM1CON1				REG8(0x12 | _BK2)
#define CM2CON0				REG8(0x13 | _BK2)
#define CM2CON1				REG8(0x14 | _BK2)
#define CMOUT				REG8(0x15 | _BK2)
#define BORCON				REG8(0x16 | _BK2)
#define FVRCON				REG8(0x17 | _BK2)
#define DACCON0				REG8(0x18 | _BK2)
#define DACCON1				REG8(0x19 | _BK2)
#define SRCON0				REG8(0x1a | _BK2)
#define SRCON1				REG8(0x1b | _BK2)
#define APFCON0				REG8(0x1d | _BK2)
#define APFCON1				REG8(0x1e | _BK2)

#define ANSELA				REG8(0x0c | _BK3)
#define ANSELC				REG8(0x0e | _BK3)
#define EEADRL				REG8(0x11 | _BK3)
#define EEADRH				REG8(0x12 | _BK3)
#define EEADR				REG16(0x11 | _BK3)
#define EEDATL				REG8(0x13 | _BK3)
#define EEDATH				REG8(0x14 | _BK3)
#define EEDAT				REG16(0x13 | _BK3)
#define EECON1				REG8(0x15 | _BK3)
#define EECON2				REG8(0x16 | _BK3)
#define RCREG				REG8(0x19 | _BK3)
#define TXREG				REG8(0x1a | _BK3)
#define SPBRGL				REG8(0x1b | _BK3)
#define SPBRGH				REG8(0x1c | _BK3)
#define RCSTA				REG8(0x1d | _BK3)
#define TXSTA				REG8(0x1e | _BK3)
#define BAUDCON				REG8(0x1f | _BK3)

#define WPUA				REG8(0x0c | _BK4)
#define WPUB				REG8(0x0d | _BK4)
#define WPUC				REG8(0x0e | _BK4)
#define SSP1BUF				REG8(0x11 | _BK4)
#define SSP1ADD				REG8(0x12 | _BK4)
#define SSP1MSK				REG8(0x13 | _BK4)
#define SSP1STAT			REG8(0x14 | _BK4)
#define SSP1CON1			REG8(0x15 | _BK4)
#define SSP1CON2			REG8(0x16 | _BK4)
#define SSP1CON3			REG8(0x17 | _BK4)
#define SSP2BUF				REG8(0x19 | _BK4)
#define SSP2ADD				REG8(0x1a | _BK4)
#define SSP2MSK				REG8(0x1b | _BK4)
#define SSP2STAT			REG8(0x1c | _BK4)
#define SSP2CON1			REG8(0x1d | _BK4)
#define SSP2CON2			REG8(0x1e | _BK4)
#define SSP2CON3			REG8(0x1f | _BK4)

#define CCPR1L				REG8(0x11 | _BK5)
#define CCPR1H				REG8(0x12 | _BK5)
#define CCPR1				REG16(0x11 | _BK5)
#define CCP1CON				REG8(0x13 | _BK5)
#define PWM1CON				REG8(0x14 | _BK5)
#define CCP1AS				REG8(0x15 | _BK5)
#define PSTR1CON			REG8(0x16 | _BK5)
#define CCPR2L				REG8(0x18 | _BK5)
#define CCPR2H				REG8(0x19 | _BK5)
#define CCPR2				REG16(0x18 | _BK5)
#define CCP2CON				REG8(0x1a | _BK5)
#define PWM2CON				REG8(0x1b | _BK5)
#define CCP2AS				REG8(0x1c | _BK5)
#define PSTR2CON			REG8(0x1d | _BK5)
#define CCPTMRS0			REG8(0x1e | _BK5)
#define CCPTMRS1			REG8(0x1f | _BK5)

#define CCPR3L				REG8(0x11 | _BK6)
#define CCPR3H				REG8(0x12 | _BK6)
#define CCPR3				REG16(0x11 | _BK6)
#define CCP3CON				REG8(0x13 | _BK6)
#define PWM3CON				REG8(0x14 | _BK6)
#define CCP3AS				REG8(0x15 | _BK6)
#define PSTR3CON			REG8(0x16 | _BK6)
#define CCPR4L				REG8(0x18 | _BK6)
#define CCPR4H				REG8(0x19 | _BK6)
#define CCPR4				REG16(0x18 | _BK6)
#define CCP4CON				REG8(0x1a | _BK6)
#define CCPR5L				REG8(0x1c | _BK6)
#define CCPR5H				REG8(0x1d | _BK6)
#define CCPR5				REG16(0x1c | _BK6)
#define CCP5CON				REG8(0x1e | _BK6)

#define INLVLA       		REG8(0x0c | _BK7)
#define INLVLC				REG8(0x0e | _BK7)
#define IOCBP				REG8(0x14 | _BK7)
#define IOCBN				REG8(0x15 | _BK7)
#define IOCBF				REG8(0x16 | _BK7)

#define TMR4				REG8(0x15 | _BK8)
#define PR4					REG8(0x16 | _BK8)
#define T4CON				REG8(0x17 | _BK8)
#define TMR6				REG8(0x1c | _BK8)
#define PR6					REG8(0x1d | _BK8)
#define T6CON				REG8(0x1e | _BK8)

#define LCDCON				REG8(0x11 | _BK15)
#define LCDPS				REG8(0x12 | _BK15)
#define LCDREF				REG8(0x13 | _BK15)
#define LCDCST				REG8(0x14 | _BK15)
#define LCDRL				REG8(0x15 | _BK15)
#define LCDSE0				REG8(0x18 | _BK15)
#define LCDSE1				REG8(0x19 | _BK15)
#define LCDDATA0			REG8(0x20 | _BK15)
#define LCDDATA1			REG8(0x21 | _BK15)
#define LCDDATA3			REG8(0x23 | _BK15)
#define LCDDATA4			REG8(0x24 | _BK15)
#define LCDDATA5			REG8(0x26 | _BK15)
#define LCDDATA6			REG8(0x27 | _BK15)
#define LCDDATA7			REG8(0x29 | _BK15)
#define LCDDATA8			REG8(0x2a | _BK15)


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
sbit LCDIF				@	(&PIR2 << 3) | 2;
sbit CCP2IF				@	(&PIR2 << 3) | 0;

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

sbit CCP2IE				@	(&PIE2 << 3) | 0;
sbit LCDIE				@	(&PIE2 << 3) | 2;
sbit BCL1IE				@	(&PIE2 << 3) | 3;
sbit EEIE				@	(&PIE2 << 3) | 4;
sbit C1IE				@	(&PIE2 << 3) | 5;
sbit C2IE				@	(&PIE2 << 3) | 6;
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

sbit CCP2SEL			@	(&APFCON1 << 3) | 0;
sbit P2BEL				@	(&APFCON1 << 3) | 1;
sbit P1CSEL				@	(&APFCON1 << 3) | 2;
sbit P1DSEL				@	(&APFCON1 << 3) | 3;
sbit SS2SEL				@	(&APFCON1 << 3) | 4;
sbit SDO2SEL			@	(&APFCON1 << 3) | 5;

sbit TXCKSEL			@	(&APFCON0 << 3) | 2;
sbit T1GSEL				@	(&APFCON0 << 3) | 3;
sbit SSSEL				@	(&APFCON0 << 3) | 5;
sbit SDOSEL				@	(&APFCON0 << 3) | 6;
sbit RXDTSEL			@	(&APFCON0 << 3) | 7;

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

sbit ABDOVF				@	(&BAUDCON << 3)| 7;
sbit RCIDL				@	(&BAUDCON << 3)| 6;
sbit SCKP				@	(&BAUDCON << 3)| 4;
sbit BRG16				@	(&BAUDCON << 3)| 3;
sbit WUE				@	(&BAUDCON << 3)| 1;
sbit ADBEN				@	(&BAUDCON << 3)| 0;

sbit CSRC				@	(&TXSTA << 3) | 7;
sbit TX9				@	(&TXSTA << 3) | 6;
sbit TXEN				@	(&TXSTA << 3) | 5;
sbit SYNC				@	(&TXSTA << 3) | 4;
sbit SENDB				@	(&TXSTA << 3) | 3;
sbit BRGH				@	(&TXSTA << 3) | 2;
sbit TRMT				@	(&TXSTA << 3) | 1;
sbit TX9D				@	(&TXSTA << 3) | 0;

#define FUSE0         	0x8007
#define FUSE1         	0x8008

#endif
