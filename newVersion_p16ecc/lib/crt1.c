#include <pic16e.h>

/*
	float value is in IEEE-754 standard format:
		1-bit sign;
		8-bit exponent;
		23-bit mantissa (fraction).
*/

#define C_FLAG_SET	(STATUSbits->C == 1)	//(STATUS & 1)
#define Z_FLAG_SET	(STATUSbits->Z == 1) 	// (STATUS & 4)
#define IS_ACC_ZERO	(*(long*)&_ACC0_ == 0)

void _floatAdd(void);
void _floatSub(void);

static void _floatIsStackZero()
{
	asm("moviw	3[INDF1]");
	FSR0L = WREG;
	asm("moviw	2[INDF1]");
	FSR0L |= WREG;
	asm("moviw	1[INDF1]");
	asm("iorwf	FSR0L, W");
	asm("iorwf	INDF1, W");
}

static void _floatAlignRadix()
{
	asm("moviw	1[INDF1]");
	asm("rlf	WREG, W");
	asm("rlf	INDF1, W");
	asm("movwf 	FSR0L");
	asm("movwi	--INDF1");			// X's exponent

	asm("rlf	0x72, W");
	asm("rlf	0x73, W");
	asm("movwi	--INDF1");			// Y's exponent

	asm("subwf	FSR0L, F");
	asm("movf	STATUS, W");		// save STATUS (b0 = C flag)
	asm("andlw	0x05");				// only keep C and Z flags
	FSR0H = WREG;

	asm("movf	INDF1, W");			// get Y's exponent
	if ( FSR0H & 1 ) 				// X >= Y, then
		asm("moviw	1[INDF1]");		// get X's exponent

	_ACC3_ = WREG;
	asm("addfsr	1, 2");

	asm("moviw	1[INDF1]");			// recover inherited integer part
	asm("iorlw	0x80");				// of matissas
	asm("movwi	1[INDF1]");
	_ACC2_ |= 0x80;

	while ( FSR0L )
	{
		if ( FSR0H & 1 )	// X >= Y
		{
			*(unsigned short*)&_ACC0_ >>= 1;
			FSR0L--;
		}
		else				// X < Y
		{
			asm("moviw	1[INDF1]");
			asm("lsrf	WREG, W");
			asm("movwi	1[INDF1]");
			asm("moviw	2[INDF1]");
			asm("rrf	WREG, W");
			asm("movwi	2[INDF1]");
			asm("moviw	3[INDF1]");
			asm("rrf	WREG, W");
			asm("movwi	3[INDF1]");
			FSR0L++;
		}
	}
}

static void _negACCInt4()
{
	*(long*)&_ACC0_ = -*(long*)&_ACC0_;
}

static void _negStackInt4()
{
	asm("moviw	3[INDF1]");
	asm("sublw	0");
	asm("movwi	3[INDF1]");
	asm("moviw	2[INDF1]");
	asm("subwfb	0x7f, W");
	asm("movwi	2[INDF1]");
	asm("moviw	1[INDF1]");
	asm("subwfb	0x7f, W");
	asm("movwi	1[INDF1]");
	asm("movf	INDF1, W");
	asm("subwfb	0x7f, W");
	asm("movwf	INDF1");
}

/////////////////////////////////////////////////////////////////////
void _uint4ToFloatACC(void)
{
	if ( IS_ACC_ZERO )
		return;

	FSR0L = 127 + 23;
	while ( _ACC3_ ) {
		*(unsigned long*)&_ACC0_ >>= 1;
		FSR0L++;
	}
	while ( !(_ACC2_ & 0x80) ) {
		*(unsigned short*)&_ACC0_ <<= 1;
		FSR0L--;
	}

	if ( !(FSR0L & 1) ) _ACC2_ &= 0x7f;
	_ACC3_ = FSR0L >> 1;
}

void _int4ToFloatACC(void)
{
	if ( IS_ACC_ZERO )
		return;

	FSR0H = 0;
	if ( _ACC3_ & 0x80 )		// negtive value?
	{
		_negACCInt4();
		FSR0H |= 0x80;			// set the sign bit
	}

	FSR0L = 127 + 23;
	while ( _ACC3_ ) {
		*(unsigned long*)&_ACC0_ >>= 1;
		FSR0L++;
	}
	while ( !(_ACC2_ & 0x80) ) {
		*(unsigned short*)&_ACC0_ <<= 1;
		FSR0L--;
	}

	if ( !(FSR0L & 1) ) _ACC2_ &= 0x7f;
	_ACC3_  = (FSR0L >> 1);
	_ACC3_ |= FSR0H;
}

