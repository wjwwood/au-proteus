// filename ************** ezLCD.C *****************************
// Busy-wait I/O routines to DP512 serial port 1,  
// interfaced to ezLCD-002 used to display information locally
// Jonathan W. Valvano 7/5/08

// Copyright 2008 by Jonathan W. Valvano, valvano@mail.utexas.edu 
//    You may use, edit, run or distribute this file 
//    as long as the above copyright notice remains 


#include <mc9s12dp512.h>     /* derivative information */
#include "ezLCD.h"     


//-------------------------SCI0Init------------------------
// Initialize Serial port SCI, connected to ezLCD-002
// Input: baudRate is tha baud rate in bits/sec
// Output: none
// assumes a module clock frequency of 24 MHz
void SCI1Init(unsigned long baudRate){
  SCI1BDH = 0;   // SCIBDL = MCLK/(16*BaudRate) 
  SCI1BDL= (unsigned char)(1500000/baudRate); 
  SCI1CR1 = 0;
/* bit value meaning
    7   0    LOOPS, no looping, normal
    6   0    WOMS, normal high/low outputs
    5   0    RSRC, not appliable with LOOPS=0
    4   0    M, 1 start, 8 data, 1 stop
    3   0    WAKE, wake by idle (not applicable)
    2   0    ILT, short idle time (not applicable)
    1   0    PE, no parity
    0   0    PT, parity type (not applicable with PE=0) */ 
  SCI1CR2 = 0x0C; 
/* bit value meaning
    7   0    TIE, no transmit interrupts on TDRE
    6   0    TCIE, no transmit interrupts on TC
    5   0    RIE, no receive interrupts on RDRF
    4   0    ILIE, no interrupts on idle
    3   1    TE, enable transmitter
    2   1    RE, enable receiver
    1   0    RWU, no receiver wakeup
    0   0    SBK, no send break */ 
}


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

//decimal values of upper left corner of key
//y values are approx due to fraction
//pen up = 255

//        0   30  60  90  120 150 180  210
//      -----------------------------------
//  0   | 0   1   2   3    4   5   6   7  |
//  27  | 16  17  18  19   20  21  22  23 |
//  53  | 32  33  34  35   36  37  38  39 |
//  81  | 48  49  50  51   52  53  54  55 |
//  108 | 64  65  66  67   68  69  70  71 |
//  135 | 80  81  82  83   84  85  86  87 |
//      -----------------------------------





// let y be the row number 0,1,2,3,4,5
// let x be the column number 0-7
// key = 16*y+x  on pen down
// key = $FF     on pen up
unsigned char ezLCD_InTouch(void){
  while((SCI1SR1 & RDRF) == 0){};
  return(SCI1DRL);
}

//---------------------ezLCD_InTouchS--------------------------------
//same as ezLCD_InTouch except surpresses pen up output
unsigned char ezLCD_InTouchS(){
  while((SCI1SR1 & RDRF) == 0 || (SCI1DRL > 87)){};
  return(SCI1DRL);
}

        
//-------------------------OutByte------------------------
// Wait for buffer to be empty, output 8-bit to ezLCD-002
// busy-waiting synchronization
// Input: 8-bit data to be transferred
// Output: none
void OutByte(unsigned char data){
  while((SCI1SR1 & TDRE) == 0){};
  SCI1DRL = data;
}

   
//-------------------------ezLCD_TouchStatus--------------------------
// Checks if new youch screen input is ready, TRUE if new input is ready
// Input: none
// Output: TRUE if a call to ezLCD_InTouch will return right away with data
//         FALSE if a call to ezLCD_InTouch will wait for input
char ezLCD_TouchStatus(void){
  return(SCI1SR1 & RDRF);
}


//-----------------------OutStatus----------------------------
// Checks if output data buffer is empty, TRUE if empty
// Input: none
// Output: TRUE if a call to OutChar will output and return right away
//         FALSE if a call to OutChar will wait for output to be ready
char OutStatus(void){
  return(SCI1SR1 & TDRE);
}


//-------------------------OutString------------------------
// Output String (NULL termination), interrupt synchronization
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void OutString(char *pt){
  while(*pt){
    OutByte(*pt);
    pt++;
  }
}


//-------------------------ezLCD_Init------------------------
// Initialize ezLCD-002, clear screen
// Input: none
// Output: none
// assumes a module clock frequency of 24 MHz
// Busy-wait synchronization, 115200 bits/sec using SCI01
void ezLCD_Init(void){
  SCI1Init(115200);
  ezLCD_Clear();  // clear screen
}


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
void ezLCD_Arc(unsigned char radius,unsigned char begin, unsigned char end){
/* ARC
Description: Draws an Arc in Current Color, with the center at Current Position,
starting on Begin Angle and ending on the End Angle.
Class: Multi Byte Command
Code: 2Fhex, 47dec, / ASCII
See Also: SET_XY, SET_COLOR, CIRCLE_R
Angle Coding: The angle range is from 0 to 255.
To transform degrees to ARC angle units:
Angle_lcd = Angle_deg x 32 / 45
For example:
  32  = 45°
  64  = 90°
  128 = 180°
  192 = 270°
  0   = 0° = 360°
The angle is drawn clockwise with the zero positioned at the top of a screen */
  OutByte(47);       // Byte 0 ARC (Command)
  OutByte(radius);   // Byte 1     (Radius)
  OutByte(begin);    // Byte 2     (Arc Begin Angle)
  OutByte(end);      // Byte 3     (Arc End Angle)
}


//-------------------------ezLCD_Box------------------------
// Draw a rectangle from the current position to x2,y2
// Input: x2,y2 a position
// Output: none
// One corner is current position, other corner is x2,y2
// Drawn with Current Color
void ezLCD_Box(unsigned char x2,unsigned char y2){
/* BOX
Description: Draws a rectangle.
Class: Multi Byte Command
Code: 42hex, 66dec, B ASCII
One corner is current position, other corner is X2,Y2
  0,0 ------------------239,0
  0,1 |................|239,1
      |................|
      |................|
      |................|
0,158 |................|239,158
0,159 ------------------239,159
See Also: SET_XY, BOX_FILL */
  OutByte(66);    // Byte 0 BOX (Command)
  OutByte(x2);    // Byte 1 X2 (Corner Column)
  OutByte(y2);    // Byte 2 Y2 (Corner Row)
}


