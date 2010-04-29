/* File - i2c_master.c
   driver providing functions to read and write data via I2C
    
   Last Modified by - Sarvesh Nagarajan
   Date modified - 01/14/2009
   Reference : Embedded Microcomputer Systems by Dr. Valvano
*/

#include <mc9s12dp512.h>
#include "i2c_master.h"

// ************I2C_Init**************************
// Run first
// Input:  none
// Output: none
void I2C_Init(void){
  IBCR = 0x80; // enable, no interrupts, slave mode (later changed to busmaster)
  IBFD = 0x1F; // 100KHz assuming 24 MHz bus clock
  IBSR = 0x02; //clear IBIF
}


// ************I2C_Recv16**************************
// Receives 16 bits from specified slave device
// Input:  addr - slave I2C address
//         reg  - register to access
// Output: received data
unsigned short I2C_Recv16(unsigned char addr, unsigned char reg){
  unsigned char data1, data2 = 0;
  unsigned short orig_tcnt;

  IBCR |=0x30;//send start sets 6812 as busmaster
  IBDR = addr; //send slave address

  orig_tcnt= TCNT;
  //asm sei //disable interrupts
  while((IBSR&0x02)==0){ 
    if(TCNT - orig_tcnt > I2C_TIMEOUT) return 65535;
  } 

  IBSR = 0x02;
  IBDR = reg;  // register to read from
  
  orig_tcnt= TCNT;
  while((IBSR&0x02)==0){ 
    if(TCNT - orig_tcnt > I2C_TIMEOUT) return 65535;
  }

  IBSR = 0x02;
  IBCR |=0x04;//repeated start
  IBDR = addr|0x01;        // send read bit

  orig_tcnt= TCNT;
  while((IBSR&0x02)==0){ 
    if(TCNT - orig_tcnt > I2C_TIMEOUT) return 65535;
  }

  IBSR = 0x02;
  IBCR&=~0x18;
  data1 = IBDR; //dummy read

  orig_tcnt= TCNT;
  while((IBSR&0x02)==0){ 
    if(TCNT - orig_tcnt > I2C_TIMEOUT) return 65535;
  }

  IBSR = 0x02;
  IBCR|=0x08;//TXAK = 1
  data1 = IBDR;//first byte captured

  orig_tcnt= TCNT;
  while((IBSR&0x02)==0){ 
    if(TCNT - orig_tcnt > I2C_TIMEOUT) return 65535;
  } 

  IBSR = 0x02;
  IBCR|=0x08;//TXAK = 1;
  
  IBCR &=~0x38;//send stop bit
  data2 = IBDR; //capture second byte

  
  return ((data1<<8)+data2);
  
  
}  
  
// ************I2C_Recv8**************************
// Receives 8 bits from specified slave device
// Input:  addr - slave I2C address
//         reg  - register to access
// Output: received data  
unsigned short I2C_Recv8(unsigned char addr, unsigned char reg){

  unsigned char data3;

  IBCR |= 0x30; //send start sets 6812 as busmaster
  IBDR = addr;  //send slave address
  while((IBSR&0x02)==0){ }
  IBSR = 0x02;
  
  IBDR = reg;   // register to read from
  while((IBSR&0x02)==0){ }
  IBSR = 0x02;
  
  IBCR |=0x04;//repeated start
  
  IBDR = addr|0x01; // send read bit
  while((IBSR&0x02)==0){ }
  IBSR = 0x02;
  
  IBCR&=~0x10;
  //IBCR|=0x08;
  
  data3 = IBDR; // dummy read
  while((IBSR&0x02)==0){ }
  IBSR = 0x02;
  
  IBCR&=~0x38;// send stop bit
  
  data3 = IBDR; // get the required data
  
  return (short)data3;
}


// ************I2C_Send8**************************
// Sends 8 bits to specified slave device
// Input:  addr - slave I2C address
//         reg  - register to access
//         data - data to be sent   
// Output: none  

void I2C_Send8(unsigned char addr, unsigned char reg, unsigned char data){
  
 IBCR |= 0x30; //send start sets 6812 as busmaster
 
 IBDR = addr;  //send slave address
  while((IBSR&0x02)==0){ }
  IBSR = 0x02;
  
  IBDR = reg;   // register to write to
  while((IBSR&0x02)==0){ }
  IBSR = 0x02;
  
  IBDR = data; // send byte
  while((IBSR&0x02)==0){ }
  IBSR = 0x02;
  
  IBCR&=~0x30; // send stop bit
  
}
  
  

  



  
  
  