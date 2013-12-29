/*
  IRK! Infrared Remote Controlled USB Keyboard $Rev:  $
  Copyright (C) 2010-2014 Andrew J. Armstrong

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
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

/*
-------------------------------------------------------------------------------

NAME     - IRK! Infrared USB Keyboard

FUNCTION - This implements a USB keyboard that is controlled by a learning
           infrared remote control. Note: your IR remote MUST be able to LEARN
           infrared control signals. You CANNOT use a pre-programmed remote
           control with IRK!.

           The original intention was to allow a user to control MythTV home
           theatre software using an infrared remote control, but because it
           is seen by a PC as just another USB keyboard then it could be used
           to control other PC functions as well.

           What makes this unit different from the rest is that it can TEACH
           a learning remote control the IR code associated with a particular
           USB keystroke. As a result, there are NO pre-programmed IR codes and
           IRK! does not have to support all the different types of infrared
           remotes on the market - it TEACHES the remote IRK!'s own infrared
           commands - which can be anything! Currently, IRK! uses Pulse Width
           Modulation (PWM) to encode/decode the infrared signals.

FEATURES -
           IRK! teaches your LEARNING remote control IR codes
           IRK! lets you map IR codes to USB keystrokes (e.g. Shift+Enter)
           IRK! sends the USB keystroke to your PC when you press an IR button
           IRK! can send USB System Control codes (Power off, sleep, wake) to your PC
           IRK! devices can have one of 256 addresses (so you can have multiple IRK!'s)
           IRK! supports USB Consumer Device functions (e.g. Mute)
           IRK! has a programmable LCD backlight delay (or ON/OFF commands)
           IRK! needs no host drivers on Windows, Linux etc

PIN USAGE -                      PIC18F25K50
                            .------------------.
            (Sense USB) --> | RE3  1    28 RB7 | <-- (DOWN button)
            (LCD D4)    <-- | RA0  2    27 RB6 | <-- (UP button)
            (LCD D5)    <-- | RA1  3    26 RB5 | <-- (OK button)
            (LCD D6)    <-- | RA2  4    25 RB4 | <-- (SHIFT button)
            (LCD D7)    <-- | RA3  5    24 RB3 | <-- (IR receiver)
            (LCD RS)    <-- | RA4  6    23 RB2 | <-- (ALT button)
            (LCD EN)    <-- | RA5  7    22 RB1 | <-- (CTL/GUI button)
            (Ground)    --- | VSS  8    21 RB0 | <-- (TEACH button)
            (n/c)       --- | RA7  9    20 VDD | --- (+5V)
            (AUX switch)<-- | RA6  10   19 VSS | --- (Ground)
            (LED)       <-- | RC0  11   18 RC7 | --> (RST switch)
            (Backlight) <-- | RC1  12   17 RC6 | --> (PWR switch)
            (IR LED)    <-- | RC2  13   16 RC5 | <-> (USB D+)
            (220nF)     --- | VUSB 14   15 RC4 | <-> (USB D-)
                            '------------------'

CONFIG    - The PIC18F25K50 configuration fuses should be set as follows (items
            marked with '*' are essential for IRK! to operate correctly):

            CONFIG1L 2B 00101011
                          1      = *LS48MHZ: System clock is expected at 48 MHz, FS/LS USB clock divide-by is set to 8
                           01    = *CPUDIV: CPU system clock (48 MHz) divided by 2 (= 24 MHZ)
                             0   =  Unimplemented
                              x  =  CFGPLLEN: (ignored, set dynamically)
                               x =  PLLSEL: (ignored, set dynamically)
            CONFIG1H 28 00101000
                        0        =  IESO: Oscillator Switchover mode disabled
                         0       =  FCMEN: Fail-Safe Clock Monitor disabled
                          1      = *PCLKEN: Primary Clock is always enabled
                           0     =  Unimplemented
                            1000 = *FOSC: Internal oscillator block
            CONFIG2L 5F 01011111
                        0        =  Unimplemented
                         1       =  LPBOR: Low-Power Brown-out Reset disabled
                          0      =  Unimplemented
                           11    =  BORV: VBOR set to 1.9V nominal
                             11  =  BOREN: Brown-out Reset enabled in hardware only (SBOREN is disabled)
                               1 =  PWRTEN: Power-up Timer disabled
            CONFIG2H 3C 00111100
                        00       =  Unimplemented
                          1111   =  WDTPS: Watchdog Timer Postscale Select = 1:32768
                              00 =  WDTEN: Watchdog Timer disabled in hardware, SWDTEN bit disabled
            CONFIG3L 00 00000000
                        00000000 =  Unimplemented
            CONFIG3H 53 01010000
                        0        = *MCLRE: RE3 input pin enabled; MCLR disabled
                         1       =  SDOMX: SDO is on RB3
                          0      =  Unimplemented
                           1     =  T3CMX: T3CKI is on RC0
                            00   =  Unimplemented
                              0  =  PDADEN: ANSELB<5:0> resets to 0, PORTB<4:0> pins are configured as digital I/O on Reset
                               0 = *CCP2MX: CCP2 input/output is multiplexed with RB3
            CONFIG4L 81 10000001 =  Default
            CONFIG5L 0F 00001111 =  Default
            CONFIG5H C0 11000000 =  Default
            CONFIG6L 0F 00001111 =  Default

OPERATION - The user is presented with an LCD 2 x 16 display that looks like:

            .----------------.
            |ux line1........|
            |yy line2........|
            '----------------'

            u = The USB Usage Page index (i.e. used to assign meaning to the
                following x and yy codes). Press and hold the SHIFT button and
                then press the UP/DOWN buttons to cycle through the usage page
                indexes (0 to F and back to 0). At the moment, IRK! uses only
                the following usage page indexes:
                0 = Keyboard (A, B, C etc)
                1 = System Control (Power off, Sleep, Wake etc)
                2 = Consumer Devices (Mute, Vol+, Vol-, Play/Pause etc)
                F = IRK! Local Functions (LCD backlight on/off etc)
            x = The key modifiers (Ctl, Alt, Shift) for Keyboard usage, else
                the top nybble of xyy (the function to be performed) for all
                other usages.
            line1 = Text that describes the ux code
            yy = See below. Press and/or hold the UP and DOWN buttons to change
                 the value of xyy.
            line2 = Text that describes the yy (or xyy) code.


            The meanings id u, x and yy are as follows:

            For the Keyboard usage:
            ux yy
            0      Keyboard
                     x = Keyboard modifier (CTRL/ALT/SHIFT) flag bits
                     yy = USB keyboard function code
            00 yy   Key yy pressed with no keyboard modifier keys
            01 yy   CTRL key pressed along with key yy
            02 yy   SHIFT key presed along with key yy
            04 yy   ALT key pressed along with key yy
            08 yy   GUI key pressed along with key yy
            Examples:
            00 05   b key pressed
            02 05   SHIFT+b key pressed (i.e. upper case B)
            06 05   ALT+SHIFT+b key pressed



            For the System Control usage:
            ux yy
            1      System Control
                     xyy = USB System Control function
            Examples:
            10 81   Power off
            10 82   Sleep
            10 83   Wake



            2      Consumer Devices
                   xyy = USB Consumer Device function
            Examples:
            20 E2   Mute
            20 E9   Vol+
            20 EA   Vol-
            21 95   Internet Browser



            The following are spare usage indexes:
            ux yy
            3      --reserved for future use--
            4      --reserved for future use--
            5      --reserved for future use--
            6      --reserved for future use--
            7      --reserved for future use--
            8      --reserved for future use--
            9      --reserved for future use--
            A      --reserved for future use--
            B      --reserved for future use--
            C      --reserved for future use--
            D      --reserved for future use--
            E      --reserved for future use--



            For the Local IRK! Functions usage:
            ux yy
            F      Local IRK! functions (i.e. not USB)
            F0 00   Set Address ... the next LCD display will be:
                    aa <-- Address
                    00 Set Address
                    You then press up/down to cycle the address (aa) then
                    press OK to select it. The default address is AA.
            F0 01   Power Switch (4066 analog switch number 1)
            F0 02   Reset Switch (4066 analog switch number 2)
            F0 03   Init USB
            F0 04   Back light off
            F0 05   Back light on
            F0 06   Set light delay  ... the next LCD display will be:
                    xx <-- nnn secs (where nnn is xx in decimal)
                    FF <-- ON       (keep back light always on)
                    00 <-- OFF      (keep back light always off)
                    You then press up/down to cycle the delay (xx) then
                    press OK to select it.
            F0 07   Debug on (displays debug info when an IR code is received)
            F0 08   Debug off (displays no debug info)
            F0 0A   Auxiliary Switch (4066 analog switch number 3)



FORMATS -  1. The IR transmission format sent to, and received from, your
           Learning Remote control is:

            aa aa' ux ux' yy yy'

            That is:
            aa  = The address of this IRK! device
            aa' = The address of this IRK! device inverted
            ux  = The ux byte as described above
            ux' = The ux byte inverted
            yy  = The yy byte as decribed above
            yy' = The yy byte inverted

            The same data format is received from your Learning Remote when
            you press one of the IRK! functions recorded by it.

           2. The USB transmission format sent to your PC varies depending on
           the USB usage as follows:

            Usage                      USB Report format
            0 (Keyboard)               'K' 0x 00 yy
            1 (System Control)         'S' yy
            2 (Consumer Device)        'C' 0x yy
            3                          --reserved--
            4                          --reserved--
            5                          --reserved--
            6                          --reserved--
            7                          --reserved--
            8                          --reserved--
            9                          --reserved--
            A                          --reserved--
            B                          --reserved--
            C                          --reserved--
            D                          --reserved--
            E                          --reserved--
            F (Local IRK! Device)      No USB report sent

            The reason for translating the Usage nybble to an ASCII character
            for the Report ID (e.g. 0 becomes 'K') is to make it easier to
            interpret USB traces captured on the host. That is the only reason.
            You can just as easily configure the USB Report Descriptors to use
            0x00 for Keyboard, 0x01 for System Control etc.
            The USB Report Id is used by the host HID driver to distinguish
            between the various reports sent by IRK!.
            See USBdsc.c to see how the Report Id is defined.
            See the "Device Class Definition for Human Interface Devices (HID)"
            documentation, specifically "Section 5.6 Reports" at:
            http://www.usb.org/developers/devclass_docs/HID1_11.pdf
            ...for more information on Report Id.

EXAMPLE  - 1. The user wants to program the Enter key on the OK button of a
              learning remote control. The steps to follow are:
              a. Plug IRK! into a host USB port
              b. The default usage mode is Keyboard, which is displayed as
                 "00 Keyboard" in the first line of the LCD, so just
                 press and hold the UP button until the second line of the LCD
                 shows "28 Enter".
              c. Switch your learning remote control into "Learn" mode and
                 select the remote's button to be programmed.
              d. Press IRK!'s TEACH button. This will send an IR code that
                 means "Enter" to your learning remote.
              e. Repeat until the learning remote has learned that IR code.
              f. Press the programmed button on the learning remote control.
              g. Verify that IRK! has received the IR code (it will flash
                 its LED when it has recognised a valid IR code). Verify 
                 that the "Enter" USB Keyboard function has been performed
                 at the host.

           2. The user wants to program the "Mute" Consumer Devices function
              on the "Mute" button of a learning remote control. The default
              usage mode of IRK! is "Keyboard", so you need to switch IRK! into
              "Consumer Dev" mode first:
              a. Press and hold IRK!'s SHIFT button until the second line of
                 the LCD shows "^^ Select Usage".
              b. Press IRK!'s UP button until "20 Consumer Dev" is displayed in
                 the first line of the LCD.
              c. Press IRK!'s OK button to enter "Consumer Dev" usage mode.
              d. Press and hold IRK!'s UP button until "E2 Mute" is displayed
                 on the second line of the LCD. If you overshoot then press the
                 DOWN button repeatedly until you reach "E2 Mute".
              e. Press OK to verify that the Mute function is accepted by the
                 host.
              f. Put your learning remote control into "Learn" mode and choose
                 one of the remote's buttons to be programmed as "Mute".
              g. Position your learning remote's IR receiver to within 5 cm of
                 IRK!'s IR transmitter LED.
              h. Press IRK!'s TEACH button. This will send an IR code that 
                 means "Mute" to your learning remote.
              i. Keep pressing TEACH until the learning remote has learned that
                 IR code.
              j. Press the programmed button on the learning remote control.
              k. Verify that IRK! has received the IR code (it will flash
                 its LED when it has recognised a valid IR code). Verify 
                 that the "Mute" USB consumer device function has been performed
                 at the host.


NOTES    - 1. The source code is written in MikroC Pro from mikroe.com

           2. Ensure that total RAM usage does not exceed 1k (4 x 256 byte banks).
              If this is not done, then the compiler may allocate variables to
              banks 4-7 which are also used by the USB module causing unpredictable
              results.

           3. Variable prefix convention:
              Prefix Short for       Width  Type
              ------ --------------- ------ ----------------
                b    boolean          1 bit flag
                n    number           8 bit unsigned integer
                w    word            16 bit unsigned integer
                i    int             16 bit signed integer
                l    long            32 bit signed integer
               ul    unsigned long   32 bit unsigned integer
                c    char             8 bit character
                s    string           8 bit character array

           4. Timer0 is used for LCD display backlight timeouts
              Timer1 is used for IR signal capture timings
              Timer2 is used for IR signal transmission (PWM)
              Timer3 is used for key repeats

           5. IRK! will send *any* USB code that you select to the host - not
              just that ones that it displays with a name. For example, you can
              program the USB Keyboard code for the "LANG1" key (code 90) even
              though IRK! displays this code as a blank line. To see a list of
              all the USB keyboard codes, refer to the "HID Usage Tables"
              document at:
              http://www.usb.org/developers/devclass_docs/Hut1_12v2.pdf
              ...specifically Table 12 "Keyboard/Kepad Page"

           6. The AGC training burst at the start of each transmission is not
              really needed anymore.  The Vishay TSOP4838 data sheet indicates
              that IR bursts (at 38 kHz) can last for between 260 and 1800 micro-
              seconds and that the silence after each burst must be for at least
              370 microseconds. In particular, if an IR burst exceeds 1800 micro-
              seconds then there must be silence of at least 4 x the burst
              duration.  This means that having a 9000 us AGC training burst
              would be treated as environmental noise and the sensitivity of the
              TSOP4838 would be reduced for 4 x 9000 us (36000 us) - which may
              effectively reduce the distance at which the IR receiver can
              operate. As the AGC training burst is not really needed, it has
              been reduced in duration so as not to adversely affect the
              operation of the TSOP4838 receiver as of v2.04.

REFERENCE - USB Human Interface Device Usage Tables at:
            http://www.usb.org/developers/devclass_docs/Hut1_12v2.pdf

            USB Device Class Definition for Human Interface Devices (HID) at:
            http://www.usb.org/developers/devclass_docs/HID1_11.pdf

            USB 2.0 Specification at:
            http://www.usb.org/developers/docs/usb_20_101111.zip
            Specifically, the file in that zip archive called usb_20.pdf

AUTHORS  - Init Name                 Email
           ---- -------------------- ------------------------------------------
           AJA  Andrew J. Armstrong  androidarmstrong@gmail.com

HISTORY  - Date     Ver   By  Reason (most recent at the top please)
           -------- ----- --- -------------------------------------------------
           20131229 3.03  AJA Added local IRK! functions to turn on and off the
                              4066 analog switches (PWR, RST and AUX). The
                              existing code only asserted them for 250 ms.
           20131228 3.02  AJA SMT version now works at 48 MHz. It will also work
                              at 24 MHz but the mikroProg Suite software has a
                              bug at the moment which prevents setting the
                              configuration bits appropriately.
           20131222 3.01  AJA Fixed description of what the front panel buttons
                              do in README.md. Changed some configuration fuses 
                              CONFIGxx) for PIC18F25K50. Reworked the timer value 
                              calculations so that the MCU clock frequency can be
                              reverted to 24 MHz more easily.
           20131126 3.00  AJA This version is for a PIC18F25K50 which has
                              an internal oscillator that can drive the USB
                              module - so no crystal is needed anymore.
           20130413 2.11  AJA Recompiled for MikroC 6.0 which appears to be
                              case-sensitive now (for example, library routines
                              must be spelled properly).
           20130213 2.10  AJA Detect whether any buttons are pressed at power
                              up (or, equivalently, an ICSP programmer is still
                              plugged in) and display a "Release Buttons!" 
                              message until the situation is rectified.
                              Enabled USB Full Speed mode, so IRK will run in
                              either Full Speed or Low Speed mode. Before IRK
                              only worked in Low Speed mode.
                              Recompiled with MikroC v5.80.
           20121212 2.09  AJA Reserved RAM Bank 4 for USB use only. Corrected
                              button press handling when setting backlight
                              delay, device address or selecting usage.
                              Added ability to set the GUI key flag by pressing
                              and holding the CTL key. The GUI key is a generic
                              name for the Windows key, the Apple (Command) key,
                              or the Super key on Linux systems.
           20121211 2.08  AJA Saved a huge amount of RAM and expanded text
                              descriptions dramatically by copying text from
                              ROM to RAM as it is needed (to display on the LCD
                              for example). This is done using the _TEXT
                              function and by specifying "const" on all functions
                              that return a string.
           20121025 2.07  AJA Added extra Consumer Device function descriptions.
                              These are optional, but make IRK easier to use.
           20121020 2.06  AJA Increased the WIDTH_ERROR_MARGIN from 100 to 300
                              microseconds (that to half the smallest pulse
                              width). This has fixed the sensitivity problem.
                              Also added a debug on/off local IRK function.
                              Now displays graphic symbols for up/down/left/
                              right/pause and backslash on the LCD.
           20121009 2.05  AJA Fixed the pulse width check introduced in 2.04.
           20120813 2.04  AJA Reduced training burst interval to conform with
                              the TSOP4838 data sheet. Training burst reduced
                              from 9000 to 1000 microseconds. Silence after
                              training burst reduced from 4500 to 600 microseconds.
                              Added extra pulse width checks.
           20120312 2.03  AJA Repackaged the lot into a single zip file
           20120310 2.02  AJA USB buffer overwrite problem seems to have been
                              fixed in MikroC Pro v5.40 so I've removed the
                              workaround of using a 2nd buffer. Fine tuned the
                              USBdsc.c settings - use a slower polling rate and
                              a few other minor changes.
           20120308 2.01  AJA Allow local IRK! functions such as Backlight On
                              to be programmed to a remote control button.
           20120215 2.00  AJA Rewrite to support Consumer Devices usage page
                              and other usage pages in the future
           20101122 1.19  AJA Detect USB ready using HID_Write (not HID_Read)
           20101120 1.18  AJA Added "Init USB" function
           20101106 1.17  AJA Use LATC for outputs
           20101023 1.16  AJA Don't try to send keystroke when USB is not ready
           20101007 1.15  AJA Workaround USB send problem by using 2nd buffer
           20100923 1.14  AJA Detect when power good from USB then init USB
           20100911 1.13  AJA Recompiled with MikroC v3.8 (new USB interface)
           20100828 1.12  AJA Added button debounce delay
           20100810 1.11  AJA USB D+ and D- swapped (doc updated)
           20100806 1.10  AJA LCD backlight polarity swapped (doc updated)
           20100725 1.09  AJA Added LCD backlight control
           20100619 1.08  AJA Added reset switch and power switch logic
           20100616 1.07  AJA Use HS crystal instead of crystal oscillator
           20100531 1.06  AJA Recognised broadcast address
           20100522 1.04  AJA Added "set address" facility
           20100519 1.03  AJA Added System Control (power) codes
           20100501 1.00  AJA Initial version (using MikroC v3.2)

-------------------------------------------------------------------------------
*/
#include "IRK.h"

