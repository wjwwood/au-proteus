// filename ******** PWM.c **************
// Servo driver for HITEC servos and 9S12DP512s
// Written by Nicholas Paine
// Last modified 3/27/08
// Modified by Valvano 7/24/08 making Channel 1 94.1kHz

#include <mc9s12dp512.h>     /* derivative information */
#include "Servo_PWM.h"
#include "LED.h"
#include "Scheduler.h"

unsigned char ActualSetpoint1 = 128, ActualSetpoint2 = 128, ActualSetpoint3 = 128, ActualSetpoint4 = 128;
unsigned char TargetSetpoint1 = 128, TargetSetpoint2 = 128, TargetSetpoint3 = 128, TargetSetpoint4 = 128;
unsigned char MaxChangePerPeriod = 6;

unsigned char Servo1Enabled, Servo2Enabled, Servo3Enabled, Servo4Enabled;
unsigned char updateServos=0;

void servoPeriodicBG(void){
  updateServos = 1;    
}

// ---------Servo_Init---------
// enable PWM channels 0-7
// inputs: none
// outputs: none
// concatenates 2/3, 4/5, 6/7
void Servo_Init(void){
  DDRT |= 0x04;     //set directional bit PT2--output
  PWME = 0x00;      // disable channels 0-7
  PWMPOL |= 0xFF;   // PP0-7 high then low
// original code, removed by Valvano 7/24/08
//  PWMCLK |= 0xFF;  // use clock SA,SB
// new code, Valvano 7/24/08
  PWMCLK |= 0xFC;   // use clock SA,SB on channels 7-2
  PWMCLK &= ~0x02;  // use clock A on channel1 (motor)
// end of new code
  PWMPRCLK = 0x00;  // A,B = ECLK = 24MHz 
  PWMCAE = 0x00;    // all left aligned
  PWMCTL = 0xF0;    // 4 16-bit channels
  PWMSCLA = 12;     // 1MHz SA clk
  PWMSCLB = 12;     // 1MHz SB clk
//  PWMPER1 = 255;    // Channel1 is 255*41.667ns=10.6us, 24MHz/255=94.1kHz
  PWMPER01 = 2047;    // Channel1 is 2047*41.667ns=85.3us, 24MHz/2047=11.7kHz
  PWMPER23 = SERVO_PULSE_PERIOD;  
  PWMPER45 = SERVO_PULSE_PERIOD;  
  PWMPER67 = SERVO_PULSE_PERIOD;
  Motor_set(MOTOR_STOP);
  _Servo2_set(128+SERVO_CENTER_CALIB);
  _Servo3_set(128);
  _Servo4_set(128); 
  PWME = 0xFF;       // enable channels 0-7
  //Servo1Enabled = 1; // ??Nick??? is this a bug?? should be 0??
  Servo2Enabled = 1;
  Servo3Enabled = 1;
  Servo4Enabled = 1;
  
  (void) Scheduler_AddEvent_hz(&servoPeriodicBG, SERVO_UPDATE_HZ);
}

void ServoPeriodicFG(void){ //periodic interrupt to update servo positions
  short as2, ts2, as3, ts3, as4, ts4, mcpp;

  if(updateServos){
    as2 = (short) ActualSetpoint2;
    ts2 = (short) TargetSetpoint2;
    as3 = (short) ActualSetpoint3;
    ts3 = (short) TargetSetpoint3;
    as4 = (short) ActualSetpoint4;
    ts4 = (short) TargetSetpoint4;
    mcpp = (short) MaxChangePerPeriod;
  
   if(as2 - mcpp > ts2) ActualSetpoint2 = ActualSetpoint2 - MaxChangePerPeriod;
    else if(as2 + mcpp < ts2) ActualSetpoint2 = ActualSetpoint2 + MaxChangePerPeriod;
    else ActualSetpoint2 = TargetSetpoint2;
  
    if(as3 - mcpp > ts3) ActualSetpoint3 = ActualSetpoint3 - MaxChangePerPeriod;
    else if(as3 + mcpp < ts3) ActualSetpoint3 = ActualSetpoint3 + MaxChangePerPeriod;
    else ActualSetpoint3 = TargetSetpoint3;
  
    if(as4 - mcpp > ts4) ActualSetpoint4 = ActualSetpoint4 - MaxChangePerPeriod;
    else if(as4 + mcpp < ts4) ActualSetpoint4 = ActualSetpoint4 + MaxChangePerPeriod;
    else ActualSetpoint4 = TargetSetpoint4;
  
    _Servo2_set(ActualSetpoint2);   //128 straight
    _Servo3_set(ActualSetpoint3);
    _Servo4_set(ActualSetpoint4);
 
    updateServos = 0;
  }
}