//-------------------------ezLCD_BoxFill------------------------
// Draw a filled rectangle from the current position to x2,y2 with Current Color
// Input: x2,y2 a position
// Output: none
// One corner is current position, other corner is x2,y2
// Filled with Current Color
void ezLCD_BoxFill(unsigned char x2,unsigned char y2){
/* BOX_FILL
Description: Draws a rectangle filled with Current Color
Class: Multi Byte Command
Code: 43hex, 67dec, C ASCII
One corner is current position, other corner is X2,Y2

  0,0 ------------------239,0
  0,1 |................|239,1
      |................|
      |................|
      |................|
0,158 |................|239,158
0,159 ------------------239,159
See Also: SET_XY, BOX */
  OutByte(67);    // Byte 0 BOX_FILL (Command)
  OutByte(x2);    // Byte 1 X2 (Corner Column)
  OutByte(y2);    // Byte 2 Y2 (Corner Row)
}


//-------------------------ezLCD_Circle------------------------
// Draw a circle with the center at Current Position,
// Input: radius is the size of the circle
// Output: none
// Drawn with Current Color
void ezLCD_Circle(unsigned char radius){
/* CIRCLE_R
Description: Draws a circle in Current Color at Current Position
Class: Double Byte Command
Code: 29hex, 41dec, ) ASCII
See Also: SET_XY, SET_COLOR */
  OutByte(41);      // Byte 0 CIRCLE_R (Command)
  OutByte(radius);  // Byte 1 radius
}


//-------------------------ezLCD_CircleFill------------------------
// Draw a filled circle with the center at Current Position,
// Input: radius is the size of the circle
// Output: none
// filled with Current Color
void ezLCD_CircleFill(unsigned char radius){
/* CIRCLE_R_FILL
Description: Draws a circle in Current Color at Current Position, filled with Current Color
Class: Double Byte Command
Code: 39hex, 57dec, 9 ASCII
See Also: SET_XY, SET_COLOR */
  OutByte(57);     // Byte 0 CIRCLE_R_FILL (Command)
  OutByte(radius); // Byte 1 (Radius)
}


//-------------------------ezLCD_Clear------------------------
// Clears LCD screen
// Input: none
// Output: none
void ezLCD_Clear(void){
/* CLS
Description: Clears screen
Class: Single Byte
Code: 21hex, 33dec,
See Also: SET_COLOR */
  OutByte(33);  // Byte 0 CLS (Command)
}


//-------------------------ezLCD_HLine------------------------
// Draw a horizontal line from the current position to x2
// Input: x2 a position
// Output: none
// One end of the line is current position (x,y), other end is (x2,y)
// Drawn with Current Color
void ezLCD_HLine(unsigned char x2){
/* H_LINE
Description: Fast draws a horizontal line from Current Position
to the column specified by the parameter.
Class: Double Byte Command
Code: 40hex, 64dec, @ ASCII
Note: The screen size is 240x160. However, the valid
X range is 0 - 255
  0,0 ------------------239,0
  0,1 |................|239,1
      |................|
      |................|
      |................|
0,158 |................|239,158
0,159 ------------------239,159
See Also: V_LINE, SET_XY  */
  OutByte(64);    // Byte 0 H_LINE (Command)
  OutByte(x2);    // Byte 1 x7 x6 x5 x4 x3 x2 x1 x0 ( x2 )
}


//-------------------------ezLCD_Light------------------------
// Turns on/off the LCD light
// Input: bOn,true for on, false for off
// Output: none
void ezLCD_Light(unsigned short bOn){
/* LIGHT_ON
Description: Turns on
Class: Single Byte
Code: 22hex, 34dec,
See Also: LIGHT_OFF */

/* LIGHT_OFF
Description: Turns off the screen light
Class: Single Byte Command
Code: 23hex, 35dec, # ASCII
Byte 0 LIGHT_OFF (Command)
See Also: LIGHT_ON  */
  if(bOn){
    OutByte(34);    // Byte 0 LIGHT_ON (Command)
  } else{
    OutByte(35);    // Byte 0 LIGHT_OFF (Command)
  }
}


//-------------------------ezLCD_Line------------------------
// Draw a line from the current position to x2,y2
// Input: x2,y2 a position
// Output: none
// One end of the line is current position, other end is x2,y2
// Drawn with Current Color
void ezLCD_Line(unsigned char x2,unsigned char y2){
/* LINE_TO_XY
Description: Draws a line in Current Color, 
from the Current Position to the to specified position
Class: Multi Byte Command
Code: 28hex, 40dec, ( ASCII
Note: The screen size is 240x160. However, the valid
x and y ranges are 0 - 255
  0,0 ------------------239,0
  0,1 |................|239,1
      |................|
      |................|
      |................|
0,158 |................|239,158
0,159 ------------------239,159
See Also: SET_XY, SET_COLOR, PLOT */
  OutByte(40);    // Byte 0 LINE_TO_XY (Command)
  OutByte(x2);    // Byte 1 x7 x6 x5 x4 x3 x2 x1 x0 (x2)
  OutByte(y2);    // Byte 2 y7 y6 y5 y4 y3 y2 y1 y0 (y2)
}


/* PICTURE
Description: Puts a bitmap picture over the entire screen
Class: Multi Byte Command
Code: 2Ahex, 42dec, * ASCII
Byte 0 PICTURE (Command)
Byte     1 b1 b0 g2 g1 g0 r2 r1 r0 (x=0, y=159)
Byte     2 b1 b0 g2 g1 g0 r2 r1 r0 (x=1, y=159)
Byte     3 b1 b0 g2 g1 g0 r2 r1 r0 (x=2,  y=159)
Byte   240 b1 b0 g2 g1 g0 r2 r1 r0 (x=239, y=159)
Byte   241 b1 b0 g2 g1 g0 r2 r1 r0 (x=0,   y=158)
Byte 38399 b1 b0 g2 g1 g0 r2 r1 r0 (x=238, y=0)
Byte 38400 b1 b0 g2 g1 g0 r2 r1 r0 (x=239, y=0)
See Also: SET_XY, SET_COLOR, PUT_BITMAP */


