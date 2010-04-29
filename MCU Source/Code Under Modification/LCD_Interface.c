//LCD_interface.c

#include <mc9s12dp512.h>
#include "Scheduler.h"
#include "LCD_Interface.h"
#include "LCD.h"
#include "ezLCD.h"
#include "LED.h"

#include "Servo_PWM.h"


Button MtrCtr = {0};
TxtData MainTxt = {0};
NumData MainNums = {0}; 
Button ServoLeft = {0};
  
  
 void _initMtrCtr(){
 
 MtrCtr.name = "MtrCtr"; 
 MtrCtr.key = 1;
 MtrCtr.xlength = 2;
 MtrCtr.ylength = 2;
 MtrCtr.icon = NOICON;
 MtrCtr.color = GREEN;
 MtrCtr.callback = &LCDI_MtrCtrScreen;
 MtrCtr.status = 1;
 
 MainTxt.key = 32;
 MainTxt.xlength = 3;
 MainTxt.ylength = 4;
 MainTxt.elements= 3;
 MainTxt.isnum = 0;
 MainTxt.data[0] = "Mtr Vel";
 MainTxt.data[1] = "Steer Angle";
 MainTxt.data[2] = "Data 3";
 MainTxt.status = 1;
 
 MainNums.key = 35;
 MainNums.xlength = 1;
 MainNums.ylength = 3;
 MainNums.elements = 3;
 MainNums.isnum = 1;
 MainNums.data[0] = Servo_GetSteeringAngle();
 MainNums.data[1] = 200;
 MainNums.data[2] = 0;
 MainNums.status= 1;
 
 /*
 ServoLeft.name = "Left";
 ServoLeft.key = 6;
 ServoLeft.xlength = 2;
 ServoLeft.ylength =2;
 ServoLeft.icon = NOICON;
 ServoLeft.color = BLUE;
 ServoLeft.callback = &Servo_SetSteeringAngle;
 ServoLeft.param = 2500;
 ServoLeft.status = 1;
 */
 
 }

  
short LCDIAction;




void LCDIPeriodicBG(void){
LED_ORANGE1 ^= 1;
  LCDIAction = 1;
}

void LCDI_Init(void){
   LCD_Init();
   _initMtrCtr();
   ezLCD_Light(1);
   (void) Scheduler_AddEvent_hz(&LCDIPeriodicBG,LCD_HZ);
  }
  
  
 short countup =0;

void LCDIFG(void){
  
  if(LCDIAction){
  LED_RED1 ^= 1;
  
  
    //LCD_Clear();
    //LCD_DrawButtons();
  
    
  LCD_ClearAllData();
  LCD_DrawData();
  MainNums.data[2] = 25;
  
    
  
  LCDIAction = 0;
     } 
      
     
}





void LCDI_MainScreen(){

  LCD_Clear();
  LCD_PutButton(&MtrCtr);
  //LCD_PutButton(&ServoLeft);
  
  LCD_PutData(&MainTxt);
  LCD_PutData(&MainNums);
  LCD_DrawButtons();
  LCD_DrawData();
  }
  

void LCDI_MtrCtrScreen(){
MtrCtr.color = RED;
LCD_Clear();




LCD_ClearAllData();
LCD_DrawData();
LCD_DrawButtons();



}

void LCDI_ProcessButton(){
    LCD_ProcessButton(ezLCD_InTouch()); 
   }
   
  
   

  
  




