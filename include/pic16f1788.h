#ifndef _PIC16F1788_H
#define _PIC16F1788_H

#define __DEVICE   			"pic16f1788"
#include <pic16e_core.h>

#define __FLASH_SIZE        (1024*16)
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

typedef struct {
	unsigned char TMR1IF : 1;
	unsigned char TMR2IF : 1;
	unsigned char CCP1IF : 1;
	unsigned char SSPIF	 : 1;
	unsigned char TXIF	 : 1;
	unsigned char RCIF	 : 1;
	unsigned char ADIF	 : 1;
	unsigned char TMR1GIF: 1;
} PIR1_t;
#define PIR1bits		(PIR1_t*)(&PIR1)

typedef struct {
	unsigned char CCP2IF : 1;
	unsigned char C3IF	 : 1;
	unsigned char C4IF	 : 1;
	unsigned char BCLIF	 : 1;
	unsigned char EEIF	 : 1;
	unsigned char C1IF	 : 1;
	unsigned char C2IF	 : 1;
	unsigned char OSFIF	 : 1;
} PIR2_t;
#define PIR2bits		(PIR2_t*)(&PIR2)

typedef struct {
	unsigned char dummy  : 4;
	unsigned char CCP3IF : 1;
} PIR3_t;
#define PIR3bits		(PIR3_t*)(&PIR3)

typedef struct {
	unsigned char PSMC1SIF: 1;
	unsigned char PSMC2SIF: 1;
	unsigned char PSMC3SIF: 1;
	unsigned char PSMC4SIF: 1;
	unsigned char PSMC1TIF: 1;
	unsigned char PSMC2TIF: 1;
	unsigned char PSMC3TIF: 1;
	unsigned char PSMC4TIF: 1;
} PIR4_t;
#define PIRbits			(PIR4_t*)(&PIR4)

typedef struct {
	unsigned char TMR1ON : 1;
	unsigned char dummy	 : 1;
	unsigned char T1SYNC : 1;
	unsigned char T1OSCEN: 1;
	unsigned char T1CKPS : 2;
	unsigned char TMR1CS : 2;
} T1CON_t;
#define T1CONbits		(T1CON_t*)(&T1CON)

typedef struct {
	unsigned char T1GSS	 : 2;
	unsigned char T1GVAL : 1;
	unsigned char T1GGO	 : 1;
	unsigned char T1GSPM : 1;
	unsigned char T1GTM	 : 1;
	unsigned char T1GPOL : 1;
	unsigned char TMR1GE : 1;
} T1GCON_t;
#define T1GCONbits		(T1GCON_t*)(&T1GCON)

typedef struct {
	unsigned char T2CKPS : 2;
	unsigned char TMR2ON : 1;
	unsigned char T2OUTPS: 4;
} T2CON_t;
#define T2CONTbits		(T2CON_t*)(&T2CON)

typedef struct {
	unsigned char RBIF	: 1;
	unsigned char INTF	: 1;
	unsigned char TMR0IF: 1;
	unsigned char RBIE	: 1;
	unsigned char INTE	: 1;
	unsigned char TMR0IE: 1;
	unsigned char PEIE	: 1;
	unsigned char GIE	: 1;
} INTCON_t;
#define INTCONbits		(INTCON_t*)(&INTCON)

typedef struct {
	unsigned char TMR1IE : 1;
	unsigned char TMR2IE : 1;
	unsigned char CCP1IE : 1;
	unsigned char SSPIE	 : 1;
	unsigned char TXIE	 : 1;
	unsigned char RCIE	 : 1;
	unsigned char ADIE	 : 1;
	unsigned char TMR1GIE: 1;
} PIE_t;
#define PIEbits			(PIE_t*)(&PIE1)

typedef struct {
	unsigned char CCP2IE : 1;
	unsigned char dummy	 : 1;
	unsigned char LCDIE	 : 1;
	unsigned char BCLIE	 : 1;
	unsigned char EEIE	 : 1;
	unsigned char C1IE	 : 1;
	unsigned char C2IE	 : 1;
	unsigned char OSFIE	 : 1;
} PIE2_t;
#define PIE2bits		(PIE2_t*)(&PIE2)

typedef struct {
	unsigned char dummy1 : 1;
	unsigned char TMR4IE : 1;
	unsigned char dummy2 : 1;
	unsigned char TMR6IE : 1;
	unsigned char CCP3IE : 1;
	unsigned char CCP4IE : 1;
	unsigned char CCP5IE : 1;
} PIE3_t;
#define PIE3bits		(PIE3_t*)(&PIE3)