//-------------------------ezLCD_Dot0------------------------
// Draw a dot at the current position 
// Input: none
// Output: none
// Drawn with Current Color
void ezLCD_Dot0(void){
/* PLOT
Description: Plots a point at Current Position in Current Color
Class: Single Byte Command
Code: 26hex, 38dec, & ASCII
  0,0 ------------------239,0
  0,1 |................|239,1
      |................|
      |................|
      |................|
0,158 |................|239,158
0,159 ------------------239,159
See Also: SET_XY, SET_COLOR */
  OutByte(38);    // Byte 0 PLOT (Command)
}


//-------------------------ezLCD_Dot------------------------
// Draw a dot at x2,y2
// Input: x2,y2 a position
// Output: none
// One end of the line is current position, other end is x2,y2
// Drawn with Current Color
void ezLCD_Dot(unsigned char x2,unsigned char y2){
/* PLOT_XY
Description: Plots a point in Current Color, at specified position.
Class: Multi Byte Command
Code: 27hex, 39dec, ' ASCII
  0,0 ------------------239,0
  0,1 |................|239,1
      |................|
      |................|
      |................|
0,158 |................|239,158
0,159 ------------------239,159
Note: The screen size is 240x160. However, the valid
x and y ranges are 0 - 255 */
  OutByte(39);    // Byte 0 PLOT_XY (Command)
  OutByte(x2);    // Byte 1 x7 x6 x5 x4 x3 x2 x1 x0 (x2)
  OutByte(y2);    // Byte 2 y7 y6 y5 y4 y3 y2 y1 y0 (y2)
}


//-------------------------ezLCD_OutChar------------------------
// Draw a character at the current position 
// Input: ASCII character
// Output: none
// Drawn with Current Color
void ezLCD_OutChar(char letter){
/* PRINT_CHAR
Description: Prints a character at Current Position
Class: Double Byte Command
Code: 2Chex, 44dec, , ASCII
See Also: SELECT_FONT, PRINT_STRING */
  OutByte(44);     // Byte 0 PRINT_CHAR (Command)
  OutByte(letter); // Byte 1 ASCII
}


//-------------------------ezLCD_OutCharBack------------------------
// Draw a character at the current position 
// Input: ASCII character
// Output: none
// Drawn with background color
void ezLCD_OutCharBack(char letter){
/* PRINT_CHAR_BG
Description: Prints a character at Current Position on the background
specified by SET_BG_COLOR command
Class: Double Byte Command
Code: 3Chex, 60dec, < ASCII
See Also: SELECT_FONT, SET_BG_COLOR, PRINT_STRING_BG */
  OutByte(60);     // Byte 0 PRINT_CHAR_BG (Command)
  OutByte(letter); // Byte 1 ASCII
}


//-------------------------ezLCD_OutString------------------------
// Draw a character at the current position 
// Input: pointer to a null-terminated ASCII string
// Output: none
// Drawn with Current Color
void ezLCD_OutString(char *pt){
/* PRINT_STRING
Description: Prints null-terminated String
starting at Current Position
Class: Multi Byte Command
Code: 2Dhex, 45dec, - ASCII
Byte 0 PRINT_STRING (Command)
Byte 1 ASCII (First Character)
Byte 2 ASCII (Second Character)
Byte n ASCII(Last Character)
Byte n+1 0 (NULL)
See Also: SELECT_FONT, PRINT_CHAR */
  OutByte(45);     // Byte 0 PRINT_STRING (Command)
  OutString(pt);   // Bytes 1 to n ASCII string
  OutByte(0);      // Byte n+1 0 (NULL)
}


//-------------------------ezLCD_OutStringBack------------------------
// Draw a character at the current position 
// Input: pointer to a null-terminated ASCII string
// Output: none
// Drawn with background color
void ezLCD_OutStringBack(char *pt){
/* PRINT_STRING_BG
Description: Prints null-terminated String starting at Current Position
on the background specified by SET_BG_COLOR command
Class: Multi Byte Command
Code: 3Dhex, 61dec, = ASCII
Byte 0 PRINT_STRING_BG (Command)
Byte 1 ASCII (First Character)
Byte 2 ASCII (Second Character)
Byte n ASCII(Last Character)
Byte n+1 0 (NULL)
See Also: SELECT_FONT, SET_BG_COLOR, PRINT_CHAR_BG */
  OutByte(61);     // Byte 0 PRINT_STRING_BG (Command)
  OutString(pt);   // Bytes 1 to n ASCII string
  OutByte(0);      // Byte n+1 0 (NULL)
}


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
void ezLCD_BitMap(unsigned char width, unsigned char height, unsigned char *pt){
unsigned short i;
unsigned short count = width*height;
/* PUT_BITMAP
Description: Puts Bitmap on the screen starting at Current Position, then UP and RIGHT
Class: Multi Byte Command
Code: 2Ehex, 46dec, . ASCII
Byte 0 PUT_BITMAP (Command)
Byte 1 width (Bitmap Width)
Byte 2 height (Bitmap Height)
Byte 3 (pixel at: X, Y)
Byte 4 (pixel at: X+1, Y)
Byte width+2 (pixel at: X+width-1, Y)
Byte width+3 (pixel at: X, Y-1)
Byte height*width+2 (pixel at: X+width-1, Y-height+1)
Note: The total number of bytes is: width*height+3
See Also: SET_XY, SET_COLOR, PICTURE */
  OutByte(46);     // Byte 0 PUT_BITMAP (Command)
  OutByte(width);  // Byte 1 width (Bitmap Width)
  OutByte(height); // Byte 2 height (Bitmap Height)
  for(i=count; i>0; i--){
    OutByte(*pt);  // pixel value (color)
    pt++;
  }
}


