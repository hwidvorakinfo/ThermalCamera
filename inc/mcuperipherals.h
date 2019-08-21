/*
 * mcuperipherals.h
 *
 *  Created on: Dec 29, 2014
 *      Author: daymoon
 */

#ifndef INCLUDES_MCUPERIPHERALS_H_
#define INCLUDES_MCUPERIPHERALS_H_

#include "stm32f4xx.h"
#include "defs.h"
#include "stm32f4xx_conf.h"
#include "displaylib.h"

#define TP_SPI					SPI2
#define TP_PORT					GPIOA
#define TP_CS					GPIO_Pin_15
#define TP_IRQ					GPIO_Pin_8
#define TP_SCK 					GPIO_Pin_13
#define TP_SO					GPIO_Pin_14
#define TP_SI					GPIO_Pin_15
#define DEBUG_PIN_PORT			GPIOC
#define DEBUG_PIN_1				GPIO_Pin_6
#define DEBUG_PIN_2				GPIO_Pin_8
#define BLCNT_PORT				GPIOA
#define BLCNT_PIN				GPIO_Pin_13
#define LASER_PORT				GPIOB
#define LASER_PIN				GPIO_Pin_8

#define TP_IRQ_EXTI 			EXTI_Line8
#define TP_IRQ_EXTIchannel		EXTI9_5_IRQn
#define TP_IRQ_EXTIpin			EXTI_PinSource8

#define TRANSFER_DONE			0
#define TRANSFER_STARTED		10

typedef enum {
	SPI_UNUSED,
	SPI_TOUCHPANEL,
	SPI_WIFI,
	SPI_CODEC
} SPI_SEMAPHORE;

extern void Init_display_HW(void);
extern void Init_display_TP(void);
extern void Init_wifi(void);
extern void SPI_touch_panel_fast_init(void);
extern void SPI_touch_panel_slow_init(void);
extern uint8_t Touch_panel_ReadWrite(uint8_t writedat);
extern void set_spi_semaphore(SPI_SEMAPHORE state);
extern SPI_SEMAPHORE get_spi_semaphore(void);
extern void Debug_pins_init(void);
extern void Set_DMA_SPI_transfer_status(uint8_t status);
extern uint8_t Get_DMA_SPI_transfer_status(void);

#endif /* INCLUDES_MCUPERIPHERALS_H_ */
