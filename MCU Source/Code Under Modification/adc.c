/* ************************ adc.c *****************************
 * Jonathan W. Valvano 8/15/06
 * Simple I/O routines ADC port 
 * ************************************************************ */
 
// Copyright 2006 by Jonathan W. Valvano, valvano@mail.utexas.edu 
//    You may use, edit, run or distribute this file 
//    as long as the above copyright notice remains 

#include <mc9s12dp512.h>     /* derivative information */
#include "adc.h"

//******** ADC_In *************** 
// perform 10-bit analog to digital conversion
// input: chan is 0 to 7 specifying analog channel to sample
// output: 10-bit ADC sample (left justified) 
// analog input    left justified   right justified
//  0.000               0	               0
//  0.005            0040	               1
//  0.010            0080                2
//  1.250            4000              100
//  2.500            8000							 200
//  5.000            FFC0						   3FF
// uses busy-wait synchronization
// bit 7 DJM Result Register Data Justification
//       1=right justified, 0=left justified
// bit 6 DSGN Result Register Data Signed or Unsigned Representation
//       1=signed, 0=unsigned
// bit 5 SCAN Continuous Conversion Sequence Mode
//       1=continuous, 0=single
// bit 4 MULT Multi-Channel Sample Mode
//       1=multiple channel, 0=single channel
// bit 3 0
// bit 2-0 CC,CB,CA channel number 0 to 7
// example  data = ADC_In(0x82); // samples right-justified channel 2
unsigned short ADC0_In(unsigned short chan){ 
  ATD0CTL5 = (unsigned char)chan;  // start sequence
  while((ATD0STAT1&0x01)==0){};    // wait for CCF0 
  return ATD0DR0; 
}

unsigned short ADC1_In(unsigned short chan){ 
  ATD1CTL5 = (unsigned char)chan;  // start sequence
  while((ATD1STAT1&0x01)==0){};    // wait for CCF0 
  return ATD1DR0; 
}

//******** ADC_Init *************** 
// Initialize ADC, sequence length=1, 8-bit mode
// input: none
// output: none
// errors: none
void ADC0_Init(){
  ATD0CTL2 = 0x80; // enable ADC
  // bit 7 ADPU=1 enable
  // bit 6 AFFC=0 ATD Fast Flag Clear All
  // bit 5 AWAI=0 ATD Power Down in Wait Mode
  // bit 4 ETRIGLE=0 External Trigger Level/Edge Control
  // bit 3 ETRIGP=0 External Trigger Polarity
  // bit 2 ETRIGE=0 External Trigger Mode Enable
  // bit 1 ASCIE=0 ATD Sequence Complete Interrupt Enable
  // bit 0 ASCIF=0 ATD Sequence Complete Interrupt Flag
  
  ATD0CTL3 = 0x08; 
  // bit 6 S8C =0 Sequence length = 1
  // bit 5 S4C =0 
  // bit 4 S2C =0 
  // bit 3 S1C =1
  // bit 2 FIFO=0 no FIFO mode
  // bit 1 FRZ1=0 no freeze
  // bit 0 FRZ0=0 
  
  ATD0CTL4 = 0x85; // enable ADC
  // bit 7 SRES8=0 A/D Resolution Select
  //      1 => n=8 bit resolution
  //      0 => n=10 bit resolution
  // bit 6 SMP1=0 Sample Time Select 
  // bit 5 SMP0=0 s=4
  // bit 4 PRS4=0 ATD Clock Prescaler m=5
  // bit 3 PRS3=0 ATD Clock Prescaler
  // bit 2 PRS2=1 ATD Clock Prescaler
  // bit 1 PRS1=0 ATD Clock Prescaler
  // bit 0 PRS0=1 ATD Clock Prescaler
  // Prescale = 2(m+1) = 12 cycles
  // ADC clock period = 2(m+1)/24MHz = 0.5us
  // choose m so 2(m+1)/Eperiod is between 0.5 to 2us
  // Sample time s=4
  // SMP1 SMP0  s sample time
  //  0    0    4 ADC clock periods
  //  0    1    6 ADC clock periods
  //  1    0   10 ADC clock periods
  //  1    1   18 ADC clock periods
  // ADC conversion time  2(m+1)(s+n)/24MHz = 7us
  // if multiple samples are selected, then each requires 7us
  
}

void ADC1_Init(){
  ATD1CTL2 = 0x80; // enable ADC
  // bit 7 ADPU=1 enable
  // bit 6 AFFC=0 ATD Fast Flag Clear All
  // bit 5 AWAI=0 ATD Power Down in Wait Mode
  // bit 4 ETRIGLE=0 External Trigger Level/Edge Control
  // bit 3 ETRIGP=0 External Trigger Polarity
  // bit 2 ETRIGE=0 External Trigger Mode Enable
  // bit 1 ASCIE=0 ATD Sequence Complete Interrupt Enable
  // bit 0 ASCIF=0 ATD Sequence Complete Interrupt Flag
  
  ATD1CTL3 = 0x08; 
  // bit 6 S8C =0 Sequence length = 1
  // bit 5 S4C =0 
  // bit 4 S2C =0 
  // bit 3 S1C =1
  // bit 2 FIFO=0 no FIFO mode
  // bit 1 FRZ1=0 no freeze
  // bit 0 FRZ0=0 
  
  ATD1CTL4 = 0x85; // enable ADC
  // bit 7 SRES8=0 A/D Resolution Select
  //      1 => n=8 bit resolution
  //      0 => n=10 bit resolution
  // bit 6 SMP1=0 Sample Time Select 
  // bit 5 SMP0=0 s=4
  // bit 4 PRS4=0 ATD Clock Prescaler m=5
  // bit 3 PRS3=0 ATD Clock Prescaler
  // bit 2 PRS2=1 ATD Clock Prescaler
  // bit 1 PRS1=0 ATD Clock Prescaler
  // bit 0 PRS0=1 ATD Clock Prescaler
  // Prescale = 2(m+1) = 12 cycles
  // ADC clock period = 2(m+1)/24MHz = 0.5us
  // choose m so 2(m+1)/Eperiod is between 0.5 to 2us
  // Sample time s=4
  // SMP1 SMP0  s sample time
  //  0    0    4 ADC clock periods
  //  0    1    6 ADC clock periods
  //  1    0   10 ADC clock periods
  //  1    1   18 ADC clock periods
  // ADC conversion time  2(m+1)(s+n)/24MHz = 7us
  // if multiple samples are selected, then each requires 7us
  
}