//-------------------------ezLCD_Icon------------------------
// Puts ICON with it's upper-left corner positioned at the Current Position 
// Input: Icon number  
// Output: none
// Use the ezLCDrom utility to store icons in the ezLCD ROM
void ezLCD_Icon(unsigned char number){
/* PUT_ICON
Description: Displays the icon 
with it's upper-left corner positioned at the Current Position.
The icon is read from the ezLCD ROM.
Use the ezLCDrom utility to store icons in the ezLCD ROM
Class: Double Byte Command
Code: 57hex, 87dec, W ASCII
See Also: SET_XY */
  OutByte(87);      // Byte 0 PUT_ICON (Command)
  OutByte(number);  // Byte 1 Icon ID (Icon ID)
}


//-------------------------ezLCD_SFIcon------------------------
// Puts serial flash ICON with it's upper-left corner positioned at the Current Position 
// Input: Icon number (Icon ID: 0 to 254) 
// Output: none
// Use the ezLCDflash utility to store icons in the ezLCD Serial Flash
void ezLCD_SFIcon(unsigned char number){
/* PUT_SF_ICON
Description: Displays the serial flash icon 
with it's upper-left corner positioned at the Current Position.
The icon is read from the ezLCD-002 1Mbyte Serial Flash.
Use the ezLCDflash utility to store icons in the ezLCD Serial Flash
Class: Double Byte Command
Code: 58hex, 88dec, X ASCII
Byte 0 PUT_SF_ICON (Command)
Byte 1 Icon ID (Icon ID: 0 to 254)
See Also: SET_XY */
  OutByte(88);      // Byte 0 PUT_SF_ICON (Command)
  OutByte(number);  // Byte 1 Icon ID (Icon ID)
}


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
void ezLCD_Font(unsigned char font){
/* SELECT_FONT
Description: Sets the Current Font
Class: Double Byte Command
Code: 2Bhex, 43dec, + ASCII
Note: The following fonts are implemented
:     Small Fonts 0,1,2
      Large Fonts 3,4.5
See Also: PRINT_STRING, PRINT_CHAR */
  OutByte(43);      // Byte 0 SELECT_FONT (Command)
  OutByte(font);    // Byte 1 font number (font number)
}


//-------------------------ezLCD_BackColor------------------------
// Sets the Background Color 
// Input: color = b1 b0 g2 g1 g0 r2 r1 r0
// Output: none
// used in the commands ezLCD_OutCharBack, ezLCD_OutStringBack
void ezLCD_BackColor(unsigned char color){
/* SET_BG_COLOR
Description: Sets the Background Color for the following instructions:
   PRINT_CHAR_BG
   PRINT_STRING_BG
Class: Double Byte Command
Code: 34hex, 52dec, 4 ASCII
Note: The default NATURAL palette has the following color coding:
b1 b0 g2 g1 g0 r2 r1 r0
See Also: PRINT_CHAR_BG, PRINT_STRING_BG, PALETTE */
  OutByte(52);      // Byte 0 SET_BG_COLOR (Command)
  OutByte(color);   // Byte 1 color (Color Code)
}


//-------------------------ezLCD_Color------------------------
// Sets the main drawing Color 
// Input: color = b1 b0 g2 g1 g0 r2 r1 r0
// Output: none
// used in many commands 
void ezLCD_Color(unsigned char color){
/* SET_COLOR
Description: Sets the Current Color
Class: Double Byte Command
Code: 24hex, 36dec, $ ASCII
Note: The default NATURAL palette has the following color coding:
b1 b0 g2 g1 g0 r2 r1 r0
7 6 5 4 3 2 1 0
See Also: CLS, PLOT, PALETTE */
  OutByte(36);      // Byte 0 SET_COLOR (Command)
  OutByte(color);   // Byte 1 color (Color Code)
}


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
void ezLCD_XY(unsigned char x,unsigned char y){
/* SET_XY
Description: Sets the Current Position
Class: Multi Byte Command
Code: 25hex, 37dec, % ASCII
Byte 0 SET_XY (Command)
Byte 1 x7 x6 x5 x4 x3 x2 x1 x0 (x)
Byte 2 y7 y6 y5 y4 y3 y2 y1 y0 (y)
  0,0 ------------------239,0
  0,1 |................|239,1
      |................|
      |................|
      |................|
0,158 |................|239,158
0,159 ------------------239,159
Note: The screen size is 240x160. However, the valid
x and y ranges are 0 - 255 */
  OutByte(37);   // Byte 0 PLOT_XY (Command)
  OutByte(x);    // Byte 1 x7 x6 x5 x4 x3 x2 x1 x0 (x)
  OutByte(y);    // Byte 2 y7 y6 y5 y4 y3 y2 y1 y0 (y)
}


//-------------------------ezLCD_North------------------------
// Sets the orientation of the text so top of text faces up
// Input: none
// Output: none
// used for text output commands
void ezLCD_North(void){
  OutByte(96);   // Byte 0 TEXT_NORTH (Command)
}
//-------------------------ezLCD_East------------------------
// Sets the orientation of the text so top of text faces right
// Input: none
// Output: none
// used for text output commands
void ezLCD_East(void){
  OutByte(97);   // Byte 0 TEXT_EAST (Command)
}
//-------------------------ezLCD_South------------------------
// Sets the orientation of the text so top of text faces down
// Input: none
// Output: none
// used for text output commands
void ezLCD_South(void){
  OutByte(98);   // Byte 0 TEXT_SOUTH (Command)
}
//-------------------------ezLCD_West------------------------
// Sets the orientation of the text so top of text faces left
// Input: none
// Output: none
// used for text output commands
void ezLCD_West(void){
  OutByte(99);   // Byte 0 TEXT_WEST (Command)
}

