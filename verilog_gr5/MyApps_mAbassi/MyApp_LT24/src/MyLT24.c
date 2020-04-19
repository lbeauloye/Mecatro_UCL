/*******************************************************************************
* MyLT24                                                                       *
********************************************************************************
* Description:                                                                 *
* Functions to handle the LT24 Touch Screen                                    *
********************************************************************************
* Version : 1.00 - Aug 2015                                                    *
*******************************************************************************/

/*
 * The origin (0,0) is at the top left.
 * A portrait to landscape tranformation is done in LCD_DrawPoint (AD7843.c)
 */

#define  MyLT24

#include "MyLT24.h"

char * MyGet_IP(char * theStr);
char * MyGet_MAC(char * theStr);

void MyLT24_Init(void)
{
    MyLT24_Display.interlace = 0;
    MyLT24_Display.bytes_per_pixel = 2;
    MyLT24_Display.color_depth = 24;
    MyLT24_Display.height = SCREEN_HEIGHT;
    MyLT24_Display.width = SCREEN_WIDTH;
    
    LCD_Init();
    LCD_Clear(0X0000);
    
    LT24_SPI_Init();
    
    int x, y;
    
    x = MyLT24_Display.width / 2 - 60;
    y = MyLT24_Display.height / 2 - 10;
    
    vid_print_string_alpha(x, y, BLUE_24, BLACK_24, tahomabold_20, &MyLT24_Display, "Terasic");
    vid_print_string_alpha(x, y+22, BLUE_24, BLACK_24, tahomabold_20, &MyLT24_Display, "Touch Demo");
    
    RECT rc;
    
    rc.left = x;
    rc.top = y;
    rc.right = x + 100;
    rc.bottom = y + 50;
    
    vid_draw_box (rc.left, rc.top, rc.right, rc.bottom, WHITE_24, DO_NOT_FILL, &MyLT24_Display);
    vid_draw_circle(x-20, y-20, 10, RED_24, DO_FILL, &MyLT24_Display);
    
    //GUI_Test1 (&Display);
    //GUI_Test2 (&Display);
    
    int theWidht = 320/4 - 10 - 10/4;

    rc.left = 10;
    rc.bottom = 10;
    rc.right = rc.left + theWidht;
    rc.top = rc.bottom + 40;
    
    MyButton_Init(&theButton1, &rc, "Test1", &MyLT24_Display);
    MyButton_Draw(&theButton1);
    rc.left =  rc.right + 10;
    rc.right = rc.left + theWidht;
    MyButton_Init(&theButton2, &rc, "Test2", &MyLT24_Display);
    MyButton_Draw(&theButton2);
    rc.left =  rc.right + 10;
    rc.right = rc.left + theWidht;
    MyButton_Init(&theButton3, &rc, "Test3", &MyLT24_Display);
    MyButton_Draw(&theButton3);
    rc.left =  rc.right + 10;
    rc.right = rc.left + theWidht;
    MyButton_Init(&theButton4, &rc, "Test4", &MyLT24_Display);
    MyButton_Draw(&theButton4);
    
    int i;
    for (i=0; i<MyTerminal_MaxLine-1; i++)
        strcpy(theTerminalString[i], "\0");
}

/******************************************************************************/

void MyLT24_Task(void)
{
    POINT thePoint;
    char  theStr[32];
    
    if (LT24_IsPressed()) {
        LT24_GetXY(&thePoint);
        //printf("LT24_ADC : IRQ=%d, BUSY=%d\r\n", alt_read_word(h2p_LT24_ADC_IRQ_N_addr),alt_read_word(h2p_LT24_ADC_BUSY_addr));
        //printf("LT24 x = %d, y = %d \r\n", thePoint.x, thePoint.y);
        MyButton_Task(&theButton1, &thePoint, true);
        MyButton_Task(&theButton2, &thePoint, true);
        MyButton_Task(&theButton3, &thePoint, true);
        MyButton_Task(&theButton4, &thePoint, true);
    } else {
        if (MyButton_Task(&theButton1, &thePoint, false)) MyLT24_Terminal("Button 1");
        if (MyButton_Task(&theButton2, &thePoint, false)) MyLT24_Terminal("Button 2");
        if (MyButton_Task(&theButton3, &thePoint, false)) MyLT24_Terminal("Button 3");
        if (MyButton_Task(&theButton4, &thePoint, false)) MyLT24_Terminal("Button 4");
    }
}

/******************************************************************************/

void MyLT24_Terminal(char *theStr)
{
    int i, y=60;
    
    for (i=MyTerminal_MaxLine-1; i>0; i--)
        strcpy(theTerminalString[i], theTerminalString[i-1]);
    if (strlen(theStr) < MyTerminal_MaxCharLine) {
        strcpy(theTerminalString[0], theStr);
    } else {
        strncpy(theTerminalString[0], theStr, MyTerminal_MaxCharLine);
        theTerminalString[0][MyTerminal_MaxCharLine-1] = '\0';
    }
    
    for (i=0; i<MyTerminal_MaxLine; i++) {
        vid_draw_box (0, y, SCREEN_WIDTH, y+26, BLACK_24, DO_FILL, &MyLT24_Display);
        vid_print_string_alpha(10, y, WHITE_24, BLACK_24, tahomabold_20, &MyLT24_Display, theTerminalString[i]);
        y+=26;
    }
}

/******************************************************************************/
