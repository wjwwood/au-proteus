// filename ************** Tach.h **************
// Speed measurement for the Proteus Robot Project
// Written by Paine {n.a.paine@gmail.com}
// Last modified 1/15/10

#ifndef _TACH_H
#define _TACH_H 1

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



#define TACH_RESET_HZ 10 //check if we're stopped every second
                //may need to increase for controller response
                
#define TACH_FILTER_LEN 9

// ************Tach_Init**************************
// Activate Input capture 0
// PT1, PT0 are encoder inputs
// Input:  none
// Output: none
// Errors: assumes TCNT active at 3 MHz
void Tach_Init(void);

//a periodic function to detect if we're stopped
void TachPeriodicFG(void);

void TachFilterFG(void);

//background task
void tachPeriodicBG(void);

// occurs on the rising edge of PT0, the encoder channel A
// improvement, need to sum and divide upon vel read
void interrupt 8 IC0Handler(void);

//returns velocity in cm/s
short Tach_GetVel(void);

//returns distance traveled and resets distance
short Tach_getDistanceR(void);

#endif //_TACH_H