void _uint4ToFloatStack()
{
	_floatIsStackZero();
	if ( !Z_FLAG_SET )
	{
		FSR0L = 127 + 23;
		while ( INDF1 )			// MSB
		{
			asm("lsrf	INDF1, F");
			asm("moviw	1[INDF1]");
			asm("rrf	WREG, W");
			asm("movwi	1[INDF1]");
			asm("moviw	2[INDF1]");
			asm("rrf	WREG, W");
			asm("movwi	2[INDF1]");
			asm("moviw	3[INDF1]");
			asm("rrf	WREG, W");
			asm("movwi	3[INDF1]");
			FSR0L++;
		}
		asm("moviw	1[INDF1]");
		while ( !(WREG & 0x80) )
		{
			asm("moviw	3[INDF1]");
			asm("lslf	WREG, W");
			asm("movwi	3[INDF1]");
			asm("moviw	2[INDF1]");
			asm("rlf	WREG, W");
			asm("movwi	2[INDF1]");
			asm("moviw	1[INDF1]");
			asm("rlf	WREG, W");
			asm("movwi	1[INDF1]");
			FSR0L--;
		}

		if ( !(FSR0L & 1) ) {
//			asm("moviw	1[INDF1]");
			asm("andlw	0x7f");
			asm("movwi	1[INDF1]");
		}
		INDF1 = FSR0L >> 1;
	}
}

void _int4ToFloatStack()
{
	_floatIsStackZero();
	if ( !Z_FLAG_SET )
	{
		FSR0H = 0;
		if ( INDF1 & 0x80 )		// negtive value?
		{
			_negStackInt4();
			FSR0H |= 0x80;		// keep the sign
		}

		FSR0L = 127 + 23;
		while ( INDF1 )			// MSB
		{
			asm("lsrf	INDF1, F");
			asm("moviw	1[INDF1]");
			asm("rrf	WREG, W");
			asm("movwi	1[INDF1]");
			asm("moviw	2[INDF1]");
			asm("rrf	WREG, W");
			asm("movwi	2[INDF1]");
			asm("moviw	3[INDF1]");
			asm("rrf	WREG, W");
			asm("movwi	3[INDF1]");
			FSR0L++;
		}
		asm("moviw	1[INDF1]");
		while ( !(WREG & 0x80) )
		{
			asm("moviw	3[INDF1]");
			asm("lslf	WREG, W");
			asm("movwi	3[INDF1]");
			asm("moviw	2[INDF1]");
			asm("rlf	WREG, W");
			asm("movwi	2[INDF1]");
			asm("moviw	1[INDF1]");
			asm("rlf	WREG, W");
			asm("movwi	1[INDF1]");
			FSR0L--;
		}

		if ( !(FSR0L & 1) ) {
//			asm("moviw	1[INDF1]");
			asm("andlw	0x7f");
			asm("movwi	1[INDF1]");
		}

		WREG  = FSR0L >> 1;
		INDF1 = WREG | FSR0H;
	}
}

void _floatToInt4ACC(void)
{
	if ( IS_ACC_ZERO )
		return;

	FSR0H = _ACC3_;		// save the sign
	asm("rlf	0x72, W");
	asm("rlf	0x73, W");
	FSR0L = WREG;		// exponent byte

	if ( FSR0L < (0x7f - 23) )
	{
		*(long*)&_ACC0_ = 0;
		return;
	}

	FSR0L  -= 23;
	_ACC2_ |= 0x80;
	_ACC3_  = 0;

	while ( FSR0L & 0x80 )
	{
		*(unsigned long*)&_ACC0_ <<= 1;
		FSR0L--;
	}

	while ( FSR0L != 0x7f )
	{
		*(unsigned short*)&_ACC0_ >>= 1;
		FSR0L++;
	}

	if ( FSR0H & 0x80 )
		_negACCInt4();
}

void _floatEqu(void)
{
	asm("moviw	3[INDF1]");		// LSB
	asm("xorwf	0x70, W");
	asm("movwf	FSR0L");

	asm("moviw	2[INDF1]");
	asm("xorwf	0x71, W");
	asm("iorwf	FSR0L, F");

	asm("moviw	1[INDF1]");
	asm("xorwf	0x72, W");
	asm("iorwf	FSR0L, F");

	asm("movf	INDF1, W");		// MSB
	asm("xorwf	0x73, W");
	asm("iorwf	FSR0L, W");

	asm("addfsr 1, 4");
}

