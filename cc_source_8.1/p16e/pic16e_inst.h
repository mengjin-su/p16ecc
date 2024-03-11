#ifndef P16E_INST_H
#define P16E_INST_H

// --- P16Fxxxx instruction set ---
#define _ADDWF		(char*)"addwf"
#define _ANDWF		(char*)"andwf"
#define _CLRF		(char*)"clrf"
#define _CLRW		(char*)"clrw"
#define _COMF		(char*)"comf"
#define _DECF		(char*)"decf"
#define _DECFSZ		(char*)"decfsz"
#define _INCF		(char*)"incf"
#define _INCFSZ		(char*)"incfsz"
#define _IORWF		(char*)"iorwf"
#define _MOVF		(char*)"movf"
#define _MOVWF		(char*)"movwf"
#define _RLF        (char*)"rlf"
#define _RRF        (char*)"rrf"
#define _SUBWF		(char*)"subwf"
#define _NOP		(char*)"nop"
#define _SWAPF		(char*)"swapf"
#define _XORWF		(char*)"xorwf"
#define _BCF		(char*)"bcf"
#define _BSF		(char*)"bsf"
#define _BTFSC		(char*)"btfsc"
#define _BTFSS		(char*)"btfss"
#define _CALL		(char*)"call"
#define _GOTO		(char*)"goto"
#define _RETFIE		(char*)"retfie"
#define _RETLW		(char*)"retlw"
#define _RETURN		(char*)"return"
#define _ADDLW		(char*)"addlw"
#define _ANDLW		(char*)"andlw"
#define _IORLW		(char*)"iorlw"
#define _MOVLW		(char*)"movlw"
#define _SUBLW		(char*)"sublw"
#define _XORLW		(char*)"xorlw"
#define _SLEEP		(char*)"sleep"

// Enhanced PIC16 instructions
#define _ADDWFC		(char*)"addwfc"
#define _SUBWFB		(char*)"subwfb"
#define _LSLF		(char*)"lslf"		// logical shift left
#define _LSRF		(char*)"lsrf"		// logical shift right
#define _ASRF		(char*)"asrf"		// arithmatic shift right
#define _MOVLP		(char*)"movlp"		// load PCLATH
#define _MOVLB		(char*)"movlb"		// load BSR
#define _BRA		(char*)"bra"
#define _BRW		(char*)"brw"
#define _CALLW		(char*)"callw"
#define _ADDFSR		(char*)"addfsr"
#define _MOVIW		(char*)"moviw"
#define _MOVWI		(char*)"movwi"
#define _RESET		(char*)"reset"
#define _CLRWDT		(char*)"clrwdt"

// pseudo instructions
#define _BSEL		(char*)".bsel"
#define _PSEL		(char*)".psel"
#define _SEGMENT	(char*)".segment"
#define _END		(char*)".end"
#define _RS			(char*)".rs"
#define _EQU		(char*)".equ"
#define _FCALL		(char*)".fcall"
#define _DW         (char*)".dw"
#define _INVOKE		(char*)".invoke"
#define _DEVICE		(char*)".device"

#endif
