#include <p16f1455.h>
#include "GenericTypeDefs.h"
//#include "usb_config.h"
#include "usb_device_local.h"
#include "usb_hal_pic16f1.h"
#include "usb_device.h"
#include "tool.h"

#ifndef USE_USB_BUS_SENSE_IO
    #undef USB_BUS_SENSE
    #define USB_BUS_SENSE 1
#endif

#define _DTS_CHECKING_ENABLED _DTSEN

volatile USB_DEVICE_STATE USBDeviceState;
volatile BYTE USBActiveConfiguration;
volatile BYTE USBAlternateInterface[USB_MAX_NUM_INT];
volatile BDT_ENTRY *pBDTEntryEP0OutCurrent;
volatile BDT_ENTRY *pBDTEntryEP0OutNext;
volatile BDT_ENTRY *pBDTEntryOut[USB_MAX_EP_NUMBER+1];
volatile BDT_ENTRY *pBDTEntryIn[USB_MAX_EP_NUMBER+1];
volatile BYTE shortPacketStatus;
volatile BYTE controlTransferState;
volatile IN_PIPE inPipe;
volatile OUT_PIPE outPipe;
volatile BYTE *pDst;
volatile BOOL RemoteWakeup;
volatile BOOL USBBusIsSuspended;
volatile BYTE USTATcopy;
volatile BYTE endpoint_number;
volatile BOOL BothEP0OutUOWNsSet;
volatile EP_STATUS ep_data_in[USB_MAX_EP_NUMBER+1];
volatile EP_STATUS ep_data_out[USB_MAX_EP_NUMBER+1];
volatile BYTE USBStatusStageTimeoutCounter;
volatile BOOL USBDeferStatusStagePacket;
volatile BOOL USBStatusStageEnabledFlag1;
volatile BOOL USBStatusStageEnabledFlag2;
volatile BOOL USBDeferINDataStagePackets;
volatile BOOL USBDeferOUTDataStagePackets;

volatile BDT_ENTRY BDT[BDT_NUM_ENTRIES]      @ BDT_BASE_ADDR;
volatile CTRL_TRF_SETUP SetupPkt             @ CTRL_TRF_SETUP_ADDR;
volatile BYTE CtrlTrfData[USB_EP0_BUFF_SIZE] @ CTRL_TRF_DATA_ADDR;

//Device descriptor
extern const USB_DEVICE_DESCRIPTOR device_dsc;

//Array of configuration descriptors
extern const BYTE *const USB_CD_Ptr[];
extern const BYTE *const USB_SD_Ptr[];

/** Macros *********************************************************/

/** Function Prototypes ********************************************/
//External
//This is the prototype for the required user event handler
BOOL USER_USB_CALLBACK_EVENT_HANDLER(int event, void *pdata, WORD size);

//Internal Functions
static void USBCtrlEPService(void);
static void USBCtrlTrfSetupHandler(void);
static void USBCtrlTrfInHandler(void);
static void USBCheckStdRequest(void);
static void USBStdGetDscHandler(void);
static void USBCtrlEPServiceComplete(void);
static void USBCtrlTrfTxService(void);
static void USBCtrlTrfRxService(void);
static void USBStdSetCfgHandler(void);
static void USBStdGetStatusHandler(void);
static void USBStdFeatureReqHandler(void);
static void USBCtrlTrfOutHandler(void);
static void USBConfigureEndpoint(BYTE EPNum, BYTE direction);
static void USBWakeFromSuspend(void);
static void USBSuspend(void);
static void USBStallHandler(void);
static void DisableNonZeroEndpoints(BYTE last_ep_num);

/****************************************************************************
  Function:
    void USBAdvancePingPongBuffer(BDT_ENTRY** buffer)

  Description:
    This function will advance the passed pointer to the next buffer based on
    the ping pong option setting.  This function should be used for EP1-EP15
    only.  This function is not valid for EP0.

  Precondition:
    None

  Parameters:
    BDT_ENTRY** - pointer to the BDT_ENTRY pointer that you want to be advanced
    to the next buffer state

  Return Values:
    None

  Remarks:
    None

  ***************************************************************************/
#define USBAdvancePingPongBuffer(buffer) *((BYTE*)buffer) ^=  USB_NEXT_PING_PONG
#define USBHALPingPongSetToOdd(buffer)   *((BYTE*)buffer) |=  USB_NEXT_PING_PONG
#define USBHALPingPongSetToEven(buffer)  *((BYTE*)buffer) &= ~USB_NEXT_PING_PONG

/**************************************************************************
    Function:
        void USBDeviceInit(void)

    Description:
        This function initializes the device stack it in the default state. The
        USB module will be completely reset including all of the internal
        variables, registers, and interrupt flags.

    Precondition:
        This function must be called before any of the other USB Device
        functions can be called, including USBDeviceTasks().

    Parameters:
        None

    Return Values:
        None

    Remarks:
        None

  ***************************************************************************/
void USBDeviceInit(void)
{
    BYTE i;

    USBDisableInterrupts();
    UEIR = 0;  // Clear all USB error flags
    UIR  = 0;  // Clears all USB interrupts

    UEP0 = 0;                         //Clear all of the endpoint control registers

    DisableNonZeroEndpoints(USB_MAX_EP_NUMBER);

    SetConfigurationOptions();

    //Clear all of the BDT entries
    memset(BDT, 0, sizeof(BDT));

    // Assert reset request to all of the Ping Pong buffer pointers
    USBPingPongBufferReset = 1;

    // Reset to default address
    UADDR = 0x00;

    // Make sure packet processing is enabled
    USBPacketDisable = 0;

    //Stop trying to reset ping pong buffer pointers
    USBPingPongBufferReset = 0;

    // Flush any pending transactions
    while(USBTransactionCompleteIF == 1)
    {
        USBClearInterruptFlag(USBTransactionCompleteIFReg,USBTransactionCompleteIFBitNum);
        //Initialize USB stack software state variables
        inPipe.info = 0;
        outPipe.info = 0;
        outPipe.wCount = 0;
    }

    //Set flags to TRUE, so the USBCtrlEPAllowStatusStage() function knows not to
    //try and arm a status stage, even before the first control transfer starts.
    USBStatusStageEnabledFlag1 = TRUE;
    USBStatusStageEnabledFlag2 = TRUE;
    //Initialize other flags
    USBDeferINDataStagePackets = FALSE;
    USBDeferOUTDataStagePackets = FALSE;
    USBBusIsSuspended = FALSE;

	//Initialize all pBDTEntryIn[] and pBDTEntryOut[]
	//pointers to NULL, so they don't get used inadvertently.
	for(i = 0; i < (BYTE)(USB_MAX_EP_NUMBER+1); i++)
	{
		pBDTEntryIn[i] = 0;
		pBDTEntryOut[i] = 0;
		ep_data_in[i] = 0;
        ep_data_out[i] = 0;
	}

    //Get ready for the first packet
    pBDTEntryIn[0] = (volatile BDT_ENTRY*)&BDT[EP0_IN_EVEN];
    // Initialize EP0 as a Ctrl EP
    UEP0 = EP_CTRL|USB_HANDSHAKE_ENABLED;

	//Prepare for the first SETUP on EP0 OUT
    BDT[EP0_OUT_EVEN].ADR = ConvertToPhysicalAddress(&SetupPkt);
    BDT[EP0_OUT_EVEN].CNT = USB_EP0_BUFF_SIZE;
    BDT[EP0_OUT_EVEN].STAT= _USIE|_DAT0|_BSTALL;

    // Clear active configuration
    USBActiveConfiguration = 0;

    //Indicate that we are now in the detached state
    USBDeviceState = DETACHED_STATE;
}

