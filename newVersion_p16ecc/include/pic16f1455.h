#ifndef _P16F1455_H
#define _P16F1455_H

#define __DEVICE    "pic16f1455"
#include <pic16e.h>

#define __FLASH_SIZE        (1024*8)
#define __SRAM_SIZE         1024
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
#define ADCON2				REG8(0x1f | _BK1)

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
#define APFCON				REG8(0x1d | _BK2)

#define ANSELA				REG8(0x0c | _BK3)
#define ANSELC				REG8(0x0e | _BK3)
#define PMADRL				REG8(0x11 | _BK3)
#define PMADRH				REG8(0x12 | _BK3)
#define PMADR				REG16(0x11 | _BK3)
#define PMDATL				REG8(0x13 | _BK3)
#define PMDATH				REG8(0x14 | _BK3)
#define PMDAT				REG16(0x13 | _BK3)
#define PMCON1				REG8(0x15 | _BK3)
#define PMCON2				REG8(0x16 | _BK3)
#define VREGCON				REG8(0x17 | _BK3)
#define RCREG				REG8(0x19 | _BK3)
#define TXREG				REG8(0x1a | _BK3)
#define SPBRGL				REG8(0x1b | _BK3)
#define SPBRGH				REG8(0x1c | _BK3)
#define RCSTA				REG8(0x1d | _BK3)
#define TXSTA				REG8(0x1e | _BK3)
#define BAUDCON				REG8(0x1f | _BK3)

#define PORTAbits			(REG8_t*)(&PORTA)
#define PORTCbits			(REG8_t*)(&PORTC)
#define TRISAbits			(REG8_t*)(&TRISA)
#define TRISCbits			(REG8_t*)(&TRISC)
#define LATAbits			(REG8_t*)(&LATA)
#define LATCbits			(REG8_t*)(&LATC)

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

#define WPUA				REG8(0x0c | _BK4)
#define WPUB				REG8(0x0d | _BK4)
#define SSP1BUF				REG8(0x11 | _BK4)
#define SSP1ADD				REG8(0x12 | _BK4)
#define SSP1MSK				REG8(0x13 | _BK4)
#define SSP1STAT			REG8(0x14 | _BK4)
#define SSP1CON1			REG8(0x15 | _BK4)
#define SSP1CON2			REG8(0x16 | _BK4)
#define SSP1CON3			REG8(0x17 | _BK4)

#define IOCAP				REG8(0x11 | _BK7)
#define IOCAN				REG8(0x12 | _BK7)
#define IOCAF				REG8(0x13 | _BK7)
#define IOCBP				REG8(0x14 | _BK7)
#define IOCBN				REG8(0x15 | _BK7)
#define IOCBF				REG8(0x16 | _BK7)
#define CLKRCON				REG8(0x1a | _BK7)
#define CRCON				REG8(0x1b | _BK7)
#define ACTCON				CRCON

#define PWM1DCL				REG8(0x11 | _BK12)
#define PWM1DCH				REG8(0x12 | _BK12)
#define PWM1CON				REG8(0x13 | _BK12)
#define PWM2DCL				REG8(0x14 | _BK12)
#define PWM2DCH				REG8(0x15 | _BK12)
#define PWM2CON				REG8(0x16 | _BK12)

#define CWG1DBR				REG8(0x11 | _BK13)
#define CWG1DBF				REG8(0x12 | _BK13)
#define CWG1CON0			REG8(0x13 | _BK13)
#define CWG1CON1			REG8(0x14 | _BK13)
#define CWG1CON2			REG8(0x15 | _BK13)

#define UCON				REG8(0x0e | _BK29)
#define USTAT				REG8(0x0f | _BK29)
#define UIR					REG8(0x10 | _BK29)
#define UCFG				REG8(0x11 | _BK29)
#define UIE					REG8(0x12 | _BK29)
#define UEIR				REG8(0x13 | _BK29)
#define UFRMH				REG8(0x14 | _BK29)
#define UFRML				REG8(0x15 | _BK29)
#define UADDR				REG8(0x16 | _BK29)
#define UEIE				REG8(0x17 | _BK29)
#define UEP0				REG8(0x18 | _BK29)
#define UEP1				REG8(0x19 | _BK29)
#define UEP2				REG8(0x1a | _BK29)
#define UEP3				REG8(0x1b | _BK29)
#define UEP4				REG8(0x1c | _BK29)
#define UEP5				REG8(0x1d | _BK29)
#define UEP6				REG8(0x1e | _BK29)
#define UEP7				REG8(0x1f | _BK29)

