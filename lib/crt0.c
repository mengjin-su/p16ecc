#include <pic16e.h>

#define ACC0	*((unsigned char*)0x70)
#define ACC1	*((unsigned char*)0x71)
#define ACC2	*((unsigned char*)0x72)
#define ACC3	*((unsigned char*)0x73)
#define FLAG_LOC			0x7e
#define BUFFERED_FSR1L		0x7c
#define BUFFERED_FSR1H		0x7d
#define ISR_FSR1L			0x7a
#define ISR_FSR1H			0x7b

static void _negACC24(void);
static void _negACC32(void);

/////////////////////////////////////////////////////////////////////
void _pcall(void)
{
	PCLATH = WREG;
	asm("moviw INDF1++");
	PCL = WREG;
}

/*****************************
	Copy function parameters,
	from stack pointed by FSR1 to the RAM pointed by FSR0
*****************************/
void _copyPar(void)
{
	do {
		asm("moviw 	INDF1++");
		asm("movwi 	--INDF0");
	} while ( --BSR );
}

/*****************************
	8-bit multilication
	return ACC1/ACC0 = product
*****************************/
void _mul8(void)
{
	FSR0 = 0;
	FSR0H |= 0x08;
	do {
		_C = 0;			// C flag = 0
		_BTFSC(ACC0, 0);
		_ADDWF_F(FSR0L);
		_RRF_F(FSR0L);
		_RRF_F(ACC0);
	} while ( --FSR0H );
	ACC1 = FSR0L;
}

void _mul16(void)
{
	FSR0 = 0;
	WREG = 16;	// count = 16
	asm ("movwi   --INDF1");

	do
	{
		_C = 0;
		if ( ACC0 & 1 )
		{
			asm("moviw   2[INDF1]");
			_ADDWF_F (FSR0L);
			asm("moviw   1[INDF1]");
			_ADDWFC_F(FSR0H);
		}

		_RRF_F(FSR0H);
		_RRF_F(FSR0L);
		_RRF_F(ACC1);
		_RRF_F(ACC0);

	} while ( --INDF1 );

	asm ("addfsr  1, 3");
}

void _mul24(void)
{
	FSR0 = 0;
	ACC3 = 0;
	WREG = 24;
	asm ("movwi   --INDF1");
	do
	{
		_C = 0;
		if ( ACC0 & 1 )
		{
			asm("moviw   3[INDF1]");
			_ADDWF_F (FSR0L);
			asm("moviw   2[INDF1]");
			_ADDWFC_F(FSR0H);
			asm("moviw   1[INDF1]");
			_ADDWFC_F(ACC3);
		}
		_RRF_F(ACC3);
		_RRF_F(FSR0H);
		_RRF_F(FSR0L);
		_RRF_F(ACC2);
		_RRF_F(ACC1);
		_RRF_F(ACC0);

	} while ( --INDF1 );

	asm ("addfsr  1, 4");
}

void _mul32(void)
{
	WREG = ACC2;
	asm ("movwi   --INDF1");
	WREG = ACC3;
	asm ("movwi   --INDF1");
	WREG = 32;
	asm ("movwi   --INDF1");
	FSR0 = 0;
	ACC2 = 0;
	ACC3 = 0;
	do
	{
		_C = 0;
		if ( ACC0 & 1 )
		{
			asm("moviw   6[INDF1]");
			_ADDWF_F (FSR0L);
			asm("moviw   5[INDF1]");
			_ADDWFC_F(FSR0H);
			asm("moviw   4[INDF1]");
			_ADDWFC_F(ACC2);
			asm("moviw   3[INDF1]");
			_ADDWFC_F(ACC3);
		}
		_RRF_F(ACC3);
		_RRF_F(ACC2);
		_RRF_F(FSR0H);
		_RRF_F(FSR0L);
		asm("moviw   1[INDF1]");
		_RRF_F(WREG);
		asm("movwi   1[INDF1]");
		asm("moviw   2[INDF1]");
		_RRF_F(WREG);
		asm("movwi   2[INDF1]");
		_RRF_F(ACC1);
		_RRF_F(ACC0);

	} while ( --INDF1 );
	asm("moviw   1[INDF1]");	ACC3 = WREG;
	asm("moviw   2[INDF1]");	ACC2 = WREG;
	asm("addfsr  1, 7");
}

void _mul16indf(void)
{
	ACC0 = 0;
	ACC1 = 0;
	WREG = 16;
	asm ("movwi   --INDF1");
	do
	{
		_C = 0;
		if ( INDF0 & 1 )
		{
			asm("moviw 2[INDF1]"); 	// X[0]
			_ADDWF_F (ACC0);
			asm("moviw 1[INDF1]"); 	// X[1]
			_ADDWFC_F(ACC1);
		}

		_RRF_F(ACC1);
		_RRF_F(ACC0);
		asm("moviw 1[INDF0]");
		_RRF_F(WREG);
		asm("movwi 1[INDF0]");
		_RRF_F(INDF0);

	} while ( --INDF1 );
	asm ("addfsr  1, 3");
}