/**************************************************************************
  Function:
        void USBDeviceTasks(void)

  Summary:
    This function is the main state machine/transaction handler of the USB
    device side stack.  When the USB stack is operated in "USB_POLLING" mode
    (usb_config.h user option) the USBDeviceTasks() function should be called
    periodically to receive and transmit packets through the stack. This
    function also takes care of control transfers associated with the USB
    enumeration process, and detecting various USB events (such as suspend).
    This function should be called at least once every 1.8ms during the USB
    enumeration process. After the enumeration process is complete (which can
    be determined when USBGetDeviceState() returns CONFIGURED_STATE), the
    USBDeviceTasks() handler may be called the faster of: either once
    every 9.8ms, or as often as needed to make sure that the hardware USTAT
    FIFO never gets full.  A good rule of thumb is to call USBDeviceTasks() at
    a minimum rate of either the frequency that USBTransferOnePacket() gets
    called, or, once/1.8ms, whichever is faster.  See the inline code comments
    near the top of usb_device.c for more details about minimum timing
    requirements when calling USBDeviceTasks().

    When the USB stack is operated in "USB_INTERRUPT" mode, it is not necessary
    to call USBDeviceTasks() from the main loop context.  In the USB_INTERRUPT
    mode, the USBDeviceTasks() handler only needs to execute when a USB
    interrupt occurs, and therefore only needs to be called from the interrupt
    context.

  Description:
    This function is the main state machine/transaction handler of the USB
    device side stack.  When the USB stack is operated in "USB_POLLING" mode
    (usb_config.h user option) the USBDeviceTasks() function should be called
    periodically to receive and transmit packets through the stack. This
    function also takes care of control transfers associated with the USB
    enumeration process, and detecting various USB events (such as suspend).
    This function should be called at least once every 1.8ms during the USB
    enumeration process. After the enumeration process is complete (which can
    be determined when USBGetDeviceState() returns CONFIGURED_STATE), the
    USBDeviceTasks() handler may be called the faster of: either once
    every 9.8ms, or as often as needed to make sure that the hardware USTAT
    FIFO never gets full.  A good rule of thumb is to call USBDeviceTasks() at
    a minimum rate of either the frequency that USBTransferOnePacket() gets
    called, or, once/1.8ms, whichever is faster.  See the inline code comments
    near the top of usb_device.c for more details about minimum timing
    requirements when calling USBDeviceTasks().

    When the USB stack is operated in "USB_INTERRUPT" mode, it is not necessary
    to call USBDeviceTasks() from the main loop context.  In the USB_INTERRUPT
    mode, the USBDeviceTasks() handler only needs to execute when a USB
    interrupt occurs, and therefore only needs to be called from the interrupt
    context.

    Typical usage:
    <code>
    void main(void)
    {
        USBDeviceInit();
        while(1)
        {
            USBDeviceTasks(); //Takes care of enumeration and other USB events
            if((USBGetDeviceState() \< CONFIGURED_STATE) ||
               (USBIsDeviceSuspended() == TRUE))
            {
                //Either the device is not configured or we are suspended,
                // so we don't want to execute any USB related application code
                continue;   //go back to the top of the while loop
            }
            else
            {
                //Otherwise we are free to run USB and non-USB related user
                //application code.
                UserApplication();
            }
        }
    }
    </code>

  Precondition:
    Make sure the USBDeviceInit() function has been called prior to calling
    USBDeviceTasks() for the first time.
  Remarks:
    USBDeviceTasks() does not need to be called while in the USB suspend mode,
    if the user application firmware in the USBCBSuspend() callback function
    enables the ACTVIF USB interrupt source and put the microcontroller into
    sleep mode.  If the application firmware decides not to sleep the
    microcontroller core during USB suspend (ex: continues running at full
    frequency, or clock switches to a lower frequency), then the USBDeviceTasks()
    function must still be called periodically, at a rate frequent enough to
    ensure the 10ms resume recovery interval USB specification is met.  Assuming
    a worst case primary oscillator and PLL start up time of <5ms, then
    USBDeviceTasks() should be called once every 5ms in this scenario.

    When the USB cable is detached, or the USB host is not actively powering
    the VBUS line to +5V nominal, the application firmware does not always have
    to call USBDeviceTasks() frequently, as no USB activity will be taking
    place.  However, if USBDeviceTasks() is not called regularly, some
    alternative means of promptly detecting when VBUS is powered (indicating
    host attachment), or not powered (host powered down or USB cable unplugged)
    is still needed.  For self or dual self/bus powered USB applications, see
    the USBDeviceAttach() and USBDeviceDetach() API documentation for additional
    considerations.

  **************************************************************************/

void USBDeviceTasks(void)
{
    BYTE i;

    #ifdef USB_POLLING
    //If the interrupt option is selected then the customer is required
    //  to notify the stack when the device is attached or removed from the
    //  bus by calling the USBDeviceAttach() and USBDeviceDetach() functions.
    if ( USB_BUS_SENSE != 1 )
    {
         // Disable module & detach from bus
         UCON = 0;

         // Mask all USB interrupts
         UIE = 0;

         //Move to the detached state
         USBDeviceState = DETACHED_STATE;
         USBClearUSBInterrupt();
         return;
    }

    //if we are in the detached state
    if ( USBDeviceState == DETACHED_STATE )
    {
	    //Initialize register to known value
        UCON = 0;

        // Mask all USB interrupts
        UIE = 0;

        //Enable/set things like: pull ups, full/low-speed mode,
        //set the ping pong mode, and set internal transceiver
        SetConfigurationOptions();

        // Enable module & attach to bus
        while (!USBEN) USBEN = 1;


        //moved to the attached state
        USBDeviceState = ATTACHED_STATE;
    }
	#endif  //#if defined(USB_POLLING)

    if ( USBDeviceState == ATTACHED_STATE )
    {
        /*
         * After enabling the USB module, it takes some time for the
         * voltage on the D+ or D- line to rise high enough to get out
         * of the SE0 condition. The USB Reset interrupt should not be
         * unmasked until the SE0 condition is cleared. This helps
         * prevent the firmware from misinterpreting this unique event
         * as a USB bus reset from the USB host.
         */

        if(!USBSE0Event)
        {
            UIR = 0;				// Clear all USB interrupts
            #ifdef USB_POLLING
            UIE = 0;                   // Mask all USB interrupts
            #endif
            USBResetIE = 1;            // Unmask RESET interrupt
            USBIdleIE = 1;             // Unmask IDLE interrupt
            USBDeviceState = POWERED_STATE;

        }
    }

    /*
     * Task A: Service USB Activity Interrupt
     */
    if ( USBActivityIF && USBActivityIE )
    {
        USBClearInterruptFlag(USBActivityIFReg,USBActivityIFBitNum);
        USBWakeFromSuspend();
    }

    /*
     * Pointless to continue servicing if the device is in suspend mode.
     */
    if ( USBSuspendControl == 1 )
    {
        USBClearUSBInterrupt();
        return;
    }

    /*
     * Task B: Service USB Bus Reset Interrupt.
     * When bus reset is received during suspend, ACTVIF will be set first,
     * once the UCONbits.SUSPND is clear, then the URSTIF bit will be asserted.
     * This is why URSTIF is checked after ACTVIF.
     *
     * The USB reset flag is masked when the USB state is in
     * DETACHED_STATE or ATTACHED_STATE, and therefore cannot
     * cause a USB reset event during these two states.
     */
    if ( USBResetIF && USBResetIE )
    {
        USBDeviceInit();

		#ifdef USB_INTERRUPT
        USBUnmaskInterrupts();
		#endif

        USBDeviceState = DEFAULT_STATE;
        USBClearInterruptFlag(USBResetIFReg,USBResetIFBitNum);
    }

    /*
     * Task C: Service other USB interrupts
     */
    if ( USBIdleIF && USBIdleIE )
    {
        USBSuspend();
        USBClearInterruptFlag(USBIdleIFReg,USBIdleIFBitNum);
    }

    if ( USBSOFIF )
    {
        if ( USBSOFIE )
            USB_SOF_HANDLER(EVENT_SOF, 0, 1);

        USBClearInterruptFlag(USBSOFIFReg,USBSOFIFBitNum);

        #ifdef USB_ENABLE_STATUS_STAGE_TIMEOUTS
            //Supporting this feature requires a 1ms timebase for keeping track of the timeout interval.

            //Decrement our status stage counter.
            if ( USBStatusStageTimeoutCounter )
                USBStatusStageTimeoutCounter--;

            //Check if too much time has elapsed since progress was made in
            //processing the control transfer, without arming the status stage.
            //If so, auto-arm the status stage to ensure that the control
            //transfer can [eventually] complete, within the timing limits
            //dictated by section 9.2.6 of the official USB 2.0 specifications.
            if ( USBStatusStageTimeoutCounter == 0 )
            {
                USBCtrlEPAllowStatusStage();    //Does nothing if the status stage was already armed.
            }
        #endif
    }

    if ( USBStallIF && USBStallIE )
        USBStallHandler();

    if ( USBErrorIF && USBErrorIE )
    {
        USB_ERROR_HANDLER(EVENT_BUS_ERROR,0,1);
        UEIR = 0;       // This clears UERRIF
    }

    /*
     * Pointless to continue servicing if the host has not sent a bus reset.
     * Once bus reset is received, the device transitions into the DEFAULT
     * state and is ready for communication.
     */
    if ( USBDeviceState < DEFAULT_STATE )
    {
	    USBClearUSBInterrupt();
	    return;
	}

    /*
     * Task D: Servicing USB Transaction Complete Interrupt
     */
    if ( USBTransactionCompleteIE )
    {
	    for(i = 0; i < 4u; i++)	//Drain or deplete the USAT FIFO entries.  If the USB FIFO ever gets full, USB bandwidth
		{						//utilization can be compromised, and the device won't be able to receive SETUP packets.
		    if ( USBTransactionCompleteIF )
		    {
    		    //Save and extract USTAT register info.  Will use this info later.
                USTATcopy = USTAT;
                endpoint_number = USTAT_EP(USTATcopy);

                USBClearInterruptFlag(USBTransactionCompleteIFReg,USBTransactionCompleteIFBitNum);

                //Keep track of the hardware ping pong state for endpoints other
                //than EP0, if ping pong buffering is enabled.
                if ( USTAT_DIRECTION(USTATcopy) == OUT_FROM_HOST)
                    ep_data_out[endpoint_number] ^= EP_PINGPONG_STATE_FLAG;
                else
                    ep_data_in[endpoint_number] ^= EP_PINGPONG_STATE_FLAG;

                //USBCtrlEPService only services transactions over EP0.
                //It ignores all other EP transactions.
                if(endpoint_number == 0)
                {
                    USBCtrlEPService();
                }
                else
                {

                    USB_TRANSFER_COMPLETE_HANDLER(EVENT_TRANSFER, (BYTE*)&USTATcopy, 0);
                }
		    }
		    else
		    	break;
		}
	}

    USBClearUSBInterrupt();
}//end of USBDeviceTasks()

