// filename ************** Scheduler.c **************
// A parameterized periodic scheduler for the 9S12
// Written by Paine <n.a.paine@gmail.com>
// Functionality added by Craven <cascraven@gmail.com>
// Last updated 12/15/09

/** Feature Usage **
   
   Output Compare 2 interrupt
*/


#include <mc9s12dp512.h>     /* derivative information */
#include "Scheduler.h"

#if SCHED_DEBUG
#include "SCI_FIFO.h"
#endif

//#define TCNTS_PER_NS (1000/3)

void doNothing(void);

//event control block
typedef struct ECB{
  unsigned char isAvailable;
  unsigned char enabled;
  unsigned short event_TOFs;
  unsigned short event_TCNTs;
  unsigned short elapsed_TOFs;
  unsigned short elapsed_TCNTs;
  unsigned short runCount;
  void (*funcPtr)(void);
};

typedef struct ECB ECBtype;

unsigned short waitNext;    
ECBtype ECBList[MAX_EVENTS];
unsigned char numEvents = 0;
void (*NextEvent)(void) = &doNothing;
unsigned char NextEventIndex = 0;
unsigned short LastTCNT = 0;


void doNothing(void){
  return;
}

void _Scheduler_update(){
  unsigned short currentTCNT, TCNT_ticks;
  unsigned char i;
  
  currentTCNT = TCNT;
  TCNT_ticks = currentTCNT - LastTCNT;
  LastTCNT = currentTCNT;
  
  for(i=0; i<MAX_EVENTS; i++){
    if(ECBList[i].enabled){
      ECBList[i].elapsed_TCNTs += TCNT_ticks;
      if(ECBList[i].elapsed_TCNTs < TCNT_ticks){
        ECBList[i].elapsed_TOFs++;
      
      }
    }
  } 
}

//per is ticks between events
//for this project, ticks are every 333.333 ns
unsigned char Scheduler_AddEvent_ticks(void(*fp)(void), unsigned long per){
  return Scheduler_AddEvent(fp, per/65536, per%65536);
}

//add event in units of ns
//TCNT @ 333.33ns
//overflows if ns > 1.43165577e9
unsigned char Scheduler_AddEvent_ns(void(*fp)(void), unsigned long ns){
  return Scheduler_AddEvent_ticks(fp, (ns*3)/1000); //change this line if using different TCNT freq
}

//add event in units of 1/t
//min freq = 1hz
//for longer periods use Scheduler_AddEvent_ticks() 
unsigned char Scheduler_AddEvent_hz(void(*fp)(void), unsigned long freq){
  return Scheduler_AddEvent_ns(fp, 1000000000UL/freq);
}

//change event period in units of TCNT ticks
void Scheduler_SetPer_ticks(unsigned char id, unsigned long per){
  if(MAX_EVENTS<=id || 1>id)
    return; //do not go out of bounds or remove dummy event
	asm sei;
  ECBList[id].event_TOFs = per/65536;
  ECBList[id].event_TCNTs = per%65536;
  ECBList[id].elapsed_TOFs = 0;
  ECBList[id].elapsed_TCNTs = 0;
	asm cli;
}

void Scheduler_SetPer_ns(unsigned char id, unsigned long ns){
  Scheduler_SetPer_ticks(id, (ns*3)/1000); //change this line if using different TCNT freq
}

//change event period in unit of 1/sec
void Scheduler_SetFreq_hz(unsigned char id, unsigned long freq){
  if(0==freq)	 {
    Scheduler_DisableEvent(id);	
  	return;
  } else{
    Scheduler_EnableEvent(id);
    Scheduler_SetPer_ns(id, 1000000000UL/freq);
  }
}

