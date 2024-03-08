#ifndef CDC_H
#define CDC_H

/** I N C L U D E S **********************************************************/
#include "GenericTypeDefs.h"
#include "usb_config.h"

/** D E F I N I T I O N S ****************************************************/

/* Class-Specific Requests */
#define SEND_ENCAPSULATED_COMMAND   0x00
#define GET_ENCAPSULATED_RESPONSE   0x01
#define SET_COMM_FEATURE            0x02
#define GET_COMM_FEATURE            0x03
#define CLEAR_COMM_FEATURE          0x04
#define SET_LINE_CODING             0x20
#define GET_LINE_CODING             0x21
#define SET_CONTROL_LINE_STATE      0x22
#define SEND_BREAK                  0x23

/* Notifications *
 * Note: Notifications are polled over
 * Communication Interface (Interrupt Endpoint)
 */
#define NETWORK_CONNECTION          0x00
#define RESPONSE_AVAILABLE          0x01
#define SERIAL_STATE                0x20


/* Device Class Code */
#define CDC_DEVICE                  0x02

/* Communication Interface Class Code */
#define COMM_INTF                   0x02

/* Communication Interface Class SubClass Codes */
#define ABSTRACT_CONTROL_MODEL      0x02

/* Communication Interface Class Control Protocol Codes */
#define V25TER                      0x01    // Common AT commands ("Hayes(TM)")


/* Data Interface Class Codes */
#define DATA_INTF                   0x0A

/* Data Interface Class Protocol Codes */
#define NO_PROTOCOL                 0x00    // No class specific protocol required


/* Communication Feature Selector Codes */
#define ABSTRACT_STATE              0x01
#define COUNTRY_SETTING             0x02

/* Functional Descriptors */
/* Type Values for the bDscType Field */
#define CS_INTERFACE                0x24
#define CS_ENDPOINT                 0x25

/* bDscSubType in Functional Descriptors */
#define DSC_FN_HEADER               0x00
#define DSC_FN_CALL_MGT             0x01
#define DSC_FN_ACM                  0x02    // ACM - Abstract Control Management
#define DSC_FN_DLM                  0x03    // DLM - Direct Line Managment
#define DSC_FN_TELEPHONE_RINGER     0x04
#define DSC_FN_RPT_CAPABILITIES     0x05
#define DSC_FN_UNION                0x06
#define DSC_FN_COUNTRY_SELECTION    0x07
#define DSC_FN_TEL_OP_MODES         0x08
#define DSC_FN_USB_TERMINAL         0x09
/* more.... see Table 25 in USB CDC Specification 1.1 */

/* CDC Bulk IN transfer states */
#define CDC_TX_READY                0
#define CDC_TX_BUSY                 1
#define CDC_TX_BUSY_ZLP             2       // ZLP: Zero Length Packet
#define CDC_TX_COMPLETING           3

#define LINE_CODING_TARGET &cdc_notice.SetLineCoding._byte[0]
#define LINE_CODING_PFUNC  USB_CDC_SET_LINE_CODING_HANDLER

#define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D3_VAL 0x00
#define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D2_VAL 0x00
#define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D1_VAL 0x02
#define USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D0_VAL 0x00

#define USB_CDC_ACM_FN_DSC_VAL  \
    USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D3_VAL |\
    USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D2_VAL |\
    USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D1_VAL |\
    USB_CDC_SUPPORT_ABSTRACT_CONTROL_MANAGEMENT_CAPABILITIES_D0_VAL


#define CDCSetBaudRate(baudRate)            line_coding.info.dwDTERate=baudRate


#define CDCSetCharacterFormat(charFormat)   line_coding.bCharFormat=charFormat
#define NUM_STOP_BITS_1     0   //1 stop bit - used by CDCSetLineCoding() and CDCSetCharacterFormat()
#define NUM_STOP_BITS_1_5   1   //1.5 stop bit - used by CDCSetLineCoding() and CDCSetCharacterFormat()
#define NUM_STOP_BITS_2     2   //2 stop bit - used by CDCSetLineCoding() and CDCSetCharacterFormat()