/*******************************************************************************
  Function:
        void USBEnableEndpoint(BYTE ep, BYTE options)
  *****************************************************************************/
void USBEnableEndpoint(BYTE ep, BYTE options)
{
    unsigned char* p;

    if ( options & USB_OUT_ENABLED )
    {
        USBConfigureEndpoint(ep, OUT_FROM_HOST);
    }
    if ( options & USB_IN_ENABLED )
    {
        USBConfigureEndpoint(ep, IN_TO_HOST);
    }

    p = (unsigned char*)(&UEP0+ep);
    *p = options;
}


/*************************************************************************
  Function:
    void * USBTransferOnePacket(BYTE ep, BYTE dir, BYTE* data, BYTE len)

  Summary:
    Transfers a single packet (one transaction) of data on the USB bus.

  Description:
    The USBTransferOnePacket() function prepares a USB endpoint
    so that it may send data to the host (an IN transaction), or
    receive data from the host (an OUT transaction).  The
    USBTransferOnePacket() function can be used both to receive	and
    send data to the host.  This function is the primary API function
    provided by the USB stack firmware for sending or receiving application
    data over the USB port.

    The USBTransferOnePacket() is intended for use with all application
    endpoints.  It is not used for sending or receiving applicaiton data
    through endpoint 0 by using control transfers.  Separate API
    functions, such as USBEP0Receive(), USBEP0SendRAMPtr(), and
    USBEP0SendROMPtr() are provided for this purpose.

    The	USBTransferOnePacket() writes to the Buffer Descriptor Table (BDT)
    entry associated with an endpoint buffer, and sets the UOWN bit, which
    prepares the USB hardware to allow the transaction to complete.  The
    application firmware can use the USBHandleBusy() macro to check the
    status of the transaction, to see if the data has been successfully
    transmitted yet.


    Typical Usage
    <code>
    //make sure that the we are in the configured state
    if(USBGetDeviceState() == CONFIGURED_STATE)
    {
        //make sure that the last transaction isn't busy by checking the handle
        if(!USBHandleBusy(USBInHandle))
        {
	        //Write the new data that we wish to send to the host to the INPacket[] array
	        INPacket[0] = USEFUL_APPLICATION_VALUE1;
	        INPacket[1] = USEFUL_APPLICATION_VALUE2;
	        //INPacket[2] = ... (fill in the rest of the packet data)

            //Send the data contained in the INPacket[] array through endpoint "EP_NUM"
            USBInHandle = USBTransferOnePacket(EP_NUM,IN_TO_HOST,(BYTE*)&INPacket[0],sizeof(INPacket));
        }
    }
    </code>

  Conditions:
    Before calling USBTransferOnePacket(), the following should be true.
    1.  The USB stack has already been initialized (USBDeviceInit() was called).
    2.  A transaction is not already pending on the specified endpoint.  This
        is done by checking the previous request using the USBHandleBusy()
        macro (see the typical usage example).
    3.  The host has already sent a set configuration request and the
        enumeration process is complete.
        This can be checked by verifying that the USBGetDeviceState()
        macro returns "CONFIGURED_STATE", prior to calling
        USBTransferOnePacket().

  Input:
    BYTE ep - The endpoint number that the data will be transmitted or
	          received on
    BYTE dir - The direction of the transfer
               This value is either OUT_FROM_HOST or IN_TO_HOST
    BYTE* data - For IN transactions: pointer to the RAM buffer containing
                 the data to be sent to the host.  For OUT transactions: pointer
                 to the RAM buffer that the received data should get written to.
   BYTE len - Length of the data needing to be sent (for IN transactions).
              For OUT transactions, the len parameter should normally be set
              to the endpoint size specified in the endpoint descriptor.

  Return Values:
    void * - handle to the transfer.  The handle is a pointer to
                 the BDT entry associated with this transaction.  The
                 status of the transaction (ex: if it is complete or still
                 pending) can be checked using the USBHandleBusy() macro
                 and supplying the void * provided by
                 USBTransferOnePacket().

  Remarks:
    If calling the USBTransferOnePacket() function from within the USBCBInitEP()
    callback function, the set configuration is still being processed and the
    USBDeviceState may not be == CONFIGURED_STATE yet.  In this	special case,
    the USBTransferOnePacket() may still be called, but make sure that the
    endpoint has been enabled and initialized by the USBEnableEndpoint()
    function first.

  *************************************************************************/
void * USBTransferOnePacket(BYTE ep,BYTE dir,BYTE* data,BYTE len)
{
    volatile BDT_ENTRY* handle;

    //If the direction is IN
    if ( dir != 0 )
    {
        //point to the IN BDT of the specified endpoint
        handle = pBDTEntryIn[ep];
    }
    else
    {
        //else point to the OUT BDT of the specified endpoint
        handle = pBDTEntryOut[ep];
    }

    //Error checking code.  Make sure the handle (pBDTEntryIn[ep] or
    //pBDTEntryOut[ep]) is initialized before using it.
    if ( handle == 0 )
	    return 0;

    //Set the data pointer, data length, and enable the endpoint
    handle->ADR = ConvertToPhysicalAddress(data);
    handle->CNT = len;
    handle->STAT &= _DTSMASK;
    handle->STAT |= _USIE | (_DTSEN & _DTS_CHECKING_ENABLED);

    //Point to the next buffer for ping pong purposes.
    if ( dir != OUT_FROM_HOST )
        USBAdvancePingPongBuffer(&pBDTEntryIn[ep]);
    else
        USBAdvancePingPongBuffer(&pBDTEntryOut[ep]);

    return (void *)handle;
}

