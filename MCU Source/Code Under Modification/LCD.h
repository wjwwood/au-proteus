#ifndef myLCD_H
#define myLCD_H 1

#define LABELS 0
#define NUMBERS 1
#define NOICON 99

#define MAX_BUTTONS 15  
#define MAX_DATA 15


#ifndef GREEN
#define GREEN  0x38
#endif
#ifndef RED
#define RED  0x07
#endif
#ifndef BLUE
#define BLUE  0xC0
#endif
#ifndef NAVY
#define NAVY  0x80
#endif
#ifndef WHITE
#define WHITE  0xFF
#endif
#ifndef BLACK
#define BLACK  0x00
#endif
#ifndef VIOLET
#define VIOLET  0xC4
#endif
#ifndef PURPLE
#define PURPLE  0xC4
#endif
#ifndef YELLOW
#define YELLOW  0x3F
#endif
#ifndef LIGHTYELLOW
#define LIGHTYELLOW  0x24
#endif


 //decimal values of upper left corner of key
//y values are approx due to fraction
//pen up = infinity?

//        0   30  60  90  120 150 180  210
//      -----------------------------------
//  0   | 0   1   2   3    4   5   6   7  |
//  27  | 16  17  18  19   20  21  22  23 |
//  53  | 32  33  34  35   36  37  38  39 |
//  81  | 48  49  50  51   52  53  54  55 |
//  108 | 64  65  66  67   68  69  70  71 |
//  135 | 80  81  82  83   84  85  86  87 |
//      -----------------------------------



typedef void (*PFun)();

//Type Button
typedef struct{
  char *name;  //name that will be outputted to screen if no icon
  char key;       //top left key of button
  char xlength;   //length in keys 
  char ylength;   //length in keys
  char icon;      //icon to be displayed to screen, NOICON if no icon is to be used  
  char color;     //color 
  PFun callback; //callback function 
  //int param;
  char* keyArray; //populated by LCD_PutButton *DO NOT NEED TO INITIALIZE*
  unsigned char status; //0: disabled, else: enabled
  }Button;


//Type Dataset
typedef struct{
  char key;     //top left key of text (for formatting)
  char xlength; //length in keys
  char ylength; //length in keys
  char elements;//number of elements in data[] 
  char isnum;   //1 if the content of data[] is numbers, 0 if text
  char line;    //should be 0                                                                                                     
  char* data[10];  //for reasons that are unclear, setting this to a larger value will result in program failure under certain unknown circumstances.
  unsigned char status; //0: disabled, else: enabled
  }TxtData;
  
  //Type Dataset
typedef struct{
  char key;     //top left key of text (for formatting)
  char xlength; //length in keys
  char ylength; //length in keys
  char elements;//number of elements in data[] 
  char isnum;   //1 if the content of data[] is numbers, 0 if text
  char line;    //should be 0                                                                                                     
  short data[10];  //for reasons that are unclear, setting this to a larger value will result in program failure under certain unknown circumstances.
  unsigned char status; //0: disabled, else: enabled
  }NumData;
   
   
 // typedef TxtData Dataset;
  typedef NumData Dataset;
  



void LCD_Init(void);

//-------------------------clear--------------------//
//clears LCD screen
void LCD_Clear(void);

//-------------------LCD_ClearAll--------------
//clears the screen,datalist,and button list
void LCD_ClearAll(void);

//---------------------ezLCD_InTouchS--------------------------------
//same as ezLCD_InTouch except surpresses pen up output
 unsigned char ezLCD_InTouchS(void);
 
//-------LCD_Col---------------
//determines which column number a touch came from 
//input:touch key
//output:column number 0-7 
char LCD_Col(char touch);
 
//-------LCD_Row---------------
//determines which row number a touch came from 
//input:touch key
//output:row number 0-5 
char LCD_Row(char touch);

