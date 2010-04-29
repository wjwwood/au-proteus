// filename ************** LCD.c **************
// interface for ezLCD-002
// Written by Steven Fontinelle sfontinelle@mail.utexas.edu
//
/*Notes:
   2 data structures (Button and Dataset found in myLCD.h) have been created for easy sandbox design of I/O
   this file contains functions to create, manage, and destroy these structs.
                       
   *callback functions for buttons currently cannot take any parameters
   *user touch is currently polled for at which point 
   the touch is compared against the keyArray of buttons in the global ButtonList,
   and appropriate action is taken.
   *certain sizes for arrays such as MAX_BUTTONS, MAX_DATA, Button.name, and Dataset.data are arbitrary 
   but for unknown reasons, errors can arise if they are too large.
   
   suggestions for future: 
   *have user touch cause an interrupt rather than polling for it, and poll data for easy 
   real time updating of data.
*/

#include <mc9s12dp512.h>
#include <stdlib.h>
#include "ezLCD.H"                                                                         
#include "LCD.h"
#include "Timer.h"


char BREAK_POLL = 0; //set to 1 in callback function to break out of PollButtons

//global ButtonList and DataList declarations
Button* ButtonList[MAX_BUTTONS];
char numButton = 0;
Dataset* DataList[MAX_DATA];
char numData = 0;


//helper function
short pow(short base, short exp){
  char i;
  short sum = base;
  
  if(exp == 0){return 1;} 
  else{
    for(i=1;i<exp;i=i+1){
     sum =  sum * base; 
    }
   }
  return sum;
}
 
 
//Wrapper for low level init
void LCD_Init(){
  ezLCD_Init();  
}

//-------------------------clear--------------------
//clears LCD screen
void LCD_Clear(){
  ezLCD_Color(WHITE);
  ezLCD_Clear();
  ezLCD_Color(BLACK);  
}


//-------------------LCD_ClearAll--------------
//clears the screen,datalist,and button list
void LCD_ClearAll(){
  LCD_Clear();
  LCD_ClearDataList();
  LCD_ClearButtonsList();
}

 
//-------LCD_Col---------------
//determines which column number a touch came from 
//input:touch key
//output:column number 0-7 
 
//decimal values of upper left corner of key
//y values are approx due to fraction
//pen up = infinity

//        0   30  60  90  120 150 180  210
//      -----------------------------------
//  0   | 0   1   2   3    4   5   6   7  |
//  27  | 16  17  18  19   20  21  22  23 |
//  53  | 32  33  34  35   36  37  38  39 |
//  81  | 48  49  50  51   52  53  54  55 |
//  108 | 64  65  66  67   68  69  70  71 |
//  135 | 80  81  82  83   84  85  86  87 |
//      -----------------------------------
 

char LCD_Col(char touch){

return (char)(touch % 16);

}

//-------LCD_Row---------------
//determines which row number a touch came from 
//input:touch key
//output:row number 0-5 
 

char LCD_Row(char touch){

return (char)(touch / 16);

}

 //-------------------------LCD_Generate_Keys------------------------
//helper function for LCD_PutButtons. Generates a list of keys based on the key, x, and y length of a button
//to be checked against during LCD_PollButtons for proper output
//Input: Pointer to a Button
//Output: none

void LCD_Generate_Keys(Button* A){                       
   char i;
   char j;
   char index = 0;
   A->keyArray = malloc(A->xlength * A->ylength);
   // A->keyArray[A->xlength * A->ylength];
 
   for(i = 0; i<A->ylength;i = i + 1){
     for(j = 0; j <A->xlength; j = j + 1){
       A->keyArray[index] = A->key + (i * 16) + j;
       index = index + 1;
     }
   }
   
   
}

//-------------------LCD_KBInput---------------------
//displays number keys at bottom of screen for touch input, returns when enter is touched
// '-' toggles negative or positive
// <-- backspace
//use a stylus or pencil eraser 
//input:none
//output: typed number

