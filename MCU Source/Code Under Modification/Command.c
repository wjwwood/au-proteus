//Command.c
//Serial interface processing block for the Proteus Robot
//Written by Paine {n.a.paine@gmail.com}
//Last modified 1/15/10

/* Commands come over the serial port with the following structure
  {begin char} {opcode} {data 1} {data 2} ... {data N} {end char}
  
  SCI_Proteus.h takes care of low level protocol handling, using the {begin char} and {end char} to capture command packets
  These packets are turned over to Command.c and look like {opcode} {data 1} ... {data N}
  Command.c processes these commands
  */

#include <mc9s12dp512.h>     /* derivative information */
#include "Command.h"
#include "LED.h"
#include "Scheduler.h"
#include "Servo_PWM.h"
#include "Tach.h"
#include "MotorControl.h"
#include "SCI_PROTEUS.h"
#include "Sharp_IR.h"


    
extern char cmd_new_packet, cmd_read_idx;
extern char inFromSerial[2][MAX_CMD_LEN]; //double buffer 
char outToSerial[MAX_PACKET_LEN]; 
unsigned char mode;
unsigned char safeId;

unsigned char Active=0; 

//In safe mode this function is called periodically to see if the motors have been
//recently set.  If not, we may have lost connectivity with our host and we should
//stop moving.
unsigned char motorSafe=0;
void periodicSafeMotor(){
  if(!motorSafe){
    MC_SetVel(0);
  }
  motorSafe = 0;
}    

