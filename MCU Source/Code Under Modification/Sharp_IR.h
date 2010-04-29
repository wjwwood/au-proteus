// filename ******** Sharp_IR.h **************
// A Sharp IR driver for the Proteus Robot
// For use with Proteus IR Switch V3+ circuit board

/*
 ** Pin connections **
    
    PAD00 -> Front Left Sensor
    PAD01 -> Front Center Sensor
    PAD02 -> Front Right Sensor
    PAD03 -> Rear Left Sensor
    PAD04 -> Rear Center Sensor
    PAD05 -> Rear Right Sensor
    

 ** Feature Usage **
   
   6 pins on ADC0
*/

#ifndef _Sharp_IR_H
#define _Sharp_IR_H 1

#define ADC_IR_FL 0x83 
#define ADC_IR_FC 0x84
#define ADC_IR_FR 0x85
#define ADC_IR_RL 0x80
#define ADC_IR_RC 0x81
#define ADC_IR_RR 0x82 

//all return units of mm
unsigned short IR_getFL(void); //front left
unsigned short IR_getFC(void); //front center
unsigned short IR_getFR(void); //front right
unsigned short IR_getRR(void); //rear left
unsigned short IR_getRC(void); //rear center
unsigned short IR_getRL(void); //rear right

#endif /* _Sharp_IR_H */