typedef struct {
	unsigned char PS	 : 3;
	unsigned char PSA	 : 1;
	unsigned char T0SE	 : 1;
	unsigned char T0CS	 : 1;
	unsigned char INTEDG : 1;
	unsigned char WPUEN	 : 1;
} OPTION_REG_t;
#define OPTIONbits		(OPTION_REG_t*)(&OPTION_REG)

typedef struct {
	unsigned char BOR	 : 1;
	unsigned char POR	 : 1;
	unsigned char RI	 : 1;
	unsigned char RMCLR	 : 1;
	unsigned char RWDT	 : 1;
	unsigned char dummy	 : 1;
	unsigned char STKUNF : 1;
	unsigned char STKOVF : 1;
} PCON_t;
#define PCONbits		(PCON_t*)(&PCON)

typedef struct {
	unsigned char SWDTEN : 1;
	unsigned char WDTPS	 : 5;
} WDTCON_t;
#define WDTCONbits		(WDTCON_t*)(&WDTCON)

typedef struct {
	unsigned char TUN	 : 6;
} OSCTUNE_t;
#define OSCTUNEbits		(OSCTUNE_t*)(&OSCTUNE)

typedef struct {
	unsigned char SCS	 : 2;
	unsigned char dummy	 : 1;
	unsigned char IRCF	 : 4;
	unsigned char SPLLEN : 1;
} OSCCON_t;
#define OSCCONbits		(OSCCON_t*)(&OSCCON)

typedef struct {
	unsigned char HFIOFS : 1;
	unsigned char LFIOFR : 1;
	unsigned char MFIOFR : 1;
	unsigned char HFIOFL : 1;
	unsigned char HFIOFR : 1;
	unsigned char OSTS	 : 1;
	unsigned char PLLR	 : 1;
	unsigned char T1OSCR : 1;
} OSCSTAT_t;
#define OSCSTATbits		(OSCSTAT_t*)(&OSCSTAT)

typedef struct {
	unsigned char ADON	 : 1;
	unsigned char GO	 : 1;
	unsigned char CHS	 : 5;
	unsigned char ADRMD	 : 1;
} ADCON0_t;
#define ADCON0bits		(ADCON0_t*)(&ADCON0)

typedef struct {
	unsigned char ADPREF : 2;
	unsigned char ADNREF : 1;
	unsigned char dummy	 : 1;
	unsigned char ADCS	 : 3;
	unsigned char ADFM	 : 1;
} ADCON1_t;
#define ADCON1bits		(ADCON1_t*)(&ADCON1)

typedef struct {
	unsigned char CHSN	 : 4;
	unsigned char TRIGSEL: 4;
} ADCON2_t;
#define ADCON2bits		(ADCON2_t*)(&ADCON2)

typedef struct {
	unsigned char SYNC	 : 1;
	unsigned char HYS	 : 1;
	unsigned char SP	 : 1;
	unsigned char ZLF	 : 1;
	unsigned char POL	 : 1;
	unsigned char OE	 : 1;
	unsigned char OUT	 : 1;
	unsigned char ON	 : 1;
} CMCON0_t;
#define CM1CON0bits		(CMCON0_t*)(&CM1CON0)
#define CM2CON0bits		(CMCON0_t*)(&CM2CON0)
#define CM3CON0bits		(CMCON0_t*)(&CM3CON0)
#define CM4CON0bits		(CMCON0_t*)(&CM4CON0)

typedef struct {
	unsigned char NCH	 : 3;
	unsigned char PCH	 : 3;
	unsigned char INTN	 : 1;
	unsigned char INTP	 : 1;
} CMCON_t;
#define CM1CON1bits		(CMCON_t*)(&CM1CON1)
#define CM2CON1bits		(CMCON_t*)(&CM2CON1)
#define CM3CON1bits		(CMCON_t*)(&CM3CON1)
#define CM4CON1bits		(CMCON_t*)(&CM4CON1)

typedef struct {
	unsigned char MC1OUT : 1;
	unsigned char MC2OUT : 1;
	unsigned char MC3OUT : 1;
	unsigned char MC4OUT : 1;
} CMOUT_t;
#define CMOUTbits		(CMOUT_t*)(&CMOUT)

typedef struct {
	unsigned char BORRDY : 1;
	unsigned char dummy	 : 5;
	unsigned char BORFS	 : 1;
	unsigned char SBOREN : 1;
} BORCON_t;
#define BORCONbits		(BORCON_t*)(&BORCON)

