/*******************************************************************************
* Header file for Simple_button                                                *
*******************************************************************************/

#ifndef SIMPLE_BUTTON_H_
#define SIMPLE_BUTTON_H_

typedef struct {
    RECT    buttonRect;
    char *  buttonString;
    int		buttonStatus;
    int     buttonColorRect;
    int     buttonColorNormal;
    int     buttonColorHighlight;
    int     buttonColorText;
    alt_video_display * buttonDisplay;
} BUTTON;

/*******************************************************************************
* Functions Prototypes                                                         *
*******************************************************************************/

void	    MyButton_Init(BUTTON *theButton, RECT *theRect, char *theStr, alt_video_display *theDisplay);
void        MyButton_Draw(BUTTON *theButton);
void        MyButton_Highlight(BUTTON *theButton);
int         MyButton_IsSelected(BUTTON *theButton, POINT *thePt);
int         MyButton_Task(BUTTON *theButton, POINT *thePt, int IsPressed);

/*******************************************************************************
* Global Variables for MyButton Functions                                      *
*******************************************************************************/

#ifdef   MyBUTTON
#define  MyBUTTON_EXT
#define  MyBUTTON_INIT =FALSE
#else
#define  MyBUTTON_EXT  extern
#define  MyBUTTON_INIT
#endif

#define  MyButtonCOLOR_RECT         GREY_24
#define  MyButtonCOLOR_NORMAL       LIGHTGREY_24
#define  MyButtonCOLOR_HIGHLIGHT    OLIVE_24
#define  MyButtonCOLOR_TEXT         BLUE_24    //BLACK_24

enum {
	MyButtonStatus_NORMAL,
	MyButtonStatus_HIGHLIGHT
};

/*******************************************************************************
* Local Static Variables for MyButton Functions                                *
*******************************************************************************/



#endif /* SIMPLE_BUTTON_H_ */
