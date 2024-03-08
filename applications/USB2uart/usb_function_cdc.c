#include <p16f1455.h>
#include "GenericTypeDefs.h"
#include "usb_common.h"
#include "usb_ch9.h"
#include "usb_hal_pic16f1.h"
#include "usb_function_cdc.h"
#include "usb_device.h"


/** V A R I A B L E S ********************************************************/
#define IN_DATA_BUFFER_ADDRESS 		0x2140
#define OUT_DATA_BUFFER_ADDRESS 	0x2190
#define LINE_CODING_ADDRESS 		0x20A0
#define NOTICE_ADDRESS 				(LINE_CODING_ADDRESS + LINE_CODING_LENGTH)

unsigned char cdc_data_tx[CDC_DATA_IN_EP_SIZE]  @IN_DATA_BUFFER_ADDRESS;
unsigned char cdc_data_rx[CDC_DATA_OUT_EP_SIZE] @OUT_DATA_BUFFER_ADDRESS;

LINE_CODING line_coding @LINE_CODING_ADDRESS;    // Buffer to store line coding information
CDC_NOTICE cdc_notice   @NOTICE_ADDRESS;

BYTE cdc_rx_len;            // total rx length
BYTE cdc_trf_state;         // States are defined cdc.h
POINTER pCDCSrc;            // Dedicated source pointer
POINTER pCDCDst;            // Dedicated destination pointer
BYTE cdc_tx_len;            // total tx length
BYTE cdc_mem_type = 0;      // _ROM(0), _RAM(1)

void * CDCDataOutHandle;
void * CDCDataInHandle;

BYTE control_signal_bitmap;
DWORD BaudRateGen;			// BRG value calculated from baudrate

/**************************************************************************
  SEND_ENCAPSULATED_COMMAND and GET_ENCAPSULATED_RESPONSE are required
  requests according to the CDC specification.
  However, it is not really being used here, therefore a dummy buffer is
  used for conformance.
 **************************************************************************/
#define dummy_length    0x08
BYTE dummy_encapsulated_cmd_response[dummy_length];

#ifdef USB_CDC_SET_LINE_CODING_HANDLER
void USB_CDC_SET_LINE_CODING_HANDLER(void);
#endif

/** P R I V A T E  P R O T O T Y P E S ***************************************/
void USBCDCSetLineCoding(void);


/** C L A S S  S P E C I F I C  R E Q ****************************************/
/******************************************************************************
 	Function:
 		void USBCheckCDCRequest(void)

 	Description:
 		This routine checks the most recently received SETUP data packet to
 		see if the request is specific to the CDC class.  If the request was
 		a CDC specific request, this function will take care of handling the
 		request and responding appropriately.

 	PreCondition:
 		This function should only be called after a control transfer SETUP
 		packet has arrived from the host.

	Parameters:
		None

	Return Values:
		None

	Remarks:
		This function does not change status or do anything if the SETUP packet
		did not contain a CDC class specific request.
  *****************************************************************************/
void USBCheckCDCRequest(void)
{
    /*
     * If request recipient is not an interface then return
     */
    if ( SetupPkt__Recipient != USB_SETUP_RECIPIENT_INTERFACE_BITFIELD ) return;

    /*
     * If request type is not class-specific then return
     */
    if ( SetupPkt__RequestType != USB_SETUP_TYPE_CLASS_BITFIELD ) return;

    /*
     * Interface ID must match interface numbers associated with
     * CDC class, else return
     */
    if ( SetupPkt.id.bIntfID != CDC_COMM_INTF_ID &&
         SetupPkt.id.bIntfID != CDC_DATA_INTF_ID  ) return;

    switch ( SetupPkt.req.bRequest )
    {
        //****** These commands are required ******//
        case SEND_ENCAPSULATED_COMMAND:
         //send the packet
            inPipe.pSrc.bRam  = (BYTE*)dummy_encapsulated_cmd_response;
            inPipe.wCount = dummy_length;
            inPipe.info &= ~PIPE_CTRL_TRF_MEM_MASK;
            inPipe.info |= USB_EP0_RAM;
            inPipe.info |= PIPE_BUSY_MASK;
            break;
        case GET_ENCAPSULATED_RESPONSE:
            // Populate dummy_encapsulated_cmd_response first.
            inPipe.pSrc.bRam = (BYTE*)dummy_encapsulated_cmd_response;
            inPipe.info |= PIPE_BUSY_MASK;
            break;
        //****** End of required commands ******//

        case SET_LINE_CODING:
            outPipe.wCount = SetupPkt.req.wLength;
            outPipe.pDst.bRam = (BYTE*)LINE_CODING_TARGET;
            outPipe.pFunc = LINE_CODING_PFUNC;
            outPipe.info |= PIPE_BUSY_MASK;
            break;

        case GET_LINE_CODING:
            USBEP0SendRAMPtr(
                (BYTE*)&line_coding,
                LINE_CODING_LENGTH,
                USB_EP0_INCLUDE_ZERO);
            break;

        case SET_CONTROL_LINE_STATE:
            control_signal_bitmap = (BYTE)SetupPkt.val.W_Value;
            inPipe.info |= PIPE_BUSY_MASK;
            break;

        default:
            break;
    }//end switch(SetupPkt.bRequest)

}//end USBCheckCDCRequest