// ************Scheduler_AddEvent**************************
// Schedule's an event to occur periodically
// Input:  fp - function pointer to run periodically
//         NOTE!: make this function as short as possible to reduce jitter
//         see the implementation of Scheduler_Init for an example of how to use this function
//
//         period_TOFs - timer overflows in period (bits 31-16 of period)
//         period_TCNTs - timer ticks in period (bits 15-0 of period)
// Output: NULL if failed (MAX_EVENTS shoud be increased) or the id number of the event
unsigned char Scheduler_AddEvent(void(*fp)(void), unsigned short period_TOFs, unsigned short period_TCNTs){
  unsigned char i;
  //step 1: update state of all events (time elapses)
  _Scheduler_update();
  
  //step 2: add event
  
  if(numEvents < MAX_EVENTS){
    for(i=0; i<MAX_EVENTS; i++){
      if( ECBList[i].isAvailable ) {
        ECBList[i].isAvailable = 0; //mark in use
        ECBList[i].enabled = 1;
        ECBList[i].event_TOFs = period_TOFs;
        ECBList[i].event_TCNTs = period_TCNTs;
        ECBList[i].elapsed_TOFs = 0;
        ECBList[i].elapsed_TCNTs = 0;
        ECBList[i].runCount = 0;
        ECBList[i].funcPtr = fp;
        numEvents++;
        return i;
      }
    }
  }
  return 0;
}

// ************Scheduler_RemoveEventForAction**************************
// Remove an event
// Input:  fp - function pointer used to add an event AddEvent
//         NOTE: if two events use the same fp the first one will be removed
// Output: NULL if unsuccessful (fp doesnt exist) or id removed
unsigned char Scheduler_RemoveEventForAction(void(*fp)(void)){
  unsigned char i;
  
  if(numEvents > 1){ //always need one event
    for(i=0; i<MAX_EVENTS; i++){
      if( ECBList[i].funcPtr == fp ) {
        ECBList[i].isAvailable = 1;//mark free
        ECBList[i].enabled = 0; //disable
        numEvents--;
        return i;
      }
    }
  }
  return 0;
}

// ************Scheduler_RemoveEvent**************************
// Remove an event
// Input:  id the id returned by addevent
//         NOTE: if two events use the same fp the first one will be removed
// Output: 0 if unsuccessful (fp doesnt exist)
unsigned char Scheduler_RemoveEvent(unsigned char id){
  if(MAX_EVENTS<=id || 1>id)
    return 0; //do not go out of bounds or remove dummy event
  ECBList[id].isAvailable = 1;//mark free
  ECBList[id].enabled=0;   //disable
  numEvents--;
  return 1;

}

unsigned char Scheduler_EnableEvent(unsigned char id){
  if(MAX_EVENTS<=id || 1>id)
    return 0;
  ECBList[id].enabled=1;
  return 1;
}

unsigned char Scheduler_DisableEvent(unsigned char id){
  if(MAX_EVENTS<=id || 1>id)
    return 0;
  ECBList[id].enabled=0;
  return 1;
}

unsigned char Scheduler_IsEnabled(unsigned char id){
  if(MAX_EVENTS<=id || 1>id)
    return 0;
  return ECBList[id].enabled;
}

// ************Scheduler_Init**************************
// Run before scheduling events
// Input:  none
// Output: none
void Scheduler_Init(void){
  unsigned char i;
  for(i=0; i<MAX_EVENTS; i++){
    ECBList[i].isAvailable = 1; //mark as free
    ECBList[i].enabled = 0;     //event is disabled
    ECBList[i].runCount = 0;    //hasn't been run yet
  }
  asm sei
  TIOS  |= 0x04;  // activate TC2 as output compare
  TIE  |= 0x04;     // arm OC2
  TC2 = TCNT+50; // first interrupt right away
  asm cli
  (void)Scheduler_AddEvent(&doNothing,0,60000);

}