/**************************************************************************
    Function:
        void USBDeviceAttach(void)

    Summary:
        Checks if VBUS is present, and that the USB module is not already
        initalized, and if so, enables the USB module so as to signal device
        attachment to the USB host.

    Description:
        This function indicates to the USB host that the USB device has been
        attached to the bus.  This function needs to be called in order for the
        device to start to enumerate on the bus.

    Precondition:
        Should only be called when USB_INTERRUPT is defined.  Also, should only
        be called from the main() loop context.  Do not call USBDeviceAttach()
        from within an interrupt handler, as the USBDeviceAttach() function
        may modify global interrupt enable bits and settings.

        For normal USB devices:
        Make sure that if the module was previously on, that it has been turned off
        for a long time (ex: 100ms+) before calling this function to re-enable the module.
        If the device turns off the D+ (for full speed) or D- (for low speed) ~1.5k ohm
        pull up resistor, and then turns it back on very quickly, common hosts will sometimes
        reject this event, since no human could ever unplug and reattach a USB device in a
        microseconds (or nanoseconds) timescale.  The host could simply treat this as some kind
        of glitch and ignore the event altogether.
    Parameters:
        None

    Return Values:
        None

    Remarks:
		See also the USBDeviceDetach() API function documentation.
****************************************************************************/
#ifdef USB_INTERRUPT
void USBDeviceAttach(void)
{
    //if we are in the detached state
    if ( USBDeviceState == DETACHED_STATE )
    {
 	    //Initialize registers to known states.
        UCON = 0;

        // Mask all USB interrupts
        UIE = 0;

        //Configure things like: pull ups, full/low-speed mode,
        //set the ping pong mode, and set internal transceiver
        SetConfigurationOptions();

        USBEnableInterrupts();  //Modifies global interrupt settings

        // Enable module & attach to bus
        while ( !USBEN ) USBEN = 1;

        //moved to the attached state
        USBDeviceState = ATTACHED_STATE;
    }
}
#endif


/*******************************************************************************
  Function: void USBCtrlEPAllowStatusStage(void);

  Summary: This function prepares the proper endpoint 0 IN or endpoint 0 OUT
            (based on the controlTransferState) to allow the status stage packet
            of a control transfer to complete.  This function gets used
            internally by the USB stack itself, but it may also be called from
            the application firmware, IF the application firmware called
            the USBDeferStatusStage() function during the initial processing
            of the control transfer request.  In this case, the application
            must call the USBCtrlEPAllowStatusStage() once, after it has fully
            completed processing and handling the data stage portion of the
            request.

            If the application firmware has no need for delaying control
            transfers, and therefore never calls USBDeferStatusStage(), then the
            application firmware should not call USBCtrlEPAllowStatusStage().

  Description:

  Conditions:
    None

  Input:

  Return:

  Remarks:
    None
  *****************************************************************************/
void USBCtrlEPAllowStatusStage(void)
{
    //Check and set two flags, prior to actually modifying any BDT entries.
    //This double checking is necessary to make certain that
    //USBCtrlEPAllowStatusStage() can be called twice simultaneously (ex: once
    //in main loop context, while simultaneously getting an interrupt which
    //tries to call USBCtrlEPAllowStatusStage() again, at the same time).
    if(USBStatusStageEnabledFlag1 == FALSE)
    {
        USBStatusStageEnabledFlag1 = TRUE;
        if(USBStatusStageEnabledFlag2 == FALSE)
        {
            USBStatusStageEnabledFlag2 = TRUE;

            //Determine which endpoints (EP0 IN or OUT needs arming for the status
            //stage), based on the type of control transfer currently pending.
            if(controlTransferState == CTRL_TRF_RX)
            {
                pBDTEntryIn[0]->CNT = 0;
                pBDTEntryIn[0]->STAT= _USIE|_DAT1|(_DTSEN & _DTS_CHECKING_ENABLED);
            }
            else if(controlTransferState == CTRL_TRF_TX)
            {
        		BothEP0OutUOWNsSet = FALSE;	//Indicator flag used in USBCtrlTrfOutHandler()

        		pBDTEntryEP0OutCurrent->CNT = USB_EP0_BUFF_SIZE;
        		pBDTEntryEP0OutCurrent->ADR = ConvertToPhysicalAddress(&SetupPkt);
        		pBDTEntryEP0OutCurrent->STAT= _USIE|_BSTALL; //Prepare endpoint to accept a SETUP transaction
        		BothEP0OutUOWNsSet = TRUE;	//Indicator flag used in USBCtrlTrfOutHandler()

                //This EP0 OUT buffer receives the 0-byte OUT status stage packet.
        		pBDTEntryEP0OutNext->CNT = USB_EP0_BUFF_SIZE;
        		pBDTEntryEP0OutNext->ADR = ConvertToPhysicalAddress(&SetupPkt);
        		pBDTEntryEP0OutNext->STAT= _USIE;           // Note: DTSEN is 0
            }
        }
    }
}


/*******************************************************************************
  Function: void USBCtrlEPAllowDataStage(void);

  Summary: This function allows the data stage of either a host-to-device or
            device-to-host control transfer (with data stage) to complete.
            This function is meant to be used in conjunction with either the
            USBDeferOUTDataStage() or USBDeferINDataStage().  If the firmware
            does not call either USBDeferOUTDataStage() or USBDeferINDataStage(),
            then the firmware does not need to manually call
            USBCtrlEPAllowDataStage(), as the USB stack will call this function
            instead.

  Description:

  Conditions: A control transfer (with data stage) should already be pending,
                if the firmware calls this function.  Additionally, the firmware
                should have called either USBDeferOUTDataStage() or
                USBDeferINDataStage() at the start of the control transfer, if
                the firmware will be calling this function manually.

  Input:

  Return:

  Remarks:
  *****************************************************************************/
void USBCtrlEPAllowDataStage(void)
{
    USBDeferINDataStagePackets  = FALSE;
    USBDeferOUTDataStagePackets = FALSE;

    if ( controlTransferState == CTRL_TRF_RX ) //(<setup><out><out>...<out><in>)
    {
        //Prepare EP0 OUT to receive the first OUT data packet in the data stage sequence.
        pBDTEntryEP0OutNext->CNT = USB_EP0_BUFF_SIZE;
        pBDTEntryEP0OutNext->ADR = ConvertToPhysicalAddress(CtrlTrfData);
        pBDTEntryEP0OutNext->STAT= _USIE|_DAT1|(_DTSEN & _DTS_CHECKING_ENABLED);
    }
    else    //else must be controlTransferState == CTRL_TRF_TX (<setup><in><in>...<in><out>)
    {
        //Error check the data stage byte count.  Make sure the user specified
        //value was no greater than the number of bytes the host requested.
		if ( SetupPkt.req.wLength < inPipe.wCount )
			inPipe.wCount = SetupPkt.req.wLength;

		USBCtrlTrfTxService();  //Copies one IN data packet worth of data from application buffer
		                        //to CtrlTrfData buffer.  Also keeps track of how many bytes remaining.

	    //Cnt should have been initialized by responsible request owner (ex: by
	    //using the USBEP0SendRAMPtr() or USBEP0SendROMPtr() API function).
		pBDTEntryIn[0]->ADR = ConvertToPhysicalAddress(CtrlTrfData);
		pBDTEntryIn[0]->STAT = _USIE|_DAT1|(_DTSEN & _DTS_CHECKING_ENABLED);
    }
}


/******************************************************************************/
/** Internal Functions *********************************************************/
/******************************************************************************/

/********************************************************************
 * Function:        void USBConfigureEndpoint(BYTE EPNum, BYTE direction)
 *******************************************************************/
static void USBConfigureEndpoint(BYTE EPNum, BYTE direction)
{
    BDT_ENTRY* handle;

    handle = (volatile BDT_ENTRY*)&BDT[EP0_OUT_EVEN]; //Get address of start of BDT
    handle += EP(EPNum,direction,0);     //Add in offset to the BDT of interest

    handle->STAT &= ~_UOWN;  //mostly redundant, since USBStdSetCfgHandler()

    if(direction == OUT_FROM_HOST)
        pBDTEntryOut[EPNum] = handle;
    else
        pBDTEntryIn[EPNum] = handle;

    handle->STAT   &= ~_DTSMASK;
    handle[1].STAT |=  _DTSMASK;
}

