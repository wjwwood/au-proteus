/* File - i2c_master.h
   Last Modified by - Sarvesh Nagarajan
   Date modified - 10/14/2008
   Reference : Embedded Microcomputer Systems by Dr. Valvano
*/

#ifndef _I2C_MASTER_H
#define _I2C_MASTER_H 1

#define I2C_TIMEOUT 30000 //10ms

// ************I2C_Init**************************
// Run first
// Input:  none
// Output: none
void I2C_Init(void);

// ************I2C_Recv16**************************
// Receives 16 bits from specified slave device
// Input:  addr - slave I2C address
//         reg  - register to access
// Output: received data
unsigned short I2C_Recv16(unsigned char addr, unsigned char reg);

// ************I2C_Recv8**************************
// Receives 8 bits from specified slave device
// Input:  addr - slave I2C address
//         reg  - register to access
// Output: received data
unsigned short I2C_Recv8(unsigned char addr, unsigned char reg);

// ************I2C_Send8**************************
// Sends 8 bits to specified slave device
// Input:  addr - slave I2C address
//         reg  - register to access
//         data - data to be sent   
// Output: none  
void I2C_Send8(unsigned char addr, unsigned char reg, unsigned char data);


#endif /* _I2C_MASTER_H */