#define CDCSetParity(parityType)            line_coding.info.bParityType=parityType
#define PARITY_NONE     0 //no parity - used by CDCSetLineCoding() and CDCSetParity()
#define PARITY_ODD      1 //odd parity - used by CDCSetLineCoding() and CDCSetParity()
#define PARITY_EVEN     2 //even parity - used by CDCSetLineCoding() and CDCSetParity()
#define PARITY_MARK     3 //mark parity - used by CDCSetLineCoding() and CDCSetParity()
#define PARITY_SPACE    4 //space parity - used by CDCSetLineCoding() and CDCSetParity()


#define CDCSetDataSize(dataBits)            line_coding.bDataBits=dataBits


#define CDCSetLineCoding(baud,format,parity,dataSize) \
                                            CDCSetBaudRate(baud),\
                                            CDCSetCharacterFormat(format),\
                                            CDCSetParity(parity),\
                                            CDCSetDataSize(dataSize)


#define USBUSARTIsTxTrfReady()              (cdc_trf_state == CDC_TX_READY)
//#define mUSBUSARTIsTxTrfReady()             USBUSARTIsTxTrfReady()


#define mUSBUSARTTxRam(pData,len)           pCDCSrc.bRam = pData,           \
                                            cdc_tx_len = len,               \
                                            cdc_mem_type = USB_EP0_RAM,     \
                                            cdc_trf_state = CDC_TX_BUSY


#define mUSBUSARTTxRom(pData,len)           pCDCSrc.bRom = pData,           \
                                            cdc_tx_len = len,               \
                                            cdc_mem_type = USB_EP0_ROM,     \
                                            cdc_trf_state = CDC_TX_BUSY


void CDCInitEP(void);
void USBCheckCDCRequest(void);
void CDCNotificationHandler(void);
BOOL USBCDCEventHandler(USB_EVENT event, void *pdata, WORD size);


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
  Output:
    BYTE -    Returns a byte indicating the total number of bytes that were actually
              received and copied into the specified buffer.  The returned value
              can be anything from 0 up to the len input value.  A return value of 0
              indicates that no new CDC bulk OUT endpoint data was available.

  **********************************************************************************/
BYTE getsUSBUSART(char *buffer, BYTE len);
BYTE getUSBData(char *buf, BYTE len);

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
void putUSBUSART(char *data, BYTE Length);

/******************************************************************************
	Function:
		void putsUSBUSART(char *data)

  Summary:
    putsUSBUSART writes a string of data to the USB including the null
    character. Use this version, 'puts', to transfer data from a RAM buffer.

  Description:
    putsUSBUSART writes a string of data to the USB including the null
    character. Use this version, 'puts', to transfer data from a RAM buffer.

    Typical Usage:
    <code>
        if(USBUSARTIsTxTrfReady())
        {
            char data[] = "Hello World";
            putsUSBUSART(data);
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
    char *data -  null\-terminated string of constant data. If a
                            null character is not found, 255 BYTEs of data
                            will be transferred to the host.

 *****************************************************************************/
void putsUSBUSART(char *data);


/**************************************************************************
  Function:
        void putrsUSBUSART(const ROM char *data)

  Summary:
    putrsUSBUSART writes a string of data to the USB including the null
    character. Use this version, 'putrs', to transfer data literals and
    data located in program memory.

  Description:
    putrsUSBUSART writes a string of data to the USB including the null
    character. Use this version, 'putrs', to transfer data literals and
    data located in program memory.

    Typical Usage:
    <code>
        if(USBUSARTIsTxTrfReady())
        {
            putrsUSBUSART("Hello World");
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
    const ROM char *data -  null\-terminated string of constant data. If a
                            null character is not found, 255 BYTEs of data
                            will be transferred to the host.

  **************************************************************************/
void putrsUSBUSART(char *data);

/************************************************************************
  Function:
        void CDCTxService(void)

  Summary:
    CDCTxService handles device-to-host transaction(s). This function
    should be called once per Main Program loop after the device reaches
    the configured state.
  Description:
    CDCTxService handles device-to-host transaction(s). This function
    should be called once per Main Program loop after the device reaches
    the configured state (after the CDCIniEP() function has already executed).
    This function is needed, in order to advance the internal software state
    machine that takes care of sending multiple transactions worth of IN USB
    data to the host, associated with CDC serial data.  Failure to call
    CDCTxService() perioidcally will prevent data from being sent to the
    USB host, over the CDC serial data interface.

    Typical Usage:
    <code>
    void main(void)
    {
        USBDeviceInit();
        while(1)
        {
            USBDeviceTasks();
            if((USBGetDeviceState() \< CONFIGURED_STATE) ||
               (USBIsDeviceSuspended() == TRUE))
            {
                //Either the device is not configured or we are suspended
                //  so we don't want to do execute any application code
                continue;   //go back to the top of the while loop
            }
            else
            {
                //Keep trying to send data to the PC as required
                CDCTxService();

                //Run application code.
                UserApplication();
            }
        }
    }
    </code>
  Conditions:
    CDCIniEP() function should have already exectuted/the device should be
    in the CONFIGURED_STATE.
  Remarks:
    None
  ************************************************************************/
