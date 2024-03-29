/*
  IRK! Infrared Remote Controlled USB Keyboard $Rev:  $
  Copyright (C) 2010-2013 Andrew J. Armstrong

  This file is part of IRK!.

  IRK! is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  IRK! is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
  02111-1307  USA

  Author:
  Andrew J. Armstrong <androidarmstrong@gmail.com>
*/

#include "IRK.h"

#define STRING_INDEX_LANGUAGE       0
#define STRING_INDEX_IRK            1
#define STRING_INDEX_IRK_DESC       2

const unsigned int USB_VENDOR_ID = 0x5249;   // 'IR'
const unsigned int USB_PRODUCT_ID = 0x214B;  // 'K!'
const char USB_SELF_POWER = 0x80;            // 0x80 = Bus powered, 0xC0 = Self powered
const char USB_MAX_POWER = 50;               // Bus power required in units of 2 mA
const char USB_TRANSFER_TYPE = 0x03;         // 0x03 Interrupt
const char EP_IN_INTERVAL = 10;              // Measured in frame counts (i.e. 1 ms units for Low Speed, 125 us units for High Speed)
                                             // IRK! is a High Speed device (24 MHz MCU clock) operating as a Low Speed device (FSEN=0)
                                             // 2**(10-1) x 125 microsecond units (for USB2 High Speed devices)
                                             // = 64 ms intervals = 15.6 times/second 
                                             // The Host interrupts PIC for keyboard input this often.
                                             // IRK can handle about 10 IR commands per second, due to the time it takes
                                             // to transmit a single command, so this USB polling rate is adequate for IRK.

const char EP_OUT_INTERVAL = 12;             // 2**(12-1) x 125 microsecond units (for USB2 High Speed devices)
                                             // = 256 ms intervals = 3.9 times/second
                                             // The Host interrupts PIC for LED status output at most this often (if LED status change is pending).
                                             // IRK does not use LED status reports, so this rate can be set quite low.

const char USB_INTERRUPT = 1;
const char USB_HID_EP = 1;
const char USB_HID_RPT_SIZE = 42  // Keyboard       --> host
                            + 21  // Keyboard       <-- host
                            + 25  // SystemControl  --> host
                            + 25; // ConsumerDevice --> host
/* Device Descriptor */
const struct
{
    char bLength;               // bLength         - Descriptor size in bytes (12h)
    char bDescriptorType;       // bDescriptorType - The constant DEVICE (01h)
    unsigned int bcdUSB;        // bcdUSB          - USB specification release number (BCD)
    char bDeviceClass;          // bDeviceClass    - Class Code
    char bDeviceSubClass;       // bDeviceSubClass - Subclass code
    char bDeviceProtocol;       // bDeviceProtocol - Protocol code
    char bMaxPacketSize0;       // bMaxPacketSize0 - Maximum packet size for endpoint 0
    unsigned int idVendor;      // idVendor        - Vendor ID
    unsigned int idProduct;     // idProduct       - Product ID
    unsigned int bcdDevice;     // bcdDevice       - Device release number (BCD)
    char iManufacturer;         // iManufacturer   - Index of string descriptor for the manufacturer
    char iProduct;              // iProduct        - Index of string descriptor for the product.
    char iSerialNumber;         // iSerialNumber   - Index of string descriptor for the serial number.
    char bNumConfigurations;    // bNumConfigurations - Number of possible configurations
} device_dsc = 
  {
      0x12,                   // bLength
      0x01,                   // bDescriptorType
      0x0110,                 // bcdUSB   (USB 1.1)
      0x00,                   // bDeviceClass
      0x00,                   // bDeviceSubClass
      0x00,                   // bDeviceProtocol
      8,                      // bMaxPacketSize0
      USB_VENDOR_ID,          // idVendor
      USB_PRODUCT_ID,         // idProduct
      0x0003,                 // bcdDevice
      STRING_INDEX_IRK,       // iManufacturer
      STRING_INDEX_IRK_DESC,  // iProduct
      0x00,                   // iSerialNumber
      0x01                    // bNumConfigurations
  };

