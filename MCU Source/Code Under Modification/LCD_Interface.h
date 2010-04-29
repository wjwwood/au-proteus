//LCD_Interface.h
#ifndef _LCD_INTERFACE_H
#define _LCD_INTERFACE_H 1

#define LCD_HZ 1


typedef void (*PFun)();

void doNothing(void);
void LCDIPeriodicBG(void);
void LCDI_Init(void);
void LCDIFG(void);
void LCDI_MainScreen(void);
void LCDI_MtrCtrScreen(void);
void LCDI_ProcessButton(void);



#endif   //_LCD_INTERFACE_H