/******************************************************************************
 * Function:        void USBCtrlEPServiceComplete(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine wrap up the remaining tasks in wCount
 *                  a Setup Request. Its main task is to set the endpoint
 *                  controls appropriately for a given situation. See code
 *                  below.
 *                  There are three main scenarios:
 *                  a) There was no handler for the Request, in this case
 *                     a STALL should be sent out.
 *                  b) The host has requested a read control transfer,
 *                     endpoints are required to be setup in a specific way.
 *                  c) The host has requested a write control transfer, or
 *                     a control data stage is not required, endpoints are
 *                     required to be setup in a specific way.
 *
 *                  Packet processing is resumed by clearing PKTDIS bit.
 *
 * Note:            None
 *****************************************************************************/
static void USBCtrlEPServiceComplete(void)
{
    USBPacketDisable = 0;

    if ( !PIPE_BUSY(inPipe.info) )
    {
        if ( PIPE_BUSY(outPipe.info) )
        {
            controlTransferState = CTRL_TRF_RX;

            if ( USBDeferOUTDataStagePackets == FALSE )
                USBCtrlEPAllowDataStage();

            USBStatusStageEnabledFlag2 = FALSE;
            USBStatusStageEnabledFlag1 = FALSE;
        }
        else
        {
            pBDTEntryEP0OutNext->CNT = USB_EP0_BUFF_SIZE;
            pBDTEntryEP0OutNext->ADR = ConvertToPhysicalAddress(&SetupPkt);
            pBDTEntryEP0OutNext->STAT= _USIE|_DAT0|(_DTSEN & _DTS_CHECKING_ENABLED)|_BSTALL;
            pBDTEntryIn[0]->STAT     = _USIE|_BSTALL;
        }
    }
    else    // A module has claimed ownership of the control transfer session.
    {
		if ( SetupPkt__DataDir == USB_SETUP_DEVICE_TO_HOST_BITFIELD )
		{
			controlTransferState = CTRL_TRF_TX;

			if ( USBDeferINDataStagePackets == FALSE )
                USBCtrlEPAllowDataStage();

            USBStatusStageEnabledFlag2 = FALSE;
            USBStatusStageEnabledFlag1 = FALSE;

            if ( USBDeferStatusStagePacket == FALSE )
                USBCtrlEPAllowStatusStage();
		}
		else
		{
			controlTransferState = CTRL_TRF_RX;     //Since this is a HOST_TO_DEVICE control transfer

			//1. Prepare OUT EP to receive the next SETUP packet.
			pBDTEntryEP0OutNext->CNT = USB_EP0_BUFF_SIZE;
			pBDTEntryEP0OutNext->ADR = ConvertToPhysicalAddress(&SetupPkt);
			pBDTEntryEP0OutNext->STAT= _USIE|_BSTALL;

			//2. Prepare for IN status stage of the control transfer
            USBStatusStageEnabledFlag2 = FALSE;
            USBStatusStageEnabledFlag1 = FALSE;
			if(USBDeferStatusStagePacket == FALSE)
            {
                USBCtrlEPAllowStatusStage();
            }
		}
    }
}


/******************************************************************************
 * Function:        void USBCtrlTrfTxService(void)
 *
 * PreCondition:    pSrc, wCount, and usb_stat.ctrl_trf_mem are setup properly.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine is used for device to host control transfers
 *					(IN transactions).  This function takes care of managing a
 *                  transfer over multiple USB transactions.
 *					This routine should be called from only two places.
 *                  One from USBCtrlEPServiceComplete() and one from
 *                  USBCtrlTrfInHandler().
 *
 * Note:
 *****************************************************************************/
static void USBCtrlTrfTxService(void)
{
    BYTE byteToSend;

    byteToSend = USB_EP0_BUFF_SIZE;
    if ( inPipe.wCount < (BYTE)USB_EP0_BUFF_SIZE )
    {
        byteToSend = inPipe.wCount;

        if ( shortPacketStatus == SHORT_PKT_NOT_USED )
            shortPacketStatus = SHORT_PKT_PENDING;
        else if ( shortPacketStatus == SHORT_PKT_PENDING )
            shortPacketStatus = SHORT_PKT_SENT;
    }

    //Keep track of how many bytes remain to be sent in the transfer, by
    //subtracting the number of bytes about to be sent from the total.
    inPipe.wCount -= byteToSend;

    //Next, load the number of bytes to send to BC7..0 in buffer descriptor.
    //Note: Control endpoints may never have a max packet size of > 64 bytes.
    //Therefore, the BC8 and BC9 bits should always be maintained clear.
    pBDTEntryIn[0]->CNT = byteToSend;

    //Now copy the data from the source location, to the CtrlTrfData[] buffer,
    //which we will send to the host.
    pDst = (volatile BYTE*)CtrlTrfData;                // Set destination pointer
    if ( PIPE_CTRL_TRF_MEM(inPipe.info) == USB_EP0_ROM )
    {
        while ( byteToSend )
        {
            *pDst++ = *inPipe.pSrc.bRom++;
            byteToSend--;
        }
    }
    else  // RAM
    {
        while ( byteToSend )
        {
            *pDst++ = *inPipe.pSrc.bRam++;
            byteToSend--;
        }
    }
}

/******************************************************************************
 * Function:        void USBCtrlTrfRxService(void)
 *
 * PreCondition:    pDst and wCount are setup properly.
 *                  pSrc is always &CtrlTrfData
 *                  usb_stat.ctrl_trf_mem is always USB_EP0_RAM.
 *                  wCount should be set to 0 at the start of each control
 *                  transfer.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine is used for host to device control transfers
 *					(uses OUT transactions).  This function receives the data that arrives
 *					on EP0 OUT, and copies it into the appropriate outPipe.pDst.bRam
 *					buffer.  Once the host has sent all the data it was intending
 *					to send, this function will call the appropriate outPipe.pFunc()
 *					handler (unless it is NULL), so that it can be used by the
 *					intended target firmware.
 *
 * Note:            None
 *****************************************************************************/
static void USBCtrlTrfRxService(void)
{
    BYTE byteToRead;
    BYTE i;

    byteToRead = pBDTEntryEP0OutCurrent->CNT;

    if ( byteToRead > outPipe.wCount )
        byteToRead = outPipe.wCount;

	outPipe.wCount -= byteToRead;

    for (i = 0; i < byteToRead; i++)
        *outPipe.pDst.bRam++ = CtrlTrfData[i];

    if ( outPipe.wCount > 0 )
    {
        pBDTEntryEP0OutNext->CNT = USB_EP0_BUFF_SIZE;
        pBDTEntryEP0OutNext->ADR = ConvertToPhysicalAddress(CtrlTrfData);

        if ( (pBDTEntryEP0OutCurrent->STAT & _DTSMASK) == 0 )
            pBDTEntryEP0OutNext->STAT = _USIE|_DAT1|(_DTSEN & _DTS_CHECKING_ENABLED);
        else
            pBDTEntryEP0OutNext->STAT = _USIE|_DAT0|(_DTSEN & _DTS_CHECKING_ENABLED);
    }
    else
    {
        pBDTEntryEP0OutNext->CNT = USB_EP0_BUFF_SIZE;
        pBDTEntryEP0OutNext->ADR = ConvertToPhysicalAddress(&SetupPkt);
        pBDTEntryEP0OutNext->STAT = _USIE|_BSTALL;

        if ( outPipe.pFunc != NULL )
            outPipe.pFunc();

        outPipe.info &= ~PIPE_BUSY_MASK;

        if ( USBDeferStatusStagePacket == FALSE )
            USBCtrlEPAllowStatusStage();
    }
}


/********************************************************************
 * Function:        void USBStdSetCfgHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine first disables all endpoints by
 *                  clearing UEP registers. It then configures
 *                  (initializes) endpoints by calling the callback
 *                  function USBCBInitEP().
 *
 * Note:            None
 *******************************************************************/