void InterfaceFG(void) {
  unsigned short u_16;
  short s_16;
  unsigned char u_8;
  char s_8;
  unsigned char i;
  //extern unsigned short SRF08_FL_cm, SRF08_FC_cm, SRF08_FR_cm, SRF08_RL_cm, SRF08_RC_cm, SRF08_RR_cm; //for sonar
  extern unsigned char MotorStall;
  
  if(cmd_new_packet){ //serial packet received from x86
    switch(inFromSerial[cmd_read_idx][0]){ //look at opcode (loosely based on roomba command set)
      case PROTEUS_OPCODE_START :
        LED_GREEN2 = 1;
        mode = PROTEUS_MODE_PASSIVE;
        Active = 1;
        break;  
      case PROTEUS_OPCODE_BAUD :
        //change SCI baud rate
        break;
      case PROTEUS_OPCODE_CONTROL :
        //what does this command do?
        break;
      case PROTEUS_OPCODE_SAFE :
        mode = PROTEUS_MODE_SAFE;
        safeId = Scheduler_AddEvent_hz(&periodicSafeMotor,1); //at 1hz
        break;
      case PROTEUS_OPCODE_FULL : 
        if(mode == PROTEUS_MODE_SAFE) {
          (unsigned char) Scheduler_RemoveEvent(safeId);    
        }
		mode = PROTEUS_MODE_FULL;
        break;   
      case PROTEUS_OPCODE_STOP : 
        if(mode == PROTEUS_MODE_SAFE) {
          (unsigned char) Scheduler_RemoveEvent(safeId);    
        }
        MC_SetVel(0);
        Servo_SetSteeringAngle(0);
        Active = 0;
        LED_GREEN2 = 0;
        break;
      case PROTEUS_OPCODE_DRIVE :  
        s_16 = (inFromSerial[cmd_read_idx][1] << 8) | (inFromSerial[cmd_read_idx][2] & 0xFF);
        //s_16 is wheel velocity in mm/s
        MC_SetVel(s_16);
        s_16 = (inFromSerial[cmd_read_idx][3] << 8) | (inFromSerial[cmd_read_idx][4] & 0xFF);
        //s_16 is steering angle, + for left, - for right
        Servo_SetSteeringAngle(s_16); 
        
        if(mode == PROTEUS_MODE_SAFE){
          motorSafe = 1;  
        }
        break;
      case PROTEUS_OPCODE_LEDS :
        s_8 = (inFromSerial[cmd_read_idx][1] & 0xFF);
		switch(s_8) {
			//   case 1 :
				// LED_BLUE1 = 1; 
				// break;
			//   case -1 :
				// LED_BLUE1 = 0; 
				// break;
			  case 2 :
				LED_BLUE2 = 1;
				break;
			  case -2 :
				LED_BLUE2 = 0;
				break;
			  case 3 :
				LED_GREEN1 = 1;
				break;
			  case -3 :
				LED_GREEN1 = 0;
				break;
			//   case 9 :
				// LED_GREEN2 = 1;
				// break;
			//   case -9 :
				// LED_GREEN2 = 0;
				// break;
			  case 4 :
				LED_YELLOW2 = 1;
				break;
			case -4 :
				LED_YELLOW2 = 0;
				break;
			case 5 :
				LED_ORANGE1 = 1;
				break;
			  case -5 :
				LED_ORANGE1 = 0;
				break;
			  case 6 :
				LED_ORANGE2 = 1;
				break;
			  case -6 :
				LED_ORANGE2 = 0;
				break;
			  case 7 :
				LED_RED1 = 1;
				break;
			  case -7 :
				LED_RED1 = 0;
				break;
			  case 8 :
				LED_RED2 = 1;
				break;
			  case -8 : 
				LED_RED2 = 0;
				break;
			//   case 10 :
				// LED_RED3 = 1;
				// break;
			//   case -10 : 
				// LED_RED3 = 0;
				// break;
			  case 0x0F : // 15 in decimal
				//LED_BLUE1 = 1;
				LED_BLUE2 = 1;
				LED_GREEN1 = 1;
				//LED_GREEN2 = 1;
				LED_YELLOW2 = 1;
				LED_ORANGE1 = 1;
				LED_ORANGE2 = 1;
				LED_RED1 = 1;
				LED_RED2 = 1;
				//LED_RED3 = 1;
				break;
			  case 0x10 : // 16 in decimal
				LED_BLUE1 = 0;
				LED_BLUE2 = 0;
				LED_GREEN1 = 0;
				LED_GREEN2 = 0; 
				LED_YELLOW2 = 0;
				LED_ORANGE1 = 0;
				LED_ORANGE2 = 0;
				LED_RED1 = 0;
				LED_RED2 = 0;
				LED_RED3 = 0;
				break;
			default LED_RED3 = 1; break;
			}
        break;
        /*
      case PROTEUS_OPCODE_SONAR_EN : //begin periodic sonar samples  
        
        LED_YELLOW1 = 1;
        (void) Scheduler_AddEvent(&SRF08_readSamp_FLRR,4,0); //these functions are defined in srf08.h
        Timer_mwait(28);
        (void) Scheduler_AddEvent(&SRF08_readSamp_FCRC,4,0);
        Timer_mwait(28);
        (void) Scheduler_AddEvent(&SRF08_readSamp_FRRL,4,0);
            
        break;
      case PROTEUS_OPCODE_SONAR_DE : //stop periodic sonar samples
        
        LED_YELLOW1 = 1;
        (void) Scheduler_RemoveEventForAction(&SRF08_readSamp_FLRR);
        (void) Scheduler_RemoveEventForAction(&SRF08_readSamp_FCRC);
        (void) Scheduler_RemoveEventForAction(&SRF08_readSamp_FRRL);
           
        break; */
      case PROTEUS_OPCODE_SENSORS :
        i=0;
        switch(inFromSerial[cmd_read_idx][1]){ //look at sensor packet number
          case PROTEUS_ODOMETRY_PACKET:
            s_16 = Tach_getDistanceR(); //distance in units of 1.2833 mm 
            outToSerial[i++] = s_16 >> 8;
            outToSerial[i++] = s_16 & 0x00FF;
        
            s_16 = Servo_GetSteeringAngle(); //angle in units of .0001 radians 
            outToSerial[i++] = s_16 >> 8;
            outToSerial[i++] = s_16 & 0x00FF;
            
            u_8 = MotorStall;
            outToSerial[i++] = u_8;
           
            //i should equal PROTEUS_ODOMETRY_PACKET_SIZE
          
            for(i=0; i<PROTEUS_ODOMETRY_PACKET_SIZE; i++){
              SCI_OutChar(SCI_X86, outToSerial[i]);  
            }
          
            break;
            
          case PROTEUS_IR_PACKET:
            u_16 = IR_getFL(); 
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
          
            u_16 = IR_getFC(); 
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
          
            u_16 = IR_getFR();
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
             
            u_16 = IR_getRL();    
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
           
            u_16 = IR_getRC(); 
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
          
            u_16 = IR_getRR(); 
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
			
			u_16 = IR_getE0(); 
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
          
            u_16 = IR_getE1(); 
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
          
            u_16 = IR_getE2();
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
             
            u_16 = IR_getE3();    
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
           
            u_16 = IR_getE4(); 
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
          
            u_16 = IR_getE5(); 
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
          
            //i should equal PROTEUS_IR_PACKET_SIZE 

            for(i=0; i<PROTEUS_IR_PACKET_SIZE; i++){
              SCI_OutChar(SCI_X86, outToSerial[i]);  
            }
            
            break;
               /*
          case PROTEUS_SONAR_PACKET:
            u_16 = SRF08_FL_cm; 
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
            
            u_16 = SRF08_FC_cm; 
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
            
            u_16 = SRF08_FR_cm; 
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
             
            u_16 = SRF08_RL_cm;    
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
          
            u_16 = SRF08_RC_cm; 
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
          
            u_16 = SRF08_RR_cm; 
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;
          
            //i should equal PROTEUS_SONAR_PACKET_SIZE

            for(i=0; i<PROTEUS_SONAR_PACKET_SIZE; i++){
              SCI_OutChar(SCI_X86, outToSerial[i]);  
            }
                 
            break;*/
            
          case PROTEUS_COMPASS_PACKET:           
            u_16 = Compass_getHeading(); 
            outToSerial[i++] = u_16 >> 8;
            outToSerial[i++] = u_16 & 0x00FF;

          
            //i should equal PROTEUS_COMPASS_PACKET_SIZE 

            for(i=0; i<PROTEUS_COMPASS_PACKET_SIZE; i++){
              SCI_OutChar(SCI_X86, outToSerial[i]);  
            }
            
            break;
            /*
              
          case PROTEUS_OPAQUE_PACKET: 
            outToSerial[i++] = line_detect;
          
            //i should equal PROTEUS_OPAQUE_PACKET_SIZE

            for(i=0; i<PROTEUS_OPAQUE_PACKET_SIZE; i++){
              SCI_OutChar(SCI_X86, outToSerial[i]);  
            }
            
            break;    */
          
          default: LED_RED3 = 1; break; //sensor command not recognized
        }//end sensor command
        break;    
      default: LED_RED3 = 1; break; //command not recognized 
    }
    cmd_new_packet = 0; //FG/BG sync
  }//end FG loop
}//interface function

