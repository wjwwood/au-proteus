/*************************PLL.h***************************
*   boosts the CPU clock to 24 MHz                       *
*                                                        *
*   Created by Theodore Deden on 20 January 2004.        *
*   Modified by Theodore Deden on 9 February 2004.       *
*   Last Modified by Jonathan Valvano 2/9/04.            *
*                                                        *
*   Distributed underthe provisions of the GNU GPL ver 2 *
*   Copying, redistributing, modifying is encouraged.    *
*                                                        *
*********************************************************/

#ifndef _PLL_H
#define _PLL_H 1

//********* PLL_Init ****************
// Set PLL clock to 24 MHz, and switch 9S12 to run at this rate
// Inputs: none
// Outputs: none
// Errors: will hang if PLL does not stabilize 
void PLL_Init(void);

#endif /* _PLL_H */