static void USBStdSetCfgHandler(void)
{
    BYTE i;

    // This will generate a zero length packet
    inPipe.info |= PIPE_BUSY_MASK;

    //Clear all of the endpoint control registers
    DisableNonZeroEndpoints(USB_MAX_EP_NUMBER);

    //Clear all of the BDT entries
    memset((void*)&BDT[0], 0x00, sizeof(BDT));

    // Assert reset request to all of the Ping Pong buffer pointers
    USBPingPongBufferReset = 1;

	for(i = 0; i < (USB_MAX_EP_NUMBER+1); i++)
	{
		ep_data_in[i] = 0;
        ep_data_out[i] = 0;
	}

    //clear the alternate interface settings
    memset(USBAlternateInterface, 0x00, USB_MAX_NUM_INT);

    //Stop trying to reset ping pong buffer pointers
    USBPingPongBufferReset = 0;

    pBDTEntryIn[0] = (volatile BDT_ENTRY*)&BDT[EP0_IN_EVEN];

	//Set the next out to the current out packet
    pBDTEntryEP0OutCurrent = (volatile BDT_ENTRY*)&BDT[EP0_OUT_EVEN];
    pBDTEntryEP0OutNext = pBDTEntryEP0OutCurrent;

    //set the current configuration
    USBActiveConfiguration = SetupPkt.cfg.bConfigurationValue;

    //if the configuration value == 0
    if(USBActiveConfiguration == 0)
    {
        USBDeviceState = ADDRESS_STATE;
    }
    else
    {
        USB_SET_CONFIGURATION_HANDLER(EVENT_CONFIGURED,(void*)&USBActiveConfiguration,1);
        USBDeviceState = CONFIGURED_STATE;
    }
}


/********************************************************************
 * Function:        void USBStdGetDscHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine handles the standard GET_DESCRIPTOR
 *                  request.
 *
 * Note:            None
 *******************************************************************/
static void USBStdGetDscHandler(void)
{
    if ( SetupPkt.req.bmRequestType == 0x80 )
    {
        inPipe.info = USB_EP0_ROM | USB_EP0_BUSY | USB_EP0_INCLUDE_ZERO;

        switch ( SetupPkt.dsc.bDescriptorType )
        {
            case USB_DESCRIPTOR_DEVICE:
                inPipe.pSrc.bRom = (BYTE*)USB_USER_DEVICE_DESCRIPTOR;
                inPipe.wCount = sizeof(device_dsc);
                break;
            case USB_DESCRIPTOR_CONFIGURATION:
                inPipe.pSrc.bRom = USB_USER_CONFIG_DESCRIPTOR[SetupPkt.dsc.bDscIndex];
                inPipe.wCount  = inPipe.pSrc.bRom[2];
                inPipe.wCount |= inPipe.pSrc.bRom[3] << 8;
                break;
            case USB_DESCRIPTOR_STRING:
                if(SetupPkt.dsc.bDscIndex < USB_NUM_STRING_DESCRIPTORS)
                {
                    inPipe.pSrc.bRom = USB_SD_Ptr[SetupPkt.dsc.bDscIndex];
                    inPipe.wCount = *inPipe.pSrc.bRom;
                }
                else
                    inPipe.info = 0;
                break;
            default:
                inPipe.info = 0;
                break;
        }
    }
}

/********************************************************************
 * Function:        void USBStdGetStatusHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine handles the standard GET_STATUS request
 *
 * Note:            None
 *******************************************************************/
static void USBStdGetStatusHandler(void)
{
    CtrlTrfData[0] = 0;                 // Initialize content
    CtrlTrfData[1] = 0;

    switch(SetupPkt__Recipient)
    {
        case USB_SETUP_RECIPIENT_DEVICE_BITFIELD:
            inPipe.info |= PIPE_BUSY_MASK;
            CtrlTrfData[0]  |= 0x01;
            if ( RemoteWakeup == TRUE )
                CtrlTrfData[0] |= 0x02;
            break;
        case USB_SETUP_RECIPIENT_INTERFACE_BITFIELD:
            inPipe.info |= PIPE_BUSY_MASK;     // No data to update
            break;
        case USB_SETUP_RECIPIENT_ENDPOINT_BITFIELD:
            inPipe.info |= PIPE_BUSY_MASK;
            {
                BDT_ENTRY *p;

                if ( SetupPkt__EPDir == 0 )
                    p = (BDT_ENTRY*)pBDTEntryOut[SetupPkt__EPNum];
                else
                    p = (BDT_ENTRY*)pBDTEntryIn[SetupPkt__EPNum];

                if ( (p->STAT & _UOWN) && (p->STAT & _BSTALL) )
                    CtrlTrfData[0]=0x01;    // Set bit0
                break;
            }
    }

    if ( PIPE_BUSY(inPipe.info) )
    {
        inPipe.pSrc.bRam = (BYTE*)CtrlTrfData;          // Set Source
        inPipe.info |= PIPE_CTRL_TRF_MEM_MASK;//USB_EP0_RAM;    // Set memory type
        inPipe.wCount = 2;                         // Set data count
    }
}

/********************************************************************
 * Function:        void USBStallHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:
 *
 * Overview:        This function handles the event of a STALL
 *                  occuring on the bus
 *
 * Note:            None
 *******************************************************************/
static void USBStallHandler(void)
{
    /*
     * Does not really have to do anything here,
     * even for the control endpoint.
     * All BDs of Endpoint 0 are owned by SIE right now,
     * but once a Setup Transaction is received, the ownership
     * for EP0_OUT will be returned to CPU.
     * When the Setup Transaction is serviced, the ownership
     * for EP0_IN will then be forced back to CPU by firmware.
     */

    /* v2b fix */
    if ( EP0STALL == 1 )
    {
        // UOWN - if 0, owned by CPU, if 1, owned by SIE
        if ( pBDTEntryEP0OutCurrent->STAT == _USIE && pBDTEntryIn[0]->STAT == (_USIE|_BSTALL) )
            pBDTEntryEP0OutCurrent->STAT = _USIE|_DAT0|(_DTSEN & _DTS_CHECKING_ENABLED)|_BSTALL;

        EP0STALL = 0;               // Clear stall status
    }

    USBClearInterruptFlag(USBStallIFReg,USBStallIFBitNum);
}

/********************************************************************
 * Function:        void USBSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:
 *
 * Overview:        This function handles if the host tries to
 *                  suspend the device
 *
 * Note:            None
 *******************************************************************/
static void USBSuspend(void)
{
    USBActivityIE = 1;                     // Enable bus activity interrupt
    USBClearInterruptFlag(USBIdleIFReg,USBIdleIFBitNum);

    USBBusIsSuspended = TRUE;

    USB_SUSPEND_HANDLER(EVENT_SUSPEND,0,0);
}

/********************************************************************
 * Function:        void USBWakeFromSuspend(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:
 *
 * Note:            None
 *******************************************************************/
static void USBWakeFromSuspend(void)
{
    USBBusIsSuspended = FALSE;

    USB_WAKEUP_FROM_SUSPEND_HANDLER(EVENT_RESUME,0,0);
    USBActivityIE = 0;
    USBClearInterruptFlag(USBActivityIFReg,USBActivityIFBitNum);
}//end USBWakeFromSuspend

/********************************************************************
 * Function:        void USBCtrlEPService(void)
 *
 * PreCondition:    USTAT is loaded with a valid endpoint address.
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        USBCtrlEPService checks for three transaction
 *                  types that it knows how to service and services
 *                  them:
 *                  1. EP0 SETUP
 *                  2. EP0 OUT
 *                  3. EP0 IN
 *                  It ignores all other types (i.e. EP1, EP2, etc.)
 *
 * Note:            None
 *******************************************************************/
