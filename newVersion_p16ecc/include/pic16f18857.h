#ifndef _PIC16F18857_H
#define _PIC16F18857_H

#define __DEVICE    "pic16f18857"
#include <pic16e.h>

#define __FLASH_SIZE        32768
#define __SRAM_SIZE         4096
#define __END_STACK_ADDR    (__SRAM_SIZE-16)
#define __BSR6

#define STATUS_SHAD			REG8(0x64 | _BK63)
#define WREG_SHAD			REG8(0x65 | _BK63)
#define BSR_SHAD			REG8(0x66 | _BK63)
#define PCLATH_SHAD			REG8(0x67 | _BK63)
#define FSR0L_SHAD			REG8(0x68 | _BK63)
#define FSR0H_SHAD			REG8(0x69 | _BK63)
#define FSR1L_SHAD			REG8(0x6a | _BK63)
#define FSR1H_SHAD			REG8(0x6b | _BK63)
#define STKPTR				REG8(0x6d | _BK63)
#define TOSL				REG8(0x6e | _BK63)
#define TOSH				REG8(0x6f | _BK63)

#define PORTA       		REG8(0x0c | _BK0)
#define PORTB       		REG8(0x0d | _BK0)
#define PORTC				REG8(0x0e | _BK0)
#define PORTE       		REG8(0x10 | _BK0)
#define TRISA       		REG8(0x11 | _BK0)
#define TRISB       		REG8(0x12 | _BK0)
#define TRISC				REG8(0x13 | _BK0)
#define LATA	       		REG8(0x16 | _BK0)
#define LATB	       		REG8(0x17 | _BK0)
#define LATC				REG8(0x18 | _BK0)
#define TMR0L				REG8(0x1c | _BK0)
#define TMR0H				REG8(0x1d | _BK0)
#define T0CON0				REG8(0x1e | _BK0)
#define T0CON1				REG8(0x1f | _BK0)

#define ADRESL				REG8(0x0c | _BK1)
#define ADRESH				REG8(0x0d | _BK1)
#define ADPREVL				REG8(0x0e | _BK1)
#define ADPREVH				REG8(0x0f | _BK1)
#define ADACCL				REG8(0x10 | _BK1)
#define ADACCH				REG8(0x11 | _BK1)
#define ADCON0				REG8(0x13 | _BK1)
#define ADCON1				REG8(0x14 | _BK1)
#define ADCON2				REG8(0x15 | _BK1)
#define ADCON3				REG8(0x16 | _BK1)
#define ADSTAT				REG8(0x17 | _BK1)
#define ADCLK				REG8(0x18 | _BK1)
#define ADACT				REG8(0x19 | _BK1)
#define ADREF				REG8(0x1a | _BK1)
#define ADCAP				REG8(0x1b | _BK1)
#define ADPRE				REG8(0x1c | _BK1)
#define ADACQ				REG8(0x1d | _BK1)
#define ADPCH				REG8(0x1e | _BK1)

#define ADCNT				REG8(0x0c | _BK2)
#define ADRPT				REG8(0x0d | _BK2)
#define ADLTHL				REG8(0x0e | _BK2)
#define ADLTHH				REG8(0x0f | _BK2)
#define ADUTHL				REG8(0x10 | _BK2)
#define ADUTHH				REG8(0x11 | _BK2)
#define ADSTPTL				REG8(0x12 | _BK2)
#define ADSTPTH				REG8(0x13 | _BK2)
#define ADFLTRL				REG8(0x14 | _BK2)
#define ADFLTRH				REG8(0x15 | _BK2)
#define ADERRL				REG8(0x16 | _BK2)
#define ADERRH				REG8(0x17 | _BK2)
#define RC1REG				REG8(0x19 | _BK2)
#define TX1REG				REG8(0x1a | _BK2)
#define SP1BRGL 			REG8(0x1b | _BK2)
#define SP1BRGH 			REG8(0x1c | _BK2)
#define RC1STA 				REG8(0x1d | _BK2)
#define TX1STA 				REG8(0x1e | _BK2)
#define BAUD1CON			REG8(0x1f | _BK2)