#define IRK_VERSION "3.03"

#define OUTPUT        0
#define INPUT         1

#define ON            1
#define OFF           0

#define HIGH          1
#define LOW           0

#define ENABLE        1
#define DISABLE       0

#define TRUE  (1==1)
#define FALSE !TRUE
#define FOREVER TRUE

#define LoByte(x) (x & 0xff)
#define HiByte(x) ((x >> 8) & 0xff)

#define BROADCAST_ADDRESS       0xFF

#define IR_MODULATION_FREQ  38000
#define DUTY_CYCLE 256 / 2                      // 1 on to 2 off

// LCD characters loaded into CGRAM:
#define UP_ARROW     0x01
#define DOWN_ARROW   0x02

// define some easy to remember types
typedef unsigned short  byte;

typedef union
{
  byte byte;
  struct
  {
    LeftControl:1;  // xxxxxxx1
    LeftShift:1;    // xxxxxx1x
    LeftAlt:1;      // xxxxx1xx
    LeftGUI:1;      // xxxx1xxx
    RightControl:1; // xxx1xxxx
    RightShift:1;   // xx1xxxxx
    RightAlt:1;     // x1xxxxxx
    RightGUI:1;     // 1xxxxxxx
  } bits;
} t_keyModifiers;

// LCD module
sbit LCD_D4 at LATA0_bit;
sbit LCD_D5 at LATA1_bit;
sbit LCD_D6 at LATA2_bit;
sbit LCD_D7 at LATA3_bit;
sbit LCD_RS at LATA4_bit;
sbit LCD_EN at LATA5_bit;

sbit LCD_D4_Direction at TRISA0_bit;
sbit LCD_D5_Direction at TRISA1_bit;
sbit LCD_D6_Direction at TRISA2_bit;
sbit LCD_D7_Direction at TRISA3_bit;
sbit LCD_RS_Direction at TRISA4_bit;
sbit LCD_EN_Direction at TRISA5_bit;