static void USBCtrlEPService(void)
{
    USBStatusStageTimeoutCounter = USB_STATUS_STAGE_TIMEOUT;

	//Check if the last transaction was on EP0 OUT endpoint (of any kind, to either the even or odd buffer if ping pong buffers used)
    if ( (USTATcopy & USTAT_EP0_PP_MASK) == USTAT_EP0_OUT_EVEN )
    {
		//Point to the EP0 OUT buffer of the buffer that arrived
        pBDTEntryEP0OutCurrent = (volatile BDT_ENTRY*)&BDT[(USTATcopy & USTAT_EP_MASK)>>1];

		//Set the next out to the current out packet
        pBDTEntryEP0OutNext = pBDTEntryEP0OutCurrent;

		//Toggle it to the next ping pong buffer (if applicable)
        *((BYTE*)&pBDTEntryEP0OutNext) ^= USB_NEXT_EP0_OUT_PING_PONG;

		//If the current EP0 OUT buffer has a SETUP packet
        if ( (pBDTEntryEP0OutCurrent->STAT & 0x3c) == (PID_SETUP << 2) )	// PID
        {
            unsigned char setup_cnt;

            for (setup_cnt = 0; setup_cnt < 8; setup_cnt++) //SETUP data packets always contain exactly 8 bytes.
            {
                *(BYTE*)((BYTE*)&SetupPkt + setup_cnt) = *(BYTE*)ConvertToVirtualAddress(pBDTEntryEP0OutCurrent->ADR);
                pBDTEntryEP0OutCurrent->ADR++;
            }
            pBDTEntryEP0OutCurrent->ADR = ConvertToPhysicalAddress(&SetupPkt);
            USBCtrlTrfSetupHandler();
        }
        else
            USBCtrlTrfOutHandler();
    }
    else if ( (USTATcopy & USTAT_EP0_PP_MASK) == USTAT_EP0_IN )
    {
        USBCtrlTrfInHandler();
    }
}

/********************************************************************
 * Function:        void USBCtrlTrfSetupHandler(void)
 *
 * PreCondition:    SetupPkt buffer is loaded with valid USB Setup Data
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine is a task dispatcher and has 3 stages.
 *                  1. It initializes the control transfer state machine.
 *                  2. It calls on each of the module that may know how to
 *                     service the Setup Request from the host.
 *                     Module Example: USBD, HID, CDC, MSD, ...
 *                     A callback function, USBCBCheckOtherReq(),
 *                     is required to call other module handlers.
 *                  3. Once each of the modules has had a chance to check if
 *                     it is responsible for servicing the request, stage 3
 *                     then checks direction of the transfer to determine how
 *                     to prepare EP0 for the control transfer.
 *                     Refer to USBCtrlEPServiceComplete() for more details.
 *
 * Note:            Microchip USB Firmware has three different states for
 *                  the control transfer state machine:
 *                  1. WAIT_SETUP
 *                  2. CTRL_TRF_TX (device sends data to host through IN transactions)
 *                  3. CTRL_TRF_RX (device receives data from host through OUT transactions)
 *                  Refer to firmware manual to find out how one state
 *                  is transitioned to another.
 *
 *                  A Control Transfer is composed of many USB transactions.
 *                  When transferring data over multiple transactions,
 *                  it is important to keep track of data source, data
 *                  destination, and data count. These three parameters are
 *                  stored in pSrc,pDst, and wCount. A flag is used to
 *                  note if the data source is from ROM or RAM.
 *
 *******************************************************************/
static void USBCtrlTrfSetupHandler(void)
{
    shortPacketStatus = SHORT_PKT_NOT_USED;
    USBDeferStatusStagePacket = FALSE;
    USBDeferINDataStagePackets = FALSE;
    USBDeferOUTDataStagePackets = FALSE;
    BothEP0OutUOWNsSet = FALSE;
    controlTransferState = WAIT_SETUP;

    pBDTEntryIn[0]->STAT &= ~_USIE;
    *((BYTE*)&pBDTEntryIn[0]) ^= USB_NEXT_EP0_IN_PING_PONG;
    pBDTEntryIn[0]->STAT &= ~_USIE;
    *((BYTE*)&pBDTEntryIn[0]) ^= USB_NEXT_EP0_IN_PING_PONG;
    pBDTEntryEP0OutNext->STAT &= ~_USIE;

    inPipe.info = 0;
    inPipe.wCount = 0;
    outPipe.info = 0;
    outPipe.wCount = 0;

    USBCheckStdRequest();                                       //Check for standard USB "Chapter 9" requests.
    USB_NONSTANDARD_EP0_REQUEST_HANDLER(EVENT_EP0_REQUEST,0,0); //Check for USB device class specific requests

    USBCtrlEPServiceComplete();
}//end USBCtrlTrfSetupHandler


/******************************************************************************
 * Function:        void USBCtrlTrfOutHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine handles an OUT transaction according to
 *                  which control transfer state is currently active.
 *
 * Note:            Note that if the the control transfer was from
 *                  host to device, the session owner should be notified
 *                  at the end of each OUT transaction to service the
 *                  received data.
 *
 *****************************************************************************/
static void USBCtrlTrfOutHandler(void)
{
    if(controlTransferState == CTRL_TRF_RX)
    {
        USBCtrlTrfRxService();	//Copies the newly received data into the appropriate buffer and configures EP0 OUT for next transaction.
    }
    else //In this case the last OUT transaction must have been a status stage of a CTRL_TRF_TX (<setup><in><in>...<OUT>  <-- this last OUT just occurred as the status stage)
    {
        controlTransferState = WAIT_SETUP;

		if(BothEP0OutUOWNsSet == FALSE)
		{
	        pBDTEntryEP0OutNext->CNT = USB_EP0_BUFF_SIZE;
	        pBDTEntryEP0OutNext->ADR = ConvertToPhysicalAddress(&SetupPkt);
	        pBDTEntryEP0OutNext->STAT= _USIE|_DAT0|(_DTSEN & _DTS_CHECKING_ENABLED)|_BSTALL;
		}
		else
			BothEP0OutUOWNsSet = FALSE;
    }
}

/******************************************************************************
 * Function:        void USBCtrlTrfInHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine handles an IN transaction according to
 *                  which control transfer state is currently active.
 *
 * Note:            A Set Address Request must not change the acutal address
 *                  of the device until the completion of the control
 *                  transfer. The end of the control transfer for Set Address
 *                  Request is an IN transaction. Therefore it is necessary
 *                  to service this unique situation when the condition is
 *                  right. Macro mUSBCheckAdrPendingState is defined in
 *                  usb9.h and its function is to specifically service this
 *                  event.
 *****************************************************************************/
static void USBCtrlTrfInHandler(void)
{
    BYTE lastDTS;

    lastDTS = (pBDTEntryIn[0]->STAT & _DTSMASK)? 1: 0;

    //switch to the next ping pong buffer
    *((BYTE*)&pBDTEntryIn[0]) ^= USB_NEXT_EP0_IN_PING_PONG;

    if ( USBDeviceState == ADR_PENDING_STATE )
    {
        UADDR = SetupPkt.dev.bDevADR;
        if ( UADDR != 0 )
            USBDeviceState=ADDRESS_STATE;
        else
            USBDeviceState=DEFAULT_STATE;
    }

    if ( controlTransferState == CTRL_TRF_TX )
    {
        pBDTEntryIn[0]->ADR = ConvertToPhysicalAddress(CtrlTrfData);
        USBCtrlTrfTxService();

        if(shortPacketStatus == SHORT_PKT_SENT)
        {
            pBDTEntryIn[0]->STAT = _USIE|_BSTALL;
        }
        else
        {
            if ( lastDTS == 0 )
                pBDTEntryIn[0]->STAT = _USIE|_DAT1|(_DTSEN & _DTS_CHECKING_ENABLED);
            else
                pBDTEntryIn[0]->STAT = _USIE|_DAT0|(_DTSEN & _DTS_CHECKING_ENABLED);
        }
    }
	else // must have been a CTRL_TRF_RX status stage IN packet (<setup><out><out>...<IN>  <-- this last IN just occurred as the status stage)
	{
        if ( PIPE_BUSY(outPipe.info) )
        {
            if ( outPipe.pFunc != NULL )
                outPipe.pFunc();

            outPipe.info &= ~PIPE_BUSY_MASK;
        }

        controlTransferState = WAIT_SETUP;
	}
}

