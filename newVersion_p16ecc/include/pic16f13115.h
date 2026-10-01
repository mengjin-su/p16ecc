#ifndef _P16F15214_H
#define _P16F15214_H

#define __DEVICE    "pic16f13115"
#include <pic16e.h>

#define __FLASH_SIZE        8192
#define __SRAM_SIZE         1024
#define __END_STACK_ADDR    (__SRAM_SIZE-16)
#define __BSR6				1					// extend PIC16F (6-bit BSR)

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
#define TRISA       		REG8(0x12 | _BK0)
#define LATA	       		REG8(0x18 | _BK0)

#define PIR0				REG8(0x0c | _BK1)
#define PIR1				REG8(0x0d | _BK1)
#define PIR2				REG8(0x0e | _BK1)
#define PIR3				REG8(0x0f | _BK1)
#define PIR4				REG8(0x10 | _BK1)
#define PIR5				REG8(0x11 | _BK1)
#define PIR6				REG8(0x12 | _BK1)
#define PIR7				REG8(0x13 | _BK1)
#define PIE0				REG8(0x16 | _BK1)
#define PIE1				REG8(0x17 | _BK1)
#define PIE2				REG8(0x18 | _BK1)
#define PIE3				REG8(0x19 | _BK1)
#define PIE4				REG8(0x1a | _BK1)
#define PIE5				REG8(0x1b | _BK1)
#define PIE6				REG8(0x1c | _BK1)
#define PIE7				REG8(0x1d | _BK1)

#define PMD0				REG8(0x0c | _BK2)
#define PMD1				REG8(0x0d | _BK2)
#define PMD2				REG8(0x0e | _BK2)
#define PMD3				REG8(0x0f | _BK2)
#define PMD4				REG8(0x10 | _BK2)
#define PMD5				REG8(0x11 | _BK2)

#define WDTCON0				REG8(0x0c | _BK3)
#define WDTCON1				REG8(0x0d | _BK3)
#define WDTPSL				REG8(0x0e | _BK3)
#define WDTPSH				REG8(0x0f | _BK3)
#define WDTTMR				REG8(0x10 | _BK3)
#define BORCON				REG8(0x11 | _BK3)
#define PCON0				REG8(0x12 | _BK3)
#define PCON1				REG8(0x13 | _BK3)
#define TMR0L				REG8(0x1c | _BK3)
#define TMR0H				REG8(0x1d | _BK3)
#define T0CON0				REG8(0x1e | _BK3)
#define T0CON1				REG8(0x1f | _BK3)

#define FVRCON				REG8(0x0c | _BK4)
#define CPCON				REG8(0x0d | _BK4)

#define CPUDOZE				REG8(0x0c | _BK5)
#define OSCCON1				REG8(0x0d | _BK5)
#define OSCCON2				REG8(0x0e | _BK5)
#define OSCCON3				REG8(0x0f | _BK5)
#define OSCSTAT				REG8(0x10 | _BK5)
#define OSCEN				REG8(0x11 | _BK5)
#define OSCTUNE				REG8(0x12 | _BK5)
#define OSCFRQ				REG8(0x13 | _BK5)

#define TMR1L				REG8(0x0c | _BK6)
#define TMR1H				REG8(0x0d | _BK6)
#define TMR1				REG16(0x0c| _BK6)
#define T1CON				REG8(0x0e | _BK6)
#define T1GCON				REG8(0x0f | _BK6)
#define T1GATE				REG8(0x10 | _BK6)
#define T1CLK				REG8(0x11 | _BK6)

#define T2TMR2				REG8(0x0c | _BK7)
#define T2PR				REG8(0x0d | _BK7)
#define T2CON				REG8(0x0e | _BK7)
#define T2HLT				REG8(0x0f | _BK7)
#define T2CLK				REG8(0x10 | _BK7)
#define T2RST				REG8(0x11 | _BK7)