// Buttons (weak pullups are enabled so they read high when not pressed)
sbit TEACH_BUTTON            at RB0_bit;
#define TEACH_BUTTON_PRESSED !TEACH_BUTTON
sbit CTL_BUTTON              at RB1_bit;
#define CTL_BUTTON_PRESSED   !CTL_BUTTON
sbit ALT_BUTTON              at RB2_bit;
#define ALT_BUTTON_PRESSED   !ALT_BUTTON
sbit SHIFT_BUTTON            at RB4_bit;
#define SHIFT_BUTTON_PRESSED !SHIFT_BUTTON
sbit OK_BUTTON               at RB5_bit;
#define OK_BUTTON_PRESSED    !OK_BUTTON
sbit UP_BUTTON               at RB6_bit;
#define UP_BUTTON_PRESSED    !UP_BUTTON
sbit DOWN_BUTTON             at RB7_bit;
#define DOWN_BUTTON_PRESSED  !DOWN_BUTTON

// Outputs
sbit ACTIVITY_LED            at LATC0_bit;  // Activity indicator LED
sbit LCD_BACKLIGHT           at LATC1_bit;  // LCD backlight (active low)
sbit IR_LED                  at LATC2_bit;  // IR Transmitter
sbit POWER_SWITCH            at RC6_bit;  // Power switch
sbit RESET_SWITCH            at RC7_bit;  // Reset switch
sbit AUX_SWITCH              at RA6_bit;  // Auxiliary switch
// Inputs
sbit USB_POWER_GOOD          at RE3_bit;  // USB Power Good detection

volatile signed short nBacklightDelay;    // Seconds to keep backlight on

volatile union
{
  unsigned int n;
  byte byte[2];
} nPulseWidth;

volatile byte nRiseOrFall;  // CCP2CON at time of capture interrupt
#define bRisingEdge nRiseOrFall.B0    // Bit 0 in CCP2CON = 1 means rising edge detected
#define bFallingEdge !bRisingEdge     // Bit 0 in CCP2CON = 0 means falling edge detected

volatile byte                      cFlags;
#define bInfraredInterruptPending  cFlags.B7
#define bDebugMode                 cFlags.B6
#define bSettingBacklightDelay     cFlags.B5
#define bSettingUsage              cFlags.B4
#define bLastUSBPower              cFlags.B3
#define bSettingDeviceAddress      cFlags.B2
#define bKeyRepeatPending          cFlags.B1
#define bUSBReady                  cFlags.B0

byte nState;
#define STATE_IR_RESET                   0
#define STATE_IR_TRAINING_RECEIVED       1
#define STATE_IR_RECEIVING_BITS          2
#define STATE_IR_COMMAND_RECEIVED        3

union 
{
  int n;
  byte b[2];
} resetCount;

byte nConfigDeviceAddress;
byte nConfigBacklightDelay;
byte nNewBacklightDelay;

// Local IRK! function codes...
#define CMD_SET_DEVICE_ADDRESS        0x00
#define CMD_PRESS_POWER_SWITCH        0x01
#define CMD_PRESS_RESET_SWITCH        0x02
#define CMD_INIT_USB                  0x03
#define CMD_SET_BACKLIGHT_OFF         0x04
#define CMD_SET_BACKLIGHT_ON          0x05
#define CMD_SET_BACKLIGHT_DELAY       0x06
#define CMD_SET_DEBUG_ON              0x07
#define CMD_SET_DEBUG_OFF             0x08
#define CMD_PRESS_AUX_SWITCH          0x0A
#define CMD_POWER_SWITCH_ON           0x11
#define CMD_RESET_SWITCH_ON           0x12
#define CMD_AUX_SWITCH_ON             0x1A
#define CMD_POWER_SWITCH_OFF          0x21
#define CMD_RESET_SWITCH_OFF          0x22
#define CMD_AUX_SWITCH_OFF            0x2A


// Note that for a Vishay TSOP4838 IR receiver module, all IR bursts should
// be between 260 and 1800 microseconds, otherwise it will treat the burst as
// noise and reduce the sensitivity of the device for a period of about 4 times
// the duration of the burst.

// Set the Timer0 constant for a 1 second interrupt:
#define CLOCK_FREQUENCY       (__FOSC__ * 1000)
#define TIMER0_PRESCALER      256
#define ONE_SECOND            (65535 - (CLOCK_FREQUENCY/4/TIMER0_PRESCALER))

// The following delays are in microseconds and are used as-is when
// transmitting an IR signal...
#define WIDTH_ERROR_MARGIN             300
#define WIDTH_SHORT                    600
#define WIDTH_LONG                    1650
#define WIDTH_TRAINING_PULSE          1000
#define WIDTH_SILENCE_AFTER_TRAINING   600

// ...however, to measure these delays when RECEIVING an IR signal, the numbers
// will only be correct if Timer1 runs at 1 MHz:

// If the MCU clock is running at 24 MHz it feeds Timer1 with 24/4 = 6 MHz
// pulses that are prescaled by a factor of 1:8, giving a Timer1 tick
// rate of 0.75 MHz, not 1 MHz, so we have to scale the above units
// by 3/4 to give microsecond units (3 Timer1 ticks is 4 microseconds)...
// #define MICROSECONDS 3 / 4

// If the MCU clock is running at 48 MHz it feeds Timer1 with 48/4 = 12 MHz
// pulses that are prescaled by a factor of 1:8, giving a Timer1 tick
// rate of 1.5 MHz, not 1 MHz, so we have to scale the above units
// by 3/2 to give microsecond units (3 Timer1 ticks is 2 microseconds)...
// #define MICROSECONDS 3 / 2

// Letting the compiler work it out for us, we have...
#define TIMER1_PRESCALER      8
#define TIMER1_RATE (CLOCK_FREQUENCY/4/TIMER1_PRESCALER)
#define MICROSECONDS(x) (x * TIMER1_RATE / 1000000)

#define TIMER3_PRESCALER      8
#define TIMER3_RATE (CLOCK_FREQUENCY/4/TIMER3_PRESCALER)
#define TIMER3_INTERRUPTS_PER_SECOND (TIMER3_RATE / 65536)
#define KEY_REPEAT_DELAY_IN_SECONDS 0.75
#define KEY_REPEAT_DELAY_IN_TICKS (KEY_REPEAT_DELAY_IN_SECONDS * TIMER3_INTERRUPTS_PER_SECOND)
volatile signed short nKeyRepeatDelay; // Number of Timer3 ticks before key repeat starts

#define FRONT_PANEL_KEY_REPEAT_RATE_IN_HZ 4.0
#define FRONT_PANEL_KEY_REPEAT_TICKS (TIMER3_INTERRUPTS_PER_SECOND / FRONT_PANEL_KEY_REPEAT_RATE_IN_HZ)
byte nTicksPerKeyRepeat;


// USB buffers must be in USB RAM, hence the "absolute" specifier...
byte BANK4_RESERVED_FOR_USB[256] absolute 0x400; // Prevent compiler from allocating
                                                 // RAM variables in Bank 4 because
                                                 // the USB hardware uses that bank.
                                                 // Refer to the PIC18F25K50 datasheet
                                                 // section "6.4.1 USB RAM" for more
                                                 // information.
byte sUSBResponse[1+1] absolute 0x500;  // Buffer for PIC <-- Host (ReportId + 1 byte)
byte sUSBCommand[1+3]  absolute 0x508;  // Buffer for PIC --> Host (ReportId + 3 bytes)

#define LCD_WIDTH 16
char sLCDLine1[LCD_WIDTH+1];
char sLCDLine2[LCD_WIDTH+1];

byte nBit;
byte cByte;
byte nByte;
int  nActivityLEDDelay;  // LED delay in units of "main loop iterations"

// The command sent/received using Infrared...
union
{
  byte b[6];
  struct
  {
    byte nAddress;
    byte nAddressInverted;
    byte nModifiers;
    byte nModifiersInverted;
    byte nCommand;
    byte nCommandInverted;
  } s;
} irCommand;

// The command sent/received using USB...
union
{
  unsigned int uxyy;        // uxyy (xyy is changed by UP/DOWN buttons)
  struct
  {                         // This overlays the Little-Endian uxyy above
    byte yy;                //   yy
    t_keyModifiers ux;      // ux   (x is changed by CTL/ATL/SHIFT buttons)
  } s;
} usbCommand;

#define USAGE_KEYBOARD             0x00
#define USAGE_SYSTEM_CONTROL       0x10
#define USAGE_CONSUMER_DEVICE      0x20
#define USAGE_LOCAL_IRK_FUNCTION   0xF0

const char * pUnshiftedKey;
const char * pShiftedKey;

// Copies a text string from ROM into a RAM buffer (saves RAM)
char * _TEXT(const char * p)
{
  static char sText[32];
  char * q;
  for (q=&sText; *q++ = *p++; );
  return &sText;
}


void actionBacklightDelay ()
{
  switch (nConfigBacklightDelay)
  {
   case 0x00:                  // Backlight always OFF
     LCD_BACKLIGHT = 1;        // Set LCD backlight off (0=On, 1=Off)
     TMR0ON_bit = 0;           // Disable the backlight timer
     break;
   case 0xFF:                  // Backlight always ON
     LCD_BACKLIGHT = 0;        // Set LCD backlight on (0=On, 1=Off)
     TMR0ON_bit = 0;           // Disable the backlight timer
     break;
   default:                    // Backlight off after nn seconds
     TMR0ON_bit = 1;           // Enable the backlight timer
     break;
  }
}

void loadBacklightDelay ()
{
  nConfigBacklightDelay = EEPROM_Read(1);   // Backlight on for nn seconds
  actionBacklightDelay();
}

void saveBacklightDelay ()
{
  EEPROM_Write(1, nConfigBacklightDelay);
  actionBacklightDelay();
}

