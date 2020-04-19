/*
 * ILI9341.h
 *
 *  Created on: 2014-2-13
 *      Author: Administrator
 */

#ifndef ILI9341_H_
#define ILI9341_H_

#include "geometry.h"

void LCD_Init();
void LCD_Clear(alt_u16 Color);
void LCD_DrawPoint(alt_u16 x,alt_u16 y,alt_u16 color );
int LT24_IsPressed(void);
void LT24_GetXY(POINT *thePoint);

#endif /* ILI9341_H_ */
