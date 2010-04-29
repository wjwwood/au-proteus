// filename ******** Compass_i2c.c **************
// A compass driver for the Proteus Robot
// Written for Robot-Electronic's CMPS03 compass

/** Pin connections **
    
    SDA, SCLK
    
 ** Feature Usage **
   
   On I2C bus
*/

#include <mc9s12dp512.h>     /* derivative information */
#include "i2c_master_intr.h"
#include "Compass_i2c.h"
#include "LED.h"
       

  //i2c driver
extern unsigned char newI2cData, I2cTransDone;
extern unsigned short I2C1_SndRcvTemp; 
unsigned char inBuf[2];
//unsigned char compass_heading;
   
   
/*   
unsigned char Compass_read8(){
  //LED_redOn();
  return I2C_Recv8(COMPASS_I2C_ADDR, COMPASS_REG8);
  //LED_redOff();
} */
   
unsigned char Compass_read8(){
  LED_RED1 ^= 1;
      
  I2C1_SelectSlave(COMPASS_I2C_ADDR);
  LED_ORANGE2 ^= 1;
  I2cTransDone = 0;
  I2C1_SendChar(COMPASS_REG8); //send register to read from
  LED_ORANGE1 ^= 1;

  while(!I2cTransDone){};
  LED_YELLOW2 ^= 1;
  newI2cData = 0;
  I2C1_RecvChar(inBuf);
  LED_YELLOW1 ^= 1;
  
  while(!newI2cData){};
  LED_GREEN2 ^= 1;

  return inBuf[0];
}    
    
unsigned short Compass_read16(){
  LED_RED1 ^= 1;
  I2C1_SelectSlave(COMPASS_I2C_ADDR);
  LED_ORANGE2 ^= 1;
  I2cTransDone = 0;
  I2C1_SendChar(COMPASS_REG16); //send register to read from
  LED_ORANGE1 ^= 1;
  while(!I2cTransDone){};
  LED_YELLOW2 ^= 1;
  newI2cData = 0;
  I2C1_RecvChar(inBuf);
  LED_YELLOW1 ^= 1;
  while(!newI2cData){};
  LED_GREEN2 ^= 1;
  return (inBuf[0] << 8) | inBuf[1];  
  
}        
