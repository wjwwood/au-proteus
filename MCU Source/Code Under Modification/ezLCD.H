#ifndef ezLCD_H
#define ezLCD_H 1

#define RDRF 0x20   // Receive Data Register Full Bit
#define TDRE 0x80   // Transmit Data Register Empty Bit
#define SP   0x20



// filename ************** ezLCD.H *****************************
// Busy-wait I/O routines to DP512 serial port 1,  
// interfaced to ezLCD-002 used to display information locally
// Jonathan W. Valvano 7/5/08

// Copyright 2008 by Jonathan W. Valvano, valvano@mail.utexas.edu 
//    You may use, edit, run or distribute this file 
//    as long as the above copyright notice remains 
/*  240 rows by 160 columns 8-bit color LCD
  0,0 ------------------239,0
  0,1 |................|239,1
      |................|
      |................|
      |................|
0,158 |................|239,158
0,159 ------------------239,159
*/

// Input: color = b1 b0 g2 g1 g0 r2 r1 r0
#define GREEN  0x38
#define RED    0x07
#define BLUE   0xC0
#define NAVY   0x80
#define WHITE  0xFF
#define BLACK  0x00
#define VIOLET 0xC4
#define PURPLE 0xC4
#define YELLOW 0x3F
#define LIGHTYELLOW 0x24

//-------------------------ezLCD_Init------------------------
// Initialize ezLCD-002, clear screen
// Input: none
// Output: none
// assumes a module clock frequency of 24 MHz
// Busy-wait synchronization, 115200 bits/sec using SCI1
void ezLCD_Init(void);

//-------------------------ezLCD_InTouch------------------------
// Wait for new input from ezLCD touchpad, busy-waiting synchronization
// The input is not echoed
// Input: none
// Output: key typed
// ezLCD-002 Touch Screen Organization
//       0   1   2   3   4   5   6   7
//    -----------------------------------
//  0 | $00 $01 $02 $03 $04 $05 $06 $07 |
//  1 | $10 $11 $12 $13 $14 $15 $16 $17 |
//  2 | $20 $21 $22 $23 $24 $25 $26 $27 |
//  3 | $30 $31 $32 $33 $34 $35 $36 $37 |
//  4 | $40 $41 $42 $43 $44 $45 $46 $47 |
//  5 | $50 $51 $52 $53 $54 $55 $56 $57 |
//    -----------------------------------
// let y be the row number 0,1,2,3,4,5
// let x be the column number 0-7
// key = 16*y+x  on pen down
// key = $FF     on pen up
unsigned char ezLCD_InTouch(void);

//---------------------ezLCD_InTouchS--------------------------------
//same as ezLCD_InTouch except surpresses pen up output
unsigned char ezLCD_InTouchS();
   
//-------------------------ezLCD_TouchStatus--------------------------
// Checks if new touch screen input is ready, TRUE if new input is ready
// Input: none
// Output: TRUE if a call to ezLCD_InTouch will return right away with data
//         FALSE if a call to ezLCD_InTouch will wait for input
char ezLCD_TouchStatus(void);


//-------------------------ezLCD_Arc------------------------
// Draw an arc in Current Color, with the center at Current Position,
// Input: radius is the size of the arc
//        begin is the starting angle of the arc
//        end is the stopping angle of the arc
// Output: none
// Angle Coding: The angle range is from 0 to 255.
// To transform degrees to ARC angle units:
// Angle_lcd = Angle_deg x 32 / 45
// For example:
//  32  = 45°
//  64  = 90°
//  128 = 180°
//  192 = 270°
//  0   = 0° = 360°
// The angle is drawn clockwise with the zero positioned at the top of a screen
void ezLCD_Arc(unsigned char radius,unsigned char begin, unsigned char end);

//-------------------------ezLCD_Box------------------------
// Draw a rectangle from the current position to x2,y2
// Input: x2,y2 a position
// Output: none
// One corner is current position, other corner is x2,y2
// Drawn with Current Color
void ezLCD_Box(unsigned char x2,unsigned char y2);


