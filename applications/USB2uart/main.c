/** INCLUDES *******************************************************/
#include <p16f1455.h>
#include "GenericTypeDefs.h"
#include "usb_hal_pic16f1.h"
#include "usb_device.h"
#include "usb_function_cdc.h"
#include "uart.h"

#pragma acc_save 0
#pragma isr_no_stack

// configure fuses ...
#pragma FUSE0   0x09e4	//0x0ea4	//0x09e4
#pragma FUSE1   0x1fcf	//0x1fc3	//0x1fcf

/** P R I V A T E  P R O T O T Y P E S ***************************************/
static void InitializeSystem(void);
void USBDeviceTasks(void);
void UserInit(void);

/** VECTOR REMAPPING ***********************************************/
void TMR0_init(void)
{
	OPTION_REG &= ~0x3f;
    OPTION_REG |=  0x07;    // prescale = 256
    TMR0IE 	    = 0;		// no interrupt
}

/******************************************************************************
 * Function:        void main(void)
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Side Effects:    None
 * Overview:        Main program entry point.
 * Note:            None
 *****************************************************************************/
void main(void)
{
	BYTE t0;

	WPUA  = 0xff;
	TRISA = 0xff;
	WPUEN = 0;		// enable weak pull-up

    InitializeSystem();

	for (UART_init();;)
	{
		USBDeviceTasks();
		UART_task();
	}
}

/********************************************************************
 * Function:        static void InitializeSystem(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        InitializeSystem is a centralize initialization
 *                  routine. All required USB initialization routines
 *                  are called from here.
 *
 *                  User application initialization routine should
 *                  also be called from here.
 *
 * Note:            None
 *******************************************************************/
static void InitializeSystem(void)
{
    #ifdef USE_USB_BUS_SENSE_IO
    tris_usb_bus_sense = INPUT_PIN; // See HardwareProfile.h
    #endif

    UserInit();
    USBDeviceInit();	//usb_device.c.  Initializes USB module SFRs and firmware
    					//variables to known states.
}//end InitializeSystem

/******************************************************************************
 * Function:        void UserInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine should take care of all of the demo code
 *                  initialization that is required.
 *
 * Note:
 *
 *****************************************************************************/
void UserInit(void)
{
	unsigned char i;

    ANSELA = 0x00;
    ANSELC = 0x00;

    OSCTUNE = 0;
    OSCCON = 0x7C;	// PLL enabled, 3x, 16MHz internal osc, SCS external
    SPLLMULT = 1;	// 1=3x, 0=4x
    ACTCON = 0x90;  // Clock recovery on, Clock Recovery enabled; SOF packet
}//end UserInit

/******************************************************************************
 * Function:        void mySetLineCodingHandler(void)
 *
 * PreCondition:    USB_CDC_SET_LINE_CODING_HANDLER is defined
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function gets called when a SetLineCoding command
 *                  is sent on the bus.  This function will evaluate the request
 *                  and determine if the application should update the baudrate
 *                  or not.
 *
 * Note:
 *
 *****************************************************************************/
#ifdef USB_CDC_SET_LINE_CODING_HANDLER
void mySetLineCodingHandler(void)
{
	unsigned short baud;

	baud = cdc_notice.GetLineCoding.info.dwDTERate;

    //If the request is not in a valid range
    if ( baud >= 110 )
    {
        //Update the baudrate info in the CDC driver
        CDCSetBaudRate(baud);

        if ( baud >= 200 )
        {
	        //Update the baudrate of the UART
	        baud = (48000000/4)/baud - 1;
	        BRGH = 1;
		}
		else
		{
	        //Update the baudrate of the UART
	        baud = (48000000/16)/baud - 1;
	        BRGH = 0;
		}
        SPBRGL = baud;
        SPBRGH = baud >> 8;

        TX9  = 0;
        uartFormat = 0;
        if ( cdc_notice.GetLineCoding.info.bCharFormat )	// stop > 1-bit
        {
        	uartFormat = 3;
        	TX9  = 1;
			TX9D = 1;	// extra STOP bit = 1
		}

        if ( cdc_notice.GetLineCoding.info.bParityType &&
        	 cdc_notice.GetLineCoding.info.bParityType < 3 )
		{
        	uartFormat = cdc_notice.GetLineCoding.info.bParityType;
        	TX9  = 1;
		}
    }
}
#endif

