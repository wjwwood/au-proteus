// filename ************** Tach.c **************
// Speed measurement for the Proteus Robot Project
// Written by Paine {n.a.paine@gmail.com}
// Last modified 1/15/10

/* Pin connections */
// PP1 PWM output to motor, interfaced in the Servo_PWM file
// PT1 is encoder channel B
// PT0 is encoder channel A   (channels may be reversed)
// Input capture 0, and TOF interrupts are employed
// Motor employs a PWM signal connected to the analog input
// http://www.robot-electronics.co.uk/htm/md03tech.htm
// The motor controllers require about 150us every 109ms
// plus about 5us for every encoder pulse

// The distance traveled is 0.12833 cm per count, 
// To redesign the encoder software, see TachometerDesign.xls
// The tires are 35 cm in circumference with weight of a laptop
// There are 100 slots in the encoder, and a 22/60 gearbox
// 35 cm circumference/100 slots*(22/60 gearbox) = 0.12833 cm per count

// The module assumes TCNT is enabled at 333 ns clock
// 5*65536*333.33ns = 109.226667 ms

#include <mc9s12dp512.h>     /* derivative information */
#include "Tach.h"
#include "Scheduler.h"
#include "util.h"

#pragma LINK_INFO DERIVATIVE "mc9s12dp512"

// Private global variables: permanent allocation, local scope
unsigned long Tach_idt=0, Tach_dt=0;
unsigned short Tach_tcnt_prev=0;
unsigned char checkTach=0;
short Tach_vel=0;
short Tach_vel_prev=0;
unsigned char Tach_dir=0; //0=fwd, 1=bkwd
unsigned long Tach_dt_filt; //filtered data
unsigned long Tach_dir_filt; //filtered direction
unsigned char Tach_new_data=0;
short Tach_distance=0;

//median filter variables
unsigned long tachBuf[TACH_FILTER_LEN];
unsigned long medFilt[TACH_FILTER_LEN];
unsigned long tachDirBuf[TACH_FILTER_LEN];
unsigned long medDirFilt[TACH_FILTER_LEN];

// ************Tach_Init**************************
// Activate Input capture 0
// PT1, PT0 are encoder inputs
// Input:  none
// Output: none
// Errors: assumes TCNT active at 3 MHz
void Tach_Init() {
  TIOS &= ~0x01;        // PT0 input capture
  DDRT &= ~0x03;        // PT0 and PT1 are inputs
  TCTL4 &= ~0x02;       // EDG0B = 0
  TCTL4 |= 0x01;        // EDG0A = 1, capture on rising of PT0
  TIE |= 0x01;          // C0I = 1, arm PT0 input capture
  Tach_dt = 0;
  Tach_idt = 0;
  Tach_tcnt_prev = 0;
  (void) Scheduler_AddEvent_hz(&tachPeriodicBG, TACH_RESET_HZ); 
}

//a periodic function to detect if we're stopped
void tachPeriodicBG(void){
	checkTach = 1;	
}

//foreground function called by main fg loop
void TachPeriodicFG(void){
  if(checkTach){
	  Tach_idt += TCNT - Tach_tcnt_prev;
	  Tach_tcnt_prev = TCNT;
	  if(Tach_idt>100000ul)  //calibration
	    Tach_dt = 0;
	  checkTach = 0;
  }
}


//a median filter to remove high freq noise
//filters both delta-time and direction
void TachFilterFG(void){
  unsigned char i,j;
  unsigned long temp;
  if(Tach_new_data){
    //first do speed
    //fifo buffer
    for(i=TACH_FILTER_LEN-1; i>0; i-=1){
      tachBuf[i]=tachBuf[i-1];  
    }
    tachBuf[0] = Tach_dt;
   
    //copy to filter buffer
    for(i=0; i<TACH_FILTER_LEN; i+=1){
      medFilt[i] = tachBuf[i];  
    }
    
    //sort filter buffer
    for(i=0; i<TACH_FILTER_LEN-1; i+=1){
      for(j=i+1; j<TACH_FILTER_LEN; j+=1){
        if(medFilt[j] < medFilt[i]){ //need to swap
          temp = medFilt[j];
          medFilt[j] = medFilt[i];
          medFilt[i] = temp;
        }
      }
    }
    Tach_dt_filt = medFilt[TACH_FILTER_LEN/2];
    
    //second do direction
    //fifo buffer
    for(i=TACH_FILTER_LEN-1; i>0; i-=1){
      tachDirBuf[i]=tachDirBuf[i-1];  
    }
    tachDirBuf[0] = Tach_dir;
   
    //copy to filter buffer
    for(i=0; i<TACH_FILTER_LEN; i+=1){
      medDirFilt[i] = tachDirBuf[i];  
    }
    
    //sort filter buffer
    for(i=0; i<TACH_FILTER_LEN-1; i+=1){
      for(j=i+1; j<TACH_FILTER_LEN; j+=1){
        if(medDirFilt[j] < medDirFilt[i]){ //need to swap
          temp = medDirFilt[j];
          medDirFilt[j] = medDirFilt[i];
          medDirFilt[i] = temp;
        }
      }
    }
    Tach_dir_filt = medDirFilt[TACH_FILTER_LEN/2];
   
    Tach_new_data = 0; 
  }
}

// occurs on the rising edge of PT0, the encoder channel A
void interrupt 8 IC0Handler(void) { 
  TFLG1 = 0x01;       // acknowledge
  Tach_idt += TCNT - Tach_tcnt_prev;
	Tach_tcnt_prev = TCNT;
	Tach_dt = Tach_idt;
	Tach_idt = 0;	
	Tach_dir = PTT_PTT1;
	Tach_new_data = 1;
	Tach_distance = Tach_dir ? Tach_distance - 1 : Tach_distance + 1;
}

//returns filtered velocity in +-cm/s
short Tach_GetVel(void){
  Tach_vel_prev = Tach_vel;
  if(0==Tach_dt)  {
  	Tach_vel = 0;
  }
  else { 
    //Tach_vel = 385000ul/Tach_dt * (Tach_dir ? -1 : 1); //calibration point
    Tach_vel = 385000ul/Tach_dt_filt * (Tach_dir_filt ? -1 : 1); //calibration point
  }
   
  return Tach_vel;
}

//returns distance traveled and resets distance traveled
short Tach_getDistanceR(void){
  short dist;
  dist = Tach_distance;
  Tach_distance = 0;
  return dist;  
}