#define CCPR1L				REG8(0x0c | _BK8)
#define CCPR1H				REG8(0x0d | _BK8)
#define CCP1CON				REG8(0x0e | _BK8)
#define CCP1CAP				REG8(0x0f | _BK8)
#define CCPR2L				REG8(0x10 | _BK8)
#define CCPR2H				REG8(0x11 | _BK8)
#define CCP2CON				REG8(0x12 | _BK8)
#define CCP2CAP				REG8(0x13 | _BK8)

#define CLBCON				REG8(0x0c | _BK10)
#define CLBINU				REG8(0x0d | _BK10)
#define CLBINH				REG8(0x0e | _BK10)
#define CLBINM				REG8(0x0f | _BK10)
#define CLBINL				REG8(0x10 | _BK10)
#define CLBOUTU				REG8(0x11 | _BK10)
#define CLBOUTH				REG8(0x12 | _BK10)
#define CLBOUTM				REG8(0x13 | _BK10)
#define CLBOUTL				REG8(0x14 | _BK10)
#define CLBCLK				REG8(0x15 | _BK10)
#define CLBOEU				REG8(0x16 | _BK10)
#define CLBOEH				REG8(0x17 | _BK10)
#define CLBOEM				REG8(0x18 | _BK10)
#define CLBOEL				REG8(0x19 | _BK10)

#define CLCnCON				REG8(0x0c | _BK13)
#define CLCnPOL				REG8(0x0d | _BK13)
#define CLCnSEL0			REG8(0x0e | _BK13)
#define CLCnSEL1			REG8(0x0f | _BK13)
#define CLCnSEL2			REG8(0x10 | _BK13)
#define CLCnSEL3			REG8(0x11 | _BK13)
#define CLCnGLS0			REG8(0x12 | _BK13)
#define CLCnGLS1			REG8(0x13 | _BK13)
#define CLCnGLS2			REG8(0x14 | _BK13)
#define CLCnGLS3			REG8(0x15 | _BK13)
#define CLCSELECT			REG8(0x16 | _BK13)
#define CLCDATA				REG8(0x17 | _BK13)

#define RC1REG				REG8(0x0c | _BK14)
#define TX1REG				REG8(0x0d | _BK14)
#define SP1BRGL				REG8(0x0e | _BK14)
#define SP1BRGH				REG8(0x0f | _BK14)
#define RC1STA				REG8(0x10 | _BK14)
#define TX1STA				REG8(0x11 | _BK14)
#define BAUD1CON			REG8(0x12 | _BK14)

#define SSP1BUF				REG8(0x0c | _BK15)
#define SSP1ADD				REG8(0x0d | _BK15)
#define SSP1MSK				REG8(0x0e | _BK15)
#define SSP1STAT			REG8(0x0f | _BK15)
#define SSP1CON1			REG8(0x10 | _BK15)
#define SSP1CON2			REG8(0x11 | _BK15)
#define SSP1CON3			REG8(0x12 | _BK15)

#define CM1CON0				REG8(0x0c | _BK16)
#define CM1CON1				REG8(0x0d | _BK16)
#define CM1NCH				REG8(0x0e | _BK16)
#define CM1PCH				REG8(0x0f | _BK16)
#define CM2CON0				REG8(0x10 | _BK16)
#define CM2CON1				REG8(0x11 | _BK16)
#define CM2NCH				REG8(0x12 | _BK16)
#define CM2PCH				REG8(0x13 | _BK16)
#define CMOUT				REG8(0x1f | _BK16)
#define PWM1DCL				REG8(0x20 | _BK16)
#define PWM1DCH				REG8(0x21 | _BK16)
#define PWM1CON				REG8(0x22 | _BK16)
#define PWM2DCL				REG8(0x23 | _BK16)
#define PWM2DCH				REG8(0x24 | _BK16)
#define PWM2CON				REG8(0x25 | _BK16)
#define PWMTMRS0			REG8(0x2f | _BK16)

#define DAC1CON0			REG8(0x0c | _BK17)
#define DAC1DATL			REG8(0x0d | _BK17)