typedef struct {
	unsigned char ADFVR	 : 2;
	unsigned char CDAFVR : 2;
	unsigned char CDAFVR1: 1;
	unsigned char TSRNG	 : 1;
	unsigned char TSEN	 : 1;
	unsigned char FVRRDY : 1;
	unsigned char FVREN	 : 1;
} FVRCON_t;
#define FVRCONbits		(FVRCON_t*)(&FVRCON)

typedef struct {
	unsigned char DAC1NSS: 1;
	unsigned char dummy1 : 1;
	unsigned char DAC1PSS: 2;
	unsigned char DAC1OE2: 1;
	unsigned char DAC1OE1: 1;
	unsigned char dummy2 : 1;
	unsigned char DAC1EN : 1;
} DAC1CON0_t;
#define DAC1CON0bits	(DAC1CON0_t*)(&DAC1CON0)

typedef struct {
	unsigned char CCP2SEL: 1;
	unsigned char RXSEL	 : 1;
	unsigned char TXSEL	 : 1;
	unsigned char SDISEL : 1;
	unsigned char SCKSEL : 1;
	unsigned char SDOSEL : 1;
	unsigned char CC1PSEL: 1;
	unsigned char C2OUTSEL:1;
} APFCON1_t;
#define APFCON1bits		(APFCON1_t*)(&APFCON1)

typedef struct {
	unsigned char RD	 : 1;
	unsigned char WR	 : 1;
	unsigned char WREN	 : 1;
	unsigned char WRERR	 : 1;
	unsigned char FREE	 : 1;
	unsigned char LWLO	 : 1;
	unsigned char CFGS	 : 1;
	unsigned char EEPGD	 : 1;
} EECON1_t;
#define EECON1bits		(EECON1_t*)(&EECON1)

typedef struct {
	unsigned char RX9D	 : 1;
	unsigned char OERR	 : 1;
	unsigned char FERR	 : 1;
	unsigned char ADDEN	 : 1;
	unsigned char CREN	 : 1;
	unsigned char SREN	 : 1;
	unsigned char RX9	 : 1;
	unsigned char SPEN	 : 1;
} RCSTA_t;
#define RC1STATbits		(RCSTA_t*)(&RC1STA)

typedef struct {
	unsigned char TX9D	 : 1;
	unsigned char TRMT	 : 1;
	unsigned char BRGH	 : 1;
	unsigned char SENDB	 : 1;
	unsigned char SYNC	 : 1;
	unsigned char TXEN	 : 1;
	unsigned char TX9	 : 1;
	unsigned char CSRC	 : 1;
} TXSTA_t;
#define TX1STAbits		(TXSTA_t*)(&TX1STA)

typedef struct {
	unsigned char BF	 : 1;
	unsigned char UA	 : 1;
	unsigned char RW	 : 1;
	unsigned char S		 : 1;
	unsigned char P		 : 1;
	unsigned char DA	 : 1;
	unsigned char CKE	 : 1;
	unsigned char SMP	 : 1;
} SSPSTAT_t;
#define SSPSTATbits		(SSPSTAT_t*)(&SSPSTAT)

typedef struct {
	unsigned char SSPM	 : 4;
	unsigned char CKP	 : 1;
	unsigned char SSPEN	 : 1;
	unsigned char SSPOV	 : 1;
	unsigned char WCOL	 : 1;
} SSPCON1_t;
#define SSPCON1bits		(SSPCON1_t*)(&SSPCON1)

typedef struct {
	unsigned char SEN	 : 1;
	unsigned char RSEN	 : 1;
	unsigned char PEN	 : 1;
	unsigned char RCEN	 : 1;
	unsigned char ACKEN	 : 1;
	unsigned char ACKDT	 : 1;
	unsigned char ACKSTAT: 1;
	unsigned char GCEN	 : 1;
} SSPCON2_t;
#define SSPCON2bits		(SSPCON2_t*)(&SSPCON2)

typedef struct {
	unsigned char DHEN	 : 1;
	unsigned char AHEN	 : 1;
	unsigned char SBCDE	 : 1;
	unsigned char SDAHT	 : 1;
	unsigned char BOEN	 : 1;
	unsigned char SCIE	 : 1;
	unsigned char PCIE	 : 1;
	unsigned char ACKTIM : 1;
} SSPCON3_t;
#define SSPCON3bits		(SSPCON3_t*)(&SSPCON3)

#define CONFIG0         REG16(0x8007)
#define CONFIG1         REG16(0x8008)
#define FUSE0           0x8007
#define FUSE1           0x8008

#endif

