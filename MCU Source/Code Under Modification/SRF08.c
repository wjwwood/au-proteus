/* File - SRF08.c
   Sonar sensor driver using I2C interface
   
   Last Modified by - Sarvesh Nagarajan,Paine
   Date modified - 07/16/2009
*/


#include "i2c_master.h"
#include "SRF08.h"
#include "Timer.h"

unsigned short SRF08_FL_cm=0, SRF08_FC_cm=0, SRF08_FR_cm=0, SRF08_RL_cm=0, SRF08_RC_cm=0, SRF08_RR_cm=0;

//edit this to include sensor addresses used
void SRF08_Init(){
  SRF08_setGain(SRF08_FL);
  SRF08_setGain(SRF08_FC);
  SRF08_setGain(SRF08_FR);
  SRF08_setGain(SRF08_RL);
  SRF08_setGain(SRF08_RC);
  SRF08_setGain(SRF08_RR);
}

// ************SRF08_emit_cm**************************
// starts ranging and scales the result to cm
// Input:  sensor module address   
// Output: none
void SRF08_emit_cm(unsigned char address){
  I2C_Send8(address, SRF08_COMMAND, SRF08_RANGING_CM);
  Timer_mwait(1);  
}

// ************SRF08_emit_in**************************
// starts ranging and scales the result to inches
// Input:  sensor module address   
// Output: none
void SRF08_emit_in(unsigned char address){
  I2C_Send8(address, SRF08_COMMAND, SRF08_RANGING_INCH);
  Timer_mwait(1);
}

// ************SRF08_emit_us**************************
// starts ranging and scales the result to us
// Input:  sensor module address   
// Output: none
void SRF08_emit_us(unsigned char address){
  I2C_Send8(address, SRF08_COMMAND, SRF08_RANGING_US);
  Timer_mwait(1);
}

// ************SRF08_getRange**************************
// gets the computed range in the desired units
// Input:  sensor module address   
// Output: result of last ranging
unsigned short SRF08_getRange(unsigned char address){
  unsigned short retVal;
  retVal = I2C_Recv16(address, SRF08_RANGE);
  Timer_mwait(1);
  return retVal;
}

// ************SRF08_setAddr**************************
// changes the address of the connected sonar module
// assumes the current address is the default factory address 0xE0
// only one sonar module should be connected to the bus here
// I guess this will be used most often
// Input:  new sensor module address   
// Output: none
void SRF08_setAddr(unsigned char new_address){
  
  I2C_Send8(SRF08_DEFAULT_ADDRESS, SRF08_COMMAND, SRF08_ADDRESS_SEQONE);
  Timer_mwait(1);
  I2C_Send8(SRF08_DEFAULT_ADDRESS, SRF08_COMMAND, SRF08_ADDRESS_SEQTWO);
  Timer_mwait(1);
  I2C_Send8(SRF08_DEFAULT_ADDRESS, SRF08_COMMAND, SRF08_ADDRESS_SEQTHREE);
  Timer_mwait(1);
  I2C_Send8(SRF08_DEFAULT_ADDRESS, SRF08_COMMAND, new_address);
  Timer_mwait(1);
}

// changes the address of the connected sonar module
// address is changed from old_address to new_address
// more than one module can be connected as long as they all have different addresses
// Furthermore, the new address of one module should not conflict with the old addresses of others
// be extremely careful while using this function
void SRF08_changeAddr(unsigned char old_address, unsigned char new_address){

  I2C_Send8(old_address, SRF08_COMMAND, SRF08_ADDRESS_SEQONE);
  Timer_mwait(1);
  I2C_Send8(old_address, SRF08_COMMAND, SRF08_ADDRESS_SEQTWO);
  Timer_mwait(1);
  I2C_Send8(old_address, SRF08_COMMAND, SRF08_ADDRESS_SEQTHREE);
  Timer_mwait(1);
  I2C_Send8(old_address, SRF08_COMMAND, new_address);
  Timer_mwait(1);
  
}

// ************SRF08_setGain**************************
// sets gain of sensor (see sensor manual)
// Input:  module address   
// Output: none
void SRF08_setGain(unsigned char address){
  I2C_Send8(address, SRF08_GAIN, SRF08_GAIN_INIT);
  Timer_mwait(1);
}

// ************SRF08_readSamp_FLRR**************************
// reads sensor before emitting sound pulse
// for use with scheduler (no args)
// Input:  none   
// Output: none
void SRF08_readSamp_FLRR(){
  SRF08_FL_cm = SRF08_getRange(SRF08_FL); 
  SRF08_RR_cm = SRF08_getRange(SRF08_RR);
  SRF08_emit_cm(SRF08_FL);
  SRF08_emit_cm(SRF08_RR); 
}

// ************SRF08_readSamp_FCRC**************************
// reads sensor before emitting sound pulse
// for use with scheduler (no args)
// Input:  none   
// Output: none
void SRF08_readSamp_FCRC(){
  SRF08_FC_cm = SRF08_getRange(SRF08_FC); 
  SRF08_RC_cm = SRF08_getRange(SRF08_RC);
  SRF08_emit_cm(SRF08_FC);
  SRF08_emit_cm(SRF08_RC); 
}

// ************SRF08_readSamp_FRRL**************************
// reads sensor before emitting sound pulse
// for use with scheduler (no args)
// Input:  none   
// Output: none
void SRF08_readSamp_FRRL(){
  SRF08_FR_cm = SRF08_getRange(SRF08_FR); 
  SRF08_RL_cm = SRF08_getRange(SRF08_RL);
  SRF08_emit_cm(SRF08_FR);
  SRF08_emit_cm(SRF08_RL); 
}




  