/* Description: Set the orientation of the text, 
Class: Single Byte Commands
Codes: 
TEXT_NORTH: 60hex, 96dec, ' ASCII top of the text is facing north (top)
TEXT_EAST : 61hex, 97dec, a ASCII top of the text is facing east (right)
TEXT_SOUTH: 62hex, 98dec, b ASCII top of the text is facing south (bottom)
TEXT_WEST : 2Fhex, 99dec, c ASCII top of the text is facing west (left)
Byte 0 TEXT_NORTH(Command)
Byte 0 TEXT_EAST(Command)
Byte 0 TEXT_SOUTH (Command)
Byte 0 TEXT_WEST(Command)
Note: TEXT_NORTH is the default text orientation */

//-------------------------ezLCD_VLine------------------------
// Draw a vertial line from the current position to y2
// Input: y2 a position
// Output: none
// One end of the line is current position (x,y), other end is (x,y2)
// Drawn with Current Color
void ezLCD_VLine(unsigned char y2){
/* V_LINE
Description: Fast draws a vertical line from Current Position,
to the row specified by the parameter.
Class: Double Byte Command
Code: 41hex, 65dec, A ASCII
  0,0 ------------------239,0
  0,1 |................|239,1
      |................|
      |................|
      |................|
0,158 |................|239,158
0,159 ------------------239,159
Note: The screen size is 240x160. However, the valid
Y range is 0 - 255 */
  OutByte(65);    // Byte 0 V_LINE (Command)
  OutByte(y2);    // Byte 1 y7 y6 y5 y4 y3 y2 y1 y0 ( y2 )
}


//*******************High level graphics**********************
unsigned char PlotPixel; 
// number of data points drawn into the same X-axis pixel
// 4 means it takes 400 calls to LCD_Plot to draw one sweep
unsigned short SubCount;  // goes PlotPixel down to 1
unsigned char Yline[100]; // 100 color points for current line
unsigned char BackColor;  // black color of plot
unsigned char Xcolumn;    // goes from 40 to 239 
unsigned short XaxisResolution;
// X axis numbers are  0 to 999
// resolution>3 means no numbers are displayed
// number->       0    9    99   999
// 0  shown as   0.    9.   99.  999.
// 1  shown as  0.0   0.9   9.9  99.9
// 2  shown as 0.00  0.09  0.99  9.99
// 3  shown as .000  .009  .099  .999

unsigned short YaxisResolution;
// Y axis numbers are  -99 to 99
// any other value means no numbers are displayed
// 0  shown as -99     -1    0   1   99
// 1  shown as -9.9  -0.1  0.0 0.1  9.9
// 2  shown as -.99  -.01  .00 .01  .99
short MinY,RangeY;        // used to scale input data


//-----------------------ezLCD_OutDigit-----------------------
// Output one decimal digit as an ASCII character
// Input: unsigned number 0 to 9 
// Output: none
void ezLCD_OutDigit(unsigned short digit){
  ezLCD_OutChar((unsigned char)(digit+'0'));    // convert to ASCII, output
}


//-----------------------ezLCD_OutDecimalPoint-----------------
// Output decimal point as an ASCII character
// Input: none 
// Output: none
void ezLCD_OutDecimalPoint(void){
  ezLCD_OutChar('.');    // decimal point output
}


//-----------------------ezLCD_OutSpace-----------------
// Output space as an ASCII character
// Input: none 
// Output: none
void ezLCD_OutSpace(void){                             
  ezLCD_OutChar(SP);    // space output
}


//----------------------ezLCD_NewLine-------------------------
// Output a new line and start it left justified on the screen
// input: none
// output: none
// unsigned short int line_count initialized in ezLCD_Init
// for text and numbers, 8 lines on th ezLCD works best
unsigned short line_count = 0;
void ezLCD_NewLine() {
  line_count += 8;
  ezLCD_OutChar(10);  
  if(line_count > 152){
    line_count = 0;
    ezLCD_Color(WHITE);
    ezLCD_Clear();
    ezLCD_Color(BLACK);
  }
  ezLCD_XY(0, line_count);
}


 //----------------------ezLCD_Top-----------------
 // make ezLCD_NewLine restart from top of screen without clearing
 //  sets global var line_count used by ezLCD_NewLine to 0
 //
 
 
 void ezLCD_Top(){
  line_count = 0;
 }

//------------------------ezLCD_OutDec-----------------------------
// Draw 16-bit unsigned decimal at the current position with the current color
// recursive and calls ezLCD_OutChar 
// Input: Unsigned Short
// Output: none
// Drawn with current color
void ezLCD_OutDec(unsigned short dec){
  if(dec >= 10) 
  {
    ezLCD_OutDec(dec/10);
    dec = dec%10;
  }
  ezLCD_OutDigit(dec);  
}











//-----------------------ezLCD_OutSDec-----------------------
// Output a 16-bit number in signed decimal format
// Input: 16-bit signed number 
// Output: none
// fixed size 6 digits of output, right justified 
void ezLCD_OutSDec(short n){  
unsigned char sign=' ';
  if(n < 0){
    n = -n;       // negative
    sign = '-';
  }
  if(n < 10){
    ezLCD_OutString("    ");
    ezLCD_OutChar(sign);
    ezLCD_OutDigit(n);         // n is between 0 and 9 
  } else if(n<100){
    ezLCD_OutString("   ");
    ezLCD_OutChar(sign);
    ezLCD_OutDigit(n/10);      // tens digit 
    ezLCD_OutDigit(n%10);      // ones digit 
  } else if(n<1000){
      ezLCD_OutString("  ");
      ezLCD_OutChar(sign);
      ezLCD_OutDigit(n/100);   // hundreds digit 
      n = n%100;
      ezLCD_OutDigit(n/10);    // tens digit 
      ezLCD_OutDigit(n%10);    // ones digit 
    }
    else if(n<10000){
      ezLCD_OutSpace();  
      ezLCD_OutChar(sign);
      ezLCD_OutDigit(n/1000);  // thousands digit 
      n = n%1000;
      ezLCD_OutDigit(n/100);   // hundreds digit 
      n = n%100;
      ezLCD_OutDigit(n/10);    // tens digit 
      ezLCD_OutDigit(n%10);    // ones digit 
    }
    else {
      ezLCD_OutChar(sign);
      ezLCD_OutDigit(n/10000); // ten-thousands digit 
      n = n%10000;
      ezLCD_OutDigit(n/1000);  // thousands digit 
      n = n%1000;
      ezLCD_OutDigit(n/100);   // hundreds digit 
      n = n%100;
      ezLCD_OutDigit(n/10);    // tens digit 
      ezLCD_OutDigit(n%10);    // ones digit 
    }
}



