// filename ************** Scheduler.h **************
// A parameterized periodic scheduler for the 9S12
// Written by Paine <n.a.paine@gmail.com>
// Functionality added by Craven <cascraven@gmail.com>
// Last updated 12/15/09

/** Feature Usage **
   
   Output Compare 1 interrupt
*/

#ifndef _SCHEDULER_H
#define _SCHEDULER_H 1

#define SCHED_DEBUG 0

#define MAX_EVENTS 15 //number of events that can be scheduled

//per is ticks between events
//for this project, ticks are every 333.333 ns
unsigned char Scheduler_AddEvent_ticks(void(*fp)(void), unsigned long per);

//add event in units of ns
//TCNT @ 333.33ns
//overflows if ns > 1.43165577e9
unsigned char Scheduler_AddEvent_ns(void(*fp)(void), unsigned long ns);

//add event in units of 1/t
//min freq = 1hz
//for longer periods use Scheduler_AddEvent_ticks() 
unsigned char Scheduler_AddEvent_hz(void(*fp)(void), unsigned long freq);

void Scheduler_SetPer_ticks(unsigned char id, unsigned long per);

void Scheduler_SetPer_ns(unsigned char id, unsigned long ns);

void Scheduler_SetFreq_hz(unsigned char id, unsigned long freq);

// ************Scheduler_AddEvent**************************
// Schedule's an event to occur periodically
// Input:  fp - function pointer to run periodically
//         NOTE!: make this function as short as possible to reduce jitter
//         see the implementation of Scheduler_Init for an example of how to use this function
//
//         period_TOFs - timer overflows in period (bits 31-16 of period)
//         period_TCNTs - timer ticks in period (bits 15-0 of period)
// Output: NULL if failed (MAX_EVENTS shoud be increased) or the id number of the event
unsigned char Scheduler_AddEvent(void(*fp)(void), unsigned short period_TOFs, unsigned short period_TCNTs);

// ************Scheduler_RemoveEventForAction**************************
// Remove an event
// Input:  fp - function pointer used to add an event AddEvent
//         NOTE: if two events use the same fp the first one will be removed
// Output: NULL if unsuccessful (fp doesnt exist) or id removed
unsigned char Scheduler_RemoveEventForAction(void(*fp)(void));

// ************Scheduler_RemoveEvent**************************
// Remove an event
// Input:  id the id returned by addevent
//         NOTE: if two events use the same fp the first one will be removed
// Output: 0 if unsuccessful (fp doesnt exist)
unsigned char Scheduler_RemoveEvent(unsigned char id);

unsigned char Scheduler_EnableEvent(unsigned char id);

unsigned char Scheduler_DisableEvent(unsigned char id);

unsigned char Scheduler_IsEnabled(unsigned char id);

// ************Scheduler_Init**************************
// Run before scheduling events
// Input:  none
// Output: none
void Scheduler_Init(void);
 

#if SCHED_DEBUG
void Scheduler_PrintEvents();
#endif //SCHED_DEBUG

#endif //_SCHEDULER_H
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
        
  