void Motor_disable(){
  PWME &= ~0x02; // disable channel 1
  Servo1Enabled = 0; 
}

void Motor_enable(){
  PWME |= 0x02; //enable channel 1
  Servo1Enabled = 1; 
}

void Servo2_disable(){
  PWME &= ~0x08; //disable channel 3
  Servo2Enabled = 0; 
}

void Servo2_enable(){
  PWME |= 0x08; //enable channel 3
  Servo2Enabled = 1; 
}

void Servo3_disable(){
  PWME &= ~0x20; //disable channel 5
  Servo3Enabled = 0; 
}

void Servo3_enable(){
  PWME |= 0x20; //enable channel 5
  Servo3Enabled = 1; 
}

void Servo4_disable(){
  PWME &= ~0x80; //disable channel 7
  Servo4Enabled = 0; 
}

void Servo4_enable(){
  PWME |= 0x80; //enable channel 7
  Servo4Enabled = 1; 
}

unsigned char Motor_status(){
 return Servo1Enabled; 
}

unsigned char Servo2_status(){
 return Servo2Enabled; 
}

unsigned char Servo3_status(){
 return Servo3Enabled; 
}

unsigned char Servo4_status(){
 return Servo4Enabled; 
}
// 0 stopped
// 2047 full speed
// Input is -2048 -> +2048
void Motor_set(short setpoint){   // motor
  if(setpoint < 0) { 
    PTT_PTT2 = 0;
    setpoint *= -1;
  }
  else
    PTT_PTT2 = 1;
  
  PWMDTY01 = setpoint;
}
// 128 straight
void _Servo2_set(unsigned char setpoint){   // steering
  unsigned short duty16;
  duty16 = (setpoint * 75 / 16) + 900;
  
  PWMDTY23 = duty16;
}

void _Servo3_set(unsigned char setpoint){
  unsigned short duty16;
  duty16 = (setpoint * 75 / 16) + 900;
  
  PWMDTY45 = duty16;
}

void _Servo4_set(unsigned char setpoint){
  unsigned short duty16;
  duty16 = (setpoint * 75 / 16) + 900;
  
  PWMDTY67 = duty16;
}
 /*
void Servo_set1(unsigned char setpoint){
  TargetSetpoint1 = setpoint; 
}  */

void Servo_set2(unsigned char setpoint){
  TargetSetpoint2 = setpoint; 
}

void Servo_set3(unsigned char setpoint){
  TargetSetpoint3 = setpoint; 
}

void Servo_set4(unsigned char setpoint){
  TargetSetpoint4 = setpoint; 
}

unsigned short Motor_get(void){
  return PWMDTY01; 
}

unsigned char Servo_get2(void){
  return TargetSetpoint2; 
}

unsigned char Servo_get3(void){
  return TargetSetpoint3; 
}

unsigned char Servo_get4(void){
  return TargetSetpoint4; 
}      

// ************Servo_GetSteeringAngle**************************
// Gives steering angle (.0001 radians) for current servo position
// Piecewise linear conversion (left and right)
// Input:  none
// Output: signed .0001 radian steering angle for current servo position
//         +: left, -: right
short Servo_GetSteeringAngle(){
   unsigned short x0,x2;
   short y0,y1,y3;
   short temp;
   x0 = Servo_get2();
   x2 = 128 + SERVO_CENTER_CALIB;
   
   if(x0 <= x2){ //steering left
    
     y1 = SERVO_STEERING_LEFT_CALIB;
      
//y0 = -y1*x0/x2 + y1   
asm  ldd   y1
asm  coma
asm  comb
asm  addd  #1
asm  ldy   x0
asm  emuls   
asm  ldx   x2
asm  edivs
asm  tfr   y,d
asm  addd  y1
asm  std   y0 
   } 
   else { //steering right
   
     y3 = SERVO_STEERING_RIGHT_CALIB;
     temp = 255 - x2;
   
//y0 = y3*(x0-x2)/(255-x2)
asm  ldd   x0
asm  subd  x2
asm  ldy   y3
asm  emuls
asm  ldx   temp
asm  edivs
asm  sty   y0 
   }

   return y0;
}