typedef struct {
	unsigned char TMR1IF	: 1;
	unsigned char TMR2IF	: 1;
	unsigned char _			: 1;
	unsigned char SSPIF		: 1;
	unsigned char TXIF		: 1;
	unsigned char RCIF		: 1;
	unsigned char ADIF		: 1;
	unsigned char TMR1GIF	: 1;
} PIR1_t;
#define PIR1bits		(PIR1_t*)(&PIR1)

typedef struct {
	unsigned char _			: 1;
	unsigned char ACTIF		: 1;
	unsigned char USBIF		: 1;
	unsigned char BCLIF		: 1;
	unsigned char __		: 1;
	unsigned char C1IF		: 1;
	unsigned char C2IF		: 1;
	unsigned char OSFIF		: 1;
} PIR2_t;
#define PIR2bits		(PIR2_t*)(&PIR2)

typedef struct {
	unsigned char TMR1ON	: 1;
	unsigned char _			: 1;
	unsigned char T1SYNC	: 1;
	unsigned char T1OSCEN	: 1;
	unsigned char T1CKPS	: 2;
	unsigned char TMR1CS	: 2;
} T1CON_t;
#define T1CONbits		(T1CON_t*)(&T1CON)

typedef struct {
	unsigned char T2CKPS	: 2;
	unsigned char TMR2ON	: 1;
	unsigned char T2OUTPS	: 4;
} T2CON_t;
#define T2CONbits		(T2CON_t*)(&T2CON)

typedef struct {
	unsigned char IOCIF		: 1;
	unsigned char INTF		: 1;
	unsigned char TMR0IF	: 1;
	unsigned char IOCIE		: 1;
	unsigned char INTE		: 1;
	unsigned char TMR0IE	: 1;
	unsigned char PEIE		: 1;
	unsigned char GIE		: 1;
} INTCON_t;
#define INTCONbits		(INTCON_t*)(&INTCON)

typedef struct {
	unsigned char TMR1IE	: 1;
	unsigned char TMR2IE	: 1;
	unsigned char _			: 1;
	unsigned char SSPIE		: 1;
	unsigned char TXIE		: 1;
	unsigned char RCIE		: 1;
	unsigned char ADIE		: 1;
	unsigned char TMR1GIE	: 1;
} PIE1_t;
#define PIE1bits		(PIE1_t*)(&PIE1)

typedef struct {
	unsigned char _			: 1;
	unsigned char ACTIE		: 1;
	unsigned char USBIE		: 1;
	unsigned char BCLIE		: 1;
	unsigned char __		: 1;
	unsigned char C1IE		: 1;
	unsigned char C2IE		: 1;
	unsigned char OSFIE		: 1;
} PIE2_t;
#define PIE2bits		(PIE2_t*)(&PIE2)

typedef struct {
	unsigned char PS		: 3;
	unsigned char PSA		: 1;
	unsigned char TMR0SE	: 1;
	unsigned char TMR0CS	: 1;
	unsigned char INTEDG	: 1;
	unsigned char WPUEN		: 1;
} OPTION_REG_t;
#define OPTION_REGbits		(OPTION_REG_t*)(&OPTION_REG)

typedef struct {
	unsigned char BOR		: 1;
	unsigned char POR		: 1;
	unsigned char RI		: 1;
	unsigned char RMCLR		: 1;
	unsigned char RWDT		: 1;
	unsigned char _			: 1;
	unsigned char STKUNF	: 1;
	unsigned char STKOVF	: 1;
} PCON_t;
#define PCONbits		(PCON_t*)(&PCON)

typedef struct {
	unsigned char SWDTEN	: 1;
	unsigned char WDTPS		: 5;
} WDTCON_t;
#define WDTCONbits		(WDTCON_t*)(&WDTCON)

typedef struct {
	unsigned char TUN0		: 6;
} OSCTUNE_t;
#define OSCTUNEbits		(OSCTUNE_t*)(&OSCTUNE)

typedef struct {
	unsigned char SCS		: 2;
	unsigned char IRCF		: 4;
	unsigned char SPLLMULT	: 1;
	unsigned char SPLLEN	: 1;
} OSCCON_t;
#define OSCCONbits		(OSCCON_t*)(&OSCCON)

typedef struct {
	unsigned char HFIOFS	: 1;
	unsigned char LFIOFR	: 1;
	unsigned char _			: 2;
	unsigned char HFIOFR	: 1;
	unsigned char OSTS		: 1;
	unsigned char PLLRDY	: 1;
	unsigned char T1OSCR	: 1;
} OSCSTAT_t;
#define OSCSTATbits		(OSCSTAT_t*)(&OSCSTAT)