/* Configuration 1 Descriptor */
const char configDescriptor1[] = 
{
    // Configuration Descriptor
    0x09,                   // bLength             - Descriptor size in bytes
    0x02,                   // bDescriptorType     - The constant CONFIGURATION (02h)
    41,0x00,                // wTotalLength        - The number of bytes in the configuration descriptor and all of its subordinate descriptors
    1,                      // bNumInterfaces      - Number of interfaces in the configuration
    1,                      // bConfigurationValue - Identifier for Set Configuration and Get Configuration requests
    STRING_INDEX_IRK,       // iConfiguration      - Index of string descriptor for the configuration
    USB_SELF_POWER,         // bmAttributes        - Self/bus power and remote wakeup settings
    USB_MAX_POWER,          // bMaxPower           - Bus power required in units of 2 mA

    // Interface Descriptor
    0x09,                   // bLength - Descriptor size in bytes (09h)
    0x04,                   // bDescriptorType - The constant Interface (04h)
    0,                      // bInterfaceNumber - Number identifying this interface
    0,                      // bAlternateSetting - A number that identifies a descriptor with alternate settings for this bInterfaceNumber.
    2,                      // bNumEndpoint - Number of endpoints supported not counting endpoint zero
    0x03,                   // bInterfaceClass - Class code  (0x03 = HID)
    0,                      // bInterfaceSubclass - Subclass code (0x00 = No Subclass)
    0,                      // bInterfaceProtocol - Protocol code (0x00 = No protocol)
                            // Valid combinations of Class, Subclass, Protocol are as follows:
                            // Class Subclass Protocol Meaning
                            //   3       0       0     Class=HID with no specific Subclass or Protocol: 
                            //                         Can have ANY size reports (not just 8-byte reports)
                            //                         IRK! uses 3-byte reports because it only ever sends one key press at a time
                            //   3       1       1     Class=HID, Subclass=BOOT device, Protocol=keyboard: 
                            //                         REQUIRES 8-byte reports in order for it to be recognised by BIOS when booting.
                            //                         That is because the entire USB protocol cannot be implemented in BIOS, so
                            //                         motherboard manufacturers have agreed to use a fixed 8-byte report during booting.
                            //   3       1       2     Class=HID, Subclass=BOOT device, Protocol=mouse
                            // The above information is documented in Appendix E.3 "Interface Descriptor (Keyboard)"
                            // of the "Device Class Definition for Human Interface Devices (HID) v1.11" document (HID1_11.pdf) from www.usb.org
    STRING_INDEX_IRK_DESC,  // iInterface - Interface string index

    // HID Class-Specific Descriptor
    0x09,                   // bLength - Descriptor size in bytes.
    0x21,                   // bDescriptorType - This descriptor's type: 21h to indicate the HID class.
    0x01,0x01,              // bcdHID - HID specification release number (BCD).
    0x00,                   // bCountryCode - Numeric expression identifying the country for localized hardware (BCD) or 00h.
    1,                      // bNumDescriptors - Number of subordinate report and physical descriptors.
    0x22,                   // bDescriptorType - The type of a class-specific descriptor that follows
    USB_HID_RPT_SIZE,0x00,  // wDescriptorLength - Total length of the descriptor identified above.

    // Endpoint Descriptor - Inbound to host (i.e. key press codes)
    0x07,                   // bLength - Descriptor size in bytes (07h)
    0x05,                   // bDescriptorType - The constant Endpoint (05h)
    USB_HID_EP | 0x80,      // bEndpointAddress - Endpoint number (0x01) and direction (0x80 = IN to host)
    USB_TRANSFER_TYPE,      // bmAttributes - Transfer type and supplementary information
    0x08,0x00,              // wMaxPacketSize - Maximum packet size supported
                            // This determines the size of the transmission time slot allocated to this device
    EP_IN_INTERVAL,         // bInterval - Service interval or NAK rate

    // Endpoint Descriptor - Outbound from host (i.e. LED indicator status bits)
    0x07,                   // bLength - Descriptor size in bytes (07h)
    0x05,                   // bDescriptorType - The constant Endpoint (05h)
    USB_HID_EP,             // bEndpointAddress - Endpoint number (0x01) and direction (0x00 = OUT from host)
    USB_TRANSFER_TYPE,      // bmAttributes - Transfer type and supplementary information
    0x08,0x00,              // wMaxPacketSize - Maximum packet size supported
                            // This determines the size of the transmission time slot allocated to this device
    EP_OUT_INTERVAL         // bInterval - Service interval or NAK rate
};