#define NVMADL				REG8(0x0c | _BK57)
#define NVMADH				REG8(0x0d | _BK57)
#define NVMDATL				REG8(0x0e | _BK57)
#define NVMDATH				REG8(0x0f | _BK57)
#define NVMCON1				REG8(0x10 | _BK57)
#define NVMCON2				REG8(0x11 | _BK57)
#define SCANCON				REG8(0x12 | _BK57)
#define SCANLADRL			REG8(0x13 | _BK57)
#define SCANLADRH			REG8(0x14 | _BK57)
#define SCANHADRL			REG8(0x16 | _BK57)
#define SCANHADRH			REG8(0x17 | _BK57)
#define SCANDPS				REG8(0x19 | _BK57)
#define SCANDTI				REG8(0x1a | _BK57)
#define CRCDATAL			REG8(0x1d | _BK57)
#define CRCDATAH			REG8(0x1e | _BK57)
#define CRCDATAU			REG8(0x1f | _BK57)
#define CRCDATAT			REG8(0x20 | _BK57)
#define CRCOUTL				REG8(0x21 | _BK57)
#define CRCOUTH				REG8(0x22 | _BK57)
#define CRCOUTU				REG8(0x23 | _BK57)
#define CRCOUTT				REG8(0x24 | _BK57)
#define CRCCON0				REG8(0x25 | _BK57)
#define CRCCON1				REG8(0x26 | _BK57)
#define CRCCON2				REG8(0x27 | _BK57)

#define ADLTHL				REG8(0x0c | _BK58)
#define ADLTHH				REG8(0x0d | _BK58)
#define ADUTHL				REG8(0x0e | _BK58)
#define ADUTHH				REG8(0x0f | _BK58)
#define ADERRL				REG8(0x10 | _BK58)
#define ADERRH				REG8(0x11 | _BK58)
#define ADSTPTL				REG8(0x12 | _BK58)
#define ADSTPTH				REG8(0x13 | _BK58)
#define ADFLTRL				REG8(0x14 | _BK58)
#define ADFLTRH				REG8(0x15 | _BK58)
#define ADACCL				REG8(0x16 | _BK58)
#define ADACCH				REG8(0x17 | _BK58)
#define ADACCU				REG8(0x18 | _BK58)
#define ADCNT				REG8(0x19 | _BK58)
#define ADRPT				REG8(0x1a | _BK58)
#define ADPREVL				REG8(0x1b | _BK58)
#define ADPREVH				REG8(0x1c | _BK58)
#define ADRESL				REG8(0x1d | _BK58)
#define ADRESH				REG8(0x1e | _BK58)
#define ADPCH				REG8(0x1f | _BK58)
#define ADACQL				REG8(0x21 | _BK58)
#define ADACQH				REG8(0x22 | _BK58)
#define ADCAP				REG8(0x23 | _BK58)
#define ADPREL				REG8(0x24 | _BK58)
#define ADPREH				REG8(0x25 | _BK58)
#define ADCON0				REG8(0x26 | _BK58)
#define ADCON1				REG8(0x27 | _BK58)
#define ADCON2				REG8(0x28 | _BK58)
#define ADCON3				REG8(0x29 | _BK58)
#define ADSTAT				REG8(0x2a | _BK58)
#define ADREF				REG8(0x2b | _BK58)
#define ADACT				REG8(0x2c | _BK58)
#define ADCLK				REG8(0x2d | _BK58)
#define ADCG1A				REG8(0x2e | _BK58)

#define RA0PPS				REG8(0x10 | _BK59)
#define RA1PPS				REG8(0x11 | _BK59)
#define RA2PPS				REG8(0x12 | _BK59)
#define RA4PPS				REG8(0x14 | _BK59)
#define RA5PPS				REG8(0x15 | _BK59)