//-------------------------ezLCD_BoxFill------------------------
// Draw a filled rectangle from the current position to x2,y2 with Current Color
// Input: x2,y2 a position
// Output: none
// One corner is current position, other corner is x2,y2
// Filled with Current Color
void ezLCD_BoxFill(unsigned char x2,unsigned char y2);

//-------------------------ezLCD_Circle------------------------
// Draw a circle with the center at Current Position,
// Input: radius is the size of the circle
// Output: none
// Drawn with Current Color
void ezLCD_Circle(unsigned char radius);

//-------------------------ezLCD_CircleFill------------------------
// Draw a filled circle with the center at Current Position,
// Input: radius is the size of the circle
// Output: none
// filled with Current Color
void ezLCD_CircleFill(unsigned char radius);

//-------------------------ezLCD_Clear------------------------
// Clears LCD screen
// Input: none
// Output: none
void ezLCD_Clear(void);

//-------------------------ezLCD_HLine------------------------
// Draw a horizontal line from the current position to x2
// Input: x2 a position
// Output: none
// One end of the line is current position (x,y), other end is (x2,y)
// Drawn with Current Color
void ezLCD_HLine(unsigned char x2);


//-------------------------ezLCD_Light------------------------
// Turns on/off the LCD light
// Input: bOn,true for on, false for off
// Output: none
void ezLCD_Light(unsigned short bOn);

//-------------------------ezLCD_Line------------------------
// Draw a line from the current position to x2,y2
// Input: x2,y2 a position
// Output: none
// One end of the line is current position, other end is x2,y2
// Drawn with Current Color
void ezLCD_Line(unsigned char x2,unsigned char y2);


//-------------------------ezLCD_Dot0------------------------
// Draw a dot at the current position 
// Input: none
// Output: none
// Drawn with Current Color
void ezLCD_Dot0(void);


//-------------------------ezLCD_Dot------------------------
// Draw a dot at x2,y2
// Input: x2,y2 a position
// Output: none
// One end of the line is current position, other end is x2,y2
// Drawn with Current Color
void ezLCD_Dot(unsigned char x2,unsigned char y2);


//-------------------------ezLCD_OutChar------------------------
// Draw a character at the current position 
// Input: ASCII character
// Output: none
// Drawn with Current Color
void ezLCD_OutChar(char letter);

//-------------------------ezLCD_OutCharBack------------------------
// Draw a character at the current position 
// Input: ASCII character
// Output: none
// Drawn with background color
void ezLCD_OutCharBack(char letter);

//------------------------ezLCD_OutUDec-----------------------------
// Draw 16-bit unsigned decimal at the current position
// Input: Unsigned Short
// Output: none
// Drawn with current color
void ezLCE_OutUDec(unsigned short dec);

//-------------------------ezLCD_OutString------------------------
// Draw a character at the current position 
// Input: pointer to a null-terminated ASCII string
// Output: none
// Drawn with Current Color
void ezLCD_OutString(char *pt);

//-------------------------ezLCD_OutStringBack------------------------
// Draw a character at the current position 
// Input: pointer to a null-terminated ASCII string
// Output: none
// Drawn with background color
void ezLCD_OutStringBack(char *pt);

//-------------------------ezLCD_BitMap------------------------
// Puts Bitmap on the screen starting at Current Position 
// Input: width is the number of pixels in the x direction
//        height is the number of pixels in the y direction
//        pt is a pointer to buffer of width*height colors
// Output: none
// Current position determines the lower left corner of the BitMap
//     which is draw up and to the right of the current position
// The bit map is stored row major, bottom row first, top row last
//      the pixels are stored left to right
// For example, let the current position be (50,70), width=4, height=3
// byte#  plotted at location
//  0     (50,70)
//  1     (51,70)
//  2     (52,70)
//  3     (53,70)
//  4     (50,69)
//  5     (51,69)
//  6     (52,69)
//  7     (53,69)
//  8     (50,68)
//  9     (51,68)
// 10     (52,68)
// 11     (53,68)
void ezLCD_BitMap(unsigned char width, unsigned char height, unsigned char *pt);

//-------------------------ezLCD_Icon------------------------
// Puts ICON with it's upper-left corner positioned at the Current Position 
// Input: Icon number  
// Output: none
// Use the ezLCDrom utility to store icons in the ezLCD ROM
void ezLCD_Icon(unsigned char number);


