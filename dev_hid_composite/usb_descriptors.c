/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "pico/unique_id.h"
#include "tusb.h"
#include "usb_descriptors.h"

/* A combination of interfaces must have a unique product id, since PC will save device driver after the first plug.
 * Same VID/PID with different interface e.g MSC (first), then CDC (later) will possibly cause system error on PC.
 *
 * Auto ProductID layout's Bitmap:
 *   [MSB]         HID | MSC | CDC          [LSB]
 */
#define _PID_MAP(itf, n)  ( (CFG_TUD_##itf) << (n) )
#define USB_PID           (0x4000 | _PID_MAP(CDC, 0) | _PID_MAP(MSC, 1) | _PID_MAP(HID, 2) | \
                           _PID_MAP(MIDI, 3) | _PID_MAP(VENDOR, 4) )

#define USB_VID   0xCafe
#define USB_BCD   0x0200

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t const desc_device =
{
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = USB_BCD,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

uint8_t const desc_configuration_[] =
{
  //Configuration Descriptor
  9,                    //bLength: byte length of configuration descriptor
  2,                    //bDescriptorType: descriptor type is CONFIGURATION
                        //wTotalLength: 2 bytes, total length of all descriptors
  (uint8_t) (34 & 0xff),            // lower byte
  (uint8_t)((34 >> 8) & 0xff),     //upper byte
  1,                    //bNumInterfaces: number of interfaces
  1,                    //bConfigurationValue: value of this configuration
  0,                    //iConfiguration: index of string descriptor describing this configuration
  0x82,                 //bmAttributes: 0x82 or 10100000 indicates this configuration is bus-powered
  50,                   //bMaxPower: 50*2 = 100mA
  //Interface Descriptor
  9, //bLength of interface descriptor
  4, //bDescriptorType is 4 which means INTERFACE
  0, //bInterfaceNumber: zero-based index of this interface
  0, //bAlternateSetting: value of this interface for selecting this interface
  1, //bNumEndpoints: number of endpoints of this interface
  3, //bInterfaceClass: 3 means HID device
  1, //bInterfaceSubClass: 1 means boot device
  1, //bInterfaceProtocol: 1 means keyboard
  0, //iInterface: index of string descriptor describing this config, same as iConfiguration
  //HID Descriptor
  9,        //bLength
  0x21,     //bDescriptorType: 0x21 for HID
            //bcdHID: 0x0111 means HID version 1.11
  0x11,     //lower byte
  0x01,     //upper byte
  0,        //bCountryCode: not used
  1,        //bNumDescriptors: there is only 1 HID descriptor
  34,       //bDescriptorType: 34 means report descriptor
            //wDescriptorLength
  (uint8_t)(65 & 0xff),
  (uint8_t)((65 >> 8) & 0xff),
  //Endpoint Descriptor (number of bytes to send)
  7, //bLength
  5, //bDescriptorType ENDPOINT
  0x81, //bEndpointAddress: ENDPOINT 1, IN
  0x03, //bmAttributes: Interrupt
  (uint8_t)(16 & 0xff),
  (uint8_t)((16>>8) & 0xff), //00010000 = 16 bytes to send
  5  //sent every 5 frames: 5 ms
};


uint8_t const hid_report_descriptor[] = {
  0x05, 0x01, //USAGE_PAGE (Generic Desktop)
  0x09, 0x06, //USAGE (Keyboard)
  0xA1, 0x01, //Collection (Application)
  0x05, 0x07, //USAGE_PAGE (Keyboard)
  0x19, 0xE0, //USAGE_MIN (Left Ctrl)
  0x29, 0xE7, //USAGE_MAX (Right GUI)
  0x15, 0x00, //Logic Min (0)
  0x25, 0x01, //Logic Max (1)
  0x75, 0x01, //report size 1
  0x95, 0x08, //report count 8
  0x81, 0x02, //input (data,variable,absolute): modifier byte, each bit tells which key(left ctrl ~ right gui) is activated
  0x95, 0x01, //report count 1
  0x75, 0x08, //report size 8
  0x81, 0x01, //input (const, variable, absolute): reserved byte
  0x95, 0x05, //report count 5
  0x75, 0x01, //report size 1
  0x05, 0x08, //usage page (leds)
  0x19, 0x01, //usage min numlock
  0x29, 0x05, //usage max kana
  0x91, 0x02, //output(data,var,abs): computer outputs state of leds to keyboard, 5 bits
  0x95, 0x01, //report count 1
  0x75, 0x03, //report size 3
  0x91, 0x01, //output(const,var,abs): 3 extra bits in led output for padding
  0x95, 0x06, //report count 6
  0x75, 0x08, //report size 8
  0x15, 0x00, //logic min 0
  0x26, 0xff, 0x00, //logic max 255
  0x05, 0x07, //usage page keyboard
  0x19, 0x00, //usage min 0
  0x2A, 0xff, 0x00, //usage max 255
  0x81, 0x00, //input (data,ary,abs): 6 keycode bytes
  0xC0 //end collection
};


// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const * tud_descriptor_device_cb(void)
{
  return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

uint8_t const desc_hid_report[] =
{
  TUD_HID_REPORT_DESC_KEYBOARD()
};

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_hid_descriptor_report_cb(uint8_t instance)
{
  (void) instance;
  return hid_report_descriptor;
}
//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

enum
{
  ITF_NUM_HID,
  ITF_NUM_TOTAL
};

#define  CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

#define EPNUM_HID   0x81

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_descriptor_configuration_cb(uint8_t index)
{
  (void) index; // for multiple configurations

  // This example use the same configuration for both high and full speed mode
  return desc_configuration_;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

// buffer to hold flash ID
char serial[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1];

// array of pointer to string descriptors
char const* string_desc_arr [] =
{
  (const char[]) { 0x09, 0x04 }, // 0: is supported language is English (0x0409)
  "TinyUSB",                     // 1: Manufacturer
  "TinyUSB Device",              // 2: Product
  serial,                        // 3: Serials, uses the flash ID
};

static uint16_t _desc_str[32];

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
  (void) langid;

  uint8_t chr_count;

  if ( index == 0)
  {
    memcpy(&_desc_str[1], string_desc_arr[0], 2);
    chr_count = 1;
  }else
  {
    // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
    // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

    if (index == 3) pico_get_unique_board_id_string(serial, sizeof(serial));
    
    if ( !(index < sizeof(string_desc_arr)/sizeof(string_desc_arr[0])) ) return NULL;

    const char* str = string_desc_arr[index];

    // Cap at max char
    chr_count = strlen(str);
    if ( chr_count > 31 ) chr_count = 31;

    // Convert ASCII string into UTF-16
    for(uint8_t i=0; i<chr_count; i++)
    {
      _desc_str[1+i] = str[i];
    }
  }

  // first byte is length (including header), second byte is string type
  _desc_str[0] = (TUSB_DESC_STRING << 8 ) | (2*chr_count + 2);

  return _desc_str;
}
