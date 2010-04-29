// filename **************SCIA_FIFO.C  *****************************
// Interrupting I/O routines to 9S12DP512 serial port 
// Jonathan W. Valvano 8/1/08
// Composed of files written by Jonathan W. Valvano
// Multiport functionality added by Paine on 4/10/09

//  This example accompanies the books
//   "Embedded Microcomputer Systems: Real Time Interfacing",
//        Thompson, copyright (c) 2006,
//   "Introduction to Embedded Microcomputer Systems: 
//    Motorola 6811 and 6812 Simulation", Brooks-Cole, copyright (c) 2002

// Copyright 2008 by Jonathan W. Valvano, valvano@mail.utexas.edu 
//    You may use, edit, run or distribute this file 
//    as long as the above copyright notice remains 
// Modified by EE345L students Charlie Gough && Matt Hawk
// Modified by EE345M students Agustinus Darmawan + Mingjie Qiu

#include <mc9s12dp512.h>     /* derivative information */
#include "SCI_FIFO.h" 


#define SCIPORTS 2 //9S12DP512 has 2 ports

// Pointer implementation of the transmit FIFO
#define TXFIFOSIZE 200
// the number of 8 bit bytes in the Fifo
// increased large enough to prevent full errors
//   the FIFO is full when it has FifoSize-1 bytes 

   
char static volatile *TxPutPt[SCIPORTS];    /* Pointer of where to put next */
char static volatile *TxGetPt[SCIPORTS];    /* Pointer of where to get next */
                  /* FIFO is empty if PutPt=GetPt */
                  /* FIFO is full  if PutPt+1=GetPt */
char static TxFifo[SCIPORTS][TXFIFOSIZE];     /* The statically allocated fifo data */


  
/*-----------------------TxFifo_Init----------------------------
  Initialize fifo to be empty
  Inputs: SCI port
  Outputs: none */
void TxFifo_Init(unsigned char port) {unsigned char SaveCCR;
/* save previous interrupt enable */
asm  tpa  
asm  staa SaveCCR 
asm  sei          /* make atomic */
  TxPutPt[port]=TxGetPt[port]=&TxFifo[port][0];   /* Empty when TxPutPt==TxGetPt */
asm  ldaa SaveCCR
asm  tap          /* end critical section */
}

/*-----------------------TxFifo_Put----------------------------
  Enter one character into the fifo
  Inputs: SCI port, 8 bit data
  Outputs: true if data is properly saved */
int TxFifo_Put (unsigned char port, char data) {  
char volatile *tempPt;
  tempPt = TxPutPt[port];
  *(tempPt) = data;                 /* try to Put data into fifo */
  tempPt++;
  if(tempPt == &TxFifo[port][TXFIFOSIZE]){   /* need to wrap?*/
    tempPt = &TxFifo[port][0];
  }
  if(tempPt == TxGetPt[port]){
    return(0);             /* Failed, fifo was full */
  }
  else{
    TxPutPt[port] = tempPt;      /* Success, so update pointer */
    return(1);
  }
}

/*-----------------------TxFifo_Get----------------------------
  Remove one character from the fifo
  Inputs: SCI port, pointer to place to save 8 bit data
  Outputs: true if data is valid */
int TxFifo_Get(unsigned char port, char *datapt){
  if(TxPutPt[port] == TxGetPt[port] ){
    return(0);     /* Empty if PutPt=GetPt */
  }
  else {
    *datapt = *(TxGetPt[port]);
    TxGetPt[port]++;
    if(TxGetPt[port] == &TxFifo[port][TXFIFOSIZE]){ 
      TxGetPt[port] = &TxFifo[port][0];
    }
    return(1); 
  }
}

/*-----------------------TxFifo_Status----------------------------
  Check the status of the fifo
  Inputs: SCI port
  Outputs: true if there is room in the FIFO */
int TxFifo_Status(unsigned char port) { 
char volatile *nextPt;
  nextPt = TxPutPt[port];
  nextPt++;                         /* next pointer */
  if(nextPt == &TxFifo[port][TXFIFOSIZE]){ /* need to wrap?*/
    nextPt = &TxFifo[port][0];
  }
  return (nextPt != TxGetPt[port]);
}

/*-----------------------TxFifo_Size----------------------------
  Check the status of the fifo
  Inputs: SCI port
  Outputs: number of elements currently stored in the FIFO */
