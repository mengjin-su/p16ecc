#ifndef USB_HAL_PIC16_H
#define USB_HAL_PIC16_H

#include "usb_config.h"

//----- USBEnableEndpoint() input defintions ----------------------------------
#define USB_HANDSHAKE_ENABLED   0x10
#define USB_HANDSHAKE_DISABLED  0x00

#define USB_OUT_ENABLED         0x04
#define USB_OUT_DISABLED        0x00

#define USB_IN_ENABLED          0x02
#define USB_IN_DISABLED         0x00

#define USB_ALLOW_SETUP         0x00
#define USB_DISALLOW_SETUP      0x08

#define USB_STALL_ENDPOINT      0x01

//----- usb_config.h input defintions -----------------------------------------
#define USB_PULLUP_ENABLE       0x10
#define USB_PULLUP_DISABLED     0x00

#define USB_INTERNAL_TRANSCEIVER 0x00
#define USB_EXTERNAL_TRANSCEIVER 0x08

#define USB_FULL_SPEED 0x04
#define USB_LOW_SPEED  0x00

//----- Interrupt Flag definitions --------------------------------------------
#define USBTransactionCompleteIE TRNIE
#define USBTransactionCompleteIF TRNIF
#define USBTransactionCompleteIFReg UIR
#define USBTransactionCompleteIFBitNum 0xF7		//AND mask for clearing TRNIF bit position 3

#define USBResetIE  URSTIE
#define USBResetIF  URSTIF
#define USBResetIFReg UIR
#define USBResetIFBitNum 0xFE					//AND mask for clearing URSTIF bit position 0

#define USBIdleIE 				IDLEIE
#define USBIdleIF 				IDLEIF
#define USBIdleIFReg 			UIR
#define USBIdleIFBitNum 		0xEF					//AND mask for clearing IDLEIF bit position 5

#define USBActivityIE 			ACTVIE
#define USBActivityIF 			ACTVIF
#define USBActivityIFReg 		UIR
#define USBActivityIFBitNum 	0xFB				//AND mask for clearing ACTVIF bit position 2

#define USBSOFIE 				SOFIE
#define USBSOFIF 				SOFIF
#define USBSOFIFReg 			UIR
#define USBSOFIFBitNum 			0xBF						//AND mask for clearing SOFIF bit position 6

#define USBStallIE 				STALLIE
#define USBStallIF 				STALLIF
#define USBStallIFReg 			UIR
#define USBStallIFBitNum 		0xDF					//AND mask for clearing STALLIF bit position 5

#define USBErrorIE 				UERRIE
#define USBErrorIF 				UERRIF
#define USBErrorIFReg 			UIR
#define USBErrorIFBitNum    	0xFD				//UERRIF bit position 1.  Note: This bit is read only and is cleared by clearing the enabled UEIR flags

//----- Event call back defintions --------------------------------------------
#define USB_SOF_INTERRUPT   0x40
#define USB_ERROR_INTERRUPT 0x02

//----- USB module control bits -----------------------------------------------
#define USBPingPongBufferReset 	PPBRST
#define USBSE0Event 			SE0
#define USBSuspendControl 		SUSPND
#define USBPacketDisable 		PKTDIS
#define USBResumeControl 		RESUME

//----- BDnSTAT bit definitions -----------------------------------------------
#define _BSTALL     0x04        //Buffer Stall enable
#define _DTSEN      0x08        //Data Toggle Synch enable
#define _INCDIS     0x10        //Address increment disable
#define _KEN        0x20        //SIE keeps buff descriptors enable
#define _DAT0       0x00        //DATA0 packet expected next
#define _DAT1       0x40        //DATA1 packet expected next
#define _DTSMASK    0x40        //DTS Mask
#define _USIE       0x80        //SIE owns buffer
#define _UCPU       0x00        //CPU owns buffer
#define _UOWN		0x80
#define _STAT_MASK  0xFF

#define USTAT_EP0_PP_MASK   0xfd
#define USTAT_EP_MASK       0x7E
#define USTAT_EP0_OUT       0x00
#define USTAT_EP0_OUT_EVEN  0x00
#define USTAT_EP0_OUT_ODD   0x02
#define USTAT_EP0_IN        0x04
#define USTAT_EP0_IN_EVEN   0x04
#define USTAT_EP0_IN_ODD    0x06