const struct
{
  char report[USB_HID_RPT_SIZE];
} hid_rpt_desc =
  {
    {
// Some rules:
// 1. If you have multiple reports to send on a single interface then ALL reports
//    on that interface MUST have a unique report id.
// 2. If you want to send one report with NO report id and another report WITH a
//    report id, then you must have separate interfaces. For example:
//    Interface 1: report with NO report id
//    Interface 2: report(s) WITH report id
//    ...otherwise, the host driver will not know whether to expect a report id
//    or not.

/*
Keyboard Input Report (PIC --> Host) 4 bytes as follows:
    .---------------------------------------.
    |          REPORT_ID_KEYBOARD           | IN: Report Id
    |---------------------------------------|
    |RGUI|RALT|RSHF|RCTL|LGUI|LALT|LSHF|LCTL| IN: Ctrl/Shift/Alt/GUI keys on left and right hand side of keyboard
    |---------------------------------------|
    |                (pad)                  | IN: pad
    |---------------------------------------|
    |                 Key                   | IN: Key that is currently pressed
    '---------------------------------------'
*/
    0x05, 0x01,                    // G USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // L USAGE (Keyboard)
    0xa1, 0x01,                    // M COLLECTION (Application)
    0x85, REPORT_ID_KEYBOARD,      //   G REPORT_ID
    0x05, 0x07,                    //   G USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   L USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   L USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   G LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   G LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   G REPORT_SIZE (1)
    0x95, 0x08,                    //   G REPORT_COUNT (8)
    0x81, 0x02,                    //   M INPUT (Data,Var,Abs)

    0x75, 0x08,                    //   G REPORT_SIZE (8)
    0x95, 0x01,                    //   G REPORT_COUNT (1)
    0x81, 0x03,                    //   M INPUT (Cnst,Var,Abs)

    0x95, 0x01,                    //   G REPORT_COUNT (1)
    0x26, 0xff, 0x00,              //   G LOGICAL_MAXIMUM (255)
    0x19, 0x00,                    //   L USAGE_MINIMUM (Reserved (no event indicated))
    0x2a, 0xff, 0x00,              //   L USAGE_MAXIMUM (255)
    0x81, 0x00,                    //   M INPUT (Data,Ary,Abs)
/*
Output Report (PIC <-- Host) 2 bytes as follows:

    .---------------------------------------.
    |          REPORT_ID_KEYBOARD           | OUT: Report Id
    |---------------------------------------|
    |    |    |    |    |    |SCRL|CAPL|NUML| OUT: NumLock,CapsLock,ScrollLock - and 5 unused pad bits
    '---------------------------------------'
*/
    0x75, 0x01,                    //   G REPORT_SIZE (1)
    0x95, 0x03,                    //   G REPORT_COUNT (3)
    0x05, 0x08,                    //   G USAGE_PAGE (LEDs)
    0x19, 0x01,                    //   L USAGE_MINIMUM (Num Lock)
    0x29, 0x03,                    //   L USAGE_MAXIMUM (Scroll Lock)
    0x25, 0x01,                    //   G LOGICAL_MAXIMUM (1)
    0x91, 0x02,                    //   M OUTPUT (Data,Var,Abs)

    0x75, 0x05,                    //   G REPORT_SIZE (5)
    0x95, 0x01,                    //   G REPORT_COUNT (1)
    0x91, 0x03,                    //   M OUTPUT (Cnst,Var,Abs)

    0xc0,                          // M END_COLLECTION

/*
System Control Input Report (PIC --> Host) 2 bytes as follows:
    .---------------------------------------.
    |        REPORT_ID_SYSTEM_CONTROL       | IN: Report Id
    |---------------------------------------|
    |           Power Control Code          | IN: Power Off, Sleep, Power On
    '---------------------------------------'
*/
    0x05, 0x01,                    // G USAGE_PAGE (Generic Desktop)
    0x09, 0x80,                    // L USAGE (System Control)
    0xa1, 0x01,                    // M COLLECTION (Application)
    0x85, REPORT_ID_SYSTEM_CONTROL,//   G REPORT_ID
    0x19, 0x00,                    //   L USAGE_MINIMUM (0x00)
    0x2a, 0xFF, 0x00,              //   L USAGE_MAXIMUM (0xFF)
    0x15, 0x00,                    //   G LOGICAL_MINIMUM (0x00)
    0x26, 0xFF, 0x00,              //   G LOGICAL_MAXIMUM (0xFF)
    0x75, 0x08,                    //   G REPORT_SIZE (8)
    0x95, 0x01,                    //   G REPORT_COUNT (1)
    0x81, 0x00,                    //   M INPUT (Data,Ary,Abs)
    0xc0,                          // M END_COLLECTION

/*
Consumer Device Input Report (PIC --> Host) 3 bytes as follows:
    .---------------------------------------.
    |       REPORT_ID_CONSUMER_DEVICE       | IN: Report Id
    |---------------------------------------|
    |    Consumer Device Code (Low byte)    | IN: Mute, Vol+, Vol- etc
    |---------------------------------------|
    |    Consumer Device Code (High byte)   | IN:
    '---------------------------------------'
*/
    0x05, 0x0C,                    // G USAGE_PAGE (Consumer Devices)
    0x09, 0x01,                    // L USAGE (Consumer Control)
    0xa1, 0x01,                    // M COLLECTION (Application)
    0x85, REPORT_ID_CONSUMER_DEVICE, //   G REPORT_ID
    0x19, 0x00,                    //   L USAGE_MINIMUM (0)
    0x2a, 0x3c, 0x02,              //   L USAGE_MAXIMUM (0x023C)
    0x15, 0x00,                    //   G LOGICAL_MINIMUM (0)
    0x26, 0x3c, 0x02,              //   G LOGICAL_MAXIMUM (0x023C)
    0x75, 0x10,                    //   G REPORT_SIZE (16)
    0x95, 0x01,                    //   G REPORT_COUNT (1)
    0x81, 0x00,                    //   M INPUT (Data,Ary,Abs)
    0xc0,                          // M END_COLLECTION
    }
  };