void _floatAdd(void)
{
	_floatIsStackZero();
	if ( Z_FLAG_SET )
	{
		asm("addfsr 1, 4");
		return;
	}

	if ( IS_ACC_ZERO )
	{
		asm("moviw	INDF1++");	_ACC3_ = WREG;	// MSB
		asm("moviw	INDF1++");	_ACC2_ = WREG;
		asm("moviw	INDF1++");	_ACC1_ = WREG;
		asm("moviw	INDF1++");	_ACC0_ = WREG;	// LSB
		return;
	}

	WREG = _ACC3_;
	asm("xorwf	INDF1, W");
	if ( WREG & 0x80 )
	{
		_ACC3_ ^= 0x80;
		_floatSub();
		return;
	}

	_floatAlignRadix();
	asm("moviw	3[INDF1]");
	asm("addwf	0x70, F");
	asm("moviw	2[INDF1]");
	asm("addwfc	0x71, F");
	asm("moviw	1[INDF1]");
	asm("addwfc	0x72, F");

	if ( C_FLAG_SET ) {
		(*(unsigned short*)&_ACC0_) >>= 1;
		if ( _ACC3_ != 0xff ) _ACC3_++;
	}

	_ACC2_ &= 0x7f;
	_ACC3_ >>= 1;
	if ( C_FLAG_SET ) _ACC2_ |= 0x80;
	if ( INDF1 & 0x80 )	_ACC3_ |= 0x80;

	asm("addfsr 1, 4");
}

void _floatSub(void)
{
	if ( IS_ACC_ZERO )
	{
		asm("moviw	INDF1++");	_ACC3_ = WREG;	// MSB
		asm("moviw	INDF1++");	_ACC2_ = WREG;
		asm("moviw	INDF1++");	_ACC1_ = WREG;
		asm("moviw	INDF1++");	_ACC0_ = WREG;	// LSB
		return;
	}

	_floatIsStackZero();
	if ( Z_FLAG_SET )
	{
		_ACC3_ ^= 0x80;
		asm("addfsr 1, 4");
		return;
	}

	WREG = _ACC3_;
	asm("xorwf	INDF1, W");
	if ( WREG & 0x80 )
	{
		_ACC3_ ^= 0x80;
		_floatAdd();
		return;
	}

	_floatAlignRadix();

	asm("moviw	3[INDF1]");	FSR0L = WREG;
	WREG = _ACC0_;	asm("subwf	FSR0L, W");	_ACC0_ = WREG;
	asm("moviw	2[INDF1]");	FSR0L = WREG;
	WREG = _ACC1_;	asm("subwfb	FSR0L, W");	_ACC1_ = WREG;
	asm("moviw	1[INDF1]");	FSR0L = WREG;
	WREG = _ACC2_;	asm("subwfb	FSR0L, W");	_ACC2_ = WREG;

	if ( !C_FLAG_SET )
	{
		if ( FSR0H == 1 )
			_ACC3_--;
		else
		{
			*(unsigned short*)&_ACC0_ = -*(unsigned short*)&_ACC0_;
			INDF1 ^= 0x80;
		}
	}

	while ( !(_ACC2_ & 0x80) && _ACC3_ )
	{
		*(unsigned short*)&_ACC0_ <<= 1;
		_ACC3_--;
	}

	if ( _ACC3_ == 0 )	// result exponent = 0?
		*(unsigned short*)&_ACC0_ = 0;
	else
	{
		_ACC2_ &= 0x7f;
		_ACC3_ >>= 1;
		if ( C_FLAG_SET ) _ACC2_ |= 0x80;
		if ( INDF1 & 0x80 ) _ACC3_ |= 0x80;
	}

	asm("addfsr 1, 4");
}

