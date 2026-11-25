//-- Include -------------------------------------------------
#include "app_fsm.h"
#include "config.h"
#include "stm32g4_sys.h"

#include "stm32g4_systick.h"
#include "stm32g4_gpio.h"
#include "stm32g4_uart.h"
#include "stm32g4_utils.h"

#include <stdio.h>
//-- Define --------------------------------------------------
#define BUFFER_RX_SIZE	10
#define TICK_PERIOD_MS	1
#define BLINK_DELAY		100	//ms
//-- Enum ----------------------------------------------------
typedef enum
{
	ST_INIT = 0	,
	ST_WAIT		,
	ST_RX		,
	ST_TX		,
	ST_POSER	,
	ST_ATK		,
	ST_VERIF	,
	ST_VICT		,
	ST_NB
}e_State;

//-- Prototypes ----------------------------------------------
void write_LED(bool b)
{
	HAL_GPIO_WritePin(LED_GREEN_GPIO, LED_GREEN_PIN, b);
}

//------------------------------------------------------------
//
// APP_FSM : Main Finite State Machine
//
//------------------------------------------------------------
void App_FSM(void)
{
	static e_State e_current_state 					=  ST_INIT;
	static uint32_t u32_timeout_ms 					= 0;
	static uint8_t au8_buffer_rx[BUFFER_RX_SIZE] 	= {0};
	static uint8_t u8_index_buffer					= 0;
	uint8_t u8_char_rx 								= 0;

	switch(e_current_state)
	{
		//-- Init ----------------------------------
		case ST_INIT :
			printf("Initialization\r\n");

			for(u8_index_buffer = 0u; u8_index_buffer < BUFFER_RX_SIZE ; u8_index_buffer++)
			{
				au8_buffer_rx[u8_index_buffer] = 0u;
			}
			BSP_GPIO_pin_config(LED_GREEN_GPIO, LED_GREEN_PIN, GPIO_MODE_OUTPUT_PP,GPIO_NOPULL,GPIO_SPEED_FREQ_HIGH,GPIO_NO_AF);
			e_current_state = ST_WAIT;
			break;

		//-- Wait for a new character --------------
		case ST_WAIT :
			if( BSP_UART_data_ready(UART2_ID) )
			{
				e_current_state = ST_RX;
				u8_index_buffer = 0u;
				u32_timeout_ms 	= 0u;
			}
			write_LED(true);		/* write_LED? Faites un ctrl+clic dessus pour voir... */
			HAL_Delay(BLINK_DELAY);	/* ... ça fonctionne aussi avec les macros, les variables. C'est votre nouveau meilleur ami */
			write_LED(false);
			HAL_Delay(BLINK_DELAY);
			break;

		//-- Save character received ----------------
		case ST_RX :

			// Receive and save each character
			if( BSP_UART_data_ready(UART2_ID) )
			{
				u32_timeout_ms 	= 0u;

				if(u8_index_buffer < BUFFER_RX_SIZE)
				{
					u8_char_rx = BSP_UART_get_next_byte(UART2_ID);
					au8_buffer_rx[u8_index_buffer] = u8_char_rx;
					u8_index_buffer++;
					printf("%c",u8_char_rx);
				}
				else
				{
					printf("String received = %s", au8_buffer_rx);
					e_current_state = ST_WAIT;
				}
			}

			// Timeout management
			u32_timeout_ms += TICK_PERIOD_MS;
			if(u32_timeout_ms > 2000)
			{
				e_current_state = ST_INIT;
			}

			break;

		default  :

			break;
		}
}