/** U S E R  A P I ***********************************************************/

/**************************************************************************
  Function:
        void CDCInitEP(void)

  Summary:
    This function initializes the CDC function driver. This function should
    be called after the SET_CONFIGURATION command (ex: within the context of
    the USBCBInitEP() function).
  Description:
    This function initializes the CDC function driver. This function sets
    the default line coding (baud rate, bit parity, number of data bits,
    and format). This function also enables the endpoints and prepares for
    the first transfer from the host.

    This function should be called after the SET_CONFIGURATION command.
    This is most simply done by calling this function from the
    USBCBInitEP() function.

    Typical Usage:
    <code>
        void USBCBInitEP(void)
        {
            CDCInitEP();
        }
    </code>
  Conditions:
    None
  Remarks:
    None
  **************************************************************************/
void CDCInitEP(void)
{
   	//Abstract line coding information
   	line_coding.info.dwDTERate = 19200;      // baud rate
   	line_coding.info.bCharFormat = 0x00;             // 1 stop bit
   	line_coding.info.bParityType = 0x00;             // None
   	line_coding.info.bDataBits = 0x08;               // 5,6,7,8, or 16

    cdc_rx_len = 0;

    USBEnableEndpoint(CDC_COMM_EP,USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
    USBEnableEndpoint(CDC_DATA_EP,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);

    CDCDataOutHandle = USBRxOnePacket(CDC_DATA_EP,(BYTE*)cdc_data_rx,sizeof(cdc_data_rx));
    CDCDataInHandle = NULL;

    cdc_trf_state = CDC_TX_READY;
}//end CDCInitEP

/**********************************************************************************
  Function:
        BYTE getsUSBUSART(char *buffer, BYTE len)

  Summary:
    getsUSBUSART copies a string of BYTEs received through USB CDC Bulk OUT
    endpoint to a user's specified location. It is a non-blocking function.
    It does not wait for data if there is no data available. Instead it
    returns '0' to notify the caller that there is no data available.

  Description:
    getsUSBUSART copies a string of BYTEs received through USB CDC Bulk OUT
    endpoint to a user's specified location. It is a non-blocking function.
    It does not wait for data if there is no data available. Instead it
    returns '0' to notify the caller that there is no data available.

    Typical Usage:
    <code>
        BYTE numBytes;
        BYTE buffer[64]

        numBytes = getsUSBUSART(buffer,sizeof(buffer)); //until the buffer is free.
        if(numBytes \> 0)
        {
            //we received numBytes bytes of data and they are copied into
            //  the "buffer" variable.  We can do something with the data
            //  here.
        }
    </code>
  Conditions:
    Value of input argument 'len' should be smaller than the maximum
    endpoint size responsible for receiving bulk data from USB host for CDC
    class. Input argument 'buffer' should point to a buffer area that is
    bigger or equal to the size specified by 'len'.
  Input:
    buffer -  Pointer to where received BYTEs are to be stored
    len -     The number of BYTEs expected.

  **********************************************************************************/
BYTE getsUSBUSART(char *buffer, BYTE len)
{
    cdc_rx_len = 0;

    if ( !USBHandleBusy(CDCDataOutHandle) )
    {
        /*
         * Adjust the expected number of BYTEs to equal
         * the actual number of BYTEs received.
         */
        if ( len > USBHandleGetLength(CDCDataOutHandle) )
            len = USBHandleGetLength(CDCDataOutHandle);

        /*
         * Copy data from dual-ram buffer to user's buffer
         */
        for (cdc_rx_len = 0; cdc_rx_len < len; cdc_rx_len++)
            buffer[cdc_rx_len] = cdc_data_rx[cdc_rx_len];

        CDCDataOutHandle = USBRxOnePacket(CDC_DATA_EP,(BYTE*)cdc_data_rx,sizeof(cdc_data_rx));

    }//end if

    return cdc_rx_len;

}//end getsUSBUSART

BYTE getUSBData(char *buf, BYTE len)
{
    cdc_rx_len = 0;

    if ( !USBHandleBusy(CDCDataOutHandle) )
    {
		BYTE data_len = USBHandleGetLength(CDCDataOutHandle);

		if ( len < data_len )
			return 0;

        // Copy data from dual-ram buffer to user's buffer
        for (; cdc_rx_len < data_len; cdc_rx_len++)
            buf[cdc_rx_len] = cdc_data_rx[cdc_rx_len];

        CDCDataOutHandle = USBRxOnePacket(CDC_DATA_EP,(BYTE*)cdc_data_rx,sizeof(cdc_data_rx));
	 }

	 return cdc_rx_len;
}

/******************************************************************************
  Function:
	void putUSBUSART(char *data, BYTE length)

  Summary:
    putUSBUSART writes an array of data to the USB. Use this version, is
    capable of transfering 0x00 (what is typically a NULL character in any of
    the string transfer functions).

  Description:
    putUSBUSART writes an array of data to the USB. Use this version, is
    capable of transfering 0x00 (what is typically a NULL character in any of
    the string transfer functions).

    Typical Usage:
    <code>
        if(USBUSARTIsTxTrfReady())
        {
            char data[] = {0x00, 0x01, 0x02, 0x03, 0x04};
            putUSBUSART(data,5);
        }
    </code>

    The transfer mechanism for device-to-host(put) is more flexible than
    host-to-device(get). It can handle a string of data larger than the
    maximum size of bulk IN endpoint. A state machine is used to transfer a
    \long string of data over multiple USB transactions. CDCTxService()
    must be called periodically to keep sending blocks of data to the host.

  Conditions:
    USBUSARTIsTxTrfReady() must return TRUE. This indicates that the last
    transfer is complete and is ready to receive a new block of data. The
    string of characters pointed to by 'data' must equal to or smaller than
    255 BYTEs.

  Input:
    char *data - pointer to a RAM array of data to be transfered to the host
    BYTE length - the number of bytes to be transfered (must be less than 255).

 *****************************************************************************/
void putUSBUSART(char *data, BYTE  length)
{
	#ifdef USB_INTERRUPT
    USBMaskInterrupts();
	#endif

    if ( cdc_trf_state == CDC_TX_READY )
        mUSBUSARTTxRam((BYTE*)data, length);     // See cdc.h

	#ifdef USB_INTERRUPT
    USBUnmaskInterrupts();
	#endif
}

/************************************************************************
  Function:
        void CDCTxService(void)
 ************************************************************************/
void CDCTxService(void)
{
    BYTE byte_to_send;
    BYTE i;

	#ifdef USB_INTERRUPT
    USBMaskInterrupts();
	#endif

    if ( USBHandleBusy(CDCDataInHandle) )
    {
		#ifdef USB_INTERRUPT
        USBUnmaskInterrupts();
		#endif
        return;
    }

    /*
     * Completing stage is necessary while [ mCDCUSartTxIsBusy()==1 ].
     * By having this stage, user can always check cdc_trf_state,
     * and not having to call mCDCUsartTxIsBusy() directly.
     */
    if ( cdc_trf_state == CDC_TX_COMPLETING )
        cdc_trf_state = CDC_TX_READY;

    /*
     * If CDC_TX_READY state, nothing to do, just return.
     */
    if ( cdc_trf_state == CDC_TX_READY )
    {
		#ifdef USB_INTERRUPT
        USBUnmaskInterrupts();
		#endif
        return;
    }

    /*
     * If CDC_TX_BUSY_ZLP state, send zero length packet
     */
    if ( cdc_trf_state == CDC_TX_BUSY_ZLP )
    {
        CDCDataInHandle = USBTxOnePacket(CDC_DATA_EP,NULL,0);
        cdc_trf_state = CDC_TX_COMPLETING;
    }
    else if ( cdc_trf_state == CDC_TX_BUSY )
    {
    	if ( cdc_tx_len > sizeof(cdc_data_tx) )
    	    byte_to_send = sizeof(cdc_data_tx);
    	else
    	    byte_to_send = cdc_tx_len;

    	cdc_tx_len = cdc_tx_len - byte_to_send;
        pCDCDst.bRam = (BYTE*)cdc_data_tx; // Set destination pointer

        i = byte_to_send;
        if ( cdc_mem_type == USB_EP0_ROM ) // Determine type of memory source
        {
            while ( i-- )
                *pCDCDst.bRam++ = *pCDCSrc.bRom++;
        }
        else // _RAM
        {
            while ( i-- )
                *pCDCDst.bRam++ = *pCDCSrc.bRam++;
        }

        if ( cdc_tx_len == 0 )
        {
            if ( byte_to_send == CDC_DATA_IN_EP_SIZE )
                cdc_trf_state = CDC_TX_BUSY_ZLP;
            else
                cdc_trf_state = CDC_TX_COMPLETING;
        }
        CDCDataInHandle = USBTxOnePacket(CDC_DATA_EP,(BYTE*)cdc_data_tx,byte_to_send);
    }

	#ifdef USB_INTERRUPT
    USBUnmaskInterrupts();
	#endif
}
