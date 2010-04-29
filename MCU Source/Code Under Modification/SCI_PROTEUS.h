#ifndef _SCI_FIFO_H
#define _SCI_FIFO_H 1
// filename **************SCIA_FIFO.H  *****************************
// Interrupting I/O routines to 9S12DP512 serial port 
// Jonathan W. Valvano 8/1/08

//  This example accompanies the books
//   "Embedded Microcomputer Systems: Real Time Interfacing",
//        Thompson, copyright (c) 2006,
//   "Introduction to Embedded Microcomputer Systems: 
//    Motorola 6811 and 6812 Simulation", Brooks-Cole, copyright (c) 2002

// Copyright 2008 by Jonathan W. Valvano, valvano@mail.utexas.edu 
//    You may use, edit, run or distribute this file 
//    as long as the above copyright notice remains 
// Modified by EE345L students Charlie Gough && Matt Hawk
// Modified by EE345M students Agustinus Darmawan + Mingjie Qiu

// standard ASCII symbols 
#define CR   0x0D
#define LF   0x0A
#define BS   0x08
#define ESC  0x1B
#define SP   0x20       
#define DEL  0x7F
#define TAB  0x09
// 0 means do not collect histograms of FIFO size, 1 for FIFO analysis
#define HISTOGRAM 0

#define ACTIVE 1
#define INACTIVE 0

#define SCI_X86 0
 
//-------------------------SCI_Init------------------------
// Initialize Serial port SCI
// Input: baudRate is the baud rate in bits/sec
// Output: none
// SCIBDL=1500000/baudRate, for example
// baudRate =    9600 bits/sec  SCIBDL=156
// baudRate =   19200 bits/sec  SCIBDL=78
// baudRate =   38400 bits/sec  SCIBDL=39
// baudRate =  115200 bits/sec  SCIBDL=13
// assumes a module clock frequency of 24 MHz
// baud rate must be faster than 5900 bits/sec
void SCI_Init(unsigned char port, unsigned long baudRate);
 
//-------------------------SCI_InStatus--------------------------
// Checks if new input is ready, TRUE if new input is ready
// Input: none
// Output: TRUE if a call to InChar will return right away with data
//         FALSE if a call to InChar will wait for input
char SCI_InStatus(unsigned char port);  

//-------------------------SCI_InChar------------------------
// Wait for new serial port input, interrupt synchronization
// The input is not echoed
// Input: none
// Output: ASCII code for key typed
char SCI_InChar(unsigned char port);  
 
//------------------------SCI_InString------------------------
// Accepts ASCII characters from the serial port
//    and adds them to a string until <enter> is typed 
//    or until max length of the string is reached.  
// It echoes each character as it is inputted.  
// If a backspace is inputted, the string is modified 
//    and the backspace is echoed
// terminates the string with a null character
// uses interrupt synchronization on RDRF
// Input: pointer to empty buffer, size of buffer
// Output: Null terminated string
void SCI_InString(unsigned char port, char *bufPt, unsigned short max); 

//----------------------SCI_InUDec-------------------------------
// InUDec accepts ASCII input in unsigned decimal format
//     and converts to a 16 bit unsigned number
//     range is 0 to 65535
// Input: none
// Output: 16-bit unsigned number
// If you enter a number above 65535, it will truncate without an error
// Backspace will remove last digit typed
unsigned short SCI_InUDec(unsigned char port);

//---------------------SCI_InUHex----------------------------------------
// Accepts ASCII input in unsigned hexadecimal (base 16) format
// Input: none
// Output: 16-bit unsigned number
// No '$' or '0x' need be entered, just the 1 to 4 hex digits
// It will convert lower case a-f to uppercase A-F
//     and converts to a 16 bit unsigned number
//     value range is 0 to FFFF
// If you enter a number above FFFF, it will truncate without an error
// Backspace will remove last digit typed
unsigned short SCI_InUHex(unsigned char port);   

//-----------------------SCI_OutStatus----------------------------
// Checks if output data buffer is empty, TRUE if empty
// Input: none
// Output: TRUE if a call to OutChar will output and return right away
//         FALSE if a call to OutChar will wait for output to be ready
char SCI_OutStatus(unsigned char port);   

//-------------------------SCI_OutChar------------------------
// Wait for buffer to be empty, output 8-bit to serial port
// interrupt synchronization
// Input: 8-bit data to be transferred
// Output: none
void SCI_OutChar(unsigned char port, char);
   
//-----------------------SCI_OutUDec-----------------------
// Output a 16-bit number in unsigned decimal format
// Input: 16-bit number to be transferred
// Output: none
// Variable format 1-5 digits with no space before or after
void SCI_OutUDec(unsigned char port, unsigned short); 

//-----------------------SCI_OutUDec32-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: SCI port, 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void SCI_OutUDec32(unsigned char port, unsigned long n);

//-----------------------SCI_OutSDec-----------------------
// Output a 16-bit number in signed decimal format
// Input: 16-bit signed number to be transferred
// Output: none
// Variable format 2-6 digits with no space before or after
void SCI_OutSDec(unsigned char port, short n);

//-------------------------SCI_OutString------------------------
// Output String (NULL termination), interrupt synchronization
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void SCI_OutString(unsigned char port, char *pt);
 
//--------------------------SCI_OutUHex----------------------------
// Output a 16 bit number in unsigned hexadecimal format
// Input: 16-bit number to be transferred
// Output: none
// Variable format 1 to 4 digits with no space before or after
void SCI_OutUHex(unsigned char port, unsigned short);

//---------------------OutCRLF---------------------
// Output a CR,LF to SCI to go to a new line
// Input: none
// Output: none
// toggle PortT bit 0 each time, debugging profile
void SCI_OutCRLF(unsigned char port);

//-----------------------SCI_OutFix1-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.1 
// Input: 16-bit unsigned number 
// Output: none
// fixed size 5 digits of output, right justified
// if input is 12345, then display is 1234.5 
void SCI_OutFix1(unsigned char port, unsigned short n);

//-----------------------SCI_OutFix2-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.01 
// Input: 16-bit unsigned number 
// Output: none
// fixed size 5 digits of output, right justified
// if input is 12345, then display is 123.45 
void SCI_OutFix2(unsigned char port, unsigned short n);

//-----------------------SCI_OutFix3-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.001 
// Input: 16-bit unsigned number 
// Output: none
// fixed size is 6 characters of output, right justified
// if input is 12345, then display is 12.345 
void SCI_OutFix3(unsigned char port, unsigned short n);

//-----------------------SCI_SOutFix2-----------------------
// Output a 16-bit number in signed decimal fixed-point
// with resolution = 0.01 
// Input: 16-bit signed number  -9999 to 9999
// Output: none
// fixed size, 6 characters, 4 digits of output, right justified
// if input is -1234, then display is -12.34 
void SCI_SOutFix2(unsigned char port, short num);

#endif /* _SCI_FIFO_H */