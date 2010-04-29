// filename ******** Compass.c **************
// A compass driver for the Proteus Robot
// Written for Robot-Electronic's CMPS03 compass

/** Pin connections **
    
    SDA, SCLK
    
 ** Feature Usage **
   
   On I2C bus
*/

#include <mc9s12dp512.h>
#include "Compass_pw.h"
#include "LED.h"
#include "Scheduler.h"


unsigned short TOF_counter=0;
unsigned short TCNT_offset;
unsigned short TCNT_begin=0;
unsigned short TOF_begin=0;
unsigned long TCNT_delta=0;
unsigned short TCNT_end;
unsigned short latchedTCNT=0;

unsigned char newHeading=0;

unsigned short t1, t2, t3;
unsigned short compass_heading=0; //absolute heading in .1 degrees (359.9 (deg) = 3599 (.1 deg))

void incTof(){
  TOF_counter += 1;
  TCNT_offset = TCNT; 
}

void Compass_Init(){
  TIOS &= ~0x08;        // PT3 input capture
  DDRT &= ~0x08;        // PT3 is input 
  TCTL4 |= 0xC0;        // EDG3A = 1, EDG3B = 1 capture on rising/ralling edge of PT3
  TIE |= 0x08;          // C3I = 1, arm PT3 input capture
  (void)Scheduler_AddEvent(&incTof, 1, 0);
}

// occurs on the rising/falling edge of PT3
void interrupt 11 IC3Handler(void) { 
  latchedTCNT = TCNT;
  TFLG1 = 0x08;       // acknowledge
  newHeading = 1;
}

//check if we need to update heading in FG
void CMPPeriodicFG(void){
  if(newHeading){
    if(PTT_PTT3) {  
      TCNT_begin = TCNT;
      TOF_begin = TOF_counter;
    } 
    else {  
      TCNT_end = TCNT;        
      if(TOF_counter == TOF_begin) {
        TCNT_delta = TCNT_end - TCNT_begin;
      } 
      else{
        //t1 = 65535 - TCNT_begin;  
        t1 = (TCNT_offset - TCNT_begin);
        t2 = 65535 * (unsigned long)(TOF_counter - TOF_begin - 1);
        //t3 = TCNT_end;
        t3 = TCNT_end - TCNT_offset;
        //TCNT_delta = (TCNT_offset - TCNT_begin) /*+ 65535 * (unsigned long)(TOF_counter - TOF_begin - 1)*/ + TCNT_end - TCNT_offset;
        TCNT_delta = (unsigned long)t1 + (unsigned long)t2 + (unsigned long)t3; 
      }
      compass_heading = (TCNT_delta - 3000)/30;
    }
  newHeading = 0;
  }
}
       
//returns most recent heading
unsigned short Compass_getHeading(){
  return compass_heading;  
}


















/*
  //i2c driver
extern unsigned char newI2cData, I2cTransDone;
extern unsigned short I2C1_SndRcvTemp; 
unsigned char inBuf[2];
//unsigned char compass_heading;
   
   
   
unsigned char Compass_read8(){
  //LED_redOn();
  return I2C_Recv8(COMPASS_I2C_ADDR, COMPASS_REG8);
  //LED_redOff();
}     */
   
   /*   interrupt driver
unsigned char Compass_read8(){
  LED_redOn();
  
  I2C1_SelectSlave(COMPASS_I2C_ADDR);
  I2cTransDone = 0;
  I2C1_SendChar(COMPASS_REG8); //send register to read from


  while(!I2cTransDone){};

  newI2cData = 0;
  I2C1_RecvChar(inBuf);
 
  
  while(!newI2cData){};
  LED_redOff();

  return inBuf[0];
}    */


       /*
unsigned short Compass_read16(){
  I2C1_SelectSlave(COMPASS_I2C_ADDR);
  I2cTransDone = 0;
  I2C1_SendChar(COMPASS_REG16); //send register to read from
  while(!I2cTransDone){};
  newI2cData = 0;
  I2C1_RecvChar(inBuf);
  while(!newI2cData){};
  return (inBuf[0] << 8) | inBuf[1];  
  
}        */