const char * getKeyWithNoShift () // Note: Literals returned as const are in ROM
{ // Keyboard key without SHIFT modifier key pressed
  switch (usbCommand.s.yy)
  {
    case 0x04: return "a";             //   4  04  a A
    case 0x05: return "b";             //   5  05  b B
    case 0x06: return "c";             //   6  06  c C
    case 0x07: return "d";             //   7  07  d D
    case 0x08: return "e";             //   8  08  e E
    case 0x09: return "f";             //   9  09  f F
    case 0x0A: return "g";             //  10  0A  g G
    case 0x0B: return "h";             //  11  0B  h H
    case 0x0C: return "i";             //  12  0C  i I
    case 0x0D: return "j";             //  13  0D  j J
    case 0x0E: return "k";             //  14  0E  k K
    case 0x0F: return "l";             //  15  0F  l L
    case 0x10: return "m";             //  16  10  m M
    case 0x11: return "n";             //  17  11  n N
    case 0x12: return "o";             //  18  12  o O
    case 0x13: return "p";             //  19  13  p P
    case 0x14: return "q";             //  20  14  q Q
    case 0x15: return "r";             //  21  15  r R
    case 0x16: return "s";             //  22  16  s S
    case 0x17: return "t";             //  23  17  t T
    case 0x18: return "u";             //  24  18  u U
    case 0x19: return "v";             //  25  19  v V
    case 0x1A: return "w";             //  26  1A  w W
    case 0x1B: return "x";             //  27  1B  x X
    case 0x1C: return "y";             //  28  1C  y Y
    case 0x1D: return "z";             //  29  1D  z Z
    case 0x1E: return "1";             //  30  1E  1 !
    case 0x1F: return "2";             //  31  1F  2 @
    case 0x20: return "3";             //  32  20  3 #
    case 0x21: return "4";             //  33  21  4 $
    case 0x22: return "5";             //  34  22  5 %
    case 0x23: return "6";             //  35  23  6 ^
    case 0x24: return "7";             //  36  24  7 &
    case 0x25: return "8";             //  37  25  8 *
    case 0x26: return "9";             //  38  26  9 (
    case 0x27: return "0";             //  39  27  0 )
    case 0x28: return "Enter";         //  40  28  Enter
    case 0x29: return "Esc";           //  41  29  Escape
    case 0x2A: return "Backspace";     //  42  2A  Backspace
    case 0x2B: return "Tab";           //  43  2B  Tab
    case 0x2C: return "Spacebar";      //  44  2C  Spacebar
    case 0x2D: return "-";             //  45  2D  - _
    case 0x2E: return "=";             //  46  2E  = +
    case 0x2F: return "[";             //  47  2F  [ {
    case 0x30: return "]";             //  48  30  ] }
    case 0x31: return "\7";            //  49  31  \ |   (character generator)
                                       //  50  32  Non-US # Non-US ~
    case 0x33: return ";";             //  51  33  ; :
    case 0x34: return "\'";            //  52  34  ‘ "
    case 0x35: return "`";             //  53  35  ` ~
    case 0x36: return ",";             //  54  36  , <
    case 0x37: return ".";             //  55  37  . >
    case 0x38: return "/";             //  56  38  / ?
    case 0x39: return "Caps Lock";     //  57  39  Caps lock
    case 0x3A: return "F1";            //  58  3A  F1
    case 0x3B: return "F2";            //  59  3B  F2
    case 0x3C: return "F3";            //  60  3C  F3
    case 0x3D: return "F4";            //  61  3D  F4
    case 0x3E: return "F5";            //  62  3E  F5
    case 0x3F: return "F6";            //  63  3F  F6
    case 0x40: return "F7";            //  64  40  F7
    case 0x41: return "F8";            //  65  41  F8
    case 0x42: return "F9";            //  66  42  F9
    case 0x43: return "F10";           //  67  43  F10
    case 0x44: return "F11";           //  68  44  F11
    case 0x45: return "F12";           //  69  45  F12
    case 0x46: return "Print Screen";  //  70  46  Print Screen
    case 0x47: return "Scroll Lock";   //  71  47  Scroll Lock
    case 0x48: return "\6";            //  72  48  Pause (character generator)
    case 0x49: return "Insert";        //  73  49  Insert
    case 0x4A: return "Home";          //  74  4A  Home
    case 0x4B: return "Page Up";       //  75  4B  Page Up
    case 0x4C: return "Delete";        //  76  4C  Delete
    case 0x4D: return "End";           //  77  4D  End
    case 0x4E: return "Page Down";     //  78  4E  Page Down
    case 0x4F: return "\3";            //  79  4F  Right (character generator)
    case 0x50: return "\4";            //  80  50  Left  (character generator)
    case 0x51: return "\2";            //  81  51  Down  (character generator)
    case 0x52: return "\1";            //  82  52  Up    (character generator)
    case 0x53: return "Num Lock";      //  83  53  Keypad Num Lock Clear

    case 0x54: return "Keypad /";      //  84  54  Keypad /
    case 0x55: return "Keypad *";      //  85  55  Keypad *
    case 0x56: return "Keypad -";      //  86  56  Keypad -
    case 0x57: return "Keypad +";      //  87  57  Keypad +
    case 0x58: return "Keypad Enter";  //  88  58  Keypad Enter
    case 0x59: return "Keypad 1";      //  89  59  Keypad 1 End
    case 0x5A: return "Keypad 2";      //  90  5A  Keypad 2 Down
    case 0x5B: return "Keypad 3";      //  91  5B  Keypad 3 PgDn
    case 0x5C: return "Keypad 4";      //  92  5C  Keypad 4 Left
    case 0x5D: return "Keypad 5";      //  93  5D  Keypad 5
    case 0x5E: return "Keypad 6";      //  94  5E  Keypad 6 Right
    case 0x5F: return "Keypad 7";      //  95  5F  Keypad 7 Home
    case 0x60: return "Keypad 8";      //  96  60  Keypad 8 Up
    case 0x61: return "Keypad 9";      //  97  61  Keypad 9 PgUp
    case 0x62: return "Keypad 0";      //  98  62  Keypad 0 Ins
    case 0x63: return "Keypad .";      //  99  63  Keypad . Del
                                       //  100 64  Non-US \ |
                                       //  101 65  Application
                                       //  102 66  Power
    case 0x67: return "Keypad =";      //  103 67  Keypad =
    case 0x68: return "F13";           //  104 68  F13
    case 0x69: return "F14";           //  105 69  F14
    case 0x6A: return "F15";           //  106 6A  F15
    case 0x6B: return "F16";           //  107 6B  F16
    case 0x6C: return "F17";           //  108 6C  F17
    case 0x6D: return "F18";           //  109 6D  F18
    case 0x6E: return "F19";           //  110 6E  F19
    case 0x6F: return "F20";           //  111 6F  F20
    case 0x70: return "F21";           //  112 70  F21
    case 0x71: return "F22";           //  113 71  F22
    case 0x72: return "F23";           //  114 72  F23
    case 0x73: return "F24";           //  115 73  F24

    default: return "";
  }
}

const char * getKeyWithShift() // Note: Literals returned as const are in ROM
{ // Keyboard key with SHIFT modifier key pressed
  switch (usbCommand.s.yy)
  {
    case 0x04: return "A";             //   4  04  a A
    case 0x05: return "B";             //   5  05  b B
    case 0x06: return "C";             //   6  06  c C
    case 0x07: return "D";             //   7  07  d D
    case 0x08: return "E";             //   8  08  e E
    case 0x09: return "F";             //   9  09  f F
    case 0x0A: return "G";             //  10  0A  g G
    case 0x0B: return "H";             //  11  0B  h H
    case 0x0C: return "I";             //  12  0C  i I
    case 0x0D: return "J";             //  13  0D  j J
    case 0x0E: return "K";             //  14  0E  k K
    case 0x0F: return "L";             //  15  0F  l L
    case 0x10: return "M";             //  16  10  m M
    case 0x11: return "N";             //  17  11  n N
    case 0x12: return "O";             //  18  12  o O
    case 0x13: return "P";             //  19  13  p P
    case 0x14: return "Q";             //  20  14  q Q
    case 0x15: return "R";             //  21  15  r R
    case 0x16: return "S";             //  22  16  s S
    case 0x17: return "T";             //  23  17  t T
    case 0x18: return "U";             //  24  18  u U
    case 0x19: return "V";             //  25  19  v V
    case 0x1A: return "W";             //  26  1A  w W
    case 0x1B: return "X";             //  27  1B  x X
    case 0x1C: return "Y";             //  28  1C  y Y
    case 0x1D: return "Z";             //  29  1D  z Z
    case 0x1E: return "!";             //  30  1E  1 !
    case 0x1F: return "@";             //  31  1F  2 @
    case 0x20: return "#";             //  32  20  3 #
    case 0x21: return "$";             //  33  21  4 $
    case 0x22: return "%";             //  34  22  5 %
    case 0x23: return "^";             //  35  23  6 ^
    case 0x24: return "&";             //  36  24  7 &
    case 0x25: return "*";             //  37  25  8 *
    case 0x26: return "(";             //  38  26  9 (
    case 0x27: return ")";             //  39  27  0 )

    case 0x2D: return "_";             //  45  2D  - _
    case 0x2E: return "+";             //  46  2E  = +
    case 0x2F: return "{";             //  47  2F  [ {
    case 0x30: return "}";             //  48  30  ] }
    case 0x31: return "|";             //  49  31  \ |
                                       //  50  32  Non-US # Non-US ~
    case 0x33: return ":";             //  51  33  ; :
    case 0x34: return "\"";            //  52  34  ‘ "
    case 0x35: return "\5";            //  53  35  ` ~  (character generator)
    case 0x36: return "<";             //  54  36  , <
    case 0x37: return ">";             //  55  37  . >
    case 0x38: return "?";             //  56  38  / ?

    case 0x59: return "Keypad End";    //  89  59  Keypad 1 End
    case 0x5A: return "Keypad Down";   //  90  5A  Keypad 2 Down
    case 0x5B: return "Keypad PgDn";   //  91  5B  Keypad 3 PgDn
    case 0x5C: return "Keypad Left";   //  92  5C  Keypad 4 Left
                                       //  93  5D  Keypad 5
    case 0x5E: return "Keypad Right";  //  94  5E  Keypad 6 Right
    case 0x5F: return "Keypad Home";   //  95  5F  Keypad 7 Home
    case 0x60: return "Keypad Up";     //  96  60  Keypad 8 Up
    case 0x61: return "Keypad PgUp";   //  97  61  Keypad 9 PgUp
    case 0x62: return "Keypad Insert"; //  98  62  Keypad 0 Ins
    case 0x63: return "Keypad Delete"; //  99  63  Keypad . Del
                                       //  100 64  Non-US \ |

    default: return "";
  }
}