void _floatMul(void)
{
	if ( IS_ACC_ZERO )
		asm("bra	_floatMul_$L0");

	_floatIsStackZero();
	if ( Z_FLAG_SET )
	{
asm("_floatMul_$L0:");
		asm("addfsr 1, 4");
asm("_floatMul_$L1:");
		*(long*)&_ACC0_ = 0;
		return;
	}

	FSR0H = INDF1 ^ _ACC3_;		// save the sign of product
	asm("moviw	1[INDF1]");
	asm("rlf	WREG, W");
	asm("rlf	INDF1, W");
	FSR0L = WREG;				// save exponent of product
	asm("rlf	0x72, W");
	asm("rlf	0x73, W");
	asm("addwf	FSR0L, F");
	if ( !C_FLAG_SET )
	{
		asm("movlw	0x7f");
		asm("subwf	FSR0L, W");
		if ( !C_FLAG_SET )	//
			asm("bra	_floatMul_$L0");
	}
	FSR0L -= 0x7f;

	asm("addfsr	1, 1");			// remove

	INDF1  |= 0x80;
	_ACC2_ |= 0x80;

	WREG = 0;
	WREG = _ACC0_; asm("movwi	--INDF1");	_ACC0_ = 0;
	WREG = _ACC1_; asm("movwi	--INDF1");	_ACC1_ = 0;
	WREG = _ACC2_; asm("movwi	--INDF1");	_ACC2_ = 0;

	STATUS = 0;
	for (_ACC3_ = 24; _ACC3_; _ACC3_--)
	{
		asm("rrf	0x72, F");
		asm("rrf	0x71, F");
		asm("rrf	0x70, F");

		asm("moviw	3[INDF1]");
		asm("rrf	WREG, W");
		asm("movwi	3[INDF1]");
		asm("moviw	4[INDF1]");
		asm("rrf	WREG, W");
		asm("movwi	4[INDF1]");
		asm("moviw	5[INDF1]");
		asm("rrf	WREG, W");
		asm("movwi	5[INDF1]");

		if ( C_FLAG_SET )
		{
			asm("moviw	2[INDF1]");
			asm("addwf	0x70, F");
			asm("moviw	1[INDF1]");
			asm("addwfc	0x71, F");
			asm("movf	INDF1, W");
			asm("addwfc	0x72, F");
		}
	}

	if ( C_FLAG_SET )
	{
		*(unsigned short*)&_ACC0_ >>= 1;
		FSR0L++;
	}

	asm("addfsr	1, 6");
	if ( FSR0L == 0 ) asm("bra	_floatMul_$L1");

	_ACC2_ &= 0x7f;
	FSR0L >>= 1;
	if ( C_FLAG_SET ) _ACC2_ |= 0x80;
	if ( FSR0H & 0x80 ) FSR0L |= 0x80;
	_ACC3_ = FSR0L;
}

void _floatDiv(void)
{
	FSR0H  = INDF1 ^ _ACC3_;	// save the sign of product
	FSR0H &= 0x80;

	_floatIsStackZero();
	if ( Z_FLAG_SET )
	{
asm("_floatDiv_$L0:");
		asm("addfsr 1, 4");
		*(long*)&_ACC0_ = 0;
		return;
	}

	if ( IS_ACC_ZERO )
	{
		asm("addfsr 1, 4");
		asm("bra	_floatDiv_$L1");
	}

	asm("moviw	1[INDF1]");
	asm("rlf	WREG, W");
	asm("rlf	INDF1, W");
	FSR0L = WREG;				// save exponent of product
	asm("rlf	0x72, W");
	asm("rlf	0x73, W");
	_ACC3_ = WREG;

	if ( FSR0L < _ACC3_ )
	{
		WREG = _ACC3_ - FSR0L;
		if ( WREG & 0x80 )		// > 127
			asm("bra	_floatDiv_$L0");

		FSR0L -= _ACC3_;
	}
	else
	{
		FSR0L -= _ACC3_;
		if ( FSR0L > 0x81 )
		{
			asm("addfsr	1, 4");
			asm("bra	_floatDiv_$L1");
		}

		if ( Z_FLAG_SET )
			FSR0H |= 4;
	}

	FSR0L +=  0x7f;

	asm("addfsr	1, 1");
	INDF1  |= 0x80;
	_ACC2_ |= 0x80;

	WREG = _ACC0_;	asm("movwi	--INDF1");
	WREG = _ACC1_;	asm("movwi	--INDF1");
	WREG = _ACC2_;	asm("movwi	--INDF1");
	asm("moviw	3[INDF1]");	_ACC2_ = WREG;
	asm("moviw	4[INDF1]");	_ACC1_ = WREG;
	asm("moviw	5[INDF1]");	_ACC0_ = WREG;

	for (_ACC3_ = 0; _ACC3_ < 25; _ACC3_++)
	{
		asm("moviw	5[INDF1]");	asm("lslf	WREG, W");	asm("movwi	5[INDF1]");
		asm("moviw	4[INDF1]");	asm("rlf	WREG, W");	asm("movwi	4[INDF1]");
		asm("moviw	3[INDF1]");	asm("rlf	WREG, W");	asm("movwi	3[INDF1]");

		asm("moviw	2[INDF1]");
		asm("subwf	0x70, F");
		asm("moviw	1[INDF1]");
		asm("subwfb	0x71, F");
		asm("moviw	0[INDF1]");
		asm("subwfb	0x72, F");
		if ( !C_FLAG_SET && !(FSR0H & 1) )
		{
			asm("moviw	2[INDF1]");
			asm("addwf	0x70, F");
			asm("moviw	1[INDF1]");
			asm("addwfc	0x71, F");
			asm("moviw	0[INDF1]");
			asm("addwfc	0x72, F");
		}
		else
		{
			asm("moviw	5[INDF1]");
			asm("iorlw	1");
			asm("movwi	5[INDF1]");
			if ( _ACC3_ == 0 ) 	// 1'st subtraction?
			{
				if ( FSR0H & 4 )
				{
					asm("addfsr	1, 3");
					asm("bra	_floatDiv_$L1");
				}

				FSR0H |= 2;		// quotion >= 1.0
				_ACC3_++;		// loop times = 24
			}
		}

		FSR0H &= ~1;
		*(unsigned short*)&_ACC0_ <<= 1;
		if ( C_FLAG_SET ) FSR0H |= 1;
	}

	asm("addfsr	1, 3");
	asm("moviw	INDF1++");	_ACC2_ = WREG;
	asm("moviw	INDF1++");	_ACC1_ = WREG;
	asm("moviw	INDF1++");	_ACC0_ = WREG;

	if ( !(FSR0H & 2) && FSR0L )
		FSR0L--;

	if ( FSR0L == 0 )	// overflow!
	{
asm("_floatDiv_$L1:");
		_ACC3_ = FSR0H | 0x7f;
		_ACC2_ = 0xff;
		_ACC1_ = 0xff;
		_ACC0_ = 0xff;
	}
	else
	{
		_ACC2_ &= 0x7f;
		FSR0L >>= 1;
		if ( C_FLAG_SET ) _ACC2_ |= 0x80;
		if ( FSR0H & 0x80 ) FSR0L |= 0x80;
		_ACC3_ = FSR0L;
	}
}

