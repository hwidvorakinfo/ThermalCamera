/*
 * mcuperipherals.c
 *
 *  Created on: Dec 29, 2014
 *      Author: daymoon
 */

#include "mcuperipherals.h"

static SPI_SEMAPHORE spi_semaphore;
static uint8_t dma_spi_status = TRANSFER_DONE;

void Init_display_GPIO(void);
void Init_display_FSMC(void);
void SPI2_Pins_Init(void);
void SPI2_Fast_Init(void);
void SPI2_Slow_Init(void);
uint8_t SPI2_ReadWrite(uint8_t writedat);

void Init_display_HW(void)
{
	Init_display_GPIO();
	Init_display_FSMC();
	Init_LCD();
}

void Init_display_TP(void)
{
	SPI2_Pins_Init();									// inicializace pinu pro periferie SPI1
	SPI_touch_panel_fast_init();						// inicializace vlastni periferie SPI1

	set_spi_semaphore(SPI_UNUSED);
}

void Init_display_GPIO(void)
{
	GPIO_InitTypeDef init={0};

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOE, ENABLE);

	/* RESET */
	init.GPIO_Pin=GPIO_Pin_1;
	init.GPIO_Mode=GPIO_Mode_OUT;
	init.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOE,&init);

	/* BL control */
	init.GPIO_Pin=BLCNT_PIN;
	init.GPIO_Mode=GPIO_Mode_OUT;
	init.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(BLCNT_PORT,&init);
	GPIO_SetBits(BLCNT_PORT, BLCNT_PIN);

	/* PORTD */
	init.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15;
	init.GPIO_Mode = GPIO_Mode_AF;
	init.GPIO_Speed = GPIO_Speed_100MHz;
	init.GPIO_OType = GPIO_OType_PP;
	init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOD, &init);

	/* PORTD */
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);     // D2
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);     // D3
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);     // NOE -> RD
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);     // NWE -> WR
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC);     // NE1 -> CS
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);     // D13
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);     // D14
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);    // D15
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC);    // A16 -> RS
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);    // D0
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);    // D1

	/* PORTE */
	init.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	init.GPIO_Mode = GPIO_Mode_AF;
	init.GPIO_Speed = GPIO_Speed_100MHz;
	init.GPIO_OType = GPIO_OType_PP;
	init.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOE, &init);

	/* PORTE */
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7, GPIO_AF_FSMC);     // D4
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8, GPIO_AF_FSMC);     // D5
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9, GPIO_AF_FSMC);     // D6
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10, GPIO_AF_FSMC);    // D7
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11, GPIO_AF_FSMC);    // D8
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12, GPIO_AF_FSMC);    // D9
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_FSMC);    // D10
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_FSMC);    // D11
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15, GPIO_AF_FSMC);    // D12
}

void SPI2_Pins_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure={0};
	NVIC_InitTypeDef NVIC_InitStructure={0};
	EXTI_InitTypeDef EXTI_InitStructure={0};

	/* Enable SPI2 and GPIOA and SYSCFG clocks */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Configure SPI2 pins as AF: TP_SCK=PB13, TP_(MI)SO=PB14 and TP_(MO)SI=PB15 */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);     // SCK
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);     // MISO
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);     // MOSI

	GPIO_InitStructure.GPIO_Pin   = TP_SCK | TP_SO | TP_SI;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure PA15 as TP_CS */
	GPIO_InitStructure.GPIO_Pin   = TP_CS;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(TP_PORT, &GPIO_InitStructure);
	XPT_release();												// TP_CS release

	/* Configure PA8 as EXTI line for TP_IRQ */
	GPIO_InitStructure.GPIO_Pin  = TP_IRQ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(TP_PORT, &GPIO_InitStructure);

	/* Connect EXTI Line0 to PA8 pin */
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, TP_IRQ_EXTIpin);

	/* Configure EXTI Line8 */
	EXTI_InitStructure.EXTI_Line = TP_IRQ_EXTI;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI9_5_IRQn Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = TP_IRQ_EXTIchannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Clear the EXTI line pending bit */
	EXTI_ClearITPendingBit(TP_IRQ_EXTI);
}

void SPI2_Fast_Init(void)
{
	SPI_InitTypeDef  SPI_InitStructure;

	/* SPI configuration -------------------------------------------------------*/
	SPI_Cmd(TP_SPI, DISABLE);
	SPI_I2S_DeInit(TP_SPI);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	// APB1=42MHz, optimalni je frekvence kolem 4MHz pro dlouhe vedeni
	// 42000kHz/8=5.25MHz
	// 42000kHz/16=2.63MHz
	// 42000kHz/4=10.5MHz
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(TP_SPI, &SPI_InitStructure);
	SPI_Cmd(TP_SPI, ENABLE);

	/* Drain SPI2 */
	while (SPI_I2S_GetFlagStatus(TP_SPI, SPI_I2S_FLAG_TXE) == RESET)
	{}
	SPI_I2S_ReceiveData(TP_SPI);
}