const char * getDesc () // Note: Literals returned as const are in ROM
{
  unsigned int nFunction;

  nFunction = usbCommand.uxyy;  // ...compiles in less memory by doing this
  switch (usbCommand.s.ux.byte & 0xF0)
  {
    case USAGE_KEYBOARD:
      pUnshiftedKey = getKeyWithNoShift();
      if (usbCommand.s.ux.bits.LeftShift)
      {
        pShiftedKey = getKeyWithShift();
        if (*pShiftedKey)
           return pShiftedKey;   // Name changes when SHIFT is pressed
        else
           return pUnshiftedKey; // Name does not change when SHIFT is pressed
      }
      else
        return pUnshiftedKey;
    case USAGE_SYSTEM_CONTROL:
      switch (nFunction)
      {
        case 0x1081: return "Power off";
        case 0x1082: return "Sleep";
        case 0x1083: return "Wake";
        default: return "";
      }
    case USAGE_CONSUMER_DEVICE:
      switch (nFunction)
      {
        case 0x2040: return "Menu";           // Menu (on/off)
        case 0x2041: return "Menu Pick";      // Menu Pick
        case 0x2042: return "Menu \1";        // Menu Up
        case 0x2043: return "Menu \2";        // Menu Down
        case 0x2044: return "Menu \4";        // Menu Left
        case 0x2045: return "Menu \3";        // Menu Right
        case 0x2046: return "Menu Exit";      // Menu Escape
        case 0x2047: return "Menu +";         // Menu Value Increase
        case 0x2048: return "Menu -";         // Menu Value Decrease
        case 0x209C: return "Ch+";            // Channel Up
        case 0x209D: return "Ch-";            // Channel Down
        case 0x20B0: return ">";              // Play
        case 0x20B1: return "\6";             // Pause
        case 0x20B2: return "Rec";            // Record
        case 0x20B3: return ">>";             // Fast Forward
        case 0x20B4: return "<<";             // Rewind
        case 0x20B5: return ">>|";            // Next Track
        case 0x20B6: return "|<<";            // Previous Track
        case 0x20B7: return "Stop";           // Stop
        case 0x20B8: return "Eject";          // Eject
        case 0x20CD: return ">/\6";           // Play / Pause
        case 0x20E2: return "Mute";           // Mute
        case 0x20E9: return "Vol+";           // Volume Up
        case 0x20EA: return "Vol-";           // Volume Down
        case 0x2183: return "Media Player";   // Application: Consumer Control Configuration
        case 0x2185: return "Text Editor";    // Application: Text Editor
        case 0x218A: return "Email";          // Application: Email
        case 0x2192: return "Calculator";     // Application: Calculator
        case 0x2194: return "Computer";       // Application: Local Machine Browser
        case 0x2196: return "Web Browser";    // Application: Web Browser
        case 0x2221: return "Web Search";     // Browser: Search
        case 0x2223: return "Web Home";       // Browser: Home
        case 0x2224: return "Web Prev";       // Browser: Back
        case 0x2225: return "Web Next";       // Browser: Next
        case 0x2226: return "Web Stop";       // Browser: Stop
        case 0x2227: return "Web Refresh";    // Browser: Reload
        case 0x222A: return "Web Bookmarks";  // Browser: Bookmarks
        default: return "";
      }
//  case 0x30:     // Reserved
//  case 0x40:     // Reserved
//  case 0x50:     // Reserved
//  case 0x60:     // Reserved
//  case 0x70:     // Reserved
//  case 0x90:     // Reserved
//  case 0x90:     // Reserved
//  case 0xA0:     // Reserved
//  case 0xB0:     // Reserved
//  case 0xC0:     // Reserved
//  case 0xD0:     // Reserved
//  case 0xE0:     // Reserved
    case USAGE_LOCAL_IRK_FUNCTION:     // IRK! local functions
      switch (usbCommand.s.yy)
      {
        case CMD_SET_DEVICE_ADDRESS:  return "Set Address";
        case CMD_PRESS_POWER_SWITCH:  return "Power Switch";
        case CMD_PRESS_RESET_SWITCH:  return "Reset Switch";
        case CMD_INIT_USB:            return "Init USB";
        case CMD_SET_BACKLIGHT_OFF:   return "Light Off";   // Set delay = 0x00
        case CMD_SET_BACKLIGHT_ON:    return "Light On";    // Set delay = 0xFF
        case CMD_SET_BACKLIGHT_DELAY: return "Light Delay"; // Set delay = 0x01 to 0xFE (1 to 254 seconds)
        case CMD_SET_DEBUG_OFF:       return "Debug Off";
        case CMD_SET_DEBUG_ON:        return "Debug On";
        case CMD_PRESS_AUX_SWITCH:    return "Aux Switch";
        case CMD_POWER_SWITCH_ON:     return "Power Sw On";
        case CMD_RESET_SWITCH_ON:     return "Reset Sw On";
        case CMD_AUX_SWITCH_ON:       return "Aux Sw On";
        case CMD_POWER_SWITCH_OFF:    return "Power Sw Off";
        case CMD_RESET_SWITCH_OFF:    return "Reset Sw Off";
        case CMD_AUX_SWITCH_OFF:      return "Aux Sw Off";
        default: return "";
      }
    default: return "";
  }
}

// Convert a byte (character) to printable hex (as in the Rexx Programming Language)
void c2x (byte c, char * p)
{
  static const char HEX[] = "0123456789ABCDEF";
  *p++   = HEX[(c & 0xF0) >> 4];
  *p     = HEX[(c & 0x0F)     ];
}

void enableBacklight()
{
  if (nConfigBacklightDelay == 0x00) return;
  if (nConfigBacklightDelay == 0xFF) return;
  LCD_BACKLIGHT = 0;           // Turn on backlight (0=On, 1=Off)
  nBacklightDelay = nConfigBacklightDelay;  // Keep backlight on for this many seconds
  TMR0H = HiByte(ONE_SECOND);  // Set high-byte of 16-bit count
  TMR0L = LoByte(ONE_SECOND);  // Set low-byte and write all 16 bits to Timer0
  TMR0IF_bit = 0;              // Clear the Timer0 interrupt flag
  TMR0IE_bit = 1;              // Enable Timer0 interrupts
  TMR0ON_bit = 1;              // Turn on Timer0
}

void showDebugInfo()
{
  byte i;
  // Display debug information instead of the last key pressed
  // Debug Line 1:
  //        <---16 chars--->
  //       +0000000000111111
  //       +0123456789012345
  // Line1: bbbbbbbbbbbbnnnn
  //        bbbbbbbbbbbb       = Last 6-byte IR command received
  //                    nnnn   = Number of resets before a good command was received
  for (i = 0; i < 6; i++)
  {
    c2x(irCommand.b[i], &sLCDLine1[i*2]);
  }
  c2x(resetCount.b[1], &sLCDLine1[12]);
  c2x(resetCount.b[0], &sLCDLine1[14]);
  sLCDLine1[16] = 0;
  // Debug Line 2:
  //        <---16 chars--->
  //       +0000000000111111
  //       +0123456789012345
  // Line2: yy ss n.n wwww r
  //        yy                 = Last valid USB usage code (xyy)
  //           ss              = Last IR processing state number (00=Reset)
  //              n.n          = Number of bytes.bits received (6.0 is normal)
  //                  wwww     = Last pulse width
  //                       v   = Last pulse was a falling edge (DOWN_ARROW)
  //                       ^   = Last pulse was a rising edge  (UP_ARROW)
  c2x(usbCommand.s.yy, &sLCDLine2);
  sLCDLine2[2] = ' ';
  c2x(nState, &sLCDLine2[3]);
  c2x(nByte, &sLCDLine2[5]);
  sLCDLine2[5] = ' ';
  c2x(nBit,  &sLCDLine2[7]);
  sLCDLine2[7] = '.';
  sLCDLine2[9] = ' ';
  c2x(nPulseWidth.byte[1], &sLCDLine2[10]);
  c2x(nPulseWidth.byte[0], &sLCDLine2[12]);
  sLCDLine2[14] = ' ';
  if (bRisingEdge)
     sLCDLine2[15] = UP_ARROW;
  else
     sLCDLine2[15] = DOWN_ARROW;
  sLCDLine2[16] = 0;
  resetCount.n = 0;
  Lcd_Cmd(_LCD_CLEAR);                // Clear display
  Lcd_Out(1,1,&sLCDLine1);
  Lcd_Out(2,1,&sLCDLine2);
}

void updateLCD()
{
  Lcd_Cmd(_LCD_CLEAR);                // Clear display
  c2x(usbCommand.s.ux.byte, sLCDLine1);
  sLCDLine1[2] = ' ';
  sLCDLine1[3] = 0;
  switch (usbCommand.s.ux.byte & 0xF0)   // 0xUM (Usage 4 bits, Modifiers 4 bits)
  {
    case USAGE_KEYBOARD:
      if (bSettingUsage)
      {
         strcat(sLCDLine1,_TEXT("Keyboard"));
      }
      else
      {
        // Display key modifiers code and description
        switch (usbCommand.s.ux.byte & 0x0F)
        {
        case 0x00:  // if no modifier keys pressed
          strcat(sLCDLine1,_TEXT("Keyboard"));
          break;
        case 0x0F:  // if all modifier keys pressed
          strcat(sLCDLine1,_TEXT("GUI + ALL")); // text won't fit otherwise
          break;
        default:
          if (usbCommand.s.ux.bits.LeftGUI)     strcat(sLCDLine1,_TEXT("GUI "));
          if (usbCommand.s.ux.bits.LeftControl) strcat(sLCDLine1,_TEXT("CTL "));
          if (usbCommand.s.ux.bits.LeftAlt)     strcat(sLCDLine1,_TEXT("ALT "));
          if (usbCommand.s.ux.bits.LeftShift)   strcat(sLCDLine1,_TEXT("SHIFT"));
        }
      }
      break;
    case USAGE_SYSTEM_CONTROL:
      strcat(sLCDLine1,_TEXT("System"));
      break;
    case USAGE_CONSUMER_DEVICE:
      strcat(sLCDLine1,_TEXT("Consumer Dev"));
      break;
    case USAGE_LOCAL_IRK_FUNCTION:
      strcat(sLCDLine1,_TEXT("IRK! Function"));
    default:
      break;
  }

  if (bSettingUsage)
  {
    strcpy(sLCDLine2,_TEXT("\1\1 Select Usage"));  // ^^ Select Usage
  }
  else if (bSettingDeviceAddress)
  {
    c2x(nConfigDeviceAddress, sLCDLine2);
    sLCDLine2[2] = ' ';
    sLCDLine2[3] = 0;
    strcat(sLCDLine2,_TEXT("\4 Address"));  // <- Address
  }
  else if (bSettingBacklightDelay)
  {
    c2x(nNewBacklightDelay, sLCDLine2);
    sLCDLine2[2] = ' ';
    sLCDLine2[3] = 0;
    strcat(sLCDLine2,_TEXT("\4 "));         // <-
    switch (nNewBacklightDelay)
    {
      case 0x00:
        strcat(sLCDLine2,_TEXT("Off"));
        break;
      case 0xFF:
        strcat(sLCDLine2,_TEXT("On"));
        break;
      default:
        ByteToStr(nNewBacklightDelay, sLCDLine2+5);
        strcat(sLCDLine2,_TEXT(" secs"));
    }
  }
  else
  {
    // Display key code and description
    c2x(usbCommand.s.yy, sLCDLine2);
    sLCDLine2[2] = ' ';
    sLCDLine2[3] = 0;
    strncat(sLCDLine2, _TEXT(getDesc()), 13);
  }
  Lcd_Out(1,1,sLCDLine1);
  Lcd_Out(2,1,sLCDLine2);
}

void enableUSB()
{
  byte i;

  enableBacklight();                       // Conditionally turn on LCD backlight
  Lcd_Out(2,1,_TEXT("Enabling USB    "));

  sUSBCommand[0] = REPORT_ID_KEYBOARD;     // Report Id = Keyboard
  sUSBCommand[1] = 0;                      // No modifiers
  sUSBCommand[2] = 0;                      // Reserved for OEM
  sUSBCommand[3] = 0;                      // No key pressed
  bUSBReady = FALSE;
  while (!bUSBReady)
  {
    HID_Enable(&sUSBResponse, &sUSBCommand);
    for (i=0; !bUSBReady && i < 50; i++)
    {
      ACTIVITY_LED = ON;    // Flash activity to indicate init in progress
      Delay_ms(50);
      ACTIVITY_LED = OFF;
      Delay_ms(50);
      bUSBReady = HID_Write(&sUSBCommand, 4) != 0; // Copy to USB buffer and try to send
    }
    if (!bUSBReady)
    {
      enableBacklight();      // Keep LCD backlight on while retrying USB init
      HID_Disable();
      Delay_ms(5000);
    }
  }
  Lcd_Out(2,1,_TEXT("USB Ready   "));
}