//_Scheduler_Next (internal use only!)
//runs the next event and outputs number of TCNTs to next event
unsigned short _Scheduler_Next(void){
  unsigned short minTOFs = 65535, minTCNTs = 65535, TOFs, TCNTs, currentTCNT, TCNT_ticks;
  unsigned char i;
  NextEvent(); //run scheduled event
  
  
  /************update state************/
  
  //step 1: update state of all events (time elapses)
  
  currentTCNT = TCNT;
  TCNT_ticks = currentTCNT - LastTCNT;
  LastTCNT = currentTCNT;
  
  for(i=0; i<MAX_EVENTS; i++){
    if(ECBList[i].enabled){   
      if(i==NextEventIndex){  //we just ran this event so set time to zero, update runCount
        ECBList[i].elapsed_TOFs = 0;
        ECBList[i].elapsed_TCNTs = 0;
        ECBList[i].runCount++;
      }
      else{
        ECBList[i].elapsed_TCNTs += TCNT_ticks;
        if(ECBList[i].elapsed_TCNTs < TCNT_ticks){ //overflow
          ECBList[i].elapsed_TOFs++;
        
        }
      }
    }
  }
  
  //step 3: normalize and find next event to schedule
  
  for(i=0; i<MAX_EVENTS; i++){
    if(ECBList[i].enabled){
      
      if( ECBList[i].event_TOFs == ECBList[i].elapsed_TOFs ){
        if( ECBList[i].event_TCNTs < ECBList[i].elapsed_TCNTs ) {
          ECBList[i].elapsed_TCNTs = ECBList[i].event_TCNTs;    
        }
      }
   
      if( ECBList[i].event_TOFs < ECBList[i].elapsed_TOFs ){
        ECBList[i].elapsed_TOFs = ECBList[i].event_TOFs;     
        ECBList[i].elapsed_TCNTs = ECBList[i].event_TCNTs;
      }
  
  
      if(ECBList[i].elapsed_TCNTs > ECBList[i].event_TCNTs){
        TOFs = ECBList[i].event_TOFs - ECBList[i].elapsed_TOFs - 1;
      }
      else{
        TOFs = ECBList[i].event_TOFs - ECBList[i].elapsed_TOFs;
      }
    
      TCNTs = ECBList[i].event_TCNTs - ECBList[i].elapsed_TCNTs;
    
      if(TOFs < minTOFs || (TOFs == minTOFs && TCNTs < minTCNTs) ){
        minTOFs = TOFs;
        minTCNTs = TCNTs;
        NextEventIndex = i;
        NextEvent = ECBList[i].funcPtr;
      }
    }
  }
  return minTCNTs; //may need to add a few ticks;
}
 

#if SCHED_DEBUG
#define Sched_SCI 0

void Scheduler_PrintEvents(){
  unsigned char i;
  _Scheduler_update(); //update times
  SCI_OutString(Sched_SCI,"**Current Scheduler Events**"); SCI_OutCRLF(Sched_SCI);
  for(i=0; i<MAX_EVENTS; i++){
    if(ECBList[i].enabled){      
      SCI_OutString(Sched_SCI," Event "); SCI_OutUDec(Sched_SCI,i); SCI_OutCRLF(Sched_SCI);
      SCI_OutString(Sched_SCI,"  Event TOFs: "); SCI_OutUDec(Sched_SCI,ECBList[i].event_TOFs); SCI_OutCRLF(Sched_SCI);
      SCI_OutString(Sched_SCI,"  Event TCNTs: "); SCI_OutUDec(Sched_SCI,ECBList[i].event_TCNTs); SCI_OutCRLF(Sched_SCI);
      SCI_OutString(Sched_SCI,"  Elapsed TOFs: "); SCI_OutUDec(Sched_SCI,ECBList[i].elapsed_TOFs); SCI_OutCRLF(Sched_SCI);
      SCI_OutString(Sched_SCI,"  Elapsed TCNTs: "); SCI_OutUDec(Sched_SCI,ECBList[i].elapsed_TCNTs); SCI_OutCRLF(Sched_SCI);
      SCI_OutString(Sched_SCI,"  Times Run: "); SCI_OutUDec(Sched_SCI,ECBList[i].runCount); SCI_OutCRLF(Sched_SCI);
      SCI_OutString(Sched_SCI,"  Func Ptr: "); SCI_OutUHex(Sched_SCI,(unsigned short)ECBList[i].funcPtr); SCI_OutCRLF(Sched_SCI);
    }
  } 
  SCI_OutString(Sched_SCI,"**End of list**"); SCI_OutCRLF(Sched_SCI); 
}
#endif
    
interrupt 10 void OC2Han(void){
  waitNext = _Scheduler_Next();       
  TFLG1 = 0x04;       // acknowledge OC2
  TC2 = TCNT + waitNext + 10;   																												   
} 
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
        
  