/********************************************************************
 * Function:        void USBCheckStdRequest(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This routine checks the setup data packet to see
 *                  if it knows how to handle it
 *
 * Note:            None
 *******************************************************************/
static void USBCheckStdRequest(void)
{
    if(SetupPkt__RequestType != USB_SETUP_TYPE_STANDARD_BITFIELD) return;

    switch(SetupPkt.req.bRequest)
    {
        case USB_REQUEST_SET_ADDRESS:
            inPipe.info |= PIPE_BUSY_MASK;
            USBDeviceState = ADR_PENDING_STATE;       // Update state only
            break;
        case USB_REQUEST_GET_DESCRIPTOR:
            USBStdGetDscHandler();
            break;
        case USB_REQUEST_SET_CONFIGURATION:
            USBStdSetCfgHandler();
            break;
        case USB_REQUEST_GET_CONFIGURATION:
            inPipe.pSrc.bRam = (BYTE*)&USBActiveConfiguration;         // Set Source
            inPipe.info |= PIPE_CTRL_TRF_MEM_MASK;  //USB_EP0_RAM;               // Set memory type
            inPipe.wCount = 1;                         // Set data count
            inPipe.info |= PIPE_BUSY_MASK;
            break;
        case USB_REQUEST_GET_STATUS:
            USBStdGetStatusHandler();
            break;
        case USB_REQUEST_CLEAR_FEATURE:
        case USB_REQUEST_SET_FEATURE:
            USBStdFeatureReqHandler();
            break;
        case USB_REQUEST_GET_INTERFACE:
            inPipe.pSrc.bRam = (BYTE*)&USBAlternateInterface[SetupPkt.id.bIntfID];  // Set source
            inPipe.info |= PIPE_CTRL_TRF_MEM_MASK;  //USB_EP0_RAM;               // Set memory type
            inPipe.wCount = 1;                         // Set data count
            inPipe.info |= PIPE_BUSY_MASK;
            break;
        case USB_REQUEST_SET_INTERFACE:
            inPipe.info |= PIPE_BUSY_MASK;
            USBAlternateInterface[SetupPkt.id.bIntfID] = SetupPkt.id.bAltID;
            break;
        case USB_REQUEST_SET_DESCRIPTOR:
            USB_SET_DESCRIPTOR_HANDLER(EVENT_SET_DESCRIPTOR,0,0);
            break;
        case USB_REQUEST_SYNCH_FRAME:
        default:
            break;
    }
}

/********************************************************************
 * Function:        void USBStdFeatureReqHandler(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          Can alter BDT entries.  Can also modify USB stack
 *                  Maintained variables.
 *
 * Side Effects:    None
 *
 * Overview:        This routine handles the standard SET & CLEAR
 *                  FEATURES requests
 *
 * Note:            This is a private function, intended for internal
 *                  use by the USB stack, when processing SET/CLEAR
 *                  feature requests.
 *******************************************************************/
static void USBStdFeatureReqHandler(void)
{
    BDT_ENTRY *p;
    EP_STATUS current_ep_data;
    unsigned char* pUEP;

    //Check if the host sent a valid SET or CLEAR feature (remote wakeup) request.
    if((SetupPkt.feat.bFeature == USB_FEATURE_DEVICE_REMOTE_WAKEUP)&&
       (SetupPkt__Recipient == USB_SETUP_RECIPIENT_DEVICE_BITFIELD))
    {
        inPipe.info |= PIPE_BUSY_MASK;
        if(SetupPkt.req.bRequest == USB_REQUEST_SET_FEATURE)
            RemoteWakeup = TRUE;
        else
            RemoteWakeup = FALSE;
    }//end if

    //Check if the host sent a valid SET or CLEAR endpoint halt request.
    if ( (SetupPkt.feat.bFeature == USB_FEATURE_ENDPOINT_HALT)                 &&
         (SetupPkt__Recipient == USB_SETUP_RECIPIENT_ENDPOINT_BITFIELD)   &&
         (SetupPkt__EPNum != 0) && (SetupPkt__EPNum <= USB_MAX_EP_NUMBER) &&
         (USBDeviceState == CONFIGURED_STATE) )
    {
		inPipe.info |= PIPE_BUSY_MASK;

        //Fetch a pointer to the BDT that the host wants to SET/CLEAR halt on.
        if ( SetupPkt__EPDir == OUT_FROM_HOST )
        {
            p = (BDT_ENTRY*)pBDTEntryOut[SetupPkt__EPNum];
            current_ep_data = ep_data_out[SetupPkt__EPNum];
        }
        else
        {
            p = (BDT_ENTRY*)pBDTEntryIn[SetupPkt__EPNum];
            current_ep_data = ep_data_in[SetupPkt__EPNum];
        }

        //If ping pong buffering is enabled on the requested endpoint, need
        //to point to the one that is the active BDT entry which the SIE will
        //use for the next attempted transaction on that EP number.
        if ( !(current_ep_data & EP_PINGPONG_STATE_FLAG) ) //Check if even
            USBHALPingPongSetToEven(p);
        else //else must have been odd
            USBHALPingPongSetToOdd(p);

        //Update the BDT pointers with the new, next entry based on the feature
        //  request
        if ( SetupPkt__EPDir == OUT_FROM_HOST )
            pBDTEntryOut[SetupPkt__EPNum] = (volatile BDT_ENTRY *)p;
        else
            pBDTEntryIn[SetupPkt__EPNum]  = (volatile BDT_ENTRY *)p;

		//Check if it was a SET_FEATURE endpoint halt request
        if ( SetupPkt.req.bRequest == USB_REQUEST_SET_FEATURE )
        {
            if ( p->STAT & _UOWN )
            {
                //Mark that we are terminating this transfer and that the user
                //  needs to be notified later
                if ( SetupPkt__EPDir == OUT_FROM_HOST )
                    ep_data_out[SetupPkt__EPNum] |= EP_TRANSFER_TERMINATED_FLAG;
                else
                    ep_data_in[SetupPkt__EPNum] |= EP_TRANSFER_TERMINATED_FLAG;
            }

			//Then STALL the endpoint
            p->STAT |= _USIE|_BSTALL;
        }
        else
        {
			//Else the request must have been a CLEAR_FEATURE endpoint halt.

            //toggle over the to the non-active BDT
            USBAdvancePingPongBuffer(&p);

            if ( p->STAT & _UOWN )
            {
                //Clear UOWN and set DTS state so it will be correct the next time
                //the application firmware uses USBTransferOnePacket() on the EP.
                p->STAT &= ~_USIE;    //Clear UOWN bit
                p->STAT |= _DAT1;     //Set DTS to DATA1
                USB_TRANSFER_TERMINATED_HANDLER(EVENT_TRANSFER_TERMINATED,p,sizeof(p));
            }
            else
            {
                //UOWN already clear, but still need to set DTS to DATA1
                p->STAT |= _DAT1;
            }

            USBAdvancePingPongBuffer(&p);

            if ( (current_ep_data & EP_TRANSFER_TERMINATED_FLAG) || (p->STAT & _UOWN) )
            {
                if ( SetupPkt__EPDir == OUT_FROM_HOST )
                    ep_data_out[SetupPkt__EPNum] &= ~EP_TRANSFER_TERMINATED_FLAG;
                else
                    ep_data_in[SetupPkt__EPNum] &= ~EP_TRANSFER_TERMINATED_FLAG;

                //clear UOWN, clear DTS to DATA0, and finally remove the STALL condition
                p->STAT &= ~(_USIE | _DAT1 | _BSTALL);
                USB_TRANSFER_TERMINATED_HANDLER(EVENT_TRANSFER_TERMINATED,p,sizeof(p));
            }
            else
                p->STAT &= ~(_USIE | _DAT1 | _BSTALL);


			//Get a pointer to the appropriate UEPn register
            pUEP = (unsigned char*)(&UEP0 + SetupPkt__EPNum);

			//Clear the STALL bit in the UEP register
            *pUEP &= ~UEP_STALL;
        }
    }
}

static void DisableNonZeroEndpoints(BYTE last_ep_num)
{
    memset(&UEP1, 0, last_ep_num);
}

/** EOF USBDevice.c *****************************************************/