// ************Servo_SetSteeringAngle**************************
// Sets steering angle (.0001 radians) 
// Piecewise linear conversion (left and right)
// Input:  Steering angle (.0001 radians)
// Output: none
//         +: left, -: right
void Servo_SetSteeringAngle(short angle){
   unsigned short x0,x2;
   short y0,y1,y3;
   y0 = angle;
   x2 = 128 + SERVO_CENTER_CALIB;
   
   if(y0 >= 0) { //left
   
     y1 = SERVO_STEERING_LEFT_CALIB;
     if(y0 > y1) y0 = y1;
   
//x0 = (y1-y0)*x2/y1
asm  ldd   y1
asm  subd  y0
asm  ldy   x2
asm  emuls
asm  ldx   y1
asm  edivs
asm  sty   x0
   } 
   else { //right
   
     y3 = SERVO_STEERING_RIGHT_CALIB;
     if(y0 < y3) y0 = y3;
     
//x0 = (255-x2)*y0/y3 + x2
asm  ldd   #255
asm  subd  x2
asm  ldy   y0
asm  emuls
asm  ldx   y3
asm  edivs
asm  tfr   y,d
asm  addd  x2
asm  std   x0
   }
   
   if(x0 < 20) x0 = 20; 
   if(x0 > 235) x0 = 235;
   Servo_set2((unsigned char)x0);
}

/*
const short steer_tab[256] = { //neccessary for accurate odometry calculations, values are in units of .0001 radians
32767	,  //  -128
32767	,  //  -127
32767	,  //  -126
32767	,  //  -125
32767	,  //  -124
32767	,  //  -123
32767	,  //  -122
32767	,  //  -121  //should never get set here
4909	,  //  -120  (first measured value)
4868	,  //  -119
4827	,  //  -118
4786	,  //  -117
4745	,  //  -116
4705	,  //  -115
4664	,  //  -114
4623	,  //  -113
4582	,  //  -112
4541	,  //  -111
4500	,  //  -110
4459	,  //  -109
4418	,  //  -108
4377	,  //  -107
4336	,  //  -106
4295	,  //  -105
4255	,  //  -104
4214	,  //  -103
4173	,  //  -102
4132	,  //  -101
4091	,  //  -100
4050	,  //  -99
4009	,  //  -98
3968	,  //  -97
3927	,  //  -96
3886	,  //  -95
3845	,  //  -94
3805	,  //  -93
3764	,  //  -92
3723	,  //  -91
3682	,  //  -90
3641	,  //  -89
3600	,  //  -88
3559	,  //  -87
3518	,  //  -86
3477	,  //  -85
3436	,  //  -84
3395	,  //  -83
3355	,  //  -82
3314	,  //  -81
3273	,  //  -80
3232	,  //  -79
3191	,  //  -78
3150	,  //  -77
3109	,  //  -76
3068	,  //  -75
3027	,  //  -74
2986	,  //  -73
2945	,  //  -72
2905	,  //  -71
2864	,  //  -70
2823	,  //  -69
2782	,  //  -68
2741	,  //  -67
2700	,  //  -66
2659	,  //  -65
2618	,  //  -64
2577	,  //  -63
2536	,  //  -62
2495	,  //  -61
2455	,  //  -60
2414	,  //  -59
2373	,  //  -58
2332	,  //  -57
2291	,  //  -56
2250	,  //  -55
2209	,  //  -54
2168	,  //  -53
2127	,  //  -52
2086	,  //  -51
2045	,  //  -50
2005	,  //  -49
1964	,  //  -48
1923	,  //  -47
1882	,  //  -46
1841	,  //  -45
1800	,  //  -44
1759	,  //  -43
1718	,  //  -42
1677	,  //  -41
1636	,  //  -40
1595	,  //  -39
1555	,  //  -38
1514	,  //  -37
1473	,  //  -36
1432	,  //  -35
1391	,  //  -34
1350	,  //  -33
1309	,  //  -32
1268	,  //  -31
1227	,  //  -30
1186	,  //  -29
1145	,  //  -28
1105	,  //  -27
1064	,  //  -26
1023	,  //  -25
982	,  //  -24
941	,  //  -23
900	,  //  -22
859	,  //  -21
818	,  //  -20
777	,  //  -19
736	,  //  -18
695	,  //  -17
655	,  //  -16
614	,  //  -15
573	,  //  -14
532	,  //  -13
491	,  //  -12
450	,  //  -11
409	,  //  -10
368	,  //  -9
327	,  //  -8
286	,  //  -7
245	,  //  -6
205	,  //  -5
164	,  //  -4
123	,  //  -3
82	,  //  -2
41	,  //  -1
0	,  //  0
-39	,  //  1
-78	,  //  2
-118	,  //  3
-157	,  //  4
-196	,  //  5
-235	,  //  6
-274	,  //  7
-313	,  //  8
-353	,  //  9
-392	,  //  10
-431	,  //  11
-470	,  //  12
-509	,  //  13
-548	,  //  14
-588	,  //  15
-627	,  //  16
-666	,  //  17
-705	,  //  18
-744	,  //  19
-783	,  //  20
-823	,  //  21
-862	,  //  22
-901	,  //  23
-940	,  //  24
-979	,  //  25
-1018	,  //  26
-1058	,  //  27
-1097	,  //  28
-1136	,  //  29
-1175	,  //  30
-1214	,  //  31
-1253	,  //  32
-1293	,  //  33
-1332	,  //  34
-1371	,  //  35
-1410	,  //  36
-1449	,  //  37
-1488	,  //  38
-1528	,  //  39
-1567	,  //  40
-1606	,  //  41
-1645	,  //  42
-1684	,  //  43
-1723	,  //  44
-1763	,  //  45
-1802	,  //  46
-1841	,  //  47
-1880	,  //  48
-1919	,  //  49
-1958	,  //  50
-1998	,  //  51
-2037	,  //  52
-2076	,  //  53
-2115	,  //  54
-2154	,  //  55
-2193	,  //  56
-2233	,  //  57
-2272	,  //  58
-2311	,  //  59
-2350	,  //  60
-2389	,  //  61
-2428	,  //  62
-2468	,  //  63
-2507	,  //  64
-2546	,  //  65
-2585	,  //  66
-2624	,  //  67
-2663	,  //  68
-2703	,  //  69
-2742	,  //  70
-2781	,  //  71
-2820	,  //  72
-2859	,  //  73
-2898	,  //  74
-2938	,  //  75
-2977	,  //  76
-3016	,  //  77
-3055	,  //  78
-3094	,  //  79
-3133	,  //  80
-3173	,  //  81
-3212	,  //  82
-3251	,  //  83
-3290	,  //  84
-3329	,  //  85
-3368	,  //  86
-3408	,  //  87
-3447	,  //  88
-3486	,  //  89
-3525	,  //  90
-3564	,  //  91
-3603	,  //  92
-3643	,  //  93
-3682	,  //  94
-3721	,  //  95
-3760	,  //  96
-3799	,  //  97
-3838	,  //  98
-3878	,  //  99
-3917	,  //  100
-3956	,  //  101
-3995	,  //  102
-4034	,  //  103
-4073	,  //  104
-4113	,  //  105
-4152	,  //  106
-4191	,  //  107
-4230	,  //  108
-4269	,  //  109
-4308	,  //  110
-4348	,  //  111
-4387	,  //  112
-4426	,  //  113
-4465	,  //  114
-4504	,  //  115
-4543	,  //  116
-4583	,  //  117
-4622	,  //  118
-4661	,  //  119
-4700	,  //  120  (last measured value)
-32768	,  //  121 //should never get set here
-32768	,  //  122
-32768	,  //  123
-32768	,  //  124
-32768	,  //  125
-32768	,  //  126
-32768	,  //  127
};
*/