void _mul24indf(void)
{
	ACC0 = 0;
	ACC1 = 0;
	ACC2 = 0;
	WREG = 24;
	asm ("movwi   --INDF1");
	do
	{
		_C = 0;
		if ( INDF0 & 1 )
		{
			asm("moviw 3[INDF1]"); 	// X[0]
			_ADDWF_F (ACC0);
			asm("moviw 2[INDF1]"); 	// X[1]
			_ADDWFC_F(ACC1);
			asm("moviw 1[INDF1]"); 	// X[2]
			_ADDWFC_F(ACC2);
		}
		_RRF_F(ACC2);
		_RRF_F(ACC1);
		_RRF_F(ACC0);
		asm("addfsr 0, 2");
		_RRF_F(INDF0);
		asm("addfsr 0, -1");
		_RRF_F(INDF0);
		asm("addfsr 0, -1");
		_RRF_F(INDF0);

	} while ( --INDF1 );
	asm ("addfsr  1, 4");
}

void _mul32indf(void)
{
	ACC0 = 0;
	ACC1 = 0;
	ACC2 = 0;
	ACC3 = 0;
	WREG = 32;
	asm ("movwi   --INDF1");
	do
	{
		_C = 0;
		if ( INDF0 & 1 )
		{
			asm("moviw 4[INDF1]"); 	// X[0]
			_ADDWF_F (ACC0);
			asm("moviw 3[INDF1]"); 	// X[1]
			_ADDWFC_F(ACC1);
			asm("moviw 2[INDF1]"); 	// X[2]
			_ADDWFC_F(ACC2);
			asm("moviw 1[INDF1]"); 	// X[3]
			_ADDWFC_F(ACC3);
		}
		_RRF_F(ACC3);
		_RRF_F(ACC2);
		_RRF_F(ACC1);
		_RRF_F(ACC0);
		asm("addfsr 0, 3");
		_RRF_F(INDF0);
		asm("addfsr 0, -1");
		_RRF_F(INDF0);
		asm("addfsr 0, -1");
		_RRF_F(INDF0);
		asm("addfsr 0, -1");
		_RRF_F(INDF0);

	} while ( --INDF1 );
	asm ("addfsr  1, 5");
}

void _divmod8(void)
{
	if ( WREG & 4 )			// 'y' signed?
	{
		if ( INDF1 & 0x80 )
			INDF1 ^= 0xff, INDF1++;
		else
			WREG &= ~4;
	}

	if ( WREG & 2 )			// 'x' signed?
	{
		if ( ACC0 & 0x80 )
		{
			ACC0 = -ACC0;
            WREG ^= 4;
		}
	}

	asm ("movwi --INDF1");	// flag bytes -> stack
	FSR0H = 8;	// loop count
	FSR0L = 0;
	asm ("moviw 1[INDF1]");
	do
	{
		_C = 1;
		_RLF_F  (ACC0);
		_RLF_F  (FSR0L);
		_SUBWF_F(FSR0L);

		if ( _C == 0 )
		{
			_ADDWF_F(FSR0L);
			_BCF(ACC0, 0);
		}
	} while ( --FSR0H );

	if ( INDF1 & 0x1 )	// mod. operation?
		ACC0 = FSR0L;

	if ( INDF1 & 0x4 )	// negative result?
		ACC0 = -ACC0;

	asm ("addfsr 1, 2");
}

