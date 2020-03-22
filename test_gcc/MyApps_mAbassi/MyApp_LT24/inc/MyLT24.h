/*******************************************************************************
* Header file for MyLT24                                                       *
*******************************************************************************/

#ifndef MyLT24_H_
#define MyLT24_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alt_types.h"
#include "alt_video_display.h"
#include "ILI9341.h"
#include "AD7843.h"
#include "fonts.h"
#include "geometry.h"
#include "simple_text.h"
#include "simple_graphics.h"
#include "simple_button.h"

/*******************************************************************************
* Functions Prototypes                                                         *
*******************************************************************************/

void MyLT24_Init(void);
void MyLT24_Task(void);
void MyLT24_Terminal(char *theStr);

/*******************************************************************************
* Global Variables for MyLT24 Functions                                        *
*******************************************************************************/

#ifdef   MyLT24
#define  MyLT24_EXT
#define  MyLT24_INIT =FALSE
#else
#define  MyLT24_EXT  extern
#define  MyLT24_INIT
#endif

#define true 1
#define false 0

#define MyTerminal_MaxLine 7
#define MyTerminal_MaxCharLine 32

MyLT24_EXT alt_video_display MyLT24_Display;
MyLT24_EXT BUTTON theButton1, theButton2, theButton3, theButton4;
MyLT24_EXT char theTerminalString[MyTerminal_MaxLine][MyTerminal_MaxCharLine];

/*******************************************************************************
* Local Static Variables for MyLT24 Functions                                  *
*******************************************************************************/



#endif /* MyLT24_H_ */