/*
// ************Servo_GetSteeringAngle**************************
// Gives steering angle (.0001 radians) for current servo position
// Input:  none
// Output: signed .0001 radian steering angle for current servo position
//         +: left, -: right
short Servo_GetSteeringAngle(){
   unsigned char set2;
   set2 = Servo_get2() + SERVO_CENTER_OFFSET;
   
   if(set2 < 8) return steer_tab[8];
   else if(set2 > 248) return steer_tab[248];
   else return steer_tab[set2];

}

// ************Servo_SetSteeringAngle**************************
// Sets steering angle (.0001 radians) 
// Input:  Steering angle (.0001 radians)
// Output: none
//         +: left, -: right
void Servo_SetSteeringAngle(short angle){
  unsigned char i;
  i=0; 
  while(steer_tab[i] >= angle){
    i += 1;
  }
  if(i-1-SERVO_CENTER_OFFSET < 8) Servo_set2(8); //max left
  else if(i-1-SERVO_CENTER_OFFSET > 248) Servo_set2(248); //max right
  else Servo_set2(i-1-SERVO_CENTER_OFFSET); 
}
*/

/*
***TO CALCULATE SERVO_SET FUNCTIONS***

SERVO_INIT INFO
using sa
a = bus clock
sa = a / (2*PWMSCLA)
pwm period = sa period * 2 * pwmper

a = 24mhz (eclk)
sa = 24mhz / 24 = 1mhz
pwm period = 5 e -7 * 2 * 20000 seems like we lost a factor of two somewhere...but it works

SETPOINT CALCULATION
.9 ms is servo left, 2.1 ms is servo right
min duty16 = .9ms/20ms * pwmper, max duty16 = 2.1ms/20ms*pwmper
range = maxduty16 - minduty16
range/255 = step value (need to find a 16-bit friendly way to calculate step value)
output = setpoint * stepvalue + minduty16

CURRENT USAGE
minduty16 = 900, maxduty16 = 2100
1200
1200/255 = 4.688 = 75/16
setpoint * 75 / 16 + 900
*/

