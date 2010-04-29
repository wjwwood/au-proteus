
#ifndef _TIMER_H
#define _TIMER_H 1

//--------------------Timer_Init---------------
// initialize timer module to 333ns(Run Mode) TCNT clock
// inputs: none
// outputs: none
// PLL is on so E clock is 24MHz
void Timer_Init(void);

//---------------------Timer_mwait---------------------
// wait specified number of msec
// Input: number of msec to wait
// Output: none
// assumes TCNT timer is running at 333ns
void Timer_mwait(unsigned short msec); 


#endif /* _TIMER_H */