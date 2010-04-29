/* ************************ adc.h *****************************
 * Jonathan W. Valvano   8/15/06
 * Simple I/O routines ADC port 
 * ************************************************************ */
 
// Copyright 2006 by Jonathan W. Valvano, valvano@mail.utexas.edu 
//    You may use, edit, run or distribute this file 
//    as long as the above copyright notice remains 


//******** ADC_Init *************** 
// Initialize ADC, sequence length=1, 10-bit mode
// input: none
// output: none
// errors: none
void ADC0_Init(void);

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
#ifndef _ADC_H
#define _ADC_H 1

unsigned short ADC0_In(unsigned short chan);

#endif //_ADC_H