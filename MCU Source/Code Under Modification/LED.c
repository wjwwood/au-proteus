#include <mc9s12dp512.h>     /* derivative information */
#include "LED.h"

void LED_Init(void){
  DDRT |= 0xF0;    //PT4-7 output to LEDs
  DDRH |= 0xFC;    //PH2-7 output to LEDs
  PTT &= ~0xF0;
  PTH &= ~0xFC;
}  