#define PPSLOCK				REG8(0x0c | _BK60)
#define INTPPS				REG8(0x0d | _BK60)
#define T0CKIPPS			REG8(0x0e | _BK60)
#define T1CKIPPS			REG8(0x0f | _BK60)
#define T1GPPS				REG8(0x10 | _BK60)
#define T2INPPS				REG8(0x19 | _BK60)
#define CCP1PPS				REG8(0x1e | _BK60)
#define CCP2PPS				REG8(0x1f | _BK60)
#define CLCIN0PPS			REG8(0x3d | _BK60)
#define CLCIN1PPS			REG8(0x3e | _BK60)
#define CLCIN2PPS			REG8(0x3f | _BK60)
#define CLCIN3PPS			REG8(0x40 | _BK60)
#define CK1PPS				REG8(0x41 | _BK60)
#define RX1PPS				REG8(0x42 | _BK60)
#define SSP1CLKPPS			REG8(0x47 | _BK60)
#define SSP1DATPPS			REG8(0x48 | _BK60)
#define SSP1SSPPS			REG8(0x49 | _BK60)
#define ADACTPPS			REG8(0x50 | _BK60)
#define CLBIN0PPS			REG8(0x57 | _BK60)
#define CLBIN1PPS			REG8(0x58 | _BK60)
#define CLBIN2PPS			REG8(0x59 | _BK60)
#define CLBIN3PPS			REG8(0x5a | _BK60)

#define UMTOAP				REG8(0x0c | _BK62)
#define UMTOAL				REG8(0x0d | _BK62)
#define UMTOAH				REG8(0x0e | _BK62)

#define ANSELA				REG8(0x0c | _BK61)
#define WPUA				REG8(0x0d | _BK61)
#define ODCONA				REG8(0x0e | _BK61)
#define SLRCONA				REG8(0x0f | _BK61)
#define INLVLA				REG8(0x10 | _BK61)
#define IOCAP				REG8(0x11 | _BK61)
#define IOCAN				REG8(0x12 | _BK61)
#define IOCAF				REG8(0x13 | _BK61)

#define PORTAbits		(REG8_t*)(&PORTA)
#define TRISAbits		(REG8_t*)(&TRISA)
#define LATAbits		(REG8_t*)(&LATA)

typedef struct {
	unsigned char RX9D		: 1;
	unsigned char OERR		: 1;
	unsigned char FERR		: 1;
	unsigned char ADDERR	: 1;
	unsigned char CREN		: 1;
	unsigned char SREN		: 1;
	unsigned char RX9		: 1;
	unsigned char SPEN		: 1;
} RCSTA_t;
#define RCSTAbits		(RCSTA_t*)(&RCSTA)

typedef struct {
	unsigned char TX9D		: 1;
	unsigned char TRMT		: 1;
	unsigned char BRGH		: 1;
	unsigned char SENDB		: 1;
	unsigned char SYNC		: 1;
	unsigned char TXEN		: 1;
	unsigned char TX9		: 1;
	unsigned char CSRC		: 1;
} TXSTA_t;
#define TXSTAbits		(TXSTA_t*)(&TXSTA)

typedef struct {
	unsigned char TMR1ON	: 1;
	unsigned char _			: 1;
	unsigned char T1SYNC	: 1;
	unsigned char T1OSCEN	: 1;
	unsigned char T1CKPS	: 2;
	unsigned char TMR1CS0	: 1;
	unsigned char TMR1CS1	: 1;
} T1CON_t;
#define T1CONbits		(T1CON_t*)(&T1CON)

typedef struct {
	unsigned char T1GSS		: 2;
	unsigned char T1GVAL	: 1;
	unsigned char T1GGO		: 1;
	unsigned char T1GSPM	: 1;
	unsigned char T1GTM		: 1;
	unsigned char T1GPOL	: 1;
	unsigned char TMR1GE	: 1;
} T1GCON_t;
#define T1GCONbits		(T1GCON_t*)(&T1GCON)

typedef struct {
	unsigned char T2CKPS	: 2;
	unsigned char TMR2ON	: 1;
	unsigned char T2OUTPS	: 4;
} T2CON_t;
#define T2CONbits		(T2CON_t*)(&T2CON)