//-----------------------ezLCD_OutFix1-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.1 
// Input: 16-bit unsigned number 
// Output: none
// fixed size 5 digits of output, right justified
// if input is 12345, then display is 1234.5 
void ezLCD_OutFix1(unsigned short n){
  if(n < 10){
    ezLCD_OutString("   0.");
    OutDigit(n);         // n is between 0 and 9 
  } else if(n<100){
    ezLCD_OutString("   ");
    OutDigit(n/10);      // tens digit 
    OutDecimalPoint();   // decimal point 
    OutDigit(n%10);      // ones digit 
  } else if(n<1000){
      ezLCD_OutString("  ");
      OutDigit(n/100);   // hundreds digit 
      n = n%100;
      OutDigit(n/10);    // tens digit 
      OutDecimalPoint(); // decimal point 
      OutDigit(n%10);    // ones digit 
    }
    else if(n<10000){
      OutSpace();  
      OutDigit(n/1000);  // thousands digit 
      n = n%1000;
      OutDigit(n/100);   // hundreds digit 
      n = n%100;
      OutDigit(n/10);    // tens digit 
      OutDecimalPoint(); // decimal point 
      OutDigit(n%10);    // ones digit 
    }
    else {
      OutDigit(n/10000); // ten-thousands digit 
      n = n%10000;
      OutDigit(n/1000);  // thousands digit 
      n = n%1000;
      OutDigit(n/100);   // hundreds digit 
      n = n%100;
      OutDigit(n/10);    // tens digit 
      OutDecimalPoint(); // decimal point 
      OutDigit(n%10);    // ones digit 
    }
}
//-----------------------ezLCD_OutFix2-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.01 
// Input: 16-bit unsigned number 
// Output: none
// fixed size 5 digits of output, right justified
// if input is 12345, then display is 123.45 
void ezLCD_OutFix2(unsigned short n){
  if(n < 10){
    ezLCD_OutString("  0.0");
    OutDigit(n);         // n is between 0 and 9 
  } else if(n<100){
    ezLCD_OutString("  0.");
    OutDigit(n/10);      // tens digit 
    OutDigit(n%10);      // ones digit 
  } else if(n<1000){
      ezLCD_OutString("  ");
      OutDigit(n/100);   // hundreds digit 
      n = n%100;
      OutDecimalPoint(); // decimal point 
      OutDigit(n/10);    // tens digit 
      OutDigit(n%10);    // ones digit 
    }
    else if(n<10000){
      OutSpace();  
      OutDigit(n/1000);  // thousands digit 
      n = n%1000;
      OutDigit(n/100);   // hundreds digit 
      n = n%100;
      OutDecimalPoint(); // decimal point 
      OutDigit(n/10);    // tens digit 
      OutDigit(n%10);    // ones digit 
    }
    else {
      OutDigit(n/10000); // ten-thousands digit 
      n = n%10000;
      OutDigit(n/1000);  // thousands digit 
      n = n%1000;
      OutDigit(n/100);   // hundreds digit 
      n = n%100;
      OutDecimalPoint(); // decimal point 
      OutDigit(n/10);    // tens digit 
      OutDigit(n%10);    // ones digit 
    }
}


//-----------------------ezLCD_OutFix2b-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.01 
// Input: 16-bit unsigned number, 0 to 9999 
// Output: none
// fixed size is 5 characters of output, right justified
// if input is 1234, then display is 12.34 
void ezLCD_OutFix2b(unsigned short n){
    
  if(n < 10){
    ezLCD_OutString(" 0.0");
    OutDigit(n);       //n is between 0 and 9
  } else if(n<100){
    ezLCD_OutString(" 0.");
    OutDigit(n/10);    // tens digit
    OutDigit(n%10);    // ones digit
  } else if(n<1000){
    OutSpace();  
    OutDigit(n/100);   // hundreds digit
    n = n%100;
    OutDecimalPoint(); // decimal point 
    OutDigit(n/10);    // tens digit
    OutDigit(n%10);    // ones digit
  }
  else if(n<10000){
    OutDigit(n/1000);  // thousands digit
    n = n%1000;
    OutDigit(n/100);   // hundreds digit
    n = n%100;
    OutDecimalPoint(); // decimal point 
    OutDigit(n/10);    // tens digit
    OutDigit(n%10);    // ones digit
  }
  else {
    ezLCD_OutString("**.**");
  }
}
//-----------------------ezLCD_OutFix3-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.001 
// Input: 16-bit unsigned number 
// Output: none
// fixed size is 6 characters of output, right justified
// if input is 12345, then display is 12.345 
void ezLCD_OutFix3(unsigned short n){
  if(n < 10){
    ezLCD_OutString(" 0.00");
    OutDigit(n);         // n is between 0 and 9 
  } else if(n<100){
    ezLCD_OutString(" 0.0");
    OutDigit(n/10);      // tens digit 
    OutDigit(n%10);      // ones digit 
  } else if(n<1000){
      ezLCD_OutString(" 0.");
      OutDigit(n/100);   // hundreds digit 
      n = n%100;
      OutDigit(n/10);    // tens digit 
      OutDigit(n%10);    // ones digit 
    }
    else if(n<10000){
      OutSpace();  
      OutDigit(n/1000);  // thousands digit 
      OutDecimalPoint(); // decimal point 
      n = n%1000;
      OutDigit(n/100);   // hundreds digit 
      n = n%100;
      OutDigit(n/10);    // tens digit 
      OutDigit(n%10);    // ones digit 
    }
    else {
      OutDigit(n/10000); // ten-thousands digit 
      n = n%10000;
      OutDigit(n/1000);  // thousands digit 
      OutDecimalPoint(); // decimal point 
      n = n%1000;
      OutDigit(n/100);   // hundreds digit 
      n = n%100;
      OutDigit(n/10);    // tens digit 
      OutDigit(n%10);    // ones digit 
    }
}