#define SSP1BUF				REG8(0x0c | _BK3)
#define SSP1ADD				REG8(0x0d | _BK3)
#define SSP1MSK				REG8(0x0e | _BK3)
#define SSP1STAT			REG8(0x0f | _BK3)
#define SSP1CON1			REG8(0x10 | _BK3)
#define SSP1CON2			REG8(0x11 | _BK3)
#define SSP1CON3			REG8(0x12 | _BK3)

#define SSP2BUF				REG8(0x16 | _BK3)
#define SSP2ADD				REG8(0x17 | _BK3)
#define SSP2MSK				REG8(0x18 | _BK3)
#define SSP2STAT			REG8(0x19 | _BK3)
#define SSP2CON1			REG8(0x1a | _BK3)
#define SSP2CON2			REG8(0x1b | _BK3)
#define SSP2CON3			REG8(0x1c | _BK3)

#define TMR1L				REG8(0x0c | _BK4)
#define TMR1H				REG8(0x0d | _BK4)
#define T1CON				REG8(0x0e | _BK4)
#define T1GCON				REG8(0x0f | _BK4)
#define T1GATE				REG8(0x10 | _BK4)
#define T1CLK				REG8(0x11 | _BK4)
#define TMR3L				REG8(0x12 | _BK4)
#define TMR3H				REG8(0x13 | _BK4)
#define T3CON				REG8(0x14 | _BK4)
#define T3GCON				REG8(0x15 | _BK4)
#define T3GATE				REG8(0x16 | _BK4)
#define T3CLK				REG8(0x17 | _BK4)
#define TMR5L				REG8(0x18 | _BK4)
#define TMR5H				REG8(0x19 | _BK4)
#define T5CON				REG8(0x1a | _BK4)
#define T5GCON				REG8(0x1b | _BK4)
#define T5GATE				REG8(0x1c | _BK4)
#define T5CLK				REG8(0x1d | _BK4)
#define CCPTMRS0			REG8(0x1e | _BK4)
#define CCPTMRS1			REG8(0x1e | _BK4)

#define T2TMR				REG8(0x0c | _BK5)
#define T2PR				REG8(0x0d | _BK5)
#define T2CON				REG8(0x0e | _BK5)
#define T2HLT				REG8(0x0f | _BK5)
#define T2CLKCON			REG8(0x10 | _BK5)
#define T2RST				REG8(0x11 | _BK5)
#define T4TMR				REG8(0x12 | _BK5)
#define T4PR				REG8(0x13 | _BK5)
#define T4CON				REG8(0x14 | _BK5)
#define T4HLT				REG8(0x15 | _BK5)
#define T4CLKCON			REG8(0x16 | _BK5)
#define T4RST				REG8(0x17 | _BK5)
#define T6TMR				REG8(0x18 | _BK5)
#define T6PR				REG8(0x19 | _BK5)
#define T6CON				REG8(0x1a | _BK5)
#define T6HLT				REG8(0x1b | _BK5)
#define T6CLKCON			REG8(0x1c | _BK5)
#define T6RST				REG8(0x1d | _BK5)

#define CCPR1L				REG8(0x0c | _BK6)
#define CCPR1H				REG8(0x0d | _BK6)
#define CCP1CON				REG8(0x0e | _BK6)
#define CCP1CAP				REG8(0x0f | _BK6)
#define CCPR2L				REG8(0x10 | _BK6)
#define CCPR2H				REG8(0x11 | _BK6)
#define CCP2CON				REG8(0x12 | _BK6)
#define CCP2CAP				REG8(0x13 | _BK6)
#define CCPR3L				REG8(0x14 | _BK6)
#define CCPR3H				REG8(0x15 | _BK6)
#define CCP3CON				REG8(0x16 | _BK6)
#define CCP3CAP				REG8(0x17 | _BK6)
#define CCPR4L				REG8(0x18 | _BK6)
#define CCPR4H				REG8(0x19 | _BK6)
#define CCP4CON				REG8(0x1a | _BK6)
#define CCP4CAP				REG8(0x1b | _BK6)
#define CCPR5L				REG8(0x1c | _BK6)
#define CCPR5H				REG8(0x1d | _BK6)
#define CCP5CON				REG8(0x1e | _BK6)
#define CCP5CAP				REG8(0x1f | _BK6)

