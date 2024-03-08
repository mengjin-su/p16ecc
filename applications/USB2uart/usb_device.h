#ifndef USBDEVICE_H
#define USBDEVICE_H

#include "usb_common.h"
#include "usb_ch9.h"

#define USB_EP0_ROM            0x00     //Data comes from RAM
#define USB_EP0_RAM            0x01     //Data comes from ROM
#define USB_EP0_BUSY           0x80     //The PIPE is busy
#define USB_EP0_INCLUDE_ZERO   0x40     //include a trailing zero packet
#define USB_EP0_NO_DATA        0x00     //no data to send
#define USB_EP0_NO_OPTIONS     0x00     //no options set

enum
{
    DETACHED_STATE    = 0x00,
    ATTACHED_STATE    = 0x01,
    POWERED_STATE     = 0x02,
    DEFAULT_STATE     = 0x04,
    ADR_PENDING_STATE = 0x08,
    ADDRESS_STATE     = 0x10,
    CONFIGURED_STATE  = 0x20
};
typedef unsigned char USB_DEVICE_STATE;


/* USB device stack events description here - DWF */
enum
{
    EVENT_CONFIGURED = EVENT_DEVICE_STACK_BASE,
    EVENT_SET_DESCRIPTOR,
    EVENT_EP0_REQUEST,
    EVENT_ATTACH,
    EVENT_TRANSFER_TERMINATED
};
typedef unsigned int USB_DEVICE_STACK_EVENTS;

/******************************************************************************/
/** External API Functions ****************************************************/
/******************************************************************************/
void USBDeviceInit(void);
void USBDeviceTasks(void);
void USBEnableEndpoint(BYTE ep, BYTE options);
void *USBTransferOnePacket(BYTE ep,BYTE dir,BYTE* data,BYTE len);
void USBStallEndpoint(BYTE ep, BYTE dir);
void USBCancelIO(BYTE endpoint);
void USBDeviceDetach(void);
void USBDeviceAttach(void);
void USBCtrlEPAllowStatusStage(void);
void USBCtrlEPAllowDataStage(void);
void USBDeferOUTDataStage(void);
void USBDeferStatusStage(void);

extern BOOL USBDeferStatusStagePacket;


//BOOL USBHandleBusy(void * handle);
#define USBHandleBusy(handle)               (handle? ((BDT_ENTRY*)handle)->STAT & _UOWN: 0)


//WORD USBHandleGetLength(void * handle);
#define USBHandleGetLength(handle)          ((BDT_ENTRY*)handle)->CNT


//WORD USBHandleGetAddr(void *);
#define USBHandleGetAddr(handle)            ConvertToVirtualAddress((((BDT_ENTRY*)handle)->ADR))


//void * USBGetNextHandle(BYTE ep_num, BYTE ep_dir);
#define USBGetNextHandle(ep_num, ep_dir)    ((ep_dir == OUT_FROM_HOST)? ((void *)pBDTEntryOut[ep_num]):((void *)pBDTEntryIn[ep_num]))


//void USBEP0Transmit(BYTE options);
#define USBEP0Transmit(options)             inPipe.info = (options | USB_EP0_BUSY)


//void USBEP0SendRAMPtr(BYTE* src, WORD size, BYTE Options);
#define USBEP0SendRAMPtr(src,size,options)	inPipe.pSrc.bRam = src,	\
											inPipe.wCount = size,	\
											inPipe.info = options | USB_EP0_BUSY | USB_EP0_RAM


//void USBEP0SendROMPtr(BYTE* src, WORD size, BYTE Options);
#define USBEP0SendROMPtr(src,size,options)  inPipe.pSrc.bRom = src,	\
											inPipe.wCount = size,	\
											inPipe.info = options | USB_EP0_BUSY | USB_EP0_ROM

/*
//void USBEP0Receive(BYTE* dest, WORD size, void (*function)());
#define USBEP0Receive(dest,size,function)  	outPipe.pDst.bRam = dest,	\
											outPipe.wCount = size,		\
											outPipe.pFunc = function,	\
											outPipe.info |= PIPE_BUSY_MASK
*/

//void * USBTxOnePacket(BYTE ep, BYTE* data, WORD len);
#define USBTxOnePacket(ep,data,len)     USBTransferOnePacket(ep,IN_TO_HOST,data,len)


//void * USBRxOnePacket(BYTE ep, BYTE* data, WORD len);
#define USBRxOnePacket(ep,data,len)		USBTransferOnePacket(ep,OUT_FROM_HOST,data,len)


BOOL USB_APPLICATION_EVENT_HANDLER(BYTE address, USB_EVENT event, void *pdata, WORD size);


void *USBDeviceCBGetDescriptor(UINT16 *length, UINT8 *ptr_type, DESCRIPTOR_ID *id);


typedef struct
{
    union
    {
        BYTE *bRam;
        BYTE *const bRom;
        WORD *wRam;
        WORD *const wRom;
    } pSrc;
    BYTE info;
    WORD wCount;
} IN_PIPE;

typedef struct
{
    union
    {
        BYTE *bRam;
        WORD *wRam;
    } pDst;
    BYTE info;
    WORD wCount;
    void (*pFunc)(void);
} OUT_PIPE;

extern volatile IN_PIPE  inPipe;
extern volatile OUT_PIPE outPipe;

#define PIPE_BUSY_MASK			0x80
#define PIPE_INCLUDE_Z_MASK		0x40
#define PIPE_CTRL_TRF_MEM_MASK	0x01
#define PIPE_BUSY(st)			((st) & PIPE_BUSY_MASK)
#define PIPE_CTRL_TRF_MEM(st)	((st) & PIPE_CTRL_TRF_MEM_MASK)


extern volatile BOOL RemoteWakeup;
extern volatile BOOL USBBusIsSuspended;
extern volatile USB_DEVICE_STATE USBDeviceState;
extern volatile BYTE USBActiveConfiguration;

#endif //USBD_H