//-----------------------ezLCD_OutFix4-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.0001 
// Input: 16-bit unsigned number 
// Output: none
// fixed size is 6 characters of output, right justified
// if input is 12345, then display is 1.2345 
void ezLCD_OutFix4(unsigned short n){
  if(n < 10){
    ezLCD_OutString("0.000");
    OutDigit(n);         // n is between 0 and 9 
  } else if(n<100){
    ezLCD_OutString("0.00");
    OutDigit(n/10);      // tens digit 
    OutDigit(n%10);      // ones digit 
  } else if(n<1000){
      ezLCD_OutString("0.0");
      OutDigit(n/100);   // hundreds digit 
      n = n%100;
      OutDigit(n/10);    // tens digit 
      OutDigit(n%10);    // ones digit 
    }
    else if(n<10000){
      ezLCD_OutString("0.");
      OutDigit(n/1000);  // thousands digit 
      n = n%1000;
      OutDigit(n/100);   // hundreds digit 
      n = n%100;
      OutDigit(n/10);    // tens digit 
      OutDigit(n%10);    // ones digit 
    }
    else {
      OutDigit(n/10000); // ten-thousands digit 
      n = n%10000;
      OutDecimalPoint(); // decimal point 
      OutDigit(n/1000);  // thousands digit 
      n = n%1000;                
      OutDigit(n/100);   // hundreds digit 
      n = n%100;
      OutDigit(n/10);    // tens digit 
      OutDigit(n%10);    // ones digit 
    }
}



//------lcdTinyOutFix------------------------
// display 4 character tiny number
// input: num 0 to 999
// output:none
// no error checking
void lcdTinyOutFix(unsigned short num){
  if(XaxisResolution == 0){      // shown as   0.    9.   99.  999.
    if(num < 10){
      OutSpace();  
      OutSpace();  
    } else if(num < 100){
      OutSpace();  
      OutDigit(num/10);          // tens digit 1 to 9
    } else{
      OutDigit(num/100);         // hundreds digit 1 to 9
      num = num%100;             // 0 to 99
      OutDigit(num/10);          // tens digit 1 to 9
    }
    OutDigit(num%10);            // ones digit 1 to 9
    OutDecimalPoint();           // decimal point 
  }else if(XaxisResolution == 1){// shown as  0.0   0.9   9.9  99.9
    if(num < 100){
      OutSpace();  
    } else{
      OutDigit(num/100);         // hundreds digit 1 to 9
      num = num%100;             // 0 to 99
    }
    OutDigit(num/10);            // tens digit 1 to 9
    OutDecimalPoint();           // decimal point 
    OutDigit(num%10);            // ones digit 1 to 9
  }else if(XaxisResolution == 2){//shown as 0.00  0.09  0.99  9.99 
    OutDigit(num/100);           // hundreds digit 1 to 9
     num = num%100;               // 0 to 99
    OutDecimalPoint();           // decimal point 
    OutDigit(num/10);            // tens digit 1 to 9
    OutDigit(num%10);            // ones digit 1 to 9
  }else if(XaxisResolution == 3){// shown as .000  .009  .099  .999
    OutDigit(num/100);           // hundreds digit 1 to 9
    OutDecimalPoint();           // decimal point 
     num = num%100;               // 0 to 99
    OutDigit(num/10);            // tens digit 1 to 9
    OutDigit(num%10);            // ones digit 1 to 9
  }
}

//-----------------------ezLCD_PlotXaxis-----------------------
// Draw X axis on ezLCD-002
// Input: min max, resolution, and label  
// Output: none
// X axis numbers min and max range from 0 to 999
// resolution>3 means no numbers are displayed
// number->       0    9    99   999
// 0  shown as   0.    9.   99.  999.
// 1  shown as  0.0   0.9   9.9  99.9
// 2  shown as 0.00  0.09  0.99  9.99
// 3  shown as .000  .009  .099  .999
// label has 13 character maximum size
// errors: min and max must be less than or equal to 999
void ezLCD_PlotXaxis(unsigned short min, unsigned short max,
  unsigned short resolution, char *label){
  unsigned short i; 
  char Xlabel[14];          // maximum of 13 characters
  if((min > max)||(max > 999)){
    return;  // bad parameters
  }
  ezLCD_Color(BLACK);
  ezLCD_XY(36,128);
  ezLCD_HLine(239);   // x axis
  ezLCD_VLine(131);   // max hash
  for(i=1; i<10; i++){
    ezLCD_XY(40+i*20,128);
    ezLCD_VLine(131); // hash
  }
  XaxisResolution = resolution;
  i = 0;
  do{
    Xlabel[i] = *label;
    label++; i++;
  }
  while(i<14);
  Xlabel[13] = 0;    // truncate to 13 characters maximum
  ezLCD_XY(60,140);  // place for X axis label
  ezLCD_Font(1);
  ezLCD_OutString(Xlabel);
  
  ezLCD_XY(35,136);  // place for X axis min number
  ezLCD_Font(0);
  lcdTinyOutFix(min);
  ezLCD_XY(210,136); // place for X axis max number
  lcdTinyOutFix(max);
}