/* 
Trace captured from a Gigabyte GK-K7100 keyboard, which supports
Keyboard (including boot keyboard), Power control functions, 
and Consumer device control functions:

05 01     G USAGE_PAGE (Generic Desktop)
09 06     L USAGE (Keyboard)
A1 01     M COLLECTION (Application)
05 08       G USAGE_PAGE (LEDs)
19 01       L USAGE_MINIMUM (Num Lock)
29 03       L USAGE_MAXIMUM (Scroll Lock)
15 00       G LOGICAL_MINIMUM (0)
25 01       G LOGICAL_MAXIMUM (1)
75 01       G REPORT_SIZE (1)
95 03       G REPORT_COUNT (3)
91 02       M OUTPUT (Data,Var,Abs)
95 05       G REPORT_COUNT (5)
91 01       G REPORT_COUNT (1)
05 07       G USAGE_PAGE (Keyboard)
19 E0       L USAGE_MINIMUM (Keyboard LeftControl)
29 E7       L USAGE_MAXIMUM (Keyboard Right GUI)
95 08       G REPORT_COUNT (8)
81 02       M INPUT (Data,Var,Abs)
75 08       G REPORT_SIZE (8)
95 01       G REPORT_COUNT (1)
81 01       M INPUT (Cnst,Ary,Abs)
19 00       L USAGE_MINIMUM (Reserved (no event indicated))
29 91       L USAGE_MAXIMUM ()
26 FF 00    G LOGICAL_MAXIMUM (255)
95 06       G REPORT_COUNT (6)
81 00       M INPUT (Data,Ary,Abs)
C0        M END_COLLECTION


05 01     G USAGE_PAGE (Generic Desktop)
09 80     L USAGE (System Control)
A1 01     M COLLECTION (Application)
85 02       G REPORT_ID (0x02)
19 81       L USAGE_MINIMUM (Power Down)
29 83       L USAGE_MAXIMUM (Wake)
15 00       G LOGICAL_MINIMUM (0)
25 01       G LOGICAL_MAXIMUM (1)
75 01       G REPORT_SIZE (1)
95 03       G REPORT_COUNT (3)
81 02       M INPUT (Data,Var,Abs)
95 05       G REPORT_COUNT (5)
81 01       M INPUT (Cnst,Ary,Abs)
C0        M END_COLLECTION

05 0C     G USAGE_PAGE (Consumer Devices)
09 01     L USAGE (Consumer Control)
A1 01     M COLLECTION (Application)
85 01       G REPORT_ID (0x01)
19 00       L USAGE_MINIMUM (Reserved (no event indicated))
2A 3C 02    L USAGE_MAXIMUM (0x023C)
15 00       G LOGICAL_MINIMUM (0)
26 3C 02    G LOGICAL_MAXIMUM (0x023C)
95 01       G REPORT_COUNT (1)
75 10       G REPORT_SIZE (16)
81 00       M INPUT (Data,Ary,Abs)
C0        M END_COLLECTION

*/




//Language code string descriptor
const struct 
{
  char bLength;
  char bDscType;
  unsigned int string[1];
} sLanguage =
  {
      4,
      0x03,
      {0x0409}
  };


//Manufacturer string descriptor
const struct
{
  char bLength;
  char bDscType;
  unsigned int string[4]; // Max number of (2-byte) "characters"
} sManufacturer =
  {
    10,           //sizeof this descriptor string
    0x03,
    {'I','R','K', '!'}
  };

//Product string descriptor
const struct tagProduct
{
  char bLength;
  char bDscType;
  unsigned int string[32]; // Max number of (2-byte) "characters"
} sProduct =
  {
    54,           //sizeof this descriptor string (including length and type fields)
    0x03,
    {'R','e','m','o','t','e',' ','C','o','n','t','r','o','l','l','e','d',' ','K','e','y','b','o','a','r','d'}
  };
  

//Array of configuration descriptors
const char* USB_config_dsc_ptr[1];

//Array of string descriptors
const char* USB_string_dsc_ptr[3];

void USB_Init_Desc()
{
  USB_config_dsc_ptr[0] = &configDescriptor1;
  USB_string_dsc_ptr[STRING_INDEX_LANGUAGE] = (const char*)&sLanguage;
  USB_string_dsc_ptr[STRING_INDEX_IRK]      = (const char*)&sManufacturer;
  USB_string_dsc_ptr[STRING_INDEX_IRK_DESC] = (const char*)&sProduct;
}