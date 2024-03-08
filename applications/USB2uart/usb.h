#ifndef _USB_H_
#define _USB_H_

// *****************************************************************************
// *****************************************************************************
// Section: All necessary USB Library headers
// *****************************************************************************
// *****************************************************************************
#include "GenericTypeDefs.h"
#include "usb_config.h"         // Must be defined by the application
#include "usb_common.h"         // Common USB library definitions
#include "usb_ch9.h"            // USB device framework definitions
#include "usb_device.h"         // USB Device abstraction layer interface
#include "usb_hal.h"            // Hardware Abstraction Layer interface

// *****************************************************************************
// *****************************************************************************
// Section: MCHPFSUSB Firmware Version
// *****************************************************************************
// *****************************************************************************
#define USB_MAJOR_VER   2       // Firmware version, major release number.
#define USB_MINOR_VER   9       // Firmware version, minor release number.
#define USB_DOT_VER     0       // Firmware version, dot release number.

#endif // _USB_H_