#define PWM6DCL				REG8(0x0c | _BK7)
#define PWM6DCH				REG8(0x0d | _BK7)
#define PWM6COM				REG8(0x0e | _BK7)
#define PWM7DCL				REG8(0x10 | _BK7)
#define PWM7DCH				REG8(0x11 | _BK7)
#define PWM7COM				REG8(0x12 | _BK7)

#define SCANLADRL			REG8(0x0c | _BK8)
#define SCANLADRH			REG8(0x0d | _BK8)
#define SCANHADRL			REG8(0x0e | _BK8)
#define SCANHADRH			REG8(0x0f | _BK8)
#define SCANCON0			REG8(0x10 | _BK8)
#define SCANTRIG			REG8(0x11 | _BK8)
#define CRCDATL				REG8(0x16 | _BK8)
#define CRCDATH				REG8(0x17 | _BK8)
#define CRCACCL				REG8(0x18 | _BK8)
#define CRCACCH				REG8(0x19 | _BK8)
#define CRCSHIFTL			REG8(0x1a | _BK8)
#define CRCSHIFTH			REG8(0x1b | _BK8)
#define CRCXORL				REG8(0x1c | _BK8)
#define CRCXORH				REG8(0x1d | _BK8)
#define CRCCON0				REG8(0x1e | _BK8)
#define CRCCON1				REG8(0x1f | _BK8)

#define PIR0				REG8(0x0c | _BK14)
#define PIR1				REG8(0x0d | _BK14)
#define PIR2				REG8(0x0e | _BK14)
#define PIR3				REG8(0x0f | _BK14)
#define PIR4				REG8(0x10 | _BK14)
#define PIR5				REG8(0x11 | _BK14)
#define PIR6				REG8(0x12 | _BK14)
#define PIR7				REG8(0x13 | _BK14)
#define PIR8				REG8(0x14 | _BK14)
#define PIE0				REG8(0x16 | _BK14)
#define PIE1				REG8(0x17 | _BK14)
#define PIE2				REG8(0x18 | _BK14)
#define PIE3				REG8(0x19 | _BK14)
#define PIE4				REG8(0x1a | _BK14)
#define PIE5				REG8(0x1b | _BK14)
#define PIE6				REG8(0x1c | _BK14)
#define PIE7				REG8(0x1d | _BK14)
#define PIE8				REG8(0x1e | _BK14)

#define CPUDOZE				REG8(0x0c | _BK17)
#define OSCCON1				REG8(0x0d | _BK17)
#define OSCCON2				REG8(0x0e | _BK17)
#define OSCCON3				REG8(0x0f | _BK17)
#define OSCSTAT				REG8(0x10 | _BK17)
#define OSCEN				REG8(0x11 | _BK17)
#define OSCTUNE				REG8(0x12 | _BK17)
#define OSCFRQ				REG8(0x13 | _BK17)

