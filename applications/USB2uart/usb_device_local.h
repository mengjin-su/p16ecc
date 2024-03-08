#ifndef LOCAL_H
#define LOCAL_H

/* Short Packet States - Used by Control Transfer Read  - CTRL_TRF_TX */
#define SHORT_PKT_NOT_USED  0
#define SHORT_PKT_PENDING   1
#define SHORT_PKT_SENT      2

/* Control Transfer States */
#define WAIT_SETUP          0
#define CTRL_TRF_TX         1
#define CTRL_TRF_RX         2

enum {EP_PINGPONG_STATE_FLAG 		= 1,
	  EP_TRANSFER_TERMINATED_FLAG	= 2};
typedef char EP_STATUS;

#define USB_NEXT_EP0_OUT_PING_PONG  0x0004
#define USB_NEXT_EP0_IN_PING_PONG   0x0004
#define USB_NEXT_PING_PONG          0x0004

#define EP0_OUT_EVEN    0
#define EP0_OUT_ODD     1
#define EP0_IN_EVEN     2
#define EP0_IN_ODD      3
#define EP1_OUT_EVEN    4
#define EP1_OUT_ODD     5
#define EP1_IN_EVEN     6
#define EP1_IN_ODD      7
#define EP2_OUT_EVEN    8
#define EP2_OUT_ODD     9
#define EP2_IN_EVEN     10
#define EP2_IN_ODD      11

#define EP(ep,dir,pp)   (4*ep+2*dir+pp)
#define BD(ep,dir,pp)   (4*(4*ep+2*dir+pp))

/****** Event callback enabling/disabling macros ********************
    This section of code is used to disable specific USB events that may not be
    desired by the user.  This can save code size and increase throughput and
    decrease CPU utiliazation.
********************************************************************/
#define USB_SUSPEND_HANDLER(event,pointer,size)                 USER_USB_CALLBACK_EVENT_HANDLER(event,pointer,size)
#define USB_WAKEUP_FROM_SUSPEND_HANDLER(event,pointer,size)     USER_USB_CALLBACK_EVENT_HANDLER(event,pointer,size)
#define USB_SOF_HANDLER(event,pointer,size)                     USER_USB_CALLBACK_EVENT_HANDLER(event,pointer,size)
#define USB_TRANSFER_TERMINATED_HANDLER(event,pointer,size)     USER_USB_CALLBACK_EVENT_HANDLER(event,pointer,size)
#define USB_ERROR_HANDLER(event,pointer,size)                   USER_USB_CALLBACK_EVENT_HANDLER(event,pointer,size)
#define USB_NONSTANDARD_EP0_REQUEST_HANDLER(event,pointer,size) USER_USB_CALLBACK_EVENT_HANDLER(event,pointer,size)
#define USB_SET_DESCRIPTOR_HANDLER(event,pointer,size)          USER_USB_CALLBACK_EVENT_HANDLER(event,pointer,size)
#define USB_SET_CONFIGURATION_HANDLER(event,pointer,size)       USER_USB_CALLBACK_EVENT_HANDLER(event,pointer,size)
#define USB_TRANSFER_COMPLETE_HANDLER(event,pointer,size)       USER_USB_CALLBACK_EVENT_HANDLER(event,pointer,size)

#endif