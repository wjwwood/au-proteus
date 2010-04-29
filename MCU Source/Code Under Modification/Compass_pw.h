// filename ******** Compass.h **************
// A compass driver for the Proteus Robot
// Written for Robot-Electronic's CMPS03 compass

#ifndef _COMPASS_H
#define _COMPASS_H 1

//compass driver for Proteus Robot

void Compass_Init(void);

unsigned short Compass_getHeading(void);
void CMPPeriodicFG(void);


#endif /* _COMPASS_H */