#define ENDPOINT_MASK 0b01111000

//----- U1EP bit definitions --------------------------------------------------
#define UEP_STALL 0x0001
// Cfg Control pipe for this ep
/* Endpoint configuration options for USBEnableEndpoint() function */
#define EP_CTRL     0x06            // Cfg Control pipe for this ep
#define EP_OUT      0x0C            // Cfg OUT only pipe for this ep
#define EP_IN       0x0A            // Cfg IN only pipe for this ep
#define EP_OUT_IN   0x0E            // Cfg both OUT & IN pipes for this ep

//----- Defintions for BDT address --------------------------------------------
#define BDT_ENTRY_SIZE          4
#define BDT_BASE_ADDR           0x2000
#define CTRL_TRF_SETUP_ADDR     (BDT_BASE_ADDR + BDT_ENTRY_SIZE*BDT_NUM_ENTRIES)
#define CTRL_TRF_DATA_ADDR      (CTRL_TRF_SETUP_ADDR + USB_EP0_BUFF_SIZE)


//----- Depricated defintions - will be removed at some point of time----------
//--------- Depricated in v2.2
#define _LS         0x00            // Use Low-Speed USB Mode
#define _FS         0x04            // Use Full-Speed USB Mode
#define _TRINT      0x00            // Use internal transceiver
#define _TREXT      0x08            // Use external transceiver
#define _PUEN       0x10            // Use internal pull-up resistor
#define _OEMON      0x40            // Use SIE output indicator

/*****************************************************************************/
/****** Type definitions *****************************************************/
/*****************************************************************************/
// BDT Entry Layout
typedef struct
{
    BYTE STAT;
    BYTE CNT;
    WORD ADR;
} BDT_ENTRY;


#define USTAT_PINGPONG(st)  (((st) >> 1) & 1)
#define USTAT_DIRECTION(st) (((st) >> 2) & 1)
#define USTAT_EP(st)        (((st) >> 3) & 0x0f)


typedef union
{
    BYTE* bRam;                         // Ram byte pointer: 2 bytes pointer pointing
    WORD* wRam;                         // Ram word poitner: 2 bytes poitner pointing
    BYTE* bRom;                     // Size depends on compiler setting
    WORD* wRom;
} POINTER;

/*****************************************************************************/
/****** Function prototypes and macro functions ******************************/
/*****************************************************************************/

#define ConvertToPhysicalAddress(a) (a)
#define ConvertToVirtualAddress(a)  ((void *)a)
#define USBClearUSBInterrupt()      USBIF = 0
#ifdef USB_INTERRUPT
    #define USBMaskInterrupts() 	USBIE = 0
    #define USBUnmaskInterrupts() 	USBIE = 1
#endif

#define USBInterruptFlag            USBIF

//STALLIE, IDLEIE, TRNIE, and URSTIE are all enabled by default and are required
#ifdef USB_INTERRUPT
    #define USBEnableInterrupts() 	USBIE = 1, PEIE = 1, GIE = 1
#endif

#define USBDisableInterrupts()  	USBIE = 0

#define SetConfigurationOptions()   UCFG = USB_PULLUP_OPTION | USB_TRANSCEIVER_OPTION | USB_SPEED_OPTION | USB_PING_PONG_MODE,\
                                    UEIE = 0x9F, UIE = 0x39 | USB_SOF_INTERRUPT | USB_ERROR_INTERRUPT



#define USBModuleDisable()  	UCON = 0, UIE = 0, USBDeviceState = DETACHED_STATE


#define USBClearInterruptFlag(reg_name, if_and_flag_mask)	reg_name &= if_and_flag_mask


/*****************************************************************************/
/****** Extern variable definitions ******************************************/
/*****************************************************************************/

//extern volatile USB_DEVICE_STATE USBDeviceState;
extern BYTE USBActiveConfiguration;
extern BDT_ENTRY* pBDTEntryOut[USB_MAX_EP_NUMBER+1];
extern BDT_ENTRY* pBDTEntryIn[USB_MAX_EP_NUMBER+1];

#endif