typedef struct {
	unsigned char BOR		: 1;
	unsigned char POR		: 1;
	unsigned char RI		: 1;
	unsigned char RMCLR		: 1;
	unsigned char RWDT		: 1;
	unsigned char _			: 1;
	unsigned char STKUNF	: 1;
	unsigned char STKOVF	: 1;
} PCON0_t;
#define PCON0bits		(PCON0_t*)(&PCON0)

typedef struct {
	unsigned char _			: 1;
	unsigned char MEMV		: 1;
	unsigned char __		: 6;
} PCON1_t;
#define PCON1bits		(PCON1_t*)(&PCON1)

typedef struct {
	unsigned char SWDTEN	: 1;
	unsigned char WDTPS		: 5;
} WDTCON_t;
#define WDTCONbits		(WDTCON_t*)(&WDTCON)

typedef struct {
	unsigned char _			: 4;
	unsigned char COSC		: 2;
} OSCCON_t;
#define OSCCONbits		(OSCCON_t*)(&OSCCON)

typedef struct {
	unsigned char _			: 1;
	unsigned char SFOR		: 1;
	unsigned char ADOR		: 1;
	unsigned char __		: 1;
	unsigned char LFOR		: 1;
	unsigned char MFOR		: 1;
	unsigned char HFOR		: 1;
} OSCSTAT_t;
#define OSCSTATbits		(OSCSTAT_t*)(&OSCSTAT)

typedef struct {
	unsigned char _			: 2;
	unsigned char ADOEN		: 1;
	unsigned char __		: 1;
	unsigned char LFOEN		: 1;
	unsigned char MFOEN		: 1;
	unsigned char HFOEN		: 1;
} OSCEN_t;
#define OSCENbits		(OSCEN_t*)(&OSCEN)

typedef struct {
	unsigned char FRQ		: 3;
} OSCFRQ_t;
#define OSCFRQbits		(OSCFRQ_t*)(&OSCFRQ)

typedef struct {
	unsigned char TUN		: 6;
} OSCTUNE_t;
#define OSCTUNEbits		(OSCTUNE_t*)(&OSCTUNE)

typedef struct {
	unsigned char INTEDG	: 1;
	unsigned char _			: 5;
	unsigned char PEIE		: 1;
	unsigned char GIE		: 1;
} INTCON_t;
#define INTCONbits		(INTCON_t*)(&INTCON)

typedef struct {
	unsigned char INTE		: 1;
	unsigned char _			: 3;
	unsigned char IOCIE		: 1;
	unsigned char TMR0IE	: 1;
} PIE0_t;
#define PIE0bits		(PIE0_t*)(&PIE0)

typedef struct {
	unsigned char ADIE		: 1;
	unsigned char SSP1IE	: 1;
	unsigned char BLC1IE	: 1;
	unsigned char TX1IE		: 1;
	unsigned char RC1IE		: 1;
	unsigned char TMR1IE	: 1;
	unsigned char TMR2IE	: 1;
	unsigned char CCP1IE	: 1;
} PIE1_t;
#define PIE1bits		(PIE1_t*)(&PIE1)

typedef struct {
	unsigned char _			: 5;
	unsigned char TMR1GIE	: 1;
	unsigned char NVMIE		: 1;
	unsigned char CCP2IE	: 1;
} PIE2_t;
#define PIE2bits		(PIE2_t*)(&PIE2)

typedef struct {
	unsigned char INTF		: 1;
	unsigned char _			: 3;
	unsigned char IOCIF		: 1;
	unsigned char TMR0IF	: 1;
} PIR0_t;
#define PIR0bits		(PIR0_t*)(&PIR0)

typedef struct {
	unsigned char ADIF		: 1;
	unsigned char SSP1IF	: 1;
	unsigned char BLC1IF	: 1;
	unsigned char TX1IF		: 1;
	unsigned char RC1IF		: 1;
	unsigned char TMR1IF	: 1;
	unsigned char TMR2IF	: 1;
	unsigned char CCP1IF	: 1;
} PIR1_t;
#define PIR1bits		(PIR1_t*)(&PIR1)

typedef struct {
	unsigned char _			: 5;
	unsigned char TMR1GIF	: 1;
	unsigned char NVMIF		: 1;
	unsigned char CCP2IF	: 1;
} PIR2_t;
#define PIR2bits		(PIR2_t*)(&PIR2)