void _divmod16(void)
{
	asm ("movwi   --INDF1");	// operating options

	if ( INDF1 & 4 )	// x is signed?
	{
		if ( _ACC1_ & 0x80 )
		{
            _COMF_F(ACC0);
            _COMF_F(ACC1);
			_INCF_F(ACC0);
			_BTFSC (STATUS, 2);
			_INCF_F(ACC1);
		}
		else
			INDF1 &= ~4;
	}

	FSR0 = 0;

	if ( INDF1 & 2 )	// y is signed?
	{
		asm ("moviw   1[INDF1]");
		if ( WREG & 0x80 )
		{
			INDF1 ^= 4;
			asm("moviw   2[INDF1]");
			_SUBWF_W(FSR0L);
			asm("movwi   2[INDF1]");
			asm("moviw   1[INDF1]");
			_SUBWFB_W(FSR0H);
			asm("movwi   1[INDF1]");
		}
	}

	WREG = 16;	// loop count = 16
	asm ("movwi   --INDF1");
	do
	{
		_C = 1;
		_RLF_F(ACC0);
		_RLF_F(ACC1);
		_RLF_F(FSR0L);
		_RLF_F(FSR0H);
		asm("moviw	3[INDF1]");
		_SUBWF_F(FSR0L);
		asm ("moviw	2[INDF1]");
		_SUBWFB_F(FSR0H);
		if ( _C == 0 )
		{
			asm("moviw	3[INDF1]");
			_ADDWF_F(FSR0L);
			asm ("moviw	2[INDF1]");
			_ADDWFC_F(FSR0H);
			ACC0 &= ~1;
		}
	} while ( --INDF1 );

	asm ("addfsr  1, 1");		//	_ADDFSR (1, 1);

	if ( INDF1 & 1 ) // mod operation?
	{
		ACC0 = FSR0L;
		ACC1 = FSR0H;
	}

	if ( INDF1 & 4 )	// negative result?
	{
        _COMF_F(ACC0);
        _COMF_F(ACC1);
		_INCF_F(ACC0);
        _BTFSC (STATUS, 2);
		_INCF_F(ACC1);
	}

    asm ("addfsr  1, 3");       //  _ADDFSR (1, 3);
}

void _divmod24(void)
{
	asm ("movwi   --INDF1");	// operating options

	if ( INDF1 & 4 )	// x is signed?
	{
        if ( ACC2 & 0x80 )
            _negACC24();
		else
			INDF1 &= ~4;
	}

    FSR0 = 0;
    ACC3 = 0; // hi byte of product

	if ( INDF1 & 2 )	// y is signed?
	{
		asm ("moviw   1[INDF1]");
		if ( WREG & 0x80 )
		{
			INDF1 ^= 4;

            asm ("moviw   3[INDF1]");
			_SUBWF_W (FSR0L);
            asm ("movwi   3[INDF1]");

        	asm ("moviw   2[INDF1]");
            _SUBWFB_W(FSR0H);
            asm ("movwi   2[INDF1]");

			asm ("moviw   1[INDF1]");
            _SUBWFB_W(ACC3);
            asm ("movwi   1[INDF1]");
		}
	}

    WREG = 24;	// loop count = 24
    asm ("movwi   --INDF1");

	do
	{
		_C = 1;
		_RLF_F(ACC0);
		_RLF_F(ACC1);
        _RLF_F(ACC2);
		_RLF_F(FSR0L);
		_RLF_F(FSR0H);
        _RLF_F(ACC3);

        asm ("moviw   4[INDF1]");
		_SUBWF_F(FSR0L);
        asm ("moviw   3[INDF1]");
		_SUBWFB_F(FSR0H);
        asm ("moviw   2[INDF1]");
        _SUBWFB_F(ACC3);

		if ( _C == 0 )
		{
            asm ("moviw   4[INDF1]");
			_ADDWF_F(FSR0L);
            asm ("moviw   3[INDF1]");
			_ADDWFC_F(FSR0H);
            asm ("moviw   2[INDF1]");
            _ADDWFC_F(ACC3);
			ACC0 &= ~1;
		}
	} while ( --INDF1 );

	asm ("addfsr  1, 1");

	if ( INDF1 & 1 ) // mod operation?
	{
		ACC0 = FSR0L;
		ACC1 = FSR0H;
        ACC2 = ACC3;
	}

	if ( INDF1 & 4 )	// negative result?
        _negACC24();

    asm ("addfsr  1, 4");
}

