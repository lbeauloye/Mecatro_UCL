/*******************************************************************************
* MyButton                                                                     *
********************************************************************************
* Description:                                                                 *
* Functions to handle the buttons                                              *
********************************************************************************
* Version : 1.00 - Aug 2015                                                    *
*******************************************************************************/

#define  MyBUTTON

#include "MyLT24.h"
#include "simple_graphics.h"
#include "simple_button.h"

void MyButton_Init(BUTTON *theButton, RECT *theRect, char *theStr, alt_video_display *theDisplay)
{
    theButton->buttonRect.left = theRect->left;
    theButton->buttonRect.right = theRect->right;
    theButton->buttonRect.top = theRect->top;
    theButton->buttonRect.bottom = theRect->bottom;
    theButton->buttonString = theStr;
    theButton->buttonStatus = MyButtonStatus_NORMAL;
    
    theButton->buttonColorRect = MyButtonCOLOR_RECT;
    theButton->buttonColorNormal = MyButtonCOLOR_NORMAL;
    theButton->buttonColorHighlight = MyButtonCOLOR_HIGHLIGHT;
    theButton->buttonColorText = MyButtonCOLOR_TEXT;
    
    theButton->buttonDisplay = theDisplay;
    }

/******************************************************************************/

void MyButton_Draw(BUTTON *theButton)
{
    int theColor;

    if (theButton->buttonStatus == MyButtonStatus_NORMAL)
    	 theColor =  theButton->buttonColorNormal;
    else theColor =  theButton->buttonColorHighlight;

	vid_draw_round_corner_box (theButton->buttonRect.left, theButton->buttonRect.bottom,
                               theButton->buttonRect.right, theButton->buttonRect.top,
                               5, theColor, 1, theButton->buttonDisplay);
    
    int strLength = vid_string_pixel_length_alpha(tahomabold_20, theButton->buttonString);
    int x = theButton->buttonRect.left + (theButton->buttonRect.right - theButton->buttonRect.left - strLength) / 2;
    int y = theButton->buttonRect.bottom + 8;
    vid_print_string_alpha(x, y, theButton->buttonColorText, theColor, tahomabold_20, theButton->buttonDisplay, theButton->buttonString);

}

/******************************************************************************/

void MyButton_Highlight(BUTTON *theButton)
{
	theButton->buttonStatus = MyButtonStatus_HIGHLIGHT;
	MyButton_Draw(theButton);
}

/******************************************************************************/

int MyButton_IsSelected(BUTTON *theButton, POINT *thePt)
{
    if ((thePt->x > theButton->buttonRect.left) && (thePt->x < theButton->buttonRect.right) &&
        (thePt->y > theButton->buttonRect.bottom) && (thePt->y < theButton->buttonRect.top)) return (1);
	return(0);
}

/******************************************************************************/

int MyButton_Task(BUTTON *theButton, POINT *thePt, int IsPressed)
{
    if (IsPressed) {
        if (MyButton_IsSelected(theButton, thePt) && (theButton->buttonStatus == MyButtonStatus_NORMAL)) {
            theButton->buttonStatus = MyButtonStatus_HIGHLIGHT;
            MyButton_Draw(theButton);
        }
    } else {
        if (theButton->buttonStatus == MyButtonStatus_HIGHLIGHT) {
            theButton->buttonStatus = MyButtonStatus_NORMAL;
            MyButton_Draw(theButton);
            return (1);
        }
    }
    return(0);
}

/******************************************************************************/