#define PPSLOCK				REG8(0x0f | _BK61)
#define INTPPS				REG8(0x10 | _BK61)
#define T0CKIPPS			REG8(0x11 | _BK61)
#define T1CKIPPS			REG8(0x12 | _BK61)
#define T1GPPS				REG8(0x13 | _BK61)
#define T3CKIPPS			REG8(0x14 | _BK61)
#define T3GPPS				REG8(0x15 | _BK61)
#define T2AINPPS			REG8(0x1c | _BK61)
#define T4AINPPS			REG8(0x1d | _BK61)
#define T6AINPPS			REG8(0x1e | _BK61)
#define CCP1PPS				REG8(0x21 | _BK61)
#define CCP2PPS				REG8(0x22 | _BK61)
#define CCP3PPS				REG8(0x23 | _BK61)
#define CCP4PPS				REG8(0x24 | _BK61)
#define CCP5PPS				REG8(0x25 | _BK61)
#define SMT1WINPPS			REG8(0x29 | _BK61)
#define SMT1SIGPPS			REG8(0x2a | _BK61)
#define SMT2WINPPS			REG8(0x2b | _BK61)
#define SMT2SIGPPS			REG8(0x2c | _BK61)
#define CWG1PPS				REG8(0x31 | _BK61)
#define CWG2PPS				REG8(0x32 | _BK61)
#define CWG3PPS				REG8(0x33 | _BK61)
#define MDCARLPPS			REG8(0x38 | _BK61)
#define MDCARHPPS			REG8(0x39 | _BK61)
#define MDSRCPPS			REG8(0x3a | _BK61)
#define CLC1IN0PPS			REG8(0x3b | _BK61)
#define CLC1IN1PPS			REG8(0x3c | _BK61)
#define CLC1IN2PPS			REG8(0x3d | _BK61)
#define CLC1IN3PPS			REG8(0x3e | _BK61)
#define ADCACTPPS			REG8(0x43 | _BK61)
#define SS1CLKPPS			REG8(0x45 | _BK61)
#define SS2CLKPPS			REG8(0x48 | _BK61)
#define RXPPS				REG8(0x4b | _BK61)
#define TXPPS				REG8(0x4c | _BK61)

#define RA0PPS				REG8(0x10 | _BK62)
#define RA1PPS				REG8(0x11 | _BK62)
#define RA2PPS				REG8(0x12 | _BK62)
#define RA3PPS				REG8(0x13 | _BK62)
#define RA4PPS				REG8(0x14 | _BK62)
#define RA5PPS				REG8(0x15 | _BK62)
#define RA6PPS				REG8(0x16 | _BK62)
#define RA7PPS				REG8(0x17 | _BK62)
#define RB0PPS				REG8(0x18 | _BK62)
#define RB1PPS				REG8(0x19 | _BK62)
#define RB2PPS				REG8(0x1a | _BK62)
#define RB3PPS				REG8(0x1b | _BK62)
#define RB4PPS				REG8(0x1c | _BK62)
#define RB5PPS				REG8(0x1d | _BK62)
#define RB6PPS				REG8(0x1e | _BK62)
#define RB7PPS				REG8(0x1f | _BK62)
#define RC0PPS				REG8(0x20 | _BK62)
#define RC1PPS				REG8(0x21 | _BK62)
#define RC2PPS				REG8(0x22 | _BK62)
#define RC3PPS				REG8(0x23 | _BK62)
#define RC4PPS				REG8(0x24 | _BK62)
#define RC5PPS				REG8(0x25 | _BK62)
#define RC6PPS				REG8(0x26 | _BK62)
#define RC7PPS				REG8(0x27 | _BK62)
#define ANSELA				REG8(0x38 | _BK62)
#define WPUA				REG8(0x39 | _BK62)
#define ODCONA				REG8(0x3a | _BK62)
#define SLRCONA				REG8(0x3b | _BK62)
#define INLVLA				REG8(0x3c | _BK62)
#define IOCAP				REG8(0x3d | _BK62)
#define IOCAN				REG8(0x3e | _BK62)
#define IOCAF				REG8(0x3f | _BK62)
#define ANSELB				REG8(0x43 | _BK62)
#define WPUB				REG8(0x44 | _BK62)
#define ODCONB				REG8(0x45 | _BK62)
#define SLRCONB				REG8(0x46 | _BK62)
#define INLVLB				REG8(0x47 | _BK62)
#define IOCBP				REG8(0x48 | _BK62)
#define IOCBN				REG8(0x49 | _BK62)
#define IOCBF				REG8(0x4a | _BK62)
#define ANSELC				REG8(0x4e | _BK62)
#define WPUC				REG8(0x4f | _BK62)
#define ODCONC				REG8(0x50 | _BK62)
#define SLRCONC				REG8(0x51 | _BK62)
#define INLVLC				REG8(0x52 | _BK62)
#define IOCCP				REG8(0x53 | _BK62)
#define IOCCN				REG8(0x54 | _BK62)
#define IOCCF				REG8(0x55 | _BK62)
#define ANSELE				REG8(0x65 | _BK62)
#define INLVLE				REG8(0x68 | _BK62)
#define IOCEP				REG8(0x69 | _BK62)
#define IOCEN				REG8(0x6a | _BK62)
#define IOCEF				REG8(0x6b | _BK62)