void disableUSB()
{
  HID_Disable();
  Lcd_Out(2,1,_TEXT("USB Disabled"));
  bUSBReady = FALSE;
}

void sendUSBKeystroke()
{
  if (bUSBReady)
  {
    sUSBCommand[0] = REPORT_ID_KEYBOARD;      // Report Id = Keyboard
    sUSBCommand[1] = usbCommand.s.ux.byte;    // Ctrl/Alt/Shift modifiers
    sUSBCommand[2] = 0;                       // Reserved for OEM
    sUSBCommand[3] = usbCommand.s.yy;         // Key pressed
    while(!HID_Write(&sUSBCommand, 4));       // Copy to USB buffer and try to send
    sUSBCommand[1] = 0;                       // No modifiers now
    sUSBCommand[3] = 0;                       // No key pressed now
    while(!HID_Write(&sUSBCommand, 4));       // Copy to USB buffer and try to send
  }
}

void sendUSBSystemControlCommand()
{
  if (bUSBReady)
  {
    sUSBCommand[0] = REPORT_ID_SYSTEM_CONTROL;// Report Id = System Control (power)
    sUSBCommand[1] = usbCommand.s.yy;         // Power function requested
    while(!HID_Write(&sUSBCommand, 2));       // Copy to USB buffer and try to send
    sUSBCommand[1] = 0;                       // No power function requested anymore
    while(!HID_Write(&sUSBCommand, 2));       // Copy to USB buffer and try to send
  }
}

void sendUSBConsumerDeviceCommand()
{
  if (bUSBReady)
  {
    sUSBCommand[0] = REPORT_ID_CONSUMER_DEVICE; // Report Id = Consumer Device
    sUSBCommand[1] = usbCommand.s.yy;         // Function requested (low byte)
    sUSBCommand[2] = usbCommand.s.ux.byte & 0x0F;    // Function requested (high byte)
    while(!HID_Write(&sUSBCommand, 3));       // Copy to USB buffer and try to send
    sUSBCommand[1] = 0;                      // Function requested low byte
    sUSBCommand[2] = 0;                      // Function requested high byte
    while(!HID_Write(&sUSBCommand, 3));      // Copy to USB buffer and try to send
  }
}

void performLocalIRKFunction()
{
  switch (usbCommand.s.yy)
  {
    case CMD_PRESS_POWER_SWITCH:
      POWER_SWITCH = ON;
      Delay_ms(250);
    case CMD_POWER_SWITCH_OFF:
      POWER_SWITCH = OFF;
      break;
    case CMD_PRESS_RESET_SWITCH:
      RESET_SWITCH = ON;
      Delay_ms(250);
    case CMD_RESET_SWITCH_OFF:
      RESET_SWITCH = OFF;
      break;
    case CMD_PRESS_AUX_SWITCH:
      AUX_SWITCH = ON;
      Delay_ms(250);
    case CMD_AUX_SWITCH_OFF:
      AUX_SWITCH = OFF;
      break;
    case CMD_POWER_SWITCH_ON:
      POWER_SWITCH = ON;
      break;
    case CMD_RESET_SWITCH_ON:
      RESET_SWITCH = ON;
      break;
    case CMD_AUX_SWITCH_ON:
      AUX_SWITCH = ON;
      break;
    case CMD_INIT_USB:
      disableUSB();
      enableUSB();
      break;
    case CMD_SET_BACKLIGHT_OFF:   // User wants backlight temporarily OFF
      nConfigBacklightDelay = 0x00;
      actionBacklightDelay();
      break;
    case CMD_SET_BACKLIGHT_ON:    // User wants backlight temporarily ON
      nConfigBacklightDelay = 0xFF;
      actionBacklightDelay();
      break;
    case CMD_SET_DEBUG_OFF:
      bDebugMode = OFF;
      updateLCD();
      break;
    case CMD_SET_DEBUG_ON:
      bDebugMode = ON;
      break;
    case CMD_SET_DEVICE_ADDRESS:
      break;    // Do nothing, user is setting the IRK! device address
    default:
      break;
  }
}

void executeCommand()
{
  nActivityLEDDelay = 10000;  // Number of main loop iterations to keep the activity LED glowing
  ACTIVITY_LED = ON;
  switch (usbCommand.s.ux.byte & 0xF0)   // 0xUM (Usage 4 bits, Modifiers 4 bits)
  {
    case USAGE_KEYBOARD:
      sendUSBKeystroke();
      break;
    case USAGE_SYSTEM_CONTROL:
      sendUSBSystemControlCommand();
      break;
    case USAGE_CONSUMER_DEVICE:
      sendUSBConsumerDeviceCommand();
      break;
    case USAGE_LOCAL_IRK_FUNCTION:
      performLocalIRKFunction();
      break;
    default:
      break;
  }
}

void gotoResetState()
{
  nState = STATE_IR_RESET;
  nByte = 0;
  nBit = 0;
  resetCount.n++;
}

void disableInfraredCapture()
{
  CCP2IE_bit = 0;           // Disable CCP2 interrupts
}

void enableInfraredCapture()
{
  CCP2IE_bit = 0;           // Disable CCP2 interrupts while resetting capture mode
  CCP2IF_bit = 0;           // Reset CCP2 interrupt flag
  CCP2CON = 0b00000000;     // Reset the CCP2 module
  CCP2CON = 0b00000100;     // Set CCP2 to capture the next falling edge
  TMR1H = 0;                // Prime Timer1 high byte
  TMR1L = 0;                // Set Timer1 low and high bytes now
  gotoResetState();
  CCP2IE_bit = 1;           // Enable CCP2 interrupts
}

void transmitInfraredShortMark()
{
  PWM1_Start();
  Delay_us(WIDTH_SHORT);    // Send a short mark
  PWM1_Stop();
}

void transmitInfraredByte (byte b)
{
  byte i;
  for (i = 8; i > 0; i--)
  {
    transmitInfraredShortMark();          // Send a short mark
    if (b & 0b10000000)       // If next bit is a 1
      Delay_us(WIDTH_LONG);   // Send a long space
    else
      Delay_us(WIDTH_SHORT);  // Send a short space
    b <<= 1;
  }
}

void transmitInfraredCommand()
{
  byte i;
  ACTIVITY_LED = ON;
  disableInfraredCapture();         // Disable IR capture while transmitting...
  irCommand.s.nAddress           =  nConfigDeviceAddress;
  irCommand.s.nAddressInverted   = ~nConfigDeviceAddress;
  irCommand.s.nModifiers         =  usbCommand.s.ux.byte;
  irCommand.s.nModifiersInverted = ~usbCommand.s.ux.byte;
  irCommand.s.nCommand           =  usbCommand.s.yy;
  irCommand.s.nCommandInverted   = ~usbCommand.s.yy;
  PWM1_Start();
  Delay_us(WIDTH_TRAINING_PULSE);
  PWM1_Stop();
  Delay_us(WIDTH_SILENCE_AFTER_TRAINING);
  for (i = 0; i < sizeof irCommand.b; i++)
  {
    transmitInfraredByte(irCommand.b[i]);
  }
  transmitInfraredShortMark();      // Send a short mark to end
  Delay_ms(100);                    // Pause between transmitted IR commands
  enableInfraredCapture();
  ACTIVITY_LED = OFF;
}

void defineCustomCharacters()
{
  // Symbol #0 is not being defined because to use it would mean putting
  // null characters in the buffers to be written to the LCD. That's ok but
  // it makes null-terminated string handling more difficult.

  // Note that the Character Generator RAM (CGRAM) expects symbols to be defined
  // as 8 rows of 8 bits, but the LCD display cell size is only 8 rows of 5 bit
  // (i.e. a 7 x 5 character cell, plus one row for an underscore).
  // Consequently, the first three bits of each row are set to 0.
  
  Lcd_Cmd(72);      // Set CGRAM address to symbol #1 (symbol #0 is at 64)
  // 01 = Up Arrow
  Lcd_Chr_CP(0b00000000);
  Lcd_Chr_CP(0b00000100);
  Lcd_Chr_CP(0b00001010);
  Lcd_Chr_CP(0b00010101);
  Lcd_Chr_CP(0b00000100);
  Lcd_Chr_CP(0b00000100);
  Lcd_Chr_CP(0b00000000);
  Lcd_Chr_CP(0b00000000);
  // 02 = Down Arrow
  Lcd_Chr_CP(0b00000000);
  Lcd_Chr_CP(0b00000100);
  Lcd_Chr_CP(0b00000100);
  Lcd_Chr_CP(0b00010101);
  Lcd_Chr_CP(0b00001010);
  Lcd_Chr_CP(0b00000100);
  Lcd_Chr_CP(0b00000000);
  Lcd_Chr_CP(0b00000000);
  // 03 = Right Arrow
  Lcd_Chr_CP(0b00000000);
  Lcd_Chr_CP(0b00000100);
  Lcd_Chr_CP(0b00000010);
  Lcd_Chr_CP(0b00011101);
  Lcd_Chr_CP(0b00000010);
  Lcd_Chr_CP(0b00000100);
  Lcd_Chr_CP(0b00000000);
  Lcd_Chr_CP(0b00000000);
  // 04 = Left Arrow
  Lcd_Chr_CP(0b00000000);
  Lcd_Chr_CP(0b00000100);
  Lcd_Chr_CP(0b00001000);
  Lcd_Chr_CP(0b00010111);
  Lcd_Chr_CP(0b00001000);
  Lcd_Chr_CP(0b00000100);
  Lcd_Chr_CP(0b00000000);
  Lcd_Chr_CP(0b00000000);
  // 05 = Tilde
  Lcd_Chr_CP(0b00000000);
  Lcd_Chr_CP(0b00000000);
  Lcd_Chr_CP(0b00001001);
  Lcd_Chr_CP(0b00010101);
  Lcd_Chr_CP(0b00010010);
  Lcd_Chr_CP(0b00000000);
  Lcd_Chr_CP(0b00000000);
  Lcd_Chr_CP(0b00000000);
  // 06 = Pause
  Lcd_Chr_CP(0b00011011);
  Lcd_Chr_CP(0b00011011);
  Lcd_Chr_CP(0b00011011);
  Lcd_Chr_CP(0b00011011);
  Lcd_Chr_CP(0b00011011);
  Lcd_Chr_CP(0b00011011);
  Lcd_Chr_CP(0b00011011);
  Lcd_Chr_CP(0b00000000);
  // 07 = Back slash
  Lcd_Chr_CP(0b00000000);
  Lcd_Chr_CP(0b00010000);
  Lcd_Chr_CP(0b00001000);
  Lcd_Chr_CP(0b00000100);
  Lcd_Chr_CP(0b00000010);
  Lcd_Chr_CP(0b00000001);
  Lcd_Chr_CP(0b00000000);
  Lcd_Chr_CP(0b00000000);
  Lcd_Cmd(_LCD_RETURN_HOME);
}

