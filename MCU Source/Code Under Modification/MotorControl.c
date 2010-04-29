//MotorControl.c
//PID motor control for the Proteus Robot
//Written by Paine {n.a.paine@gmail.com}
//Last modified 2/8/10

#include <hidef.h>           // common defines and macros 
#include <mc9s12dp512.h>     // derivative information 
#include "MotorControl.h"
#include "Scheduler.h"
#include "Tach.h"
#include "Servo_PWM.h"
#include "LED.h"

short Velocity;         // units:  cm/sec
short DesiredVelocity=0;  // units:  cm/sec
short VelocityError;    // units:  cm/sec
short PreviousError=0;     // previous VelocityError
short Direction;        // +1 forward; -1 backward; 
short Power;            // 0-1024 forward, -1023-0 backward
unsigned char setMotor=0;
short ErrorDiff;
unsigned short Kp=130; //Proportional gain
unsigned short Kidt=0; //Integral gain
unsigned short Kd=0;   //Differential gain
short Up;           // proportional term
short Ui;           // integral term 
short Ud;           // differential term
unsigned char MotorStall=0; //set this when motor power gets too high or low


void MC_Init(void){
  DesiredVelocity = 0;
  setMotor = 0;
  (void) Scheduler_AddEvent_hz(&MCPeriodicBG, MC_FREQ); 
}

void MC_SetVel(short vel){ //cm/s
  DesiredVelocity = vel;
  MotorStall = 0;
}

void MCPeriodicBG(void){
	setMotor = 1;	
}


void MCPeriodicFG(void){  
  if(setMotor){
    PreviousError = VelocityError;
    Velocity = Tach_GetVel();
    VelocityError = DesiredVelocity-Velocity;   //cm/sec
    ErrorDiff = VelocityError - PreviousError;
  //  Up = (Kp*VelocityError)/100; /
  asm  ldd  VelocityError  // cm/sec        
  asm  ldy  Kp             // powerUnits/(cm/sec)
  asm  emuls
  asm  ldx  #100
  asm  edivs
  asm  sty  Up             // powerunits 0 to 2048 
  //  Ui = Ui+(Kidt*VelocityError)/100; 
  asm  ldd  VelocityError  // cm/sec        
  asm  ldy  Kidt           // powerUnits/(cm/sec)
  asm  emuls
  asm  ldx  #100
  asm  edivs
  asm  tfr   y,d
  asm  addd  Ui
  asm  std   Ui            // powerunits 0 to 2048  
  //  Ud = (Kd*(VelocityError-PreviousError))/100; /
  asm  ldd  ErrorDiff      // cm/sec        
  asm  ldy  Kd             // powerUnits/(cm/sec)
  asm  emuls
  asm  ldx  #100
  asm  edivs
  asm  sty  Ud             // powerunits -1023 to 1024 
 
    Power += Up+Ui+Ud;
  
    if(Power > STALL_POWER || Power < -STALL_POWER){
      Motor_set(0);  
      MotorStall = 1;
      Up = 0;
      Ui = 0;
      Ud = 0;
      Power = 0;
    } 
    else{
      Motor_set(Power);
    }
    
    setMotor=0; //clear flag
  }
}
