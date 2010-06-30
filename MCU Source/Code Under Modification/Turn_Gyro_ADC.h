// filename ******** Turn_Gyro_ADC.h **************
// A Gyroscope driver for the Proteus Robot
// For use with a Gyro that corresponds to Turns
// Written by Justin Paladino {PaladinoJ@gmail.com}
/*
 ** Pin connections **
    
    PAD06 -> Gyroscope 

 ** Feature Usage **
   
   1 pin on ADC0
*/

#ifndef _Turn_Gyro_H
#define _Turn_Gyro_H 1

#define ADC_TURN_GYRO 0x86 

unsigned short readTurnGyro(void);

#endif /**/