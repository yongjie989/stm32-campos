/**
 *  Project     Campos
 *  @file		main.c
 *  @author		Gerd Bartelt - www.sebulli.com
 *  @brief		Projects main file
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

/* Includes -----------------------------------------------------------------*/
#include "main.h"
#include "usartl1.h"
#include "camera.h"
#include "lcd.h"
#include "track.h"
#include "irlink.h"
#include "power.h"

/* function prototypes ------------------------------------------------------*/
void SystemClock_Config(void);
Camera_SizeTypeDef lastSize = CAMERA_NONE;
Camera_SizeTypeDef cameraSize = CAMERA_NONE;
char txt[20]; // Temporary memory for strings
int blink = 0;
int mytick = 0;
/**
 * @brief  Main program.
 * @param  None
 * @retval None
 */
int main(void) {

	// Configure the system clock to 168 MHz
	SystemClock_Config();

	// Initialize the hardware layer module
	HAL_Init();

	// Enable systick and configure 500us tick
	HAL_SYSTICK_Config(168000000/ 2000);

	// Initialize the power module
	POWER_Init();

	// Initialize the LCD display
	LCD_Init();

	// Initialize the IR link
	IRLINK_Init();

	// Configure LEDs
	BSP_LED_Init(LED3);
	BSP_LED_Off(LED3);
	BSP_LED_Init(LED_RED);
	BSP_LED_Init(LED_GREEN);
	BSP_LED_Init(LED_BLUE);

	// Init = blue
	BSP_LED_Off(LED_GREEN);
	BSP_LED_On(LED_BLUE);
	BSP_LED_Off(LED_RED);

	// Initialize the debug port
	USARTL2_Init();

	// Initialize the camera and start video mode
	BSP_CAMERA_Init();
	BSP_CAMERA_ContinuousStart();

	// Initialize the tracking
	TRACK_Init();

	// Startup Logo
	LCD_Logo();
	LCD_Print(31,14,"1.4.1",LCD_TRANSPARENT);
	HAL_Delay(5000);
	LCD_Clr();
	LCD_DrawInfoWindow();
	frame_flag = 0;


	// Main loop
	while (1) {

		// Generate a blink flag
		if (mytick > 200) {
			mytick = 0;
			blink = !blink;

			//200ms Task
			POWER_Task();
		}


		// Debug ports
		USARTL1_RxBufferTask();

		// Update the status window on the right side of the TFT
		LCD_FocusStatusWindow();
		switch (track_status) {
		case TRACK_INIT:
			LCD_Print(35, LCD_Y_TRACK_STATUS, "Init     ", LCD_OPAQUE);
			BSP_LED_Off(LED_GREEN);
			BSP_LED_Off(LED_BLUE);
			if (blink)
				BSP_LED_On(LED_RED);	// red blinking
			else
				BSP_LED_Off(LED_RED);	// red blinking
			break;
		case TRACK_SEARCHING:
			LCD_Print(35, LCD_Y_TRACK_STATUS, "Searching", LCD_OPAQUE);
			BSP_LED_Off(LED_GREEN);
			BSP_LED_Off(LED_BLUE);
			if (blink)
				BSP_LED_On(LED_RED);	// red blinking
			else
				BSP_LED_Off(LED_RED);	// red blinking

			break;
		case TRACK_LIGHT_FOUND:
			LCD_Print(35, LCD_Y_TRACK_STATUS, "Light    ", LCD_OPAQUE);
			BSP_LED_Off(LED_GREEN);
			BSP_LED_Off(LED_BLUE);
			if (blink)
				BSP_LED_On(LED_RED);	// red blinking
			else
				BSP_LED_Off(LED_RED);	// red blinking
			break;
		case TRACK_CENTER_DETECTED:
			LCD_Print(35, LCD_Y_TRACK_STATUS, "Center   ", LCD_OPAQUE);
			BSP_LED_On(LED_GREEN); // green
			BSP_LED_Off(LED_BLUE);
			BSP_LED_Off(LED_RED);
			break;
		case TRACK_LOST:
			LCD_Print(35, LCD_Y_TRACK_STATUS, "Lost     ", LCD_OPAQUE);
			BSP_LED_Off(LED_GREEN);
			BSP_LED_Off(LED_BLUE);
			BSP_LED_On(LED_RED);	// red
			break;
		}
		sprintf(txt, "%04d.%03d", position_x, position_subx);
		LCD_Print(35, LCD_Y_POSX, txt, LCD_OPAQUE);

		sprintf(txt, "%04d.%03d", position_y, position_suby);
		LCD_Print(35, LCD_Y_POSY, txt, LCD_OPAQUE);

		sprintf(txt, "%05d", intensity);
		LCD_Print(35, LCD_Y_INTENSITY, txt, LCD_OPAQUE);

		sprintf(txt, "%05d", batteryFilt);
		LCD_Print(35, LCD_Y_BATTERY, txt, LCD_OPAQUE);

		// Mini window that shows the position of the actual window
		LCD_MiniWindow(BSP_CAMERA_GetSize());

		// Search for the light
		if (frame_flag != 0) {
			frame_flag = 0;
			TRACK_Search();

			// Send the tracking result via IR
			IRLINK_Send(track_status ,
					position_x, position_subx,
					position_y, position_suby,
					intensity);

			cameraSize = BSP_CAMERA_GetSize();

			// Clear the LCD if the size has changed
			if (cameraSize != lastSize)
				LCD_Clr();
			lastSize = cameraSize;

			// Update the LCD
			if (cameraSize == CAMERA_ZOOMED)
				LCD_Image_Zoomed(&pixels.firstByte);
			else
				LCD_Image_Total(&pixels.firstByte);
			// Debug console
			USARTL2_FrameCallback();
		}

	}

}