void _divmod32(void)
{
	asm ("movwi   --INDF1");	// operating options

	if ( INDF1 & 4 )	// x is signed?
	{
        if ( ACC3 & 0x80 )
            _negACC32();
		else
			INDF1 &= ~4;
	}

    FSR0 = 0;
	if ( INDF1 & 2 )	// y is signed?
	{
		asm ("moviw   1[INDF1]");
		if ( WREG & 0x80 )
		{
			INDF1 ^= 4;

            asm ("moviw   4[INDF1]");
			_SUBWF_W (FSR0L);
            asm ("movwi   4[INDF1]");

            asm ("moviw   3[INDF1]");
            _SUBWFB_W(FSR0L);
            asm ("movwi   3[INDF1]");

            asm ("moviw   2[INDF1]");
            _SUBWFB_W(FSR0L);
            asm ("movwi   2[INDF1]");

            asm ("moviw   1[INDF1]");
            _SUBWFB_W(FSR0L);
            asm ("movwi   1[INDF1]");
		}
	}

	WREG = 0;
	asm ("movwi   --INDF1");	// ACC4
	asm ("movwi   --INDF1");	// ACC5

    WREG = 32;   // loop count = 32
    asm ("movwi   --INDF1");

	do
	{
		_C = 1;
		_RLF_F(ACC0);
		_RLF_F(ACC1);
        _RLF_F(ACC2);
        _RLF_F(ACC3);
		_RLF_F(FSR0L);
		_RLF_F(FSR0H);
		asm ("moviw	2[INDF1]");	_RLF_F(WREG); asm ("movwi	2[INDF1]"); // ACC4
		asm ("moviw	1[INDF1]");	_RLF_F(WREG); asm ("movwi	1[INDF1]"); // ACC5

		asm ("moviw 7[INDF1]"); _SUBWF_F(FSR0L);
		asm ("moviw 6[INDF1]"); _SUBWFB_F(FSR0H);

		asm ("moviw	2[INDF1]");	asm ("movwi  --INDF1");
        asm ("moviw 6[INDF1]"); _SUBWFB_W(INDF1); asm ("movwi 3[INDF1]");

		asm ("moviw	2[INDF1]");	asm ("movwi  --INDF1");
        asm ("moviw 6[INDF1]"); _SUBWFB_W(INDF1); asm ("movwi 3[INDF1]");

		if ( _C == 0 )
		{
            asm ("moviw 9[INDF1]");	_ADDWF_F(FSR0L);
            asm ("moviw 8[INDF1]");	_ADDWFC_F(FSR0H);
            asm ("moviw 1[INDF1]");	asm ("movwi 4[INDF1]");
			asm ("movf  INDF1, W");	asm ("movwi 3[INDF1]");
			ACC0 &= ~1;
		}
		asm ("addfsr 1, 2");
	} while ( --INDF1 );

	asm ("addfsr  1, 1");		//	_ADDFSR (1, 1);
	asm ("moviw   2[INDF1]");
	if ( WREG & 1 ) 			// mod operation?
	{
		ACC0 = FSR0L;
		ACC1 = FSR0H;
		asm ("moviw 1[INDF1]");	ACC2 = WREG;
        asm ("movf  INDF1, W");	ACC3 = WREG;
	}

	asm ("addfsr  1, 2");

	if ( INDF1 & 4 )	// negative result?
        _negACC32 ();

    asm ("addfsr  1, 5");       //  _ADDFSR (1, 5);
}

void _divmod8u(void)
{
}

void _divmod16u(void)
{
}

void _divmod24u(void)
{
}

void _divmod32u(void)
{
}

void _signedCmp(void)
{
	WREG = STATUS;			// WREG := STATUS
	_BTFSC(INDF1, 7);
	WREG ^= 1;				// Carry ^= 1 if Sx1 != Sx2
	asm("addfsr FSR1, 1");
	_BTFSC(WREG, 2);		// jump if Z = 1 or C = 0
	WREG &= ~1;				//	asm16e->code(_BCF, aWREG, 0);
}

////////////////////////////////////////////////////////
static void _negACC24(void)
{
    _COMF_F(ACC0);
    _COMF_F(ACC1);
    _COMF_F(ACC2);
    _INCF_F(ACC0);
    _BTFSC (STATUS, 2);
    _INCF_F(ACC1);
    _BTFSC (STATUS, 2);
    _INCF_F(ACC2);
}

static void _negACC32(void)
{
    _COMF_F(ACC0);
    _COMF_F(ACC1);
    _COMF_F(ACC2);
    _COMF_F(ACC3);

    WREG = 1;
    _ADDWF_F(ACC0);
    WREG = 0;
    _ADDWFC_F(ACC1);
    _ADDWFC_F(ACC2);
    _ADDWFC_F(ACC3);
}

void _saveFSR1(void)
{
	_BTFSC(FLAG_LOC, 0);
	asm("bra  .+7");
	_MOVF_W(FSR1L);
	_MOVWF(BUFFERED_FSR1L);
	_MOVF_W(FSR1H);
	_MOVWF(BUFFERED_FSR1H);
	_BSF(FLAG_LOC, 1);
	asm("return");
	_MOVF_W(FSR1L);
	_MOVWF(ISR_FSR1L);
	_MOVF_W(FSR1H);
	_MOVWF(ISR_FSR1H);
}

void _restoreFSR1(void)
{
	_BTFSC(FLAG_LOC, 0);
	asm("bra  .+7");
	_MOVF_W(BUFFERED_FSR1L);
	_MOVWF(FSR1L);
	_MOVF_W(BUFFERED_FSR1H);
	_MOVWF(FSR1H);
	_BCF(FLAG_LOC, 1);
	asm("return");
	_MOVF_W(ISR_FSR1L);
	_MOVWF(FSR1L);
	_MOVF_W(ISR_FSR1H);
	_MOVWF(FSR1H);
}