typedef struct {
	unsigned char SEN		: 1;
	unsigned char PS		: 5;
	unsigned char _			: 1;
	unsigned char CS		: 1;
} WTCON_t;
#define WTCONbits		(WTCON_t*)(&WTCON)

typedef struct {
	unsigned char RD		: 1;
	unsigned char WR		: 1;
	unsigned char WREN		: 1;
	unsigned char WRERR		: 1;
	unsigned char FREE		: 1;
	unsigned char LWLO		: 1;
	unsigned char NVMREGS	: 1;
} NVMCON1_t;
#define NVMCON1bits		(NVMCON1_t*)(&NVMCON1)

typedef struct {
	unsigned char ADON		: 1;
	unsigned char GO		: 1;
	unsigned char CHS		: 5;
} ADCON0_t;
#define ADCON0bits		(ADCON0_t*)(&ADCON0)

typedef struct {
	unsigned char ADPREF	: 2;
	unsigned char _ 		: 2;
	unsigned char ADCS		: 3;
	unsigned char ADFM		: 1;
} ADCON1_t;
#define ADCON1bits		(ADCON1_t*)(&ADCON1)


typedef struct {
	unsigned char BORRDY	: 1;
	unsigned char _			: 6;
	unsigned char SBOREN	: 1;
} BORCON_t;
#define BORCONbits		(BORCON_t*)(&BORCON)

typedef struct {
	unsigned char ADFVR		: 2;
	unsigned char CDAFVR	: 2;
	unsigned char TSRNG		: 1;
	unsigned char TSEN		: 1;
	unsigned char FVRRDY	: 1;
	unsigned char FVREN		: 1;
} FVRCON_t;
#define FVRCONbits		(FVRCON_t*)(&FVRCON)


typedef struct {
	unsigned char _			: 4;
	unsigned char CKP		: 1;
	unsigned char SSPEN		: 1;
	unsigned char SSPOV		: 1;
	unsigned char WCOL		: 1;
} SSP1CON1_t;
#define SSP1CON1bits	(SSP1CON1_t*)(&SSP1CON1)

typedef struct {
	unsigned char SEN		: 1;
	unsigned char RSEN		: 1;
	unsigned char PEN		: 1;
	unsigned char RCEN		: 1;
	unsigned char ACKEN		: 1;
	unsigned char ACKDT		: 1;
	unsigned char ACKSTAT	: 1;
	unsigned char GCEN		: 1;
} SSP1CON2_t;
#define SSP1CON2bits	(SSP1CON2_t*)(&SSP1CON2)

typedef struct {
	unsigned char DHEN		: 1;
	unsigned char AHEN		: 1;
	unsigned char SBCDE		: 1;
	unsigned char SDAHT		: 1;
	unsigned char BOEN		: 1;
	unsigned char SCIE		: 1;
	unsigned char PCIE		: 1;
	unsigned char ACKTIM	: 1;
} SSP1CON3_t;
#define SSP1CON3bits	(SSP1CON3_t*)(&SSP1CON3)

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
	unsigned char _			: 2;
	unsigned char P2SEL		: 1;
	unsigned char T1GSEL	: 1;
	unsigned char __		: 1;
	unsigned char SSSEL		: 1;
	unsigned char SDOSEL	: 1;
	unsigned char CLKRSEL	: 1;
} APFCON_t;
#define APFCONbits		(APFCON_t*)(&APFCON)

typedef struct {
	unsigned char SRPR		: 1;
	unsigned char SRPS		: 1;
	unsigned char SRNQEN	: 1;
	unsigned char SRQEN		: 1;
	unsigned char SRCLK		: 3;
	unsigned char SRLEN		: 1;
} SRCON0_t;
#define SRCON0bits		(SRCON0_t*)(&SRCON0)