typedef struct {
	unsigned char ADON		: 1;
	unsigned char GO		: 1;
	unsigned char CHS		: 5;
} ADCON0_t;
#define ADCON0bits		(ADCON0_t*)(&ADCON0)

typedef struct {
	unsigned char ADPREF	: 2;
	unsigned char _			: 2;
	unsigned char ADCS		: 3;
	unsigned char ADFM		: 1;
} ADCON1_t;
#define ADCON1bits		(ADCON1_t*)(&ADCON1)

typedef struct {
	unsigned char SYNC		: 1;
	unsigned char HYS		: 1;
	unsigned char SP		: 1;
	unsigned char _			: 1;
	unsigned char POL		: 1;
	unsigned char OE		: 1;
	unsigned char OUT		: 1;
	unsigned char ON		: 1;
} CM1CON0_t;
#define CM1CON0bits		(CM1CON0_t*)(&CM1CON0)
#define CM2CON0bits		(CM1CON0_t*)(&CM2CON0)

typedef struct {
	unsigned char NCH		: 3;
	unsigned char _			: 1;
	unsigned char PCH		: 2;
	unsigned char INTN		: 1;
	unsigned char INTP		: 1;
} CM1CON1_t;
#define CM1CON1bits		(CM1CON1_t*)(&CM1CON1)
#define CM2CON1bits		(CM1CON1_t*)(&CM2CON1)

typedef struct {
	unsigned char MC1OUT	: 1;
	unsigned char MC2OUT	: 1;
} CMOUT_t;
#define CMOUTbits		(CMOUT_t*)(&CMOUT)

typedef struct {
	unsigned char BORRDY	: 1;
	unsigned char _			: 5;
	unsigned char BORFS		: 1;
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
	unsigned char _			: 2;
	unsigned char DACPSS	: 2;
	unsigned char DACOE2	: 1;
	unsigned char DACOE1	: 1;
	unsigned char __		: 1;
	unsigned char DACEN		: 1;
} DACCON0_t;
#define DACCON0bits		(DACCON0_t*)(&DACCON0)

typedef struct {
	unsigned char DACR		: 5;
} DACCON1_t;
#define DACCON1bits		(DACCON1_t*)(&DACCON1)

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
	unsigned char AHEN 		: 1;
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
	unsigned char _			: 4;
	unsigned char POL		: 1;
	unsigned char OUT		: 1;
	unsigned char OE		: 1;
	unsigned char EN		: 1;
} PWM1CON_t;
#define PWM1CONbits		(PWM1CON_t*)(&PWM1CON)
#define PWM2CONbits		(PWM1CON_t*)(&PWM2CON)

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
	unsigned char _			: 1;
	unsigned char SUSPND	: 1;
	unsigned char RESUME	: 1;
	unsigned char USBEN		: 1;
	unsigned char PKTDIS	: 1;
	unsigned char SE0		: 1;
	unsigned char PPBRST	: 1;
} UCON_t;
#define UCONbits		(UCON_t*)(&UCON)

typedef struct {
	unsigned char _			: 1;
	unsigned char PPBI		: 1;
	unsigned char DIR		: 1;
	unsigned char ENDP		: 4;
} USTAT_t;
#define USTATbits		(USTAT_t*)(&USTAT)

typedef struct {
	unsigned char URSTIF	: 1;
	unsigned char UERRIF	: 1;
	unsigned char ACTVIF	: 1;
	unsigned char TRNIF		: 1;
	unsigned char IDLEIF	: 1;
	unsigned char STALLIF	: 1;
	unsigned char SOFIF		: 1;
} UIR_t;
#define UIRbits			(UIR_t*)(&UIR)

typedef struct {
	unsigned char PPB		: 2;
	unsigned char FSEN		: 1;
	unsigned char _			: 1;
	unsigned char UPUEN		: 1;
	unsigned char __		: 2;
	unsigned char UTEYE		: 1;
} UCFG_t;
#define UCFGbits		(UCFG_t*)(&UCFG)

typedef struct {
	unsigned char URSTIE	: 1;
	unsigned char UERRIE	: 1;
	unsigned char ACTVIE	: 1;
	unsigned char TRNIE		: 1;
	unsigned char IDLEIE	: 1;
	unsigned char STALLIE	: 1;
	unsigned char SOFIE		: 1;
} UIE_t;
#define UIEbits			(UIE_t*)(&UIE)

typedef struct {
	unsigned char PIDEF		: 1;
	unsigned char CRC5EF	: 1;
	unsigned char CRC16EF	: 1;
	unsigned char DNF8EF	: 1;
	unsigned char BTOEF		: 1;
	unsigned char _			: 2;
	unsigned char BTSEF		: 1;
} UEIR_t;
#define UEIRbits		(UEIR_t*)(&UEIR)

