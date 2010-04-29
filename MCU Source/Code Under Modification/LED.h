//LED.h
//makes LEDs easy to use

#ifndef _LED_H
#define _LED_H 1
 
#define LED_BLUE1 PTT_PTT7   //LED2
#define LED_BLUE2 PTT_PTT6   //LED3
#define LED_GREEN1 PTT_PTT5  //LED4
#define LED_GREEN2 PTT_PTT4  //LED5
//#define LED_YELLOW1 PTT_PTT3 //LED6   //woops, compass is connected here
                                  
#define LED_YELLOW2 PTH_PTH2   //LED3
#define LED_ORANGE1 PTH_PTH3   //LED7
#define LED_ORANGE2 PTH_PTH4   //LED8
#define LED_RED1 PTH_PTH5   //LED9
#define LED_RED2 PTH_PTH6   //LED10
#define LED_RED3 PTH_PTH7   //LED11


void LED_Init(void);

#endif //_LED_H