unsigned short TxFifo_Size(unsigned char port) { 
unsigned short size;
  if(TxPutPt[port]>=TxGetPt[port]){
    size = TxPutPt[port]-TxGetPt[port];
  } else{
    size = TXFIFOSIZE+TxPutPt[port]-TxGetPt[port];
  }
  return size;
}



// Pointer implementation of the receive FIFO
#define RXFIFOSIZE 100
/* Number of characters in the Fifo
     the FIFO is full when it has FifoSize-1 characters */

char static volatile *RxPutPt[SCIPORTS];    /* Pointer of where to put next */
char static volatile *RxGetPt[SCIPORTS];    /* Pointer of where to get next */
                  /* FIFO is empty if PutPt=GetPt */
                  /* FIFO is full  if PutPt+1=GetPt */
char static RxFifo[SCIPORTS][RXFIFOSIZE];     /* The statically allocated fifo data */
 
/*-----------------------RxFifo_Init----------------------------
  Initialize fifo to be empty
  Inputs: SCI port
  Outputs: none */
void RxFifo_Init(unsigned char port){unsigned char SaveCCR;
/* save previous interrupt enable */
asm  tpa  
asm  staa SaveCCR 
asm  sei          /* make atomic */
  RxPutPt[port]=RxGetPt[port]=&RxFifo[port][0];   /* Empty when PutPt=GetPt */
asm  ldaa SaveCCR
asm  tap          /* end critical section */
}

/*-----------------------RxFifo_Put----------------------------
  Enter one character into the fifo
  Inputs: SCI port, 8-bit data
  Outputs: true if data is properly saved
  Since this is called by interrupt handlers no sei,cli*/
int RxFifo_Put(unsigned char port, char data){
char volatile *tempPt;
  tempPt = RxPutPt[port];
  *(tempPt) = data;              /* try to Put data into fifo */
  tempPt++;
  if(tempPt == &RxFifo[port][RXFIFOSIZE]){ /* need to wrap?*/
    tempPt = &RxFifo[port][0];
  }
  if(tempPt == RxGetPt[port]){
    return(0);             /* Failed, fifo was full */
  }     
  else{
    RxPutPt[port] = tempPt;      /* Success, so update pointer */
    return(1);
  }
}

/*-----------------------RxFifo_Get----------------------------
  Remove one character from the fifo
  Inputs: SCI port, pointer to place to save 8-bit data
  Outputs: true if data is valid */
int RxFifo_Get(unsigned char port, char *datapt){ 
  if(RxPutPt[port] == RxGetPt[port]){
    return(0);     /* Empty if PutPt=GetPt */
  }
  else{
    *datapt = *(RxGetPt[port]);
    RxGetPt[port]++;
    if(RxGetPt[port] == &RxFifo[port][RXFIFOSIZE]){ 
      RxGetPt[port] = &RxFifo[port][0];
    }
    return(1); 
  }
}

/*-----------------------RxFifo_Status----------------------------
  Check the status of the fifo
  Inputs: SCI port
  Outputs: true if there is any data in the fifo */
int RxFifo_Status(unsigned char port){ 
  return (RxPutPt[port] != RxGetPt[port]);
}

/*-----------------------RxFifo_Size----------------------------
  Check the status of the fifo
  Inputs: SCI port
  Outputs: number of elements currently stored in the FIFO */
unsigned short RxFifo_Size(unsigned char port) { 
unsigned short size;
  if(RxPutPt[port]>=RxGetPt[port]){
    size = RxPutPt[port]-RxGetPt[port];
  } else{
    size = RXFIFOSIZE+RxPutPt[port]-RxGetPt[port];
  }
  return size;
}

// Interrupting I/O routines to 9S12DP512 serial port 
#define RDRF 0x20   // Receive Data Register Full Bit
#define TDRE 0x80   // Transmit Data Register Empty Bit
#if HISTOGRAM
unsigned short RxHistogram[SCIPORTS][RXFIFOSIZE]; // collected before put
unsigned short TxHistogram[SCIPORTS][TXFIFOSIZE]; // collected before put
#endif