//-------------------LCD_KBInput---------------------
//displays number keys at bottom of screen for touch input, returns when enter is touched
// '-' toggles negative or positive
// <-- backspace
//use a stylus or pencil eraser 
//input:none
//output: typed number
short LCD_KBInput(void);

 //-------------------------LCD_ClearButtonList------------------------
//frees the keyArrays malloc'd by LCD_Generate_keys, and clears/resets the global ButtonList 
// Input: none
// Output: none
 
 void LCD_ClearButtonsList(void);
 
//-------------------------LCD_PutButton------------------------
//calls LCD_Generate_Keys to generate the keyarray for each Button
//adds the button to the global ButtonList
//Input: Pointer to a Button
//Output: none    
void LCD_PutButton(Button* A);

 //-------------------------LCD_DrawButtons------------------------
//uses buttons from global ButtonList populated by LCD_PutButton
//*Draws icon to the screen (if there is one), otherwise draws a rectangle
//where the active keys for the button are and 
//displays the Button's name inside. 
//Input: none
//Output: none
 void LCD_DrawButtons(void);


//-------------------------LCD_Generate_Keys------------------------
//helper function for LCD_PutButtons. generates a list of keys based on the key, x, and y length of a button
//to be checked against during LCD_PollButtons for proper output
// Input: Pointer to a Button
// Output: none
void LCD_Generate_Keys(Button* A);


//-------------------------LCD_ClearData----------------------
//erases data from the screen;useful for updating
//Input: Pointer to a Dataset
//Output: none 
void LCD_ClearData(Dataset* ds);

//-------------------------LCD_ClearAllData----------------------
//erases data from the screen;useful for updating
//Input: Pointer to a Dataset
//Output: none 
void LCD_ClearAllData(void);

   //-------------------------LCD_ClearDataList------------------------
//sets .line to 0 and clears the global DataList
// Input: none
// Output: none
void LCD_ClearDataList(void);

 //-------------------------LCD_PutData------------------------
//adds pointer to a Dataset to the global DataList
// Input: Dataset*
// Output: none   
void LCD_PutData(Dataset* A);

//----------------------LCD_DrawData-------------------------
// print data in the global DataList to the screen
// input: none
// output: none
void LCD_DrawData(void);

//-------------------------LCD_PollButtons------------------------
//Displays buttons and data on the screen
//Polls for user inputed touch
//compares the inputted key against the Buttons in global ButtonList generated by LCD_PutButtons
//infinite loop unless callback function sets BREAK_POLL to 1
// Input: none
// Output: none
 void LCD_PollButtons(void);
 
 //----------------------LCD_PutText-------------------------
// print data to screen
// if data overflows its y bounds, the data is cleared and the rest of the data
// continues being printed from the top of the key
// input: Dataset*
// output: none
void LCD_PutText(Dataset* ds);

 //----------------------LCD_PutNums-------------------------
// print data to screen
// if data overflows its y bounds, the data is cleared and the rest of the data
// continues being printed from the top of the key
// input: Dataset*
// output: none
void LCD_PutNums(Dataset* ds);

//draw buttons and data
void LCD_Refresh();

//-------------------------LCD_ProcessButton------------------------
// compares the input key against the Buttons in global ButtonList
// calls button's callback function
// Input: touch index
// Output: none
void LCD_ProcessButton(unsigned char touch);

//-------------------------LCD_GetButton------------------------
// Blocks on user touch
// Runs button callback function
// Input: none
// Output: none
void LCD_GetButton();

//-------------------------LCD_EnableButtons------------------------
// enables all buttons
// Input: none
// Output: none
void LCD_EnableButtons();

//-------------------------LCD_DisableButtons------------------------
// disables all buttons
// Input: none
// Output: none
void LCD_DisableButtons();

//-------------------------LCD_EnableData------------------------
// enables all data
// Input: none
// Output: none
void LCD_EnableData();

//-------------------------LCD_DisableData------------------------
// disables all data
// Input: none
// Output: none
void LCD_DisableData();

//redraws buttons and data
void LCD_Refresh();





#endif //LCD_H