void _floatCmpJLT()
{
	if ( INDF1 & 0x80 )	// x < 0
	{
		if ( !(_ACC3_ & 0x80) )
		{
			asm("addfsr	1, 4");
			STATUS |= 1;
			return;
		}
		asm("moviw	3[INDF1]");
		asm("subwf	0x70, F");
		asm("moviw	2[INDF1]");
		asm("subwfb	0x71, F");
		asm("moviw	1[INDF1]");
		asm("subwfb	0x72, F");
		asm("moviw	0[INDF1]");
		asm("subwfb	0x73, F");
	}
	else
	{
		if ( (_ACC3_ & 0x80) )
		{
			asm("addfsr	1, 4");
			STATUS = 0;
			return;
		}
		asm("moviw	3[INDF1]");
		asm("subwf	0x70, F");
		asm("moviw	2[INDF1]");
		asm("subwfb	0x71, F");
		asm("moviw	1[INDF1]");
		asm("subwfb	0x72, F");
		asm("moviw	0[INDF1]");
		asm("subwfb	0x73, F");
		asm("comf	STATUS, F");
	}

	asm("addfsr	1, 4");
	if ( IS_ACC_ZERO ) STATUS = 0;	// clear C flag
}

void _floatCmpJLE()
{
	if ( INDF1 & 0x80 )	// x < 0
	{
		if ( !(_ACC3_ & 0x80) )
		{
			asm("addfsr	1, 4");
			STATUS |= 1;
			return;
		}
		asm("moviw	3[INDF1]");
		asm("subwf	0x70, F");
		asm("moviw	2[INDF1]");
		asm("subwfb	0x71, F");
		asm("moviw	1[INDF1]");
		asm("subwfb	0x72, F");
		asm("moviw	0[INDF1]");
		asm("subwfb	0x73, F");
	}
	else				// x >= 0
	{
		if ( (_ACC3_ & 0x80) )
		{
			asm("addfsr	1, 4");
			STATUS = 0;
			return;
		}

		asm("moviw	3[INDF1]");
		asm("subwf	0x70, F");
		asm("moviw	2[INDF1]");
		asm("subwfb	0x71, F");
		asm("moviw	1[INDF1]");
		asm("subwfb	0x72, F");
		asm("moviw	0[INDF1]");
		asm("subwfb	0x73, F");
		asm("comf	STATUS, F");
	}
	asm("addfsr	1, 4");
	if ( IS_ACC_ZERO ) STATUS |= 1;	// set C flag
}