typedef struct {
	unsigned char SRRC1E	: 1;
	unsigned char _			: 1;
	unsigned char SRRCKE	: 1;
	unsigned char SRRPE		: 1;
	unsigned char SRSC1E	: 1;
	unsigned char __		: 1;
	unsigned char SRSCKE	: 1;
	unsigned char SRSPE		: 1;
} SRCON1_t;
#define SRCON1bits		(SRCON1_t*)(&SRCON1)

typedef struct {
	unsigned char RD		: 1;
	unsigned char WR		: 1;
	unsigned char WREN		: 1;
	unsigned char WRERR		: 1;
	unsigned char FREE		: 1;
	unsigned char LWLO		: 1;
	unsigned char CFGS		: 1;
	unsigned char EEPGD		: 1;
} EECON1_t;
#define EECON1bits		(EECON1_t*)(&EECON1)

typedef struct {
	unsigned char CCP1M		: 4;
	unsigned char DC1B		: 2;
	unsigned char P1M		: 2;
} CCP1CON_t;
#define CCP1CONbits		(CCP1CON_t*)(&CCP1CON)

typedef struct {
	unsigned char P1DC		: 7;
	unsigned char P1RSEN	: 1;
} PWM1CON_t;
#define PWM1CONbits		(PWM1CON_t*)(&PWM1CON)

typedef struct {
	unsigned char PSS1BD	: 2;
	unsigned char PSS1AC	: 2;
	unsigned char CCP1_AS	: 3;
	unsigned char CCP1_ASE	: 1;
} CCP1AS_t;
#define CCP1ASbits		(CCP1AS_t*)(&CCP1AS)

typedef struct {
	unsigned char STR1A		: 1;
	unsigned char STR1B		: 1;
	unsigned char _			: 2;
	unsigned char STR1STNC	: 1;
} PSTR1CON_t;
#define PSTR1CONbits	(PSTR1CON_t*)(&PSTR1CON)

typedef struct {
	unsigned char IOCAP0	: 1;
	unsigned char IOCAP1	: 1;
	unsigned char IOCAP2	: 1;
	unsigned char IOCAP3	: 1;
	unsigned char IOCAP4	: 1;
	unsigned char IOCAP5	: 1;
} IOCAP_t;
#define IOCAPbits		(IOCAP_t*)(&IOCAP)

typedef struct {
	unsigned char IOCAN0	: 1;
	unsigned char IOCAN1	: 1;
	unsigned char IOCAN2	: 1;
	unsigned char IOCAN3	: 1;
	unsigned char IOCAN4	: 1;
	unsigned char IOCAN5	: 1;
} IOCAN_t;
#define IOCANbits		(IOCAN_t*)(&IOCAN)

typedef struct {
	unsigned char IOCAF0	: 1;
	unsigned char IOCAF1	: 1;
	unsigned char IOCAF2	: 1;
	unsigned char IOCAF3	: 1;
	unsigned char IOCAF4	: 1;
	unsigned char IOCAF5	: 1;
} IOCAF_t;
#define IOCAFbits		(IOCAF_t*)(&IOCAF)

typedef struct {
	unsigned char CLKRDIV	: 3;
	unsigned char CLKRDC	: 2;
	unsigned char CLKRSLR	: 1;
	unsigned char CLKROE	: 1;
	unsigned char CLKREN	: 1;
} CLKRCON_t;
#define CLKRCONbits		(CLKRCON_t*)(&CLKRCON)

typedef struct {
	unsigned char MDBIT		: 1;
	unsigned char _			: 2;
	unsigned char MDOUT		: 1;
	unsigned char MDOPOL	: 1;
	unsigned char MDSLR		: 1;
	unsigned char MDOE		: 1;
	unsigned char MDEN		: 1;
} MDCON_t;
#define MDCONbits		(MDCON_t*)(&MDCON)

typedef struct {
	unsigned char MDMS		: 4;
	unsigned char _			: 3;
	unsigned char MDMSODIS	: 1;
} MDSRC_t;
#define MDSRCbits		(MDSRC_t*)(&MDSRC)