//-------------------------ezLCD_SFIcon------------------------
// Puts serial flash ICON with it's upper-left corner positioned at the Current Position 
// Input: Icon number (Icon ID: 0 to 254) 
// Output: none
// Use the ezLCDflash utility to store icons in the ezLCD Serial Flash
void ezLCD_SFIcon(unsigned char number);

//-------------------------ezLCD_Font------------------------
// Sets the Current Font 
// Input: font number (0 to 5), 0-2 are small, 3-5 are large 
// Output: none
// Use the ezLCDflash utility to change fonts
// 0 is 8 by 8 fixed size
// 1 is 14-point Arial
// 2 is 14-point bold Arial
// 3 is 36-point Times New Roman
// 4 is 26-point Forte
// 5 is 29-point Script MT Bold
void ezLCD_Font(unsigned char font);

//-------------------------ezLCD_BackColor------------------------
// Sets the Background Color 
// Input: color = b1 b0 g2 g1 g0 r2 r1 r0
// Output: none
// used in the commands ezLCD_OutCharBack, ezLCD_OutStringBack
void ezLCD_BackColor(unsigned char color);

//-------------------------ezLCD_Color------------------------
// Sets the main drawing Color 
// Input: color = b1 b0 g2 g1 g0 r2 r1 r0
// Output: none
// used in many commands 
void ezLCD_Color(unsigned char color);


//-------------------------ezLCD_XY------------------------
// Sets the Current Position at x,y
// Input: x,y a position
// Output: none
// used in many commands 
//  0,0 ------------------239,0
//  0,1 |................|239,1
//      |................|
//      |......(x,y).....|
//      |................|
//0,158 |................|239,158
//0,159 ------------------239,159
void ezLCD_XY(unsigned char x,unsigned char y);

//-------------------------ezLCD_North------------------------
// Sets the orientation of the text so top of text faces up
// Input: none
// Output: none
// used for text output commands
void ezLCD_North(void);
//-------------------------ezLCD_East------------------------
// Sets the orientation of the text so top of text faces right
// Input: none
// Output: none
// used for text output commands
void ezLCD_East(void);
//-------------------------ezLCD_South------------------------
// Sets the orientation of the text so top of text faces down
// Input: none
// Output: none
// used for text output commands
void ezLCD_South(void);
//-------------------------ezLCD_West------------------------
// Sets the orientation of the text so top of text faces left
// Input: none
// Output: none
// used for text output commands
void ezLCD_West(void);

//-------------------------ezLCD_VLine------------------------
// Draw a vertial line from the current position to y2
// Input: y2 a position
// Output: none
// One end of the line is current position (x,y), other end is (x,y2)
// Drawn with Current Color
void ezLCD_VLine(unsigned char y2);

//-----------------------ezLCD_OutDigit-----------------------
// Output one decimal digit as an ASCII character
// Input: unsigned number 0 to 9 
// Output: none
void ezLCD_OutDigit(unsigned short digit);

//-----------------------ezLCD_OutDecimalPoint-----------------
// Output decimal point as an ASCII character
// Input: none 
// Output: none
void ezLCD_OutDecimalPoint(void);

//-----------------------ezLCD_OutSpace-----------------
// Output space as an ASCII character
// Input: none 
// Output: none
void ezLCD_OutSpace(void);

//----------------------ezLCD_NewLine-------------------------
// Output a new line and start it left justified on the screen
// input: none
// output: none
// unsigned short int line_count initialized in ezLCD_Init
void ezLCD_NewLine(void);

//------------------------ezLCD_OutDec-----------------------------
// Draw 16-bit unsigned decimal at the current position
// Input: Unsigned Short
// Output: none
// Drawn with current color
// Description: Print an unsigned int at current position with the current color
// recursive and calls ezLCD_OutChar 
void ezLCD_OutDec(unsigned short dec);

//-----------------------ezLCD_OutSDec-----------------------
// Output a 16-bit number in signed decimal format
// Input: 16-bit signed number 
// Output: none
// fixed size 6 digits of output, right justified 
void ezLCD_OutSDec(short n);

#endif //ezLCD_H