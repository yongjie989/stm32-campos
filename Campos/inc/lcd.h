/**
 *  Project     Campos
 *  @file		lcd.h
 *  @author		Gerd Bartelt - www.sebulli.com
 *  @brief		Header file for lcd.c
 *
 *  @copyright	GPL3
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LCD_H_
#define LCD_H_


/* Includes ------------------------------------------------------------------*/
#include "lcd_io.h"


/* Defines -------------------------------------------------------------------*/

#define LCD_BLACK 0x0000
#define LCD_WHITE 0xFFFF
#define LCD_RED   0xF800

#define LCD_Y_TRACK_STATUS	1
#define LCD_Y_POSX			4
#define LCD_Y_POSY			7
#define LCD_Y_INTENSITY		10

/* Function prototypes -------------------------------------------------------*/

void LCD_Init(void);
void LCD_Print (int x, int y, char *);
void LCD_Image_Small(uint8_t* pixelp);
void LCD_Image_Large(uint8_t* pixelp);
void LCD_FocusStatusWindow(void);
void LCD_MiniWindow(void);
void LCD_Clr(void);

#endif /* LCD_H_ */
