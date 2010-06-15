// filename ******** Servo_Pot_ADC.h **************
// A Servo Potentiometer driver for the Proteus Robot
// For use with a Potentiometer that corresponds to the steering servo
// Written by Justin Paladino {PaladinoJ@gmail.com}
/*
 ** Pin connections **
    
    PAD07 -> Servo Potentiometer 

 ** Feature Usage **
   
   1 pin on ADC0
*/
#ifndef _Servo_Pot_H
#define _Servo_Pot_H 1

#define ADC_SERVO_POT 0x87 

unsigned short readServoPot(void);

#endif /**/