//Command.h
//Parameter declaration for Proteus SCI protocol
//Written by Paine {n.a.paine@gmail.com}

/* Commands come over the serial port with the following structure
  {begin char} {opcode} {data 1} {data 2} ... {data N} {end char}
  
  SCI_Proteus.h takes care of low level protocol handling, using the {begin char} and {end char} to capture command packets
  These packets are turned over to Command.c and look like {opcode} {data 1} ... {data N}
  Command.c processes these commands
  */

//command map

#ifndef _PROTEUS_H
#define _PROTEUS_H 1

#define PROTEUS_BEGIN 0x24 //'$' to start transmissions
#define PROTEUS_END 0x0A //LF terminated transmissions

/* command opcodes */
#define PROTEUS_OPCODE_START            0x61
#define PROTEUS_OPCODE_BAUD             0x62
#define PROTEUS_OPCODE_CONTROL          0x63
#define PROTEUS_OPCODE_SAFE             0x64
#define PROTEUS_OPCODE_FULL             0x65
#define PROTEUS_OPCODE_STOP             0x66
#define PROTEUS_OPCODE_DRIVE            0x67
#define PROTEUS_OPCODE_LEDS             0x68
#define PROTEUS_OPCODE_SENSORS          0x69
#define PROTEUS_OPCODE_SONAR_EN         0x6A
#define PROTEUS_OPCODE_SONAR_DE         0x6B

#define PROTEUS_DELAY_MODECHANGE_MS       5

enum{
  PROTEUS_MODE_OFF,                  
  PROTEUS_MODE_PASSIVE,              
  PROTEUS_MODE_SAFE,                 
  PROTEUS_MODE_FULL
};

enum{
  ZERO,
  PROTEUS_ODOMETRY_PACKET,
  PROTEUS_IR_PACKET,
  PROTEUS_SONAR_PACKET,
  PROTEUS_COMPASS_PACKET,
  PROTEUS_OPAQUE_PACKET
};

#define PROTEUS_ODOMETRY_PACKET_SIZE      5
#define PROTEUS_IR_PACKET_SIZE           24  //12 if no extra IR needed/used
#define PROTEUS_SONAR_PACKET_SIZE        12
#define PROTEUS_COMPASS_PACKET_SIZE       2
#define PROTEUS_OPAQUE_PACKET_SIZE        1
#define PROTEUS_PACKET_OVERHEAD           2

#define MAX_CMD_LEN                      12 
#define MAX_PACKET_LEN MAX_CMD_LEN+PROTEUS_PACKET_OVERHEAD

#define SCI_X86 0

void periodicSafeMotor(void); 

void InterfaceFG(void);

#endif /* _PROTEUS_H */