//-------------------------SCI_Init------------------------
// Initialize Serial port
// Input: baudRate is the baud rate in bits/sec
// Output: none
// SCI0BDL=1500000/baudRate, for example
// baudRate =    9600 bits/sec  SCI0BDL=156
// baudRate =   19200 bits/sec  SCI0BDL=78
// baudRate =   38400 bits/sec  SCI0BDL=39
// baudRate =  115200 bits/sec  SCI0BDL=13
// assumes a module clock frequency of 24 MHz
// baud rate must be faster than 5900 bits/sec
void SCI_Init(unsigned char port, unsigned long baudRate){
  if(port==0) {
    RxFifo_Init(port);
    TxFifo_Init(port);
    //SCI0BDH = 0;   // br=MCLK/(16*BaudRate)
   
  
    //SCI0BDL = 500000L/baudRate;   //ECLK = 8mhz
    SCI0BD = 1500000L/baudRate;	 //ECLK = 24mhz
    //SCI0BDL = 2000000L/baudRate;	 //ECLK = 32mhz
  
    SCI0CR1 = 0;
  /* bit value meaning
      7   0    LOOPS, no looping, normal
      6   0    WOMS, normal high/low outputs
      5   0    RSRC, not appliable with LOOPS=0
      4   0    M, 1 start, 8 data, 1 stop
      3   0    WAKE, wake by idle (not applicable)
      2   0    ILT, short idle time (not applicable)
      1   0    PE, no parity
      0   0    PT, parity type (not applicable with PE=0) */ 
    SCI0CR2 = 0x2C; 
  /* bit value meaning
      7   0    TIE, no transmit interrupts on TDRE
      6   0    TCIE, no transmit interrupts on TC
      5   1    RIE, no receive interrupts on RDRF
      4   0    ILIE, no interrupts on idle
      3   1    TE, enable transmitter
      2   1    RE, enable receiver
      1   0    RWU, no receiver wakeup
      0   0    SBK, no send break */ 
  }
  if(port==1) {
    RxFifo_Init(port);
    TxFifo_Init(port);
    //SCI1BDH = 0;   // br=MCLK/(16*BaudRate)
   
  
    //SCI0BDL = 500000L/baudRate;   //ECLK = 8mhz
    SCI1BD = 1500000L/baudRate;	 //ECLK = 24mhz
    //SCI0BDL = 2000000L/baudRate;	 //ECLK = 32mhz
  
    SCI1CR1 = 0;
  /* bit value meaning
      7   0    LOOPS, no looping, normal
      6   0    WOMS, normal high/low outputs
      5   0    RSRC, not appliable with LOOPS=0
      4   0    M, 1 start, 8 data, 1 stop
      3   0    WAKE, wake by idle (not applicable)
      2   0    ILT, short idle time (not applicable)
      1   0    PE, no parity
      0   0    PT, parity type (not applicable with PE=0) */ 
    //SCI1CR2 = 0x2C;
    //SCI1CR2 = 0x0C; 
  /* bit value meaning
      7   0    TIE, no transmit interrupts on TDRE
      6   0    TCIE, no transmit interrupts on TC
      5   1    RIE, no receive interrupts on RDRF
      4   0    ILIE, no interrupts on idle
      3   1    TE, enable transmitter
      2   1    RE, enable receiver
      1   0    RWU, no receiver wakeup
      0   0    SBK, no send break */ 
  }
  asm cli   /* enable interrupts */
}
    
//-------------------------SCI_InChar------------------------
// Wait for new serial port input, interrupt synchronization
// The input is not echoed
// Input: SCI port
// Output: ASCII code for key typed
char SCI_InChar(unsigned char port){ 
char letter;
  while (RxFifo_Get(port, &letter) == 0){};
  return(letter);
}

//-------------------------SCI_OutChar------------------------
// Wait for buffer to be empty, output 8-bit to serial port
// interrupt synchronization
// Input: SCI port, 8-bit data to be transferred
// Output: none
void SCI_OutChar(unsigned char port, char data){
#if HISTOGRAM
  TxHistogram[port][TxFifo_Size(port)]++;    // analysis of FIFO 
#endif
  while (TxFifo_Put(port, data) == 0){}; // spin if TxFifo is full
  if(port==0) SCI0CR2 = 0xAC; /* arm TDRE */
  if(port==1) SCI1CR2 = 0xAC; /* arm TDRE */
}
    