/*******************************************************************
 * Function:        void USBCBCheckOtherReq(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        When SETUP packets arrive from the host, some
 * 					firmware must process the request and respond
 *					appropriately to fulfill the request.  Some of
 *					the SETUP packets will be for standard
 *					USB "chapter 9" (as in, fulfilling chapter 9 of
 *					the official USB specifications) requests, while
 *					others may be specific to the USB device class
 *					that is being implemented.  For example, a HID
 *					class device needs to be able to respond to
 *					"GET REPORT" type of requests.  This
 *					is not a standard USB chapter 9 request, and
 *					therefore not handled by usb_device.c.  Instead
 *					this request should be handled by class specific
 *					firmware, such as that contained in usb_function_hid.c.
 *
 * Note:            None
 *******************************************************************/
void USBCBCheckOtherReq(void)
{
    USBCheckCDCRequest();
}//end

/*******************************************************************
 * Function:        void USBCBInitEP(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called when the device becomes
 *                  initialized, which occurs after the host sends a
 * 					SET_CONFIGURATION (wValue not = 0) request.  This
 *					callback function should initialize the endpoints
 *					for the device's usage according to the current
 *					configuration.
 *
 * Note:            None
 *******************************************************************/
void USBCBInitEP(void)
{
    //Enable the CDC data endpoints
    CDCInitEP();
}

/*******************************************************************
 * Function:        BOOL USER_USB_CALLBACK_EVENT_HANDLER(
 *                        USB_EVENT event, void *pdata, WORD size)
 *
 * PreCondition:    None
 *
 * Input:           USB_EVENT event - the type of event
 *                  void *pdata - pointer to the event data
 *                  WORD size - size of the event data
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is called from the USB stack to
 *                  notify a user application that a USB event
 *                  occured.  This callback is in interrupt context
 *                  when the USB_INTERRUPT option is selected.
 *
 * Note:            None
 *******************************************************************/
BOOL USER_USB_CALLBACK_EVENT_HANDLER(int event, void *pdata, WORD size)
{
    switch(event)
    {/*
        case EVENT_TRANSFER:
            //Add application specific callback task or callback function here if desired.
            break;
        case EVENT_SOF:
            USBCB_SOF_Handler();
            break;
        case EVENT_SUSPEND:
            USBCBSuspend();
            break;
        case EVENT_RESUME:
            USBCBWakeFromSuspend();
            break;*/
        case EVENT_CONFIGURED:
            USBCBInitEP();
            break;
/*
        case EVENT_SET_DESCRIPTOR:
            USBCBStdSetDscHandler();
            break;*/
        case EVENT_EP0_REQUEST:
            USBCBCheckOtherReq();
            break;
/*
        case EVENT_BUS_ERROR:
            USBCBErrorHandler();
            break;
        case EVENT_TRANSFER_TERMINATED:
            //Add application specific callback task or callback function here if desired.
            //The EVENT_TRANSFER_TERMINATED event occurs when the host performs a CLEAR
            //FEATURE (endpoint halt) request on an application endpoint which was
            //previously armed (UOWN was = 1).  Here would be a good place to:
            //1.  Determine which endpoint the transaction that just got terminated was
            //      on, by checking the handle value in the *pdata.
            //2.  Re-arm the endpoint if desired (typically would be the case for OUT
            //      endpoints).
            break;
        default:
            break;*/
    }
    return TRUE;
}


/** EOF main.c *************************************************/
