//main-interface2.c
//Main for Proteus serial interface to Player.
//Written by Paine {n.a.paine@gmail.com}
//Modified by Justin Paladino {PaladinoJ@gmail.com}

#include <hidef.h>           /* common defines and macros */
#include <mc9s12dp512.h>     /* derivative information */
#include "PLL.h"
#include "Timer.h"
#include "SCI_PROTEUS.h"
#include "LED.h"
#include "Scheduler.h"
#include "Servo_PWM.h"
#include "Tach.h"
#include "MotorControl.h"
#include "Command.h"
#include "adc.h"
#include "Compass_pw.h"

#pragma LINK_INFO DERIVATIVE "mc9s12dp512"

void main(void) {
  PLL_Init();   // Eclk @ 24MHz
  Timer_Init(); // TCNT @ 333.3ns, TOF @ 21.84ms
  SCI_Init(SCI_X86, 57600);
  Scheduler_Init();
  LED_Init();
  
  //can be put inside interface (not all are always used)
  Servo_Init(); 
  Tach_Init();  //tachometer init
  MC_Init(); //motor control init
  ADC0_Init();  //IR sensors
  Compass_Init();
  
  asm cli  //enable interrupts
  
  LED_BLUE1 = 1;
  
  SCI_OutString(SCI_X86, "AU-Proteus SCI: Ready...waiting for input"); 
  SCI_OutCRLF(SCI_X86);
  
  for(;;){ //foreground loop
   ServoPeriodicFG(); //in Servo_PWM.h
   TachPeriodicFG();  //in Tach.h
   TachFilterFG();    //in Tach.h
   MCPeriodicFG();    //in MotorControl.h
   CMPPeriodicFG();   //in Compass_pw.h
   InterfaceFG();     //in Command.h
  } 
}




