#pragma once

/////////////////////////////////////////////////////////////////////////////
// Definitions
/////////////////////////////////////////////////////////////////////////////

#define USE_SUSPEND_VALUES_MASK			0x8000

/////////////////////////////////////////////////////////////////////////////
// Report Lengths
/////////////////////////////////////////////////////////////////////////////

#define FEATURE_REPORT_LENGTH			64
#define INTERRUPT_REPORT_LENGTH			64

/////////////////////////////////////////////////////////////////////////////
// Report ID Definitions
/////////////////////////////////////////////////////////////////////////////

#define RESET_DEVICE			        0x40 // Reset Device
#define GETSET_UART_ENABLE		        0x41 // Get Set Receive Status
#define GET_UART_STATUS			        0x42 // Get UART Status
#define PURGE_FIFOS				        0x43 // Purge FIFOs
#define GET_GPIO_VALUES			        0x44 // Get GPIO Values
#define SET_GPIO_VALUES			        0x45 // Set GPIO Values
#define GET_VER_INFO			        0x46 // Get Version Information
#define GETSET_LOCK_BYTE		        0x47 // Get Set Lock Byte

#define GETSET_UART_CONFIG		        0x50 // Get Set UART Config
#define TRANS_LINE_BREAK		        0x51 // Transmit Line Break
#define STOP_LINE_BREAK			        0x52 // Stop Line Break

#define GETSET_USB_CONFIG		        0x60 // All customizable data that's not strings
#define GETSET_MANU1			        0x61 // Get Set Manufacturing String 1
#define GETSET_MANU2			        0x62 // Get Set Manufacturing String 2
#define GETSET_PROD1			        0x63 // Get Set Product String 1
#define GETSET_PROD2			        0x64 // Get Set Product String 2
#define GETSET_SERSTR			        0x65 // Get Set Serial String
#define GETSET_PIN_CONFIG		        0x66 // GPIO configuration

/////////////////////////////////////////////////////////////////////////////
// Bit Mask Definitions
/////////////////////////////////////////////////////////////////////////////

#define PURGE_TRANSMIT_MASK				0x01
#define PURGE_RECEIVE_MASK				0x02

/////////////////////////////////////////////////////////////////////////////
// String Buffer Definitions
/////////////////////////////////////////////////////////////////////////////

#define SHORT_STRING_LEN				60

#define LONG_STRING_LEN_1				61
#define LONG_STRING_LEN_2				63