#define TRISAbits		(REG8_t*)(&TRISA)
#define TRISBbits		(REG8_t*)(&TRISB)
#define TRIBCbits		(REG8_t*)(&TRISC)
#define PORTAbits		(REG8_t*)(&PORTA)
#define PORTBbits		(REG8_t*)(&PORTB)
#define PORTCbits		(REG8_t*)(&PORTC)
#define LATAbits		(REG8_t*)(&LATA)
#define LATBbits		(REG8_t*)(&LATB)
#define LATCbits		(REG8_t*)(&LATC)

typedef struct {
	unsigned char INTEDG	: 1;
	unsigned char _			: 5;
	unsigned char PEIE		: 1;
	unsigned char GIE		: 1;
} INTCON_t;
#define INTCONbits		(INTCON_t*)(&INTCON)

typedef struct {
	unsigned char INTF		: 1;
	unsigned char _			: 3;
	unsigned char IOCIF		: 1;
	unsigned char TMR0IF	: 1;
} PIR0_t;
#define PIR0bits		(PIR0_t*)(&PIR0)

typedef struct {
	unsigned char INTE		: 1;
	unsigned char _			: 3;
	unsigned char ICOIE		: 1;
	unsigned char TMR0IE	: 1;
} PIE0_t;
#define PIE0bits		(PIE0_t*)(&PIE0)

typedef struct {
	unsigned char BF		: 1;
	unsigned char UA		: 1;
	unsigned char RW		: 1;
	unsigned char S			: 1;
	unsigned char P			: 1;
	unsigned char DA		: 1;
	unsigned char CKE		: 1;
	unsigned char SMP		: 1;
} SSP1STAT_t;
#define SSP1STATbits	(SSP1STAT_t*)(&SSP1STAT)

typedef struct {
	unsigned char SSPM	: 4;
	unsigned char SKP	: 1;
	unsigned char SSPEN	: 1;
	unsigned char SSPOV	: 1;
	unsigned char WCOL	: 1;
} SSP1CON1_t;
#define SSP1CONbits		(SSP1CON1_t*)(&SSP1CON1)
#define SSP2CONbits		(SSP1CON1_t*)(&SSP2CON1)

typedef struct {
	unsigned char SEN	 : 1;
	unsigned char RSEN	 : 1;
	unsigned char PEN	 : 1;
	unsigned char RCEN	 : 1;
	unsigned char ACKEN	 : 1;
	unsigned char ACKDT	 : 1;
	unsigned char ACKSTAT: 1;
	unsigned char GCEN	 : 1;
} SSP1CON2_t;
#define SSP1CON2bits	(SSP1CON2_t*)(&SSP1CON2)
#define SSP2CON2bits	(SSP1CON2_t*)(&SSP2CON2)

typedef struct {
	unsigned char DHEN	 : 1;
	unsigned char AHEN	 : 1;
	unsigned char SBCDE	 : 1;
	unsigned char SDAHT	 : 1;
	unsigned char BOEN	 : 1;
	unsigned char SCIE	 : 1;
	unsigned char PCIE	 : 1;
	unsigned char ACKTIM : 1;
} SSP1CON3_t;
#define SSP1CON3bits	(SSP1CON3_t*)(&SSP1CON3)
#define SSP2CON3bits	(SSP1CON3_t*)(&SSP2CON3)

typedef struct {
	unsigned char PLLR		: 1;
	unsigned char _			: 1;
	unsigned char ADOR		: 1;
	unsigned char SOR		: 1;
	unsigned char LFOR		: 1;
	unsigned char MFOR 		: 1;
	unsigned char HFOR		: 1;
	unsigned char EXTOR 	: 1;
} OSCSTAT_t;
#define OSCSTATbits		(OSCSTAT_t*)(&OSCSTAT)