void CDCTxService(void);


/** S T R U C T U R E S ******************************************************/

/* Line Coding Structure */
#define LINE_CODING_LENGTH          0x07

typedef union _LINE_CODING
{
    BYTE _byte[LINE_CODING_LENGTH];
    struct
    {
        DWORD   dwDTERate;          // Complex data structure
        BYTE    bCharFormat;
        BYTE    bParityType;
        BYTE    bDataBits;
    } info;
} LINE_CODING;

/*
typedef union _CONTROL_SIGNAL_BITMAP
{
    BYTE _byte;
    struct
    {
        unsigned DTE_PRESENT:1;       // [0] Not Present  [1] Present
        unsigned CARRIER_CONTROL:1;   // [0] Deactivate   [1] Activate
    };
} CONTROL_SIGNAL_BITMAP;
*/

/* Functional Descriptor Structure - See CDC Specification 1.1 for details */

/* Header Functional Descriptor */
typedef struct
{
    BYTE bFNLength;
    BYTE bDscType;
    BYTE bDscSubType;
    WORD bcdCDC;
} USB_CDC_HEADER_FN_DSC;

/* Abstract Control Management Functional Descriptor */
typedef struct
{
    BYTE bFNLength;
    BYTE bDscType;
    BYTE bDscSubType;
    BYTE bmCapabilities;
} USB_CDC_ACM_FN_DSC;

/* Union Functional Descriptor */
typedef struct
{
    BYTE bFNLength;
    BYTE bDscType;
    BYTE bDscSubType;
    BYTE bMasterIntf;
    BYTE bSaveIntf0;
} USB_CDC_UNION_FN_DSC;

/* Call Management Functional Descriptor */
typedef struct
{
    BYTE bFNLength;
    BYTE bDscType;
    BYTE bDscSubType;
    BYTE bmCapabilities;
    BYTE bDataInterface;
} USB_CDC_CALL_MGT_FN_DSC;

typedef union
{
    LINE_CODING GetLineCoding;
    LINE_CODING SetLineCoding;
    unsigned char packet[CDC_COMM_IN_EP_SIZE];
} CDC_NOTICE;//, *PCDC_NOTICE;

/* Bit structure definition for the SerialState notification byte */
/*
typedef union
{
    BYTE byte;
    struct
    {
        BYTE    DCD             :1;
        BYTE    DSR             :1;
        BYTE    BreakState      :1;
        BYTE    RingDetect      :1;
        BYTE    FramingError    :1;
        BYTE    ParityError     :1;
        BYTE    Overrun         :1;
        BYTE    Reserved        :1;
    }bits;
} BM_SERIAL_STATE;
*/
/* Serial State Notification Packet Structure */
typedef struct
{
    BYTE    bmRequestType;  //Always 0xA1 for serial state notification packets
    BYTE    bNotification;  //Always SERIAL_STATE (0x20) for serial state notification packets
    UINT16  wValue;     //Always 0 for serial state notification packets
    UINT16  wIndex;     //Interface number
    UINT16  wLength;    //Should always be 2 for serial state notification packets
    BYTE    SerialState;
    BYTE    Reserved;
} SERIAL_STATE_NOTIFICATION;


/** E X T E R N S ************************************************************/
extern BYTE cdc_rx_len;
extern void *lastTransmission;

extern BYTE cdc_trf_state;
extern POINTER pCDCSrc;
extern BYTE cdc_tx_len;
extern BYTE cdc_mem_type;

extern CDC_NOTICE cdc_notice;
extern LINE_CODING line_coding;

extern CTRL_TRF_SETUP SetupPkt;
extern const BYTE configDescriptor1[];

#endif //CDC_H