short LCD_KBInput(){
  char i,j;
  char nums=0;
  char touch;
  signed short sum=0;
  char touched[8] = {0};
  char negflag = 0; 
  
  LCD_DisableButtons();
  LCD_DisableData();

  LCD_Clear();
    //draw buttons
  ezLCD_XY(0,108);
  
    for(i=1;i<=8;i=i+1){
       ezLCD_Box(i*30,135);
    }
   ezLCD_XY(0,135);
      for(i=1;i<=4;i=i+1){
        ezLCD_Box(i*30,160);
   }
   ezLCD_XY(180,135);
   ezLCD_Line(180,159);
 
  //label buttons
   ezLCD_Color(BLACK);
 ezLCD_XY(11,119);
  for(i=1;i<=8;i=i+1){
  ezLCD_OutDec(i);
  ezLCD_XY( i*30 + 11,119);
  }
  
  ezLCD_XY(11,143);
  ezLCD_OutDec(9);
  ezLCD_XY(41,143);
  ezLCD_OutDec(0);
  ezLCD_XY(71,143);
  ezLCD_OutString("-");
  ezLCD_XY(94,143);
   ezLCD_Color(RED);
  ezLCD_OutString("<--");
  ezLCD_XY(190,144);
  ezLCD_Color(GREEN);
  ezLCD_OutString("ENTER");

   i=0;
    
    //if i == 0, do not allow backspace
    //if i == 5, only allow backspace or enter
      
   while(i <= 5 && i >=0 ){
      
      touch = ezLCD_InTouchS();
      switch(touch){
      case 64: if(i!=5){touched[i]= 1; i = i + 1;}break;
      case 65: if(i!=5){touched[i]= 2; i = i + 1;}break;
      case 66: if(i!=5){touched[i]= 3; i = i + 1;}break;
      case 67: if(i!=5){touched[i]= 4; i = i + 1;}break;
      case 68: if(i!=5){touched[i]= 5; i = i + 1;}break;
      case 69: if(i!=5){touched[i]= 6; i = i + 1;}break;
      case 70: if(i!=5){touched[i]= 7; i = i + 1;}break;
      case 71: if(i!=5){touched[i]= 8; i = i + 1;}break;
      case 80: if(i!=5){touched[i]= 9; i = i + 1;}break;
      case 81: if(i!=5){touched[i]= 0; i = i + 1;}break;
      case 82: negflag = !negflag;break;
      case 83: if(i!=0){i = i - 1; touched[i] = 0;} break;
      case 84: break;
      case 85: break;
      case 86: LCD_Clear(); 
               ezLCD_Color(BLACK);
               Timer_mwait(500);
               LCD_EnableButtons();
               LCD_EnableData();
               return sum; 
      case 87: LCD_Clear(); 
               ezLCD_Color(BLACK);
               Timer_mwait(500);
               LCD_EnableButtons();
               LCD_EnableData();
               return sum; 
      }
      ezLCD_XY(125,136);
      ezLCD_Color(WHITE);
      ezLCD_BoxFill(179,158);
      ezLCD_Color(BLUE);
      ezLCD_XY(129,143);        
      
      
    
    
      sum = 0;
           
      for(j= 0;j<8;j = j + 1){
        sum = sum + (touched[j] * pow(10,i-j-1));
       }
       if(negflag == 1){sum = -sum;}
       ezLCD_OutSDec(sum);
     
     
      }
            
}

//-------------------------LCD_ClearButtonList------------------------
//frees the keyArrays malloc'd by LCD_Generate_keys, and clears/resets the global ButtonList 
//Input: none
//Output: none
 
 void LCD_ClearButtonsList(){
    char i;
    
    for(i=0;i<numButton;i=i+1){
     free((*(ButtonList[i])).keyArray);
    }
    
    for(i=0;i<MAX_BUTTONS;i = i+1){
      ButtonList[i] = 0;
    }      
    numButton=0;
           

 }
 
//-------------------------LCD_PutButton------------------------
//calls LCD_Generate_Keys to generate the keyarray for each Button
//adds the button to the global ButtonList
//Input: Pointer to a Button
//Output: none   
void LCD_PutButton(Button* A){   
   LCD_Generate_Keys(A);
   ButtonList[numButton] = (A);
   numButton = numButton + 1;
   
}
  
  //-------------------------LCD_GetButton------------------------