//-------------------------SCI_InStatus--------------------------
// Checks if new input is ready, TRUE if new input is ready
// Input: SCi port
// Output: TRUE if a call to InChar will return right away with data
//         FALSE if a call to InChar will wait for input
char SCI_InStatus(unsigned char port){  
  return((char) RxFifo_Status(port));
}

//-----------------------SCI_OutStatus----------------------------
// Checks if there is room in the FIFO, 
// TRUE if a call to OutChar will not spin
char SCI_OutStatus(unsigned char port){
  return((char) TxFifo_Status(port));
}

/*---------------------SCI0Handler---------------------------*/
// RDRF set on new receive data
// TDRE set on an empty transmit data register
interrupt 20 void SCI0Handler(void){ char data;
  //PTT_PTT3 = 1; 
  if(SCI0SR1 & RDRF){ 
#if HISTOGRAM
    RxHistogram[0][RxFifo_Size(0)]++;
#endif
    RxFifo_Put(0,SCI0DRL); // clears RDRF
  }
  if((SCI0CR2&0x80)&&(SCI0SR1&TDRE)){
    if(TxFifo_Get(0,&data)){
      SCI0DRL = data;   // clears TDRE
    }
    else{
      SCI0CR2 = 0x2c;   // disarm TDRE
    }
  }
  //PTT_PTT3 = 0; 
}

/*---------------------SCI1Handler---------------------------*/
// RDRF set on new receive data
// TDRE set on an empty transmit data register
interrupt 21 void SCI1Handler(void){ char data; 
 //PTT_PTT3 = 1;
 if(SCI1SR1 & RDRF){ 
#if HISTOGRAM
    RxHistogram[1][RxFifo_Size(1)]++;
#endif
    RxFifo_Put(1,SCI1DRL); // clears RDRF
  }
  if((SCI1CR2&0x80)&&(SCI1SR1&TDRE)){
    if(TxFifo_Get(1,&data)){
      SCI1DRL = data;   // clears TDRE
    }
    else{
      SCI1CR2 = 0x2c;   // disarm TDRE
    }
  } 
  //PTT_PTT3 = 0;
}

//-------------------------SCI_OutString------------------------
// Output String (NULL termination), interrupt synchronization
// Input: SCI port, pointer to a NULL-terminated string to be transferred
// Output: none
void SCI_OutString(unsigned char port, char *pt){
  while(*pt){
    SCI_OutChar(port, *pt);
    pt++;
  }
}

//----------------------SCI_InUDec-------------------------------
// InUDec accepts ASCI0I input in unsigned decimal format
//     and converts to a 16 bit unsigned number
//     valid range is 0 to 65535
// Input: SCI port
// Output: 16-bit unsigned number
// If you enter a number above 65535, it will truncate without an error
// Backspace will remove last digit typed
unsigned short SCI_InUDec(unsigned char port){	
unsigned short number=0, length=0;
char character;
  character = SCI_InChar(port);	
  while(character!=CR){ // accepts until carriage return input
// The next line checks that the input is a digit, 0-9.
// If the character is not 0-9, it is ignored and not echoed
    if((character>='0') && (character<='9')) {
      number = 10*number+(character-'0');   // this line overflows if above 65535
      length++;
      SCI_OutChar(port, character);
    } 
// If the input is a backspace, then the return number is
// changed and a backspace is outputted to the screen
    else if((character==BS) && length){
      number /= 10;
      length--;
      SCI_OutChar(port, character);
    }
    character = SCI_InChar(port);	
  }
  return number;
}


//-----------------------SCI_OutUDec-----------------------
// Output a 16-bit number in unsigned decimal format
// Input: SCI port, 16-bit number to be transferred
// Output: none
// Variable format 1-5 digits with no space before or after
void SCI_OutUDec(unsigned char port, unsigned short n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCI0I string 
  if(n >= 10){
    SCI_OutUDec(port, n/10);
    n = n%10;
  }
  SCI_OutChar(port, n+'0'); /* n is between 0 and 9 */
}

//-----------------------SCI_OutUDec32-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: SCI port, 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void SCI_OutUDec32(unsigned char port, unsigned long n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCI0I string 
  if(n >= 10){
    SCI_OutUDec32(port, n/10);
    n = n%10;
  }
  SCI_OutChar(port, n+'0'); /* n is between 0 and 9 */
}

