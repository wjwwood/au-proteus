#include <mc9s12dp512.h>     /* derivative information */
#include "Timer.h"

//--------------------Timer_Init---------------
// initialize timer module to 333ns(Run Mode) TCNT clock
// inputs: none
// outputs: none
// PLL is on so E clock is 24MHz
void Timer_Init(void){
  TSCR1 = 0x80;   // Enable TCNT, 8MHz in run modes
  TSCR2 = (TSCR2&0x80)+0x03;   // divide by 8 TCNT prescale
  PACTL = 0;      // timer prescale used for TCNT
/* Bottom three bits of TSCR2 (PR2,PR1,PR0) determine TCNT period
    divide  8 (24) MHz E clock    
000   1     41.67ns    
001   2     83.33ns     
010   4    166.67ns         
011   8    333.33ns   	 
100  16    666.67ns   	 
101  32      1.33us  		 
110  64      2.67us     
111 128      5.33us      */ 
// Be careful, TSCR1 and TSCR2 maybe set in other rituals
}

//---------------------Timer_mwait---------------------
// wait specified number of msec
// Input: number of msec to wait
// Output: none
// assumes TCNT timer is running at 333ns
void Timer_mwait(unsigned short msec){ 
unsigned short startTime;
  for(; msec>0; msec--){
    startTime = TCNT;
    while((TCNT-startTime) <= 3000){} //1/3us * 3000 = 1ms 
  }
}