// Blocks on user touch
// Runs button callback function
// Input: none
// Output: none
void LCD_GetButton(){
  LCD_ProcessButton(ezLCD_InTouchS());
}

//-------------------------LCD_ProcessButton------------------------
// compares the input key against the Buttons in global ButtonList
// calls button's callback function
// Input: touch index
// Output: none
void LCD_ProcessButton(unsigned char touch){
  unsigned char i;
  unsigned char j;
  
  
  
  for(i = 0;i<numButton;i = i +1){
    for(j = 0; j < ((*(ButtonList[i])).xlength * (*(ButtonList[i])).ylength); j = j +1){
      
      if(touch == (*(ButtonList[i])).keyArray[j]){
        if(ButtonList[i]->callback !=0){
          ButtonList[i]->callback();
         // ButtonList[i]->callback(ButtonList[i]->param);
          }
      }
    } 
  } 
}

   
//-------------------------LCD_PollButtons------------------------
//Displays buttons and data on the screen
//Polls for user inputed touch
//compares the inputted key against the Buttons in global ButtonList generated by LCD_PutButtons
//infinite loop unless callback function sets BREAK_POLL to 1
// Input: none
// Output: none
void LCD_PollButtons(){
  for(;;){
    //LCD_Refresh();
    BREAK_POLL=0;
    LCD_ProcessButton(ezLCD_InTouchS());
  
    if(BREAK_POLL){
      break;
    }
  }
}
 
 

 //-------------------------LCD_DrawButtons------------------------
//uses buttons from global ButtonList populated by LCD_PutButton
//*Draws icon to the screen (if there is one), otherwise draws a rectangle
//where the active keys for the button are and 
//displays the Button's name inside. 
//Input: none
//Output: none
void LCD_DrawButtons(){
  char row;
  char col;
  char i;
  Button myButton;
    
  for(i=0;i<numButton; i = i + 1){   
    if(ButtonList[i]->status){
      myButton = (*(ButtonList[i]));
    
      row = LCD_Row(myButton.key);
      col = LCD_Col(myButton.key);
    
    
      ezLCD_XY(col * 30,row *27);
    
      if(myButton.icon!=99){
        ezLCD_Icon(myButton.icon);
      } 
      else{
        ezLCD_Color(myButton.color);
        ezLCD_BoxFill(((col + myButton.xlength) * 30), ((row + myButton.ylength) * 27));
        ezLCD_XY(((col * 30) +1),((row * 30)+1));
        ezLCD_Color(BLACK);
        ezLCD_OutString(myButton.name);
      }
    }
  }
}
 
 
//----------------------LCD_DrawData-------------------------
// print data in the global DataList to the screen
// input: none
// output: none

void LCD_DrawData() {
  Dataset* ds;  
  char col, row, i,j ; 
      
  for(j=0;j<numData; j = j + 1){   
    if(DataList[j]->status){
       
      ds = ((DataList[j]));
      col = LCD_Col(ds->key);
      row = LCD_Row(ds->key);
                      
      for(i=0;i < ds->elements ; i=i+1){
        if(ds->line > (ds->ylength * 27)){
          LCD_ClearData(ds);
        }
   
        ezLCD_XY(col * 30, row *27 + ds->line);
        if(ds->isnum){
          ezLCD_OutSDec(ds->data[i]);
        } 
        else{
          ezLCD_OutString((char*)ds->data[i]);
        }
        ds->line += 8;
        ezLCD_OutChar(10);  
      }
    }
  }
}

//-------------------------LCD_ClearData----------------------
//erases data from the screen;useful for updating
//Input: none
//Output: none 
void LCD_ClearData(Dataset* ds){
  char row,col;

  col = LCD_Col((*ds).key);
  row = LCD_Row((*ds).key);
    
  ezLCD_XY(col * 30, row *27);
  ezLCD_Color(WHITE);
   ezLCD_BoxFill(((col + (*ds).xlength) * 30), ( ((*ds).ylength * 27) + 5 ));
  (*ds).line = 0;
  ezLCD_Color(BLACK);  
}