//-----------------------SCI_OutSDec-----------------------
// Output a 16-bit number in signed decimal format
// Input: SCI port, 16-bit signed number to be transferred
// Output: none
// Variable format 2-6 digits with no space before or after
void SCI_OutSDec(unsigned char port, short n){
  if(n < 0){
    SCI_OutChar(port, '-'); /* negative */
    n = -n;
  }
  SCI_OutUDec(port, n);
}


//---------------------SCI_InUHex----------------------------------------
// Accepts ASCI0I input in unsigned hexadecimal (base 16) format
// Input: SCI port
// Output: 16-bit unsigned number
// No '$' or '0x' need be entered, just the 1 to 4 hex digits
// It will convert lower case a-f to uppercase A-F
//     and converts to a 16 bit unsigned number
//     value range is 0 to FFFF
// If you enter a number above FFFF, it will truncate without an error
// Backspace will remove last digit typed
unsigned short SCI_InUHex(unsigned char port){	
unsigned short number=0, digit, length=0;
char character;
  character = SCI_InChar(port);
  while(character != CR){	
    digit = 0x10; // assume bad
    if((character>='0') && (character<='9')){
      digit = character-'0';
    }
    else if((character>='A') && (character<='F')){ 
      digit = (character-'A')+0xA;
    }
    else if((character>='a') && (character<='f')){ 
      digit = (character-'a')+0xA;
    }
// If the character is not 0-9 or A-F, it is ignored and not echoed
    if(digit<=0xF ){	
      number = number*0x10+digit;
      length++;
      SCI_OutChar(port, character);
    }
// Backspace outputted and return value changed if a backspace is inputted
    else if((character==BS) && length){
      number /= 0x10;
      length--;
      SCI_OutChar(port, character);
    }
    character = SCI_InChar(port);
  }
  return number;
}

//--------------------------SCI_OutUHex----------------------------
// Output a 16 bit number in unsigned hexadecimal format
// Input: SCI port, 16-bit number to be transferred
// Output: none
// Variable format 1 to 4 digits with no space before or after
void SCI_OutUHex(unsigned char port, unsigned short number){
// This function uses recursion to convert the number of 
//   unspecified length as an ASCI0I string
  if(number >= 0x10){
    SCI_OutUHex(port, number/0x10);
    SCI_OutUHex(port, number%0x10);
  }
  else if(number < 0xA){
    SCI_OutChar(port, number+'0');
  }
  else{
    SCI_OutChar(port, (number-0x0A)+'A');
  }
}

//------------------------SCI_InString------------------------
// Accepts ASCI0I characters from the serial port
//    and adds them to a string until <enter> is typed 
//    or until max length of the string is reached.  
// It echoes each character as it is inputted.  
// If a backspace is inputted, the string is modified 
//    and the backspace is echoed
// terminates the string with a null character
// uses interrupt synchronization on RDRF
// Input: SCI port, pointer to empty buffer, size of buffer
// Output: Null terminated string
// -- Modified by Agustinus Darmawan + Mingjie Qiu --
void SCI_InString(unsigned char port, char *bufPt, unsigned short max) {	
int length=0;
char character;
  character = SCI_InChar(port);
  while(character != CR){
    if(character == BS){
      if(length){
        bufPt--;
        length--;
        SCI_OutChar(port, BS);
      }
    }
    else if(length < max){
      *bufPt = character;
      bufPt++;
      length++; 
      SCI_OutChar(port, character);
    }
    character = SCI_InChar(port);
  }
  *bufPt = 0;
}

//---------------------SCI_OutCRLF---------------------
// Output a CR,LF to SCI0 to go to a new line
// Input: SCI port
// Output: none
// toggle PortT bit 0 each time, debugging profile
void SCI_OutCRLF(unsigned char port){
  SCI_OutChar(port, CR);
  SCI_OutChar(port, LF);
}


 

