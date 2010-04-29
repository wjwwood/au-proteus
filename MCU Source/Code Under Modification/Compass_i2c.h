// filename ******** Compass.h **************
// A compass driver for the Proteus Robot
// Written for Robot-Electronic's CMPS03 compass

/** Pin connections **
    
    SDA, SCLK
    
 ** Feature Usage **
   
   On I2C bus
*/

#ifndef _COMPASS_H
#define _COMPASS_H 1

//compass driver for Proteus Robot

void Compass_Init(void);


#define COMPASS_I2C_ADDR 0xC0 //0x60=0xC0 when converted to 8-bit addr
#define COMPASS_REG8 0x01
#define COMPASS_REG16 0x02

// returns heading in fixed point degrees 0-3599 (0 to 359.9 degrees)
unsigned short Compass_read16(void);

//returns heading in 0-255
unsigned char Compass_read8(void);


#endif /* _COMPASS_H */