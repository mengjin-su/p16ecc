#ifndef _PIC16E_CORE_H
#define _PIC16E_CORE_H

#define NULL			(void*)0
typedef unsigned char 	uint8_t;
typedef unsigned int  	uint16_t;
typedef unsigned short 	uint24_t;
typedef unsigned long 	uint32_t;
typedef char 		  	int8_t;
typedef int  		  	int16_t;
typedef short 		  	int24_t;
typedef long 		  	int32_t;

#define REG8(a)		*(volatile unsigned char*)(a)
#define REG16(a)	*(volatile unsigned int *)(a)

#define _BK0		(0 << 7)
#define _BK1		(1 << 7)
#define _BK2		(2 << 7)
#define _BK3		(3 << 7)
#define _BK4		(4 << 7)
#define _BK5		(5 << 7)
#define _BK6		(6 << 7)
#define _BK7		(7 << 7)
#define _BK8		(8 << 7)
#define _BK9		(9 << 7)
#define _BK10		(10 << 7)
#define _BK11		(11 << 7)
#define _BK12		(12 << 7)
#define _BK13		(13 << 7)
#define _BK14		(14 << 7)
#define _BK15		(15 << 7)
#define _BK16		(16 << 7)
#define _BK17		(17 << 7)
#define _BK18		(18 << 7)
#define _BK19		(19 << 7)
#define _BK20		(20 << 7)
#define _BK21		(21 << 7)
#define _BK22		(22 << 7)
#define _BK23		(23 << 7)
#define _BK24		(24 << 7)
#define _BK25		(25 << 7)
#define _BK26		(26 << 7)
#define _BK27		(27 << 7)
#define _BK28		(28 << 7)
#define _BK29		(29 << 7)
#define _BK30		(30 << 7)
#define _BK31		(31 << 7)
#define _BK32		(32 << 7)
#define _BK33		(33 << 7)
#define _BK34		(34 << 7)
#define _BK35		(35 << 7)
#define _BK36		(36 << 7)
#define _BK37		(37 << 7)
#define _BK38		(38 << 7)
#define _BK39		(39 << 7)
#define _BK40		(40 << 7)
#define _BK41		(41 << 7)
#define _BK42		(42 << 7)
#define _BK43		(43 << 7)
#define _BK44		(44 << 7)
#define _BK45		(45 << 7)
#define _BK46		(46 << 7)
#define _BK47		(47 << 7)
#define _BK48		(48 << 7)
#define _BK49		(49 << 7)
#define _BK50		(50 << 7)
#define _BK51		(51 << 7)
#define _BK52		(52 << 7)
#define _BK53		(53 << 7)
#define _BK54		(54 << 7)
#define _BK55		(55 << 7)
#define _BK56		(56 << 7)
#define _BK57		(57 << 7)
#define _BK58		(58 << 7)
#define _BK59		(59 << 7)
#define _BK60		(60 << 7)
#define _BK61		(61 << 7)
#define _BK62		(62 << 7)
#define _BK63		(63 << 7)

#define INDF0       REG8(0x00)
#define INDF1       REG8(0x01)
#define PCL         REG8(0x02)
#define STATUS      REG8(0x03)
#define FSR0L       REG8(0x04)
#define FSR0H       REG8(0x05)
#define FSR1L       REG8(0x06)
#define FSR1H       REG8(0x07)
#define BSR         REG8(0x08)
#define WREG        REG8(0x09)
#define PCLATH      REG8(0x0a)
#define INTCON      REG8(0x0b)

#define FSR0		REG16(0x4)
#define FSR1		REG16(0x6)

// --- STATUS reg. bits ---
typedef struct {
    unsigned char C   : 1;
	unsigned char DC  : 1;
	unsigned char Z	  : 1;
	unsigned char PD  : 1;
	unsigned char TO  : 1;
} STATUS_t;
#define STATUSbits	(STATUS_t*)(&STATUS)

#define _ACC0_		REG8(0x70)
#define _ACC1_		REG8(0x71)
#define _ACC2_		REG8(0x72)
#define _ACC3_		REG8(0x73)

#endif