//-----------------------SCI_OutFix1-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.1 
// Input: SCI port, 16-bit unsigned number 
// Output: none
// fixed size 5 digits of output, right justified
// if input is 12345, then display is 1234.5 
void SCI_OutFix1(unsigned char port, unsigned short n){
  if(n < 10){
    SCI_OutString(port, "   0.");
    SCI_OutChar(port, n+'0'); /* n is between 0 and 9 */
  } else if(n<100){
    SCI_OutString(port, "   ");
    SCI_OutChar(port, n/10+'0'); /* tens digit */
    SCI_OutChar(port, '.');      /* decimal point */
    SCI_OutChar(port, n%10+'0'); /* ones digit */
  } else if(n<1000){
      SCI_OutString(port, "  ");
      SCI_OutChar(port, n/100+'0'); /* hundreds digit */
      n = n%100;
      SCI_OutChar(port, n/10+'0'); /* tens digit */
      SCI_OutChar(port, '.');      /* decimal point */
      SCI_OutChar(port, n%10+'0'); /* ones digit */
    }
    else if(n<10000){
      SCI_OutChar(port, ' ');  
      SCI_OutChar(port, n/1000+'0'); /* thousands digit */
      n = n%1000;
      SCI_OutChar(port, n/100+'0'); /* hundreds digit */
      n = n%100;
      SCI_OutChar(port, n/10+'0'); /* tens digit */
      SCI_OutChar(port, '.');      /* decimal point */
      SCI_OutChar(port, n%10+'0'); /* ones digit */
    }
    else {
      SCI_OutChar(port, n/10000+'0'); /* ten-thousands digit */
      n = n%10000;
      SCI_OutChar(port, n/1000+'0'); /* thousands digit */
      n = n%1000;
      SCI_OutChar(port, n/100+'0'); /* hundreds digit */
      n = n%100;
      SCI_OutChar(port, n/10+'0'); /* tens digit */
      SCI_OutChar(port, '.');      /* decimal point */
      SCI_OutChar(port, n%10+'0'); /* ones digit */
    }
}
//-----------------------SCI_OutFix2-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.01 
// Input: SCI port, 16-bit unsigned number 
// Output: none
// fixed size 5 digits of output, right justified
// if input is 12345, then display is 123.45 
void SCI_OutFix2(unsigned char port, unsigned short n){
  if(n < 10){
    SCI_OutString(port, "  0.0");
    SCI_OutChar(port, n+'0'); /* n is between 0 and 9 */
  } else if(n<100){
    SCI_OutString(port, "  0.");
    SCI_OutChar(port, n/10+'0'); /* tens digit */
    SCI_OutChar(port, n%10+'0'); /* ones digit */
  } else if(n<1000){
      SCI_OutString(port, "  ");
      SCI_OutChar(port, n/100+'0'); /* hundreds digit */
      n = n%100;
      SCI_OutChar(port, '.');      /* decimal point */
      SCI_OutChar(port, n/10+'0'); /* tens digit */
      SCI_OutChar(port, n%10+'0'); /* ones digit */
    }
    else if(n<10000){
      SCI_OutChar(port, ' ');  
      SCI_OutChar(port, n/1000+'0'); /* thousands digit */
      n = n%1000;
      SCI_OutChar(port, n/100+'0'); /* hundreds digit */
      n = n%100;
      SCI_OutChar(port, '.');      /* decimal point */
      SCI_OutChar(port, n/10+'0'); /* tens digit */
      SCI_OutChar(port, n%10+'0'); /* ones digit */
    }
    else {
      SCI_OutChar(port, n/10000+'0'); /* ten-thousands digit */
      n = n%10000;
      SCI_OutChar(port, n/1000+'0'); /* thousands digit */
      n = n%1000;
      SCI_OutChar(port, n/100+'0'); /* hundreds digit */
      n = n%100;
      SCI_OutChar(port, '.');      /* decimal point */
      SCI_OutChar(port, n/10+'0'); /* tens digit */
      SCI_OutChar(port, n%10+'0'); /* ones digit */
    }
}