void SPI2_Slow_Init(void)
{
	SPI_InitTypeDef  SPI_InitStructure;

	/* SPI configuration -------------------------------------------------------*/
	SPI_Cmd(TP_SPI, DISABLE);
	SPI_I2S_DeInit(TP_SPI);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	// APB1=21MHz, maximalni frekvence XPT2046 je 125kHz
	// 21000kHz/256=82kHz
	// 21000kHz/128=164kHz
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(TP_SPI, &SPI_InitStructure);
	SPI_Cmd(TP_SPI, ENABLE);

	/* Drain SPI2 */
	while (SPI_I2S_GetFlagStatus(TP_SPI, SPI_I2S_FLAG_TXE) == RESET)
	{}
	SPI_I2S_ReceiveData(TP_SPI);
}

void SPI_touch_panel_fast_init(void)
{
	SPI2_Fast_Init();							// zapouzdreni kvuli jednodussimu oddeleni vrstev
}

void SPI_touch_panel_slow_init(void)
{
	SPI2_Slow_Init();							// zapouzdreni kvuli jednodussimu oddeleni vrstev
}

uint8_t SPI2_ReadWrite(uint8_t writedat)
{
	uint16_t i = 0;

    /* Loop while DR register in not empty */
    while(SPI_I2S_GetFlagStatus(TP_SPI, SPI_I2S_FLAG_TXE) == RESET);
    /* Send byte through the SPI1 peripheral */
    SPI_I2S_SendData(TP_SPI, writedat);

    /* Wait to receive a byte */
    while(SPI_I2S_GetFlagStatus(TP_SPI, SPI_I2S_FLAG_RXNE) == RESET);
    i = SPI_I2S_ReceiveData(TP_SPI) ;

    /* Return the byte read from the SPI bus */
    return i;
}

uint8_t Touch_panel_ReadWrite(uint8_t writedat)
{
	return SPI2_ReadWrite(writedat);			// zapouzdreni kvuli jednodussimu oddeleni vrstev
}

void Init_display_FSMC(void)
{

	FSMC_NORSRAMTimingInitTypeDef timing={0};
	FSMC_NORSRAMInitTypeDef init={0};

	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC,ENABLE);

	timing.FSMC_AddressSetupTime=0x00;
	timing.FSMC_DataSetupTime=0x0A;
	timing.FSMC_CLKDivision=0x0;
	timing.FSMC_AccessMode=FSMC_AccessMode_A;

  /*
   * Data/Address MUX = Disable
   * Memory Type = SRAM
   * Data Width = 16bit
   * Write Operation = Enable
   * Extended Mode = Disable
   * Asynchronous Wait = Disable
   */

	init.FSMC_Bank=FSMC_Bank1_NORSRAM1;
	init.FSMC_DataAddressMux=FSMC_DataAddressMux_Disable;
	init.FSMC_MemoryType=FSMC_MemoryType_SRAM;
	init.FSMC_MemoryDataWidth=FSMC_MemoryDataWidth_16b;
	init.FSMC_BurstAccessMode=FSMC_BurstAccessMode_Disable;
	init.FSMC_WaitSignalPolarity=FSMC_WaitSignalPolarity_Low;
	init.FSMC_WrapMode=FSMC_WrapMode_Disable;
	init.FSMC_WaitSignalActive=FSMC_WaitSignalActive_BeforeWaitState;
	init.FSMC_WriteOperation=FSMC_WriteOperation_Enable;
	init.FSMC_WaitSignal=FSMC_WaitSignal_Disable;
	init.FSMC_ExtendedMode=FSMC_ExtendedMode_Disable;
	init.FSMC_WriteBurst=FSMC_WriteBurst_Disable;
	init.FSMC_ReadWriteTimingStruct=&timing;
	init.FSMC_WriteTimingStruct=&timing;
	init.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable;

	FSMC_NORSRAMInit(&init);
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1,ENABLE);
}

void set_spi_semaphore(SPI_SEMAPHORE state)
{
	spi_semaphore = state;
}

SPI_SEMAPHORE get_spi_semaphore(void)
{
	return spi_semaphore;
}

void Debug_pins_init(void)
{
	// DEBUG PIN
	GPIO_InitTypeDef GPIO_InitStructure={0};

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin   = DEBUG_PIN_1 | DEBUG_PIN_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
	GPIO_Init(DEBUG_PIN_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(DEBUG_PIN_PORT, DEBUG_PIN_1 | DEBUG_PIN_2);
}

void Set_DMA_SPI_transfer_status(uint8_t status)
{
	dma_spi_status = status;
}

uint8_t Get_DMA_SPI_transfer_status(void)
{
	return dma_spi_status;
}