typedef struct {
	unsigned char MDCL		: 4;
	unsigned char _			: 1;
	unsigned char MDCLSYNC	: 1;
	unsigned char MDCLPOL	: 1;
	unsigned char MDCLODIS	: 1;
} MDCARL_t;
#define MDCARLbits		(MDCARL_t*)(&MDCARL)

typedef struct {
	unsigned char MDCH		: 4;
	unsigned char _			: 1;
	unsigned char MDCHSYNC	: 1;
	unsigned char MDCHPOL	: 1;
	unsigned char MDCHODIS	: 1;
} MDCARH_t;
#define MDCARHbits		(MDCARH_t*)(&MDCARH)

typedef struct {
	unsigned char OUTPS		: 4;
	unsigned char MD16		: 1;
	unsigned char OUT		: 1;
	unsigned char _			: 1;
	unsigned char EN		: 1;
} T0CON0_t;
#define T0CON0bits		(T0CON0_t*)(&T0CON0)

typedef struct {
	unsigned char CKPS		: 4;
	unsigned char ASYNC		: 1;
	unsigned char CS		: 3;
} T0CON1_t;
#define T0CON1bits		(T0CON1_t*)(&T0CON1)


#define FUSE0          (0x8007)		// CONFIG1
	#define _INT_ANAL_CALB_23	0x3fff	// bit12	- Internal Analog calb 2.3v~5.5v
	#define _INT_ANAL_CALB_18	0x2fff	// bit12	- Internal Analog calb 1.8v~3.6v
	#define _CLKOUT_DIS			0x3fff	// bit8
	#define _CLKOUT_ENA			0x3eff	// bit8
	#define _EXTOSC				0x3fff	// bit5-4
	#define _HFINTOSC_1MHZ		0x3fef	// bit5-4
	#define _LFINTOSC			0x3fdf	// bit5-4
	#define _HFINTOSC_32MHZ		0x3fcf	// bit5-4
	#define _ECH_16MHZ			0x3fff	// bit1-0
	#define _ECL				0x3ffe	// bit1-0
	#define _OSC_DIS			0x3ffd	// bit1-0
#define FUSE1          (0x8008)		// CONFIG2
	#define _DEBUG_OFF			0x3fff	// bit13
	#define _DEBUG_ON			0x1fff	// bit13
	#define _STK_OVF_RESET_ON	0x3fff	// bit12
	#define _STK_OVF_RESET_OFF	0x2fff	// bit12
	#define _PPSLOCKED_ONCE_ON	0x3fff	// bit11
	#define _PPSLOCKED_ONCE_OFF	0x37ff	// bit11
	#define _BORV_19			0x3fff	// bit9		- BOR voltage = 1.9v
	#define _BORV_285			0x3dfe	// bit9		- BOR voltage = 2.85v
	#define _BOR_RST_ON			0x3fff	// bit7-6
	#define _BOR_RST_ON_RUN		0x3fbf	// bit7-6
	#define _BOR_RST_SBOREN		0x3f7f	// bit7-6
	#define _BOR_RST_DISABLE	0x3f3f	// bit7-6
	#define _WDT_ON_11			0x3fff	// bit4-3
	#define _WDT_ON_10			0x3ff7	// bit4-3
	#define _WDT_ON_01			0x3fef	// bit4-3
	#define _WDT_OFF			0x3fe7	// bit4-3
	#define _PWRT_OFF			0x3fff	// bit2-1
	#define _PWRT_64MS			0x3ffd	// bit2-1
	#define _PWRT_16MS			0x3ffb	// bit2-1
	#define _PWRT_1MS			0x3ff9	// bit2-1
	#define _MCLR_1				0x3fff	// bit0
	#define _MCLR_0				0x3ffe	// bit0
#define FUSE2          (0x8009)		// CONFIG3
#define FUSE3          (0x800a)		// CONFIG4
	#define _LVP_ON				0x3fff
	#define _LVP_OFF			0x1fff
	#define _SAF_NOT_WP			0x3fff	// bit-11
	#define _SAF_WP				0x37ff	// bit-11
#define FUSE4          (0x800b)		// CONFIG4
#endif