//------lcdTinyOutFix2------------------------
// display 4 character tiny number
// input: num -99 to 99
// output:none
// no error checking
void lcdTinyOutFix2(short snum){ 
unsigned short num;
unsigned short bSign;
  if(snum<0){
    bSign = 1;
    num = -snum;
  } else{
    bSign = 0;
    num = snum;
  }
  if(YaxisResolution == 0){      // shown as  -99     -1    0   1   99
    if(num < 10){
      OutSpace();                // no tens digit
      if(bSign){
        ezLCD_OutChar('-');      // minus sign
      } else{
        ezLCD_OutChar('+');      // plus sign
      }
    } else{
      if(bSign){
        ezLCD_OutChar('-');      // minus sign
      } else{
        ezLCD_OutChar('+');      // plus sign
      }
      OutDigit(num/10);          // tens digit 1 to 9
    }
    OutDigit(num%10);            // ones digit 1 to 9
    OutDecimalPoint();
  }else if(YaxisResolution == 1){// shown as  -9.9  -0.1  0.0 0.1  9.9
    if(bSign){
      ezLCD_OutChar('-');        // minus sign
    } else{
      ezLCD_OutChar('+');        // plus sign
    }
    OutDigit(num/10);            // tens digit 1 to 9
    OutDecimalPoint();
    OutDigit(num%10);            // ones digit 1 to 9
  }else if(YaxisResolution == 2){// shown as -.99  -.01  .00 .01  .99 
    if(bSign){
      ezLCD_OutChar('-');        // minus sign
    } else{
      ezLCD_OutChar('+');        // plus sign
    }
    OutDecimalPoint();
    OutDigit(num/10);            // tens digit 1 to 9
    OutDigit(num%10);            // ones digit 1 to 9
  }
}


//-----------------------ezLCD_PlotYaxis-----------------------
// Draw Y axis on ezLCD-002 
// Input: min, center, max, resolution, label, and number of hash marks
//        min, center, max are the integer part ranging from -99 to +99
//        resolution is 0,1,2 where to put decimal point 
//                      any other value means no numbers are displayed
//  number->    -99    -1    0    1    99     
// 0  shown as -99.   -1.    0.   1.   99.
// 1  shown as -9.9  -0.1   0.0  0.1   9.9
// 2  shown as -.99  -.01   .00  .01   .99
//        label is an ASCII string, up to 8 characters
// Output: none
// errors: must be called once, before calling ezLCD_Plot
void ezLCD_PlotYaxis(short min, short center, short max,
  unsigned short resolution, char *label){
  char Ylabel[10];           // maximum of 8 characters
  unsigned short i;          // index into Ylabel
  if((min > max)||(center < min)||(center > max)){
    return;  // bad parameters
  }
  if((min < -99)||(max > 99)){
    return;  // bad parameters
  }
   i = 0;
  do{
    Ylabel[i] = *label;
    label++; i++;
  }
  while(i<9);
  Ylabel[8] = 0;      // truncate to 8 characters maximum
  ezLCD_Color(BLACK);
  ezLCD_XY(39,125);
  ezLCD_VLine(28);    // y axis
  ezLCD_HLine(36);    // max hash
  ezLCD_XY(39,53);
  ezLCD_HLine(36);    // 3/4 hash
  ezLCD_XY(39,78);
  ezLCD_HLine(36);    // center hash
  ezLCD_XY(39,103); 
  ezLCD_HLine(36);    // 1/4 hash
  ezLCD_XY(120,140);  // place for X axis label
  ezLCD_Color(BLACK);
  ezLCD_Font(1);
  ezLCD_West();
  ezLCD_OutString(Ylabel);
  ezLCD_North();
  YaxisResolution = resolution;

  ezLCD_Font(0);
  ezLCD_XY(32,40);   // place for max number
  lcdTinyOutFix2(max);
  ezLCD_XY(32,80);   // place for center number
  lcdTinyOutFix2(center);
  ezLCD_XY(32,120);  // place for min number
  lcdTinyOutFix2(min);
  ezLCD_North();     // default direction

}

//-----------------------ClearYLine-----------------------
// Clear the YLine to the background color of the drawing window
// Input:  none
// Output: none
void ClearYLine(void){
unsigned short i;
  for(i=0; i<100; i++){
    Yline[i] = BackColor; // 100 color points for current line
  }
  SubCount = PlotPixel;
}

//-----------------------ezLCD_PlotClear-----------------------
// Clear plot window on ezLCD-002 
// Input: min max, specify range of Y axis  
//        plotPixel number of data points drawn into the same X-axis pixel
//           4 means it takes 800 calls to ezLCD_Plot to draw one sweep
//           plotPixel can range from 1 to 100
//        color is the background color of the drawing window
// Output: none
// errors: must be called once, before calling ezLCD_Plot
void ezLCD_PlotClear(short min, short max, unsigned char plotPixel, unsigned char color){
  if((plotPixel<1)||(plotPixel>100)) plotPixel=1;
  PlotPixel = plotPixel;
  BackColor = color;     // background color of plot
  ezLCD_Color(BackColor);
  ezLCD_XY(40,127);      // one corner
  ezLCD_BoxFill(239,28); // other corner
  ClearYLine();
  RangeY = max-min;
  MinY = min;
  Xcolumn = 40;
}

//-----------------------ezLCD_Plot-----------------------
// Plot one data point
// Input: data between min and max, color to drawn point  
// Output: none
// errors: must call LCD_PlotClear first
void ezLCD_Plot(short data,unsigned char color){
unsigned short pixel;
  if(data<MinY){
    data = MinY;   // forces (data-MinY) to be positive
  }
//  data = 100*(data-MinY);
//  pixel = data/RangeY;  // 0 to 99, truncate down
  asm ldd  data
  asm subd MinY    // must be positive
  asm ldy  #100
  asm emul         // 32-bit Y:D is 100*(data-MinY)
  asm ldx  RangeY
  asm ediv         // 16-bit Y is 100*(data-MinY)/RangeY
  asm sty  pixel   // should be 0 to 100
  if(pixel > 99){
    pixel = 99;
  }
  Yline[pixel] = color;  
  SubCount--;
  if(SubCount==0){
  // output 100 bytes to LCD, erases previous data
    ezLCD_XY(Xcolumn,128);  
    ezLCD_BitMap(1,100,Yline);
    Xcolumn++;
    if(Xcolumn==240){
      Xcolumn = 40;  // wrap back
    }
    ClearYLine();
  }
}