typedef struct {
	unsigned char PIDEE		: 1;
	unsigned char CRC5EE	: 1;
	unsigned char CRC16EE	: 1;
	unsigned char DNF8EE	: 1;
	unsigned char BTOEE		: 1;
	unsigned char _			: 2;
	unsigned char BTSEE		: 1;
} UEIE_t;
#define UEIEbits		(UEIE_t*)(&UEIE)

typedef struct {
	unsigned char EPSTALL	: 1;
	unsigned char EPINEN	: 1;
	unsigned char EPOUTEN	: 1;
	unsigned char EPCONDIS	: 1;
	unsigned char EPHSHK	: 1;
} UEP_t;
#define UEP0bits		(UEP_t*)(&UEP0)
#define UEP1bits		(UEP_t*)(&UEP1)
#define UEP2bits		(UEP_t*)(&UEP2)
#define UEP3bits		(UEP_t*)(&UEP3)
#define UEP4bits		(UEP_t*)(&UEP4)
#define UEP5bits		(UEP_t*)(&UEP5)
#define UEP6bits		(UEP_t*)(&UEP6)
#define UEP7bits		(UEP_t*)(&UEP7)

#define EPSTALL0		(UEP0bits->EPSTALL)
#define EPSTALL1		(UEP1bits->EPSTALL)
#define EPSTALL2		(UEP2bits->EPSTALL)
#define EPSTALL3		(UEP3bits->EPSTALL)
#define EPSTALL4		(UEP4bits->EPSTALL)
#define EPSTALL5		(UEP5bits->EPSTALL)
#define EPSTALL6		(UEP6bits->EPSTALL)
#define EPSTALL7		(UEP7bits->EPSTALL)

#define EP0STALL		EPSTALL0
#define EP1STALL		EPSTALL1
#define EP2STALL		EPSTALL2
#define EP3STALL		EPSTALL3
#define EP4STALL		EPSTALL4
#define EP5STALL		EPSTALL5
#define EP6STALL		EPSTALL6
#define EP7STALL		EPSTALL7


#define FUSE0          (0x8000+0x07)
	#define _IESO_DISA		0x2fff	// bit12	- Internal/External switch
	#define _CLKOUT_ENA		0x3eff	// bit11	- Clock out enable
	#define _BOR_DISA		0x39ff	// bit10-9	- Brown-out reset enable
	#define _BOR_SBOREN		0x3bff	// bit10-9
	#define _BOR_SLEEP		0x3dff	// bit10-9
	#define _CP_ENA			0x3f7f	// bit7		- Code Protection
	#define _MCLR_DISA		0x3fbf	// bit6		- MCLR/VPP pin function select
	#define _PWRT_ENA		0x3fdf	// bit5		- Power-up timer enable
	#define _WDT_ENA_RUN	0x3ff7	// bit4-3	- Watchdog timer enable
	#define _WDT_SWDTEN		0x3fef	// bit4-3
	#define _WDT_DISA		0x3fe7	// bit4-3
	#define _FOSC_ECH		0x3fff	// bit2-0	- Oscillator selection
	#define _FOSC_ECM		0x3ffe	// bit2-0
	#define _FOSC_ECL		0x3ffd	// bit2-0
	#define _FOSC_INTOSC	0x3ffc	// bit2-0
	#define _FOSC_EXTRC		0x3ffb	// bit2-0
	#define _FOSC_HS		0x3ffa	// bit2-0
	#define _FOSC_XT		0x3ff9	// bit2-0
	#define _FOSC_LP		0x3ff8	// bit2-0
#define FUSE1          (0x8000+0x08)
	#define _LVP_HV_ONLY	0x1fff	// bit13
	#define _DEBUG_ENA		0x2fff	// bit12
	#define _LPBOR_ENA		0x37ff	// bit11
	#define _BORV_H_TRIP	0x3bff	// bit10
	#define _STACK_OVFL_RST	0x3dff	// bit9
	#define _PLL_DISABLE	0x3eff	// bit8
	#define _PLL_4X			0x3f7f	// bit7
	#define _USB_CLK_DIV4	0x3fbf	// bit6
	#define _CPU_DIV_3		0x3fef	// bit5-4
	#define _CPU_DIV_2		0x3fdf	// bit5-4
	#define _CPU_DIV_NONE	0x3fcf	// bit5-4
	#define _FLASH_WP_01FF	0x3ffe	// bit1-0
	#define _FLASH_WP_0FFF	0x3ffd	// bit1-0
	#define _FLASH_WP_1FFF	0x3ffc	// bit1-0

#endif