//-------------------------LCD_ClearAllData----------------------
//erases data from the screen;useful for updating
//Input: none
//Output: none 
void LCD_ClearAllData(){
  unsigned char i;
  Dataset* ds;
  char row,col;
  
  ezLCD_Color(WHITE);
  for(i = 0;i<numData;i = i +1){
    ds = DataList[i];
    col = LCD_Col(ds->key);
    row = LCD_Row(ds->key);
    
    ezLCD_XY(col * 30, row *27);
    
  // it used to be like this for some reason: ezLCD_BoxFill(((col + ds->xlength) * 30 + 20), ( (ds->ylength * 27) + 5 ));
     ezLCD_BoxFill(ds->xlength * 30,  ds->ylength * 27 );
    ds->line = 0;
    
  }   
  ezLCD_Color(BLACK);
}


   //-------------------------LCD_ClearDataList------------------------
//sets .line to 0 and clears the global DataList
// Input: none
// Output: none
 
 void LCD_ClearDataList(){
    char i;
    
      for(i=0;i<numData;i=i+1){
     (*DataList[i]).line = 0;
      }
    
    for(i=0;i<MAX_DATA;i = i+1){
      DataList[i] = 0;
    }      
    numData=0;
           

 }
    
 //-------------------------LCD_PutData------------------------
//adds pointer to a Dataset to the global DataList
// Input: Dataset*
// Output: none   
void LCD_PutData(Dataset* A){   
   
   DataList[numData] = (A);
   numData = numData + 1;
   
}
  

//-------------------------LCD_EnableButtons------------------------
// enables all buttons
// Input: none
// Output: none
void LCD_EnableButtons(){
  unsigned char i;
  for(i = 0;i<numButton;i = i +1){
    ButtonList[i]->status = 1;
  } 
}

//-------------------------LCD_DisableButtons------------------------
// disables all buttons
// Input: none
// Output: none
void LCD_DisableButtons(){
  unsigned char i;
  for(i = 0;i<numData;i = i +1){
    DataList[i]->status = 0;
  } 
}

//-------------------------LCD_EnableData------------------------
// enables all data
// Input: none
// Output: none
void LCD_EnableData(){
  unsigned char i;
  for(i = 0;i<numData;i = i +1){
    DataList[i]->status = 1;
  } 
}

//-------------------------LCD_DisableData------------------------
// disables all data
// Input: none
// Output: none
void LCD_DisableData(){
  unsigned char i;
  for(i = 0;i<numButton;i = i +1){
    ButtonList[i]->status = 0;
  } 
}

void LCD_Refresh(){
  LCD_ClearAllData(); //reset data cursors to top
  LCD_DrawButtons();
  LCD_DrawData();  
}



 //----------------------LCD_PutText-------------------------
// print data to screen
// if data overflows its y bounds, the data is cleared and the rest of the data
// continues being printed from the top of the key
// input: Dataset*
// output: none

void LCD_PutText(Dataset* ds) {
      char col, row, i ;
      
      col = LCD_Col((*ds).key);
      row = LCD_Row((*ds).key);
                     
   for(i=0;i < (*ds).elements ; i=i+1){
     if((*ds).line > ((*ds).ylength * 27)){
        LCD_ClearData(ds);}
  
     ezLCD_XY(col * 30, (row *27 + (*ds).line));
     ezLCD_OutString((char*)(*ds).data[i]);
     (*ds).line += 8;
     ezLCD_OutChar(10);  
  }
}

 //----------------------LCD_PutNums-------------------------
// print data to screen
// if data overflows its y bounds, the data is cleared and the rest of the data
// continues being printed from the top of the key
// input: Dataset*
// output: none

void LCD_PutNums(Dataset* ds) {
      char col, row, i ;
      
      col = LCD_Col((*ds).key);
      row = LCD_Row((*ds).key);
      
   for(i=0;i < (*ds).elements;i=i+1){
     if((*ds).line > (((*ds).ylength * 27) ) ){
        LCD_ClearData(ds);}
  
     ezLCD_XY(col * 30, (row *27 + (*ds).line));
     ezLCD_OutSDec((*ds).data[i]);
     (*ds).line += 8;
     ezLCD_OutChar(10);  
  }
}

 
 
 
  