typedef struct {
	unsigned char _		: 2;
	unsigned char ADOEN	: 1;
	unsigned char SOSCEN: 1;
	unsigned char LFOEN	: 1;
	unsigned char MFOEN	: 1;
	unsigned char HFOEN	: 1;
	unsigned char EXTOEN: 1;
} OSCEN_t;
#define OSCENbits		(OSCEN_t*)(&OSCEN)

typedef struct {
	unsigned char HFFRQ: 3;
} OSCFRQ_t;
#define OSCFRQbits		(OSCFRQ_t*)(&OSCFRQ)

typedef struct {
	unsigned char T0OUTPS: 4;
	unsigned char T016BIT: 1;
	unsigned char T0OUT  : 1;
	unsigned char _		 : 1;
	unsigned char T0EN	 : 1;
} T0CON0_t;
#define T0CON0bits		(T0CON0_t*)(&T0CON0)

typedef struct {
	unsigned char T0CKPS : 4;
	unsigned char T0ASYNC: 1;
	unsigned char T0CS 	 : 3;
} T0CON1_t;
#define T0CON1bits		(T0CON1_t*)(&T0CON1)

///////////////////////////////////////////////////////////////
#define FUSE1          		(0x8007)
	#define 				FCMEM_OFF		0x1fff
	#define					CSWEN_OFF		0x2fff
	#define					CLKOUTEN_ON		0x3eff
	#define 				RSTOSC2			0x3fbf
	#define 				RSTOSC_EXT1X	0x3fff
	#define 				RSTOSC_HFINT1	0x3fef
	#define 				RSTOSC_LFINT	0x3fdf
	#define 				RSTOSC_SOSC		0x3fcf
	#define 				RSTOSC_EXT4X	0x3faf
	#define 				RSTOSC_HFINTPLL	0x3f9f
	#define 				RSTOSC_HFINT32	0x3f8f
	#define					FEXTOSC_ECH		0x3fff
	#define					FEXTOSC_ECM		0x3ffe
	#define					FEXTOSC_ECL		0x3ffd
	#define					FEXTOSC_OFF		0x3ffc
	#define					FEXTOSC_HS		0x3ffa
	#define					FEXTOSC_XT		0x3ff9
	#define					FEXTOSC_LP		0x3ff8
#define FUSE2          		(0x8008)
	#define					DEBUG_ON		0x1fff
	#define					STVREN_OFF		0x2fff
	#define					PPS1WAY_OFF		0x37ff
	#define					ZCDDIS_OFF		((~(1 << 10)) & 0x3fff)
	#define					BOREN_ON		0x3fff
	#define 				BOREN_SLEEP		0x3fbf
	#define 				BOREN_SBOREN	0x3f7f
	#define 				BOREN_OFF		0x3f3f
	#define					PWRTE_ON		0x3ffd
	#define 				MCLRE_OFF		0x3ffe
#define FUSE3          		(0x8009)
	#define					WDTCCS_SW		0x3fff
	#define					WDTCCS_001		0x0fff
	#define					WDTCCS_000		0x07ff
	#define 				WDTCWS_111		0x3fff
	#define 				WDTCWS_110		((6 << 8) | 0x38ff)
	#define 				WDTCWS_101		((5 << 8) | 0x38ff)
	#define 				WDTCWS_100		((4 << 8) | 0x38ff)
	#define 				WDTCWS_011		((3 << 8) | 0x38ff)
	#define 				WDTCWS_010		((2 << 8) | 0x38ff)
	#define 				WDTCWS_001		((1 << 8) | 0x38ff)
	#define 				WDTCWS_000		0x38ff
	#define					WDTE_11			0x3fff
	#define					WDTE_10			0x3fdf
	#define					WDTE_01			0x3fbf
	#define					WDTE_00			0x3f9f
#define FUSE4          		(0x800a)
	#define					LVP_OFF			0x1fff
	#define 				SCANE_OFF		0x2fff
	#define 				WRT_OFF			0x3fff
	#define					WRT_10			0x3ffe
	#define					WRT_01			0x3ffd
	#define					WRT_00			0x3ffc
#define FUSE5          		(0x800b)
	#define					CPD_EN			0x3ffd
	#define					CP_EN			0x3ffe

#endif
