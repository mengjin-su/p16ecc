#ifndef __GENERIC_TYPE_DEFS_H_
#define __GENERIC_TYPE_DEFS_H_

enum { FALSE = 0, TRUE };    /* Undefined size */
typedef char BOOL;

/* INT is processor specific in length may vary in size */
typedef signed int          INT;
typedef signed char         INT8;
typedef signed int    		INT16;
typedef signed long     	INT32;

/* UINT is processor specific in length may vary in size */
typedef unsigned int        UINT;
typedef unsigned char       UINT8;
typedef unsigned int  		UINT16;
typedef unsigned long   	UINT32;     /* other name for 32-bit integer */

/* Alternate definitions */
typedef unsigned char       BYTE;                           /* 8-bit unsigned  */
typedef unsigned int      	WORD;                           /* 16-bit unsigned */
typedef unsigned long       DWORD;                          /* 32-bit unsigned */

typedef union {
	BYTE 	bytes[2];
	WORD 	value;
} UWORD;

typedef union {
	BYTE 	bytes[4];
	WORD	words[2];
	DWORD 	value;
} ULONG;

typedef union {
	unsigned short t;
	unsigned char  b[3];
} TimeStamp3;

typedef struct {
	char type;
	union {
		BYTE bytes[13];		// rx
	} data;
	TimeStamp3 timeStamp;
} CanRxMsg;

typedef struct {
	char type;
	union {
		BYTE bytes[13];		// rx
		struct {			// tx
			DWORD id;
			BYTE  dlc;
			BYTE  buff[8];
		} tx;
	} data;
} CanTxMsg;

typedef struct {
	BYTE addr;
	char type;
} RegIOMsg;


#endif /* __GENERIC_TYPE_DEFS_H_ */