//-----------------------SCI_OutFix3-----------------------
// Output a 16-bit number in unsigned decimal fixed-point
// with resolution = 0.001 
// Input: SCI port, 16-bit unsigned number 
// Output: none
// fixed size is 6 characters of output, right justified
// if input is 12345, then display is 12.345 
void SCI_OutFix3(unsigned char port, unsigned short n){
  if(n < 10){
    SCI_OutString(port, " 0.00");
    SCI_OutChar(port, n+'0');  /* n is between 0 and 9 */
  } else if(n<100){
    SCI_OutString(port, " 0.0");
    SCI_OutChar(port, n/10+'0'); /* tens digit */
    SCI_OutChar(port, n%10+'0'); /* ones digit */
  } else if(n<1000){
      SCI_OutString(port, " 0.");
      SCI_OutChar(port, n/100+'0'); /* hundreds digit */
      n = n%100;
      SCI_OutChar(port, n/10+'0'); /* tens digit */
      SCI_OutChar(port, n%10+'0'); /* ones digit */
    }
    else if(n<10000){
      SCI_OutChar(port, ' ');  
      SCI_OutChar(port, n/1000+'0'); /* thousands digit */
      SCI_OutChar(port, '.');      /* decimal point */
      n = n%1000;
      SCI_OutChar(port, n/100+'0'); /* hundreds digit */
      n = n%100;
      SCI_OutChar(port, n/10+'0'); /* tens digit */
      SCI_OutChar(port, n%10+'0'); /* ones digit */
    }
    else {
      SCI_OutChar(port, n/10000+'0'); /* ten-thousands digit */
      n = n%10000;
      SCI_OutChar(port, n/1000+'0'); /* thousands digit */
      SCI_OutChar(port, '.');       /* decimal point */
      n = n%1000;
      SCI_OutChar(port, n/100+'0'); /* hundreds digit */
      n = n%100;
      SCI_OutChar(port, n/10+'0'); /* tens digit */
      SCI_OutChar(port, n%10+'0'); /* ones digit */
    }
}

//-----------------------SCI_SOutFix2-----------------------
// Output a 16-bit number in signed decimal fixed-point
// with resolution = 0.01 
// Input: SCI port, 16-bit signed number  -9999 to 9999
// Output: none
// fixed size, 6 characters, 4 digits of output, right justified
// if input is -1234, then display is -12.34 
void SCI_SOutFix2(unsigned char port, short num){ unsigned short n;
  if(num<0){ /* negative */
    n = -num;
    if(n < 10){
      SCI_OutString(port, " -0.0");
      SCI_OutChar(port, n+'0'); /* n is between 0 and 9 */
    } else if(n<100){
      SCI_OutString(port, " -0.");
      SCI_OutChar(port, n/10+'0'); /* tens digit */
      SCI_OutChar(port, n%10+'0'); /* ones digit */
    } else if(n<1000){
      SCI_OutString(port, " -");
      SCI_OutChar(port, n/100+'0'); /* hundreds digit */
      n = n%100;
      SCI_OutChar(port, '.');      /* decimal point */
      SCI_OutChar(port, n/10+'0'); /* tens digit */
      SCI_OutChar(port, n%10+'0'); /* ones digit */
    }
    else if(n<10000){
      SCI_OutChar(port, '-');   
      SCI_OutChar(port, n/1000+'0'); /* thousands digit */
      n = n%1000;
      SCI_OutChar(port, n/100+'0'); /* hundreds digit */
      n = n%100;
      SCI_OutChar(port, '.');      /* decimal point */
      SCI_OutChar(port, n/10+'0'); /* tens digit */
      SCI_OutChar(port, n%10+'0'); /* ones digit */
    }
    else {
    SCI_OutString(port, "-**.**");
    }
  } else{         /* positive */
    n = num;
    if(n < 10){
      SCI_OutString(port, "  0.0");
      SCI_OutChar(port, n+'0');      /* n is between 0 and 9 */
    } else if(n<100){
      SCI_OutString(port, "  0.");
      SCI_OutChar(port, n/10+'0');   /* tens digit */
      SCI_OutChar(port, n%10+'0');   /* ones digit */
    } else if(n<1000){
      SCI_OutString(port, "  ");
      SCI_OutChar(port, n/100+'0');  /* hundreds digit */
      n = n%100;
      SCI_OutChar(port, '.');        /* decimal point */
      SCI_OutChar(port, n/10+'0');   /* tens digit */
      SCI_OutChar(port, n%10+'0');   /* ones digit */
    }
    else if(n<10000){
     SCI_OutChar(port, ' ');   
      SCI_OutChar(port, n/1000+'0'); /* thousands digit */
      n = n%1000;
      SCI_OutChar(port, n/100+'0');  /* hundreds digit */
      n = n%100;
      SCI_OutChar(port, '.');      /* decimal point */
      SCI_OutChar(port, n/10+'0'); /* tens digit */
      SCI_OutChar(port, n%10+'0'); /* ones digit */
    }
    else {
      SCI_OutString(port, " **.**");
    } 
  }
}
