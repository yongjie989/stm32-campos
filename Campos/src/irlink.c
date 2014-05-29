/**
 *  Project     Campos
 *  @file		irlink.c
 *  @author		Gerd Bartelt - www.sebulli.com
 *  @brief		infrared interface
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

#include "irlink.h"

uint16_t irdata[3] = { 0, 0, 0 };

/* local functions ----------------------------------------------------------*/

/* local variables ----------------------------------------------------------*/
TIM_HandleTypeDef htim3;
TIM_OC_InitTypeDef sConfigTim3;

int header_cnt;
int data_phase_cnt;
int data_bit_cnt;
int data_byte_cnt;

/**
 * @brief  Initialize the module and configure PWM PB5 as PWM output with 36kHz
 * @param  None
 * @retval None
 */
void IRLINK_Init(void) {

	// Timer configuration
	htim3.Instance = TIM3;
	htim3.Init.Period = 1166 - 1; // = 36kHz = 42MHz / 1166
	htim3.Init.Prescaler = 1;
	htim3.Init.ClockDivision = 1;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	HAL_TIM_PWM_Init(&htim3);

	// Configure Timer 3 channel 2 as PWM output
	sConfigTim3.OCMode = TIM_OCMODE_PWM1;
	sConfigTim3.OCIdleState = TIM_OUTPUTSTATE_ENABLE;
	sConfigTim3.Pulse = 0;
	sConfigTim3.OCPolarity = TIM_OCPOLARITY_HIGH;

	// PWM Mode
	HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigTim3, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

	header_cnt = 0;
	data_phase_cnt = 0;
	data_bit_cnt = 0;
	data_byte_cnt = 0;
}

/**
 * @brief  Outputs a 36kHz burst, or none
 * @param  value != 0 to output a burst
 * @retval None
 */
void IRLINK_Output(int value) {
	if (value != 0) {
		__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, 1166 / 2);
	} else {
		__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, 0);
	}

}

/**
 * @brief  Initializes the TIM PWM MSP.
 * @param  htim: TIM handle
 * @retval None
 */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim) {
	GPIO_InitTypeDef GPIO_InitStructure;

	// TIM3 clock enable
	__TIM3_CLK_ENABLE();

	// GPIOB clock enable
	__GPIOB_CLK_ENABLE();

	// GPIO Configuration: Pin B5 as output push-pull
	GPIO_InitStructure.Pin = GPIO_PIN_5;
	GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStructure.Alternate = GPIO_AF2_TIM3;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}


/**
 * @brief  Send the header
 * @param  None
 * @retval None
 */
void IRLINK_StartHeader(void) {
	IRLINK_Output(1);
	header_cnt = 5 + 1;
}


/**
 * @brief  Send all the data in the 1ms task
 * @param  None
 * @retval None
 */
void IRLINK_1msTask(void) {

	// Header is n ms high and then one ms low.
	if (header_cnt > 0) {
		header_cnt--;
		if (header_cnt == 0) {
			IRLINK_Output(0);
		} else {
			IRLINK_Output(1);
		}

	} else {
		if (data_byte_cnt <= 2) {

			// Manchester code
			if (data_phase_cnt == 0) {
				if (irdata[data_byte_cnt] & 0x8000) {
					IRLINK_Output(1);
				} else {
					IRLINK_Output(0);
				}
			} else {
				if (irdata[data_byte_cnt] & 0x8000) {
					IRLINK_Output(0);
				} else {
					IRLINK_Output(1);
				}
			}

			// Next phase
			data_phase_cnt++;
			if (data_phase_cnt >= 2 ) {

				// Next bit
				irdata[data_byte_cnt] <<= 1;
				data_phase_cnt = 0;
				data_bit_cnt ++;
				if (data_bit_cnt >= 8 ) {

					// Next byte
					data_bit_cnt = 0;
					data_byte_cnt ++;
				}
			}
		} else {
			// finished
			IRLINK_Output(0);
		}
	}
}



/**
 * @brief  The data to send. All data is copied to a memory structure
 *
 * @param  track_status The track_status
 * @param  position_x The position_x
 * @param  position_subx The position_subx
 * @param  position_y The position_y
 * @param  position_suby The position_suby
 * @param  intensity The intensity
 * @retval None
 */
void IRLINK_Send(Track_StatusTypeDef track_status, int position_x,
		int position_subx, int position_y, int position_suby, int intensity) {

	irdata[0] = position_x*10 + position_subx / 100;
	irdata[1] = position_y*10 + position_suby / 100;
	irdata[2] = (intensity/256)*256 + (int)track_status;

	data_phase_cnt = 0;
	data_bit_cnt = 0;
	data_byte_cnt = 0;
}