void Prolog()
{
  byte i;

  ANSELA = 0b00000000;    // Configure all PORTA bits as digital
  ANSELB = 0b00000000;    // Configure all PORTB bits as digital
  ANSELC = 0b00000000;    // Configure all PORTC bits as digital

  //        76543210
  TRISA = 0b10111111;     // PORTA: All pins input except for RA6
  TRISB = 0b11111111;     // PORTB: All pins input
  TRISC = 0b00111000;     // PORTC: Only RC3, RC4 and RC5 pins are input

  LATA = 0;
  LATB = 0;
  LATC = 0;

  NOT_RBPU_bit = 0;       // Enable PORTB weak pull-ups (PIC 18F25K50)

  ACTIVITY_LED = ON;      // Show microcontroller is working (at 1 MHz for now)
  OSCCON = 0b01110000;
//           x              0  = IDLEN: Sleep on SLEEP instruction
//            xxx           111= IRCF: HFINTOSC (16 MHz)
//               x          0  = OSTS: Status bit
//                x         0  = HFIOFS: Status bit
//                 xx       00 = SCS: Primary clock (determined by FOSC in CONFIG1H)
  while (!HFIOFS_bit);    // Wait for HFINTOSC to stabilise
  while (!OSTS_bit);      // Wait until FOSC<3:0> is being used

// You can enable the PLL and set the PLL multipler two ways:
// 1. At compile time - setting configuration fuses: CFGPLLEN and PLLSEL
//    ...but only if you use an External Clock (EC).
// 2. At run time - setting OSCCON2.PLLEN and OSCTUNE.SPLLMULT
//    ...but only if you use a High Speed crystal (HS), or
//       the High Frequency Internal Oscillator (HFINTOSC).
// This project uses the HFINTOSC set to run at 16 MHz and the only valid 
// PLL multiplier for USB is x3.
  PLLEN_bit = 0;          // OSCCON2.PLLEN = 0:    Disable the PLL, then...
  SPLLMULT_bit = 1;       // OSCTUNE.SPLLMULT = 1: Set the PLL multiplier to x3
  PLLEN_bit = 1;          // OSCCON2.PLLEN = 1:    Now re-enable the PLL
  while(!PLLRDY_bit);     // Wait for PLL to lock
  
  ACTEN_bit = 0;          // Disable Active Clock Tuning, then...
  ACTSRC_bit = 1;         // Tune HFINTOSC to match USB host clock
  ACTEN_bit = 1;          // Enable Active Clock Tuning
//  while (!ACTLOCK_bit);   // Wait until HFINTOSC is successfully tuned
  
  cFlags = 0;             // Reset all flags
  for (i=0; i < sizeof irCommand.b; i++) irCommand.b[i] = 0;
  usbCommand.uxyy = 0;

  // Set up USB
  UPUEN_bit = 1;          // USB On-chip pull-up enable
  FSEN_bit = 1;           // 1 = USB Full Speed enabled (requires 48 MHz USB clock)
                          //     IRK will work in Full Speed or Low Speed
                          // 0 = USB Full Speed disabled (requires 6 MHz USB clock)
                          //     IRK will work in Low Speed only

//----------------------------------------------------------------------------
// Set up Timer0 for LCD display backlight timeouts
//----------------------------------------------------------------------------

  T0CON   = 0b00000111;
//            x              0   = TNR0ON: Timer0 off
//             x             0   = T08BIT: Timer0 is in 16-bit mode
//              x            0   = T0CS:   Timer0 clock source is FOSC/4
//               x           0   = T0SE:   Timer0 source edge select (ignored)
//                x          0   = PSA:    Timer0 prescaler assigned
//                 xxx       111 = TOPS:   Timer0 prescaler (1:256)

// Timer0 tick rate at 24 MHz MCU clock = 24 MHz clock/4/256 = 23437.5 Hz
//       We can set a 1 second delay by storing 65535-23437=42098 in TMR0H:TMR0L
// Timer0 tick rate at 48 MHz MCU clock = 48 MHz clock/4/256 = 46875 Hz
//       We can set a 1 second delay by storing 65535-46875=18661 in TMR0H:TMR0L

//----------------------------------------------------------------------------
// Set up Timer1 for IR signal capture timings using CCP2 (permanently turned on)
//----------------------------------------------------------------------------

  T1CON   = 0b00110011;
//            xx             00 = TMR1CS: Timer1 clock source is instruction clock (Fosc/4)
//              xx           11 = TMR1PS: Timer1 prescale value is 1:8
//                x          0  = SOSCEN: Secondary Oscillator disabled
//                 x         0  = T1SYNC: Ignored because TMR1CS = 0x
//                  x        1  = RD16:   Enables register read/write of Timer1 in one 16-bit operation
//                   x       1  = TMR1ON: Timer1 is on

// Timer1 tick rate = 24 MHz clock/4/8 = 0.75 MHz (when FOSC is 24 Mhz)
//                    48 MHz clock/4/8 = 1.50 MHz (when FOSC is 48 Mhz)

//----------------------------------------------------------------------------
// Set up Timer2 for IR signal transmission (PWM)
//----------------------------------------------------------------------------

// ...this is done via library calls, so it is not configured here

//----------------------------------------------------------------------------
// Set up Timer3 for key repeats (turned on when a front panel key is long-pressed)
//----------------------------------------------------------------------------

  T3CON   = 0b00110010;
//            xx             00 = TMR3CS: Timer3 clock source is instruction clock (Fosc/4)
//              xx           11 = TMR3PS: Timer3 prescale value is 1:8
//                x          0  = SOSCEN: Secondary Oscillator disabled
//                 x         0  = T3SYNC: Ignored because TMR3CS = 0x
//                  x        1  = RD16:   Enables register read/write of Timer3 in one 16-bit operation
//                   x       0  = TMR3ON: Timer3 is off

//----------------------------------------------------------------------------
// Let the interrupts begin
//----------------------------------------------------------------------------

  PEIE_bit = 1;           // Enable peripheral interrupts
  GIE_bit = 1;            // Enable global interrupts

//----------------------------------------------------------------------------
// Set up Pulse Width Modulation (to transmit IR output signals)
//----------------------------------------------------------------------------

  PWM1_Init(IR_MODULATION_FREQ);
  PWM1_Set_Duty(DUTY_CYCLE);  // 64/256 = 1:4, 128/256 = 1:2

//----------------------------------------------------------------------------
// Set up LCD display
//----------------------------------------------------------------------------

  Lcd_Init();                         // Initialize LCD
  defineCustomCharacters();
  Lcd_Cmd(_LCD_CLEAR);                // Clear display
  Lcd_Cmd(_LCD_CURSOR_OFF);           // Cursor off
  Lcd_Out(1,1,_TEXT("IRK! v" IRK_VERSION));

  if (((PORTB & 0b11110111) ^ 0b11110111))     // If any button is pressed (low)
  {  // ...then (probably) the ICSP programmer is still connected
     // which is the same as pressing and holding a button or two
    Lcd_Out(2,1,_TEXT("Release buttons!" ));
    while (((PORTB & 0b11110111) ^ 0b11110111));
  }

  TMR3IE_bit = 1;         // Enable key repeat timer interrupts

//----------------------------------------------------------------------------
// Retrieve this device's configuration from EEPROM
//----------------------------------------------------------------------------

  nConfigDeviceAddress = EEPROM_Read(0);    // This IRK! device's IR address
  loadBacklightDelay();

//----------------------------------------------------------------------------
// Set up capture mode (to receive IR input signals)
//----------------------------------------------------------------------------

  enableInfraredCapture();
}

void interpretInfraredCommand(void)
{
  if ((irCommand.s.nAddress   != nConfigDeviceAddress) &&                 // Address byte matches this device..
      (irCommand.s.nAddress   != BROADCAST_ADDRESS))              return; // ...or is a broadcast?
  if (!(irCommand.s.nAddress   ^ irCommand.s.nAddressInverted))   return; // Address byte valid?
  if (!(irCommand.s.nModifiers ^ irCommand.s.nModifiersInverted)) return; // Modifier byte valid?
  if (!(irCommand.s.nCommand   ^ irCommand.s.nCommandInverted))   return; // Key byte valid?
                            // The infrared command is now valid, so...
  usbCommand.uxyy = irCommand.s.nModifiers << 8 | irCommand.s.nCommand;   // Build USB command from incoming IR command
  if (bDebugMode)
    showDebugInfo();
  else
    updateLCD();            // Display it on the LCD display
  executeCommand();         // Send it via USB to the host
}

void appendBit(void)
{
  nBit++;
  if (nBit > 7)
  {
    nBit = 0;
    irCommand.b[nByte] = cByte;
    nByte++;
    if (nByte >= sizeof irCommand.b)
    {
      nState = STATE_IR_COMMAND_RECEIVED;
    }
  }
}

#define SMALLEST(x) MICROSECONDS(((x) - WIDTH_ERROR_MARGIN))
#define LARGEST(x)  MICROSECONDS(((x) + WIDTH_ERROR_MARGIN))
#define IS_PULSE_WIDTH_NEAR(x) ((nPulseWidth.n > SMALLEST(x))  &  (nPulseWidth.n < LARGEST(x)))
                              
void processInfraredInterrupt(void)
{
  switch (nState)
  {
    case STATE_IR_RESET:
      if (bRisingEdge & IS_PULSE_WIDTH_NEAR(WIDTH_TRAINING_PULSE))
      {
        nState = STATE_IR_TRAINING_RECEIVED;
      }
      else
      {
        gotoResetState();
      }
      break;
    case STATE_IR_TRAINING_RECEIVED:
      if (bFallingEdge & IS_PULSE_WIDTH_NEAR(WIDTH_SILENCE_AFTER_TRAINING))
      {
        nState = STATE_IR_RECEIVING_BITS;
      }
      else
      {
        gotoResetState();
      }
      break;
    case STATE_IR_RECEIVING_BITS:
      if (bRisingEdge)  // All rising edges must be after a short burst
      {
        if (nPulseWidth.n > LARGEST(WIDTH_SHORT))
        {
          gotoResetState();
        }
        else
        {
        }
      }
      else /* Falling edge (after either a short or long silence) */
      {
        if (nPulseWidth.n > LARGEST(WIDTH_LONG))
        {
          gotoResetState();        // Too long, so it is not a 1 bit
        }
        else if (nPulseWidth.n > SMALLEST(WIDTH_LONG))
        {
          cByte <<= 1;             // Long enough for a 1 bit
          cByte |= 1;
          appendBit();             // Also goes to STATE_IR_COMMAND_RECEIVED
                                   // if enough bits have been received
        }
        else if (nPulseWidth.n > SMALLEST(WIDTH_SHORT))
        {
          cByte <<= 1;             // Short enough for a 0 bit
          appendBit();             // Also goes to STATE_IR_COMMAND_RECEIVED
                                   // if enough bits have been received
        }
        else
        {
          gotoResetState();       // Too short, so it is not a 0 bit
        }
      }
      break;
    case STATE_IR_COMMAND_RECEIVED:
      disableInfraredCapture();   // Disable infrared input while we...
      interpretInfraredCommand(); // ...process the command just received
      enableInfraredCapture();    // ...and go to STATE_IR_RESET
    default:
      break;
  }
}

