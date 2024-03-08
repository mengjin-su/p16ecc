#include <p16f1455.h>
#include "GenericTypeDefs.h"
#include "usb_hal_pic16f1.h"
#include "usb_device.h"
#include "usb_function_cdc.h"
//#include "tool.h"
//#include "spi.h"
//#include "mcp2515.h"
//#include "term.h"
#include "uart.h"

const BYTE parityTable[256] = {
	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
	0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,
	1,0,0,1,0,1,1,0,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,
};

unsigned char uartRxLen;	// USB -> UART
unsigned char uartRxPos;
unsigned char uartTxHead;	// USB <- UART
unsigned char uartTxTail;

#define UART_BUFFER_SIZE	64
_linear_ char uartRxBuffer[UART_BUFFER_SIZE];	// USB -> UART
_linear_ char uartTxBuffer[UART_BUFFER_SIZE];	// USB <- UART
char  uartFormat;
char *usbBuffer;

void UART_init(void)
{
	TRISC 	|=  (1 << 3);
	TRISC 	|=  (1 << 5);	//    UART_TRISRx=1;				// RX
	TRISC 	&= ~(1 << 4);	//    UART_TRISTx=0;				// TX
    TXSTA 	 = 0x24;       	// TX enable BRGH=1
    RCSTA 	 = 0x90;       	// Single Character RX
    SPBRGL 	 = 0x71;
    SPBRGH 	 = 0x02;      	// 0x0271 for 48MHz -> 19200 baud
    BAUDCON  = 0x08;     	// BRG16 = 1
    WREG = RCREG;			// read (clear RCIF)
	CDCSetBaudRate(19200);	// baud = (48000000/4)/baud - 1;


//	uartRxBuffer = (char*)canRxQueue;				// USB -> UART
	uartRxPos = 0;
	uartRxLen = 0;

//	uartTxBuffer = &uartRxBuffer[UART_BUFFER_SIZE];	// USB <- UART
	uartTxHead = 0;
	uartTxTail = 0;

	usbBuffer  = &uartTxBuffer[UART_BUFFER_SIZE];
	uartFormat = 0;
}

void UART_task(void)
{
	BYTE i, len, tail;

    if ( USBDeviceState < CONFIGURED_STATE || USBSuspendControl == 1 )
        return;

	if ( (uartRxPos < uartRxLen) && (TXIF || TRMT) )	// anything to send to UART?
	{
		BYTE c = uartRxBuffer[uartRxPos];
		BYTE parity = parityTable[c];
		uartRxPos++;

		switch ( uartFormat )
		{
			case 1:		// Odd
				TX9D = 1;		// TXSTA |= 1;
				if ( parity ) TX9D = 0;
				break;

			case 2:	// Even
				TX9D = 0;		// TXSTA &= 0xfe;
				if ( parity ) TX9D = 1;
				break;
		}

		TXREG = c;
	}

	if ( RCIF )
	{
		uartTxBuffer[uartTxTail] = RCREG;
		tail = (uartTxTail + 1) & (UART_BUFFER_SIZE - 1);
		if ( tail != uartTxHead ) uartTxTail = tail;
	}

	if ( uartRxPos >= uartRxLen )
		uartRxPos = uartRxLen = 0;

	// anything from USB(PC host)?
	uartRxLen += getUSBData(&uartRxBuffer[uartRxLen], UART_BUFFER_SIZE - uartRxLen);

	// anything to USB(PC host)?
	if ( USBUSARTIsTxTrfReady() && uartTxTail != uartTxHead )
	{
		len = UART_BUFFER_SIZE;
		if ( uartTxTail > uartTxHead ) len = uartTxTail;
		len -= uartTxHead;

		putUSBUSART(&uartTxBuffer[uartTxHead], len);
		uartTxHead += len;
		uartTxHead &= (UART_BUFFER_SIZE - 1);
	}

	CDCTxService();
}

void UART_put(char c)
{
	if ( uartRxPos < 64 )
		uartRxBuffer[uartRxPos++] = c;
}