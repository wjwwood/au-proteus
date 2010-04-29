#ifndef _SRF08_H
#define _SRF08_H 1

#define SRF08_DEFAULT_ADDRESS 0xE0
#define SRF08_COMMAND 0x00
#define SRF08_GAIN 0x01
#define SRF08_RANGE 0x02
#define SRF08_RANGING_CM 0x51
#define SRF08_RANGING_INCH 0x50
#define SRF08_RANGING_US 0x52
#define SRF08_ADDRESS_SEQONE 0xA0
#define SRF08_ADDRESS_SEQTWO 0xAA
#define SRF08_ADDRESS_SEQTHREE 0xA5

#define SRF08_GAIN_INIT 26

#define SRF08_FL 0xE2
#define SRF08_FC 0xE4
#define SRF08_FR 0xE6
#define SRF08_RL 0xE8
#define SRF08_RC 0xEA
#define SRF08_RR 0xEC

#define SRF08_EMIT_WAIT_MS 80




//edit this to include sensor addresses used
void SRF08_Init(void);

// ************SRF08_emit_cm**************************
// starts ranging and scales the result to cm
// Input:  sensor module address   
// Output: none
void SRF08_emit_cm(unsigned char address);

// ************SRF08_emit_in**************************
// starts ranging and scales the result to inches
// Input:  sensor module address   
// Output: none
void SRF08_emit_in(unsigned char address);

// ************SRF08_emit_us**************************
// starts ranging and scales the result to us
// Input:  sensor module address   
// Output: none
void SRF08_emit_us(unsigned char address);

// ************SRF08_getRange**************************
// gets the computed range in the desired units
// Input:  sensor module address   
// Output: result of last ranging
unsigned short SRF08_getRange(unsigned char address);

// ************SRF08_setAddr**************************
// changes the address of the connected sonar module
// assumes the current address is the default factory address 0xE0
// only one sonar module should be connected to the bus here
// I guess this will be used most often
// Input:  new sensor module address   
// Output: none
void SRF08_setAddr(unsigned char new_address);

// changes the address of the connected sonar module
// address is changed from old_address to new_address
// more than one module can be connected as long as they all have different addresses
// Furthermore, the new address of one module should not conflict with the old addresses of others
// be extremely careful while using this function
void SRF08_changeAddr(unsigned char old_address, unsigned char new_address);

// ************SRF08_setGain**************************
// sets gain of sensor (see sensor manual)
// Input:  module address   
// Output: none
void SRF08_setGain(unsigned char address);

// ************SRF08_setRange**************************
// sets range of sensor (see sensor manual)
// Input:  module address   
// Output: none
void SRF08_setRange(unsigned char address);

// ************SRF08_readSamp_FLRR**************************
// reads sensor before emitting sound pulse
// for use with scheduler (no args)
// Input:  none   
// Output: none
void SRF08_readSamp_FLRR(void);

// ************SRF08_readSamp_FCRC**************************
// reads sensor before emitting sound pulse
// for use with scheduler (no args)
// Input:  none   
// Output: none
void SRF08_readSamp_FCRC(void);

// ************SRF08_readSamp_FRRL**************************
// reads sensor before emitting sound pulse
// for use with scheduler (no args)
// Input:  none   
// Output: none
void SRF08_readSamp_FRRL(void);




#endif /* _SRF08_H */