void interrupt()            // High priority interrupt service routine
{
  USB_Interrupt_Proc();     // Always give the USB module first opportunity to process
  // The next most important interrupt is from the Infrared receiver...
  if (CCP2IF_bit)           // If capture event (rise/fall) on the CCP2 pin
  {
    nPulseWidth.byte[1] = CCPR2H; // Remember the elapsed time since last event
    nPulseWidth.byte[0] = CCPR2L;
    nRiseOrFall = CCP2CON;  // Save the rise or fall detection mode
    CCP2M0_bit ^= 1;        // Toggle rise or fall detection
//  LATA6_bit = CCP2M0_bit; // Debug CCP2 by putting a logic analyzer on RA6
    TMR1H = 0;              // Set high-byte of 16-bit time
    TMR1L = 0;              // Set low-byte and write all 16 bits to Timer1
    bInfraredInterruptPending = 1; // Indicate capture event detected
    CCP2IF_bit = 0;         // Allow the next CCP2 interrupt to occur
  }
  // Technically any or all of these interrupts can be asserted simultaneously,
  // but to ensure quick exit from the interrupt handler we only process
  // the most important and let interrupt() be driven again for any interrupts
  // that remain pending. That is why "else if" is used...
  else if (TMR3IF_bit)      // If it's a Timer3 interrupt
  {                         // 22.89 ticks/sec @48 MHz, 11.44 ticks/sec @24 MHz
    bKeyRepeatPending = TRUE;  // Indicate Timer3 rollover
    nKeyRepeatDelay--;      // Decrement delay before key repeat action starts
    TMR3IF_bit = 0;         // Clear the Timer3 interrupt flag
  }
  else if (TMR0IF_bit)      // If backlight timeout interrupt
  {
    nBacklightDelay--;      // Decrement seconds remaining with backlight on
    if (nBacklightDelay == 0)
    {
      LCD_BACKLIGHT = 1;    // Turn backlight off (0=On, 1=Off)
      TMR0ON_bit = 0;       // Turn off Timer0
    }
    TMR0H = HiByte(ONE_SECOND);  // Set high-byte of 16-bit count
    TMR0L = LoByte(ONE_SECOND);  // Set low-byte and write all 16 bits to Timer0
    TMR0IF_bit = 0;         // Clear the Timer0 interrupt flag
  }
}


void handleOKButton(void)
{
  if (bSettingUsage)
  {
    while (OK_BUTTON_PRESSED);  // Wait for user to release button
    bSettingUsage = FALSE;
  }
  else if ((usbCommand.s.ux.byte & 0xF0) == USAGE_LOCAL_IRK_FUNCTION)
  {
    while (OK_BUTTON_PRESSED);  // Wait for user to release button
    switch (usbCommand.s.yy)
    {
      case CMD_SET_DEVICE_ADDRESS:  // If user is setting this device's address
        bSettingDeviceAddress = !bSettingDeviceAddress; // Toggle between entering/exiting "set address" mode
        if (bSettingDeviceAddress)  // Entering address selection mode
        {
        }
        else // Exiting from address selection, so save the selected address in EEPROM
        {
          EEPROM_Write(0, nConfigDeviceAddress);
        }
        break;
      case CMD_SET_BACKLIGHT_DELAY: // If user is setting the LCD backlight delay time
        bSettingBacklightDelay  = !bSettingBacklightDelay; // Toggle between entering/exiting "set backlight delay" mode
        if (bSettingBacklightDelay) // Entering backlight delay selection mode
        {
          nNewBacklightDelay = nConfigBacklightDelay;
        }
        else // Exiting from backlight delay selection, so save the selected delay in EEPROM
        {
          nConfigBacklightDelay = nNewBacklightDelay;
          saveBacklightDelay();
        }
        break;
      case CMD_SET_BACKLIGHT_OFF:   // User wants backlight always OFF
        nConfigBacklightDelay = 0x00;
        saveBacklightDelay();
        break;
      case CMD_SET_BACKLIGHT_ON:    // User wants backlight always ON
        nConfigBacklightDelay = 0xFF;
        saveBacklightDelay();
        break;
      default:
        executeCommand();
        break;
    }
  }
  else                              // Send the USB function to the host
  {
    executeCommand();
    while (OK_BUTTON_PRESSED)
    { // Note: key repeat is normally a USB host function but IRK! is different
      if (bKeyRepeatPending && nKeyRepeatDelay <= 0)
      {
        executeCommand();
        bKeyRepeatPending = FALSE;
        nKeyRepeatDelay = 0;
      }
    }
  }
}

void adjustUsage(signed short nDelta)
{ // The usage nybble is the u in ux
  byte nUsage;
  nUsage = ((usbCommand.s.ux.byte >> 4) + nDelta) & 0x0F;
  usbCommand.s.ux.byte = (usbCommand.s.ux.byte & 0x0F) | (nUsage << 4);
}

void adjustDeviceAddress(signed short nDelta)
{
  nConfigDeviceAddress += nDelta;
}

void adjustBacklightDelay(signed short nDelta)
{
  nNewBacklightDelay += nDelta;
}

void adjustValueBy(signed short nDelta, void (*adjustValue)(signed short), byte (*isButtonPressed)())
{
  adjustValue(nDelta);
  updateLCD();
  while (isButtonPressed())
  {
    if (bKeyRepeatPending && nKeyRepeatDelay <= 0)
    {
      adjustValue(nDelta);
      updateLCD();
      bKeyRepeatPending = FALSE;
      nKeyRepeatDelay = 0;
    }
  }
}

void adjustCommandBy(signed short nDelta)
{
  usbCommand.uxyy = (usbCommand.uxyy & 0xF000) | ((usbCommand.uxyy & 0x0FFF) + nDelta) & 0x0FFF;
}

void adjustValueOfCommand(signed short nDelta, byte (*isButtonPressed)())
{
  adjustCommandBy(nDelta);
  updateLCD();
  nTicksPerKeyRepeat = FRONT_PANEL_KEY_REPEAT_TICKS;
  while (isButtonPressed())
  {
    if (bKeyRepeatPending && nKeyRepeatDelay <= 0)
    {
      if (--nTicksPerKeyRepeat == 0)
      {
        adjustCommandBy(nDelta);
        while ((*getDesc() == 0) // Skip over keys with no description...
            && ((usbCommand.s.yy & 0x0F) != 0x00)) // ...but don't skip functions ending in 0
        {
          adjustCommandBy(nDelta);
        }
        updateLCD();
        nTicksPerKeyRepeat = FRONT_PANEL_KEY_REPEAT_TICKS;
      }
      bKeyRepeatPending = FALSE;
      nKeyRepeatDelay = 0;
    }
  }
}

void adjustBy(signed short nDelta, byte (*isButtonPressed)())
{
  if (bSettingDeviceAddress)
    adjustValueBy(nDelta, &adjustDeviceAddress,  isButtonPressed);
  else if (bSettingBacklightDelay)
    adjustValueBy(nDelta, &adjustBacklightDelay, isButtonPressed);
  else if (bSettingUsage)
    adjustValueBy(nDelta, &adjustUsage,          isButtonPressed);
  else
    adjustValueOfCommand(nDelta, isButtonPressed);
}

void handleShiftButton(void)
{
  usbCommand.s.ux.bits.LeftShift ^= 1;     // Toggle the SHIFT key modifier
  while (SHIFT_BUTTON_PRESSED)
  {
    if (bKeyRepeatPending && nKeyRepeatDelay <= 0)
    {
      if (!bSettingUsage) // Do this only when beginning to set usage
      {
        usbCommand.uxyy &= 0xF000;    // Keep current u, clear xyy
        bSettingUsage = TRUE;
        updateLCD();
      }
      bKeyRepeatPending = FALSE;
      nKeyRepeatDelay = 0;
    }
  }
  updateLCD();
}

void handleCtlButton(void)
{
  usbCommand.s.ux.bits.LeftControl ^= 1;   // Toggle the CTL key modifier
  while (CTL_BUTTON_PRESSED)
  {
    if (bKeyRepeatPending && nKeyRepeatDelay <= 0)
    {
      usbCommand.s.ux.bits.LeftControl ^= 1;   // Revert the CTL key modifier
      usbCommand.s.ux.bits.LeftGUI ^= 1;   // Toggle the GUI key modifier
      updateLCD();
      while (CTL_BUTTON_PRESSED); // Make it a one-shot change
      bKeyRepeatPending = FALSE;
      nKeyRepeatDelay = 0;
    }
  }
  updateLCD();
}

byte isUpButtonPressed()
{
  return UP_BUTTON_PRESSED;
}

byte isDownButtonPressed()
{
  return DOWN_BUTTON_PRESSED;
}

void main()
{
  Prolog();

  while (FOREVER)
  {
    if (bInfraredInterruptPending)
    {
      processInfraredInterrupt();
      bInfraredInterruptPending = 0;
    }
    if (ACTIVITY_LED)
    {
      if (--nActivityLEDDelay == 0)
      {
        ACTIVITY_LED = OFF;
      }
    }
    if (((PORTB & 0b11110111) ^ 0b11110111))     // If any button is pressed (low)
    {
      enableBacklight();          // Conditionally turn on LCD backlight
      Delay_ms(25);               // Debounce delay
      nKeyRepeatDelay = KEY_REPEAT_DELAY_IN_TICKS;  // Number of Timer3 interrupts before starting key repeat
      TMR3H = 0;                  // Prime the Timer3 high byte
      TMR3L = 0;                  // Now clear the Timer3 counter
      TMR3ON_bit = ON;            // Turn on the key repeat timer
      if (!bSettingUsage && !bSettingDeviceAddress && !bSettingBacklightDelay)
      {
        if (TEACH_BUTTON_PRESSED)   // Transmit the current key via infrared
        {
          transmitInfraredCommand();
          while (TEACH_BUTTON_PRESSED); // Wait for button to be released
        }
        if (CTL_BUTTON_PRESSED) handleCtlButton();
        if (ALT_BUTTON_PRESSED)         // Toggle the ALT key modifier
        {
          usbCommand.s.ux.bits.LeftAlt ^= 1;
          while (ALT_BUTTON_PRESSED);   // Wait for button to be released
        }
        if (SHIFT_BUTTON_PRESSED) handleShiftButton();
      }
      if (OK_BUTTON_PRESSED)    handleOKButton();
      if (UP_BUTTON_PRESSED)    adjustBy(+1, &isUpButtonPressed);
      if (DOWN_BUTTON_PRESSED)  adjustBy(-1, &isDownButtonPressed);
      TMR3ON_bit = OFF;                 // Turn off the key repeat timer
      bKeyRepeatPending = FALSE;
      updateLCD();                      // Show final key state
    }
    if (bLastUSBPower ^ USB_POWER_GOOD) // If USB power state has changed
    {
      if (USB_POWER_GOOD)               // If USB is now powered
        enableUSB();                    // Then enable USB interface
      else                              // Else USB is now unpowered
        disableUSB();                   // So disable USB interface
      bLastUSBPower = USB_POWER_GOOD;   // Remember the current USB power state
    }
  }
}