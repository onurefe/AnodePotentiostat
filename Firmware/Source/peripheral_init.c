/**
  * @author     Onur Efe
  */ 

/* Includes ------------------------------------------------------------------*/

#include "peripheral_init.h"
#include "peripheral_mapping.h"
#include "stm32f4xx.h"
#include "stm32f4xx_conf.h"
#include "generic.h"

/* Exported functions --------------------------------------------------------*/
void Init_CRC(void)
{
  // Enable CRC clock.
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);
}

void Init_EXTI(void)
{
  EXTI_InitTypeDef extiInitStruct;
   
  /* Init push button controller nInt EXTI line */
  extiInitStruct.EXTI_Line = PB_CONTROLLER_nINT_EXTI_LINE;
  extiInitStruct.EXTI_LineCmd = ENABLE;
  extiInitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  extiInitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_Init(&extiInitStruct);
  
  /* Init ADC busy EXTI line */
  extiInitStruct.EXTI_Line = ADC_BUSY_EXTI_LINE;
  extiInitStruct.EXTI_LineCmd = ENABLE;
  extiInitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  extiInitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_Init(&extiInitStruct);
  
  /* Init Bluetooth module IRQ EXTI line. */
  extiInitStruct.EXTI_Line = BT_MODULE_IRQ_EXTI_LINE;
  extiInitStruct.EXTI_LineCmd = ENABLE;
  extiInitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
  extiInitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_Init(&extiInitStruct);
}

void Init_NVIC(void)
{
  NVIC_InitTypeDef nvicInitStruct;
  
  /* Init Push Button Controller nINT interrupt channel */
  /* Clear pending interrupt, if there is any. */
  EXTI_ClearFlag(PB_CONTROLLER_nINT_EXTI_LINE);
  NVIC_ClearPendingIRQ(PB_CONTROLLER_nINT_EXTI_IRQ_CHANNEL);
  
  nvicInitStruct.NVIC_IRQChannel = PB_CONTROLLER_nINT_EXTI_IRQ_CHANNEL;
  nvicInitStruct.NVIC_IRQChannelCmd = ENABLE;
  nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 0x0F;
  nvicInitStruct.NVIC_IRQChannelSubPriority = 0x00;
  NVIC_Init(&nvicInitStruct);
  
  /* Init ADC not busy interrupt. */
  /* Clear pending interrupt, if there is any. */
  EXTI_ClearFlag(ADC_BUSY_EXTI_LINE);
  NVIC_ClearPendingIRQ(ADC_BUSY_EXTI_IRQ_CHANNEL);
  
  nvicInitStruct.NVIC_IRQChannel = ADC_BUSY_EXTI_IRQ_CHANNEL;
  nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 0x01;
  NVIC_Init(&nvicInitStruct);
  
  /* Clear update interrupt pending bit, and enable microscheduler timer interrupt. */
  TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  NVIC_ClearPendingIRQ(TIM2_IRQn);
  
  nvicInitStruct.NVIC_IRQChannel = TIM2_IRQn;
  nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 0x02;
  NVIC_Init(&nvicInitStruct);
  
  /* Clear update interrupt pending bit, and enable voltammetry core timer interrupt. */
  TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
  NVIC_ClearPendingIRQ(TIM5_IRQn);
  
  nvicInitStruct.NVIC_IRQChannel = TIM5_IRQn;
  nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 0x03;
  NVIC_Init(&nvicInitStruct);
  
  /* Clear update interrupt bit, and enable eis control timer interrupt. */
  TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
  NVIC_ClearPendingIRQ(TIM6_DAC_IRQn);
  
  nvicInitStruct.NVIC_IRQChannel = TIM6_DAC_IRQn;
  nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 0x04;
  NVIC_Init(&nvicInitStruct);
  
  /* Init BT module IRQ EXTI line interrupt channel. */
  EXTI_ClearFlag(BT_MODULE_IRQ_EXTI_LINE);
  NVIC_ClearPendingIRQ(BT_MODULE_IRQ_EXTI_IRQ_CHANNEL);
  
  nvicInitStruct.NVIC_IRQChannel = BT_MODULE_IRQ_EXTI_IRQ_CHANNEL;
  nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 0x05;
  NVIC_Init(&nvicInitStruct);
  
  /* Init Serial Protocol IRQ channel. */
  NVIC_ClearPendingIRQ(SERIAL_PROTOCOL_IRQ_CHANNEL);
  
  nvicInitStruct.NVIC_IRQChannel = SERIAL_PROTOCOL_IRQ_CHANNEL;
  nvicInitStruct.NVIC_IRQChannelPreemptionPriority = 0x06;
  NVIC_Init(&nvicInitStruct);
}

void Init_GPIO(void)
{
  GPIO_InitTypeDef gpioInitStruct;
  
  /* Enable GPIO clocks */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  
  /* Set GPIO's to their initial states-------------------------------------- */
  
  /* Push button controller */
  GPIO_SetBits(PB_CONTROLLER_nKILL_PORT, PB_CONTROLLER_nKILL_PIN);
  
  /* Analog digital converter */
  GPIO_ResetBits(ADC_CNV_PORT, ADC_CNV_PIN);
  
  /* Analog configuration and control */
  GPIO_ResetBits(AS_SCALER_BIN_LSB_PORT, AS_SCALER_BIN_LSB_PIN);
  GPIO_ResetBits(AS_SCALER_BIN_MSB_PORT, AS_SCALER_BIN_MSB_PIN);
  GPIO_ResetBits(AS_SCALER_DEC_LSB_PORT, AS_SCALER_DEC_LSB_PIN);
  GPIO_ResetBits(AS_SCALER_DEC_MSB_PORT, AS_SCALER_DEC_MSB_PIN);
  GPIO_ResetBits(AS_OUTPUT_STAGE_EN_OUT_PORT, AS_OUTPUT_STAGE_EN_OUT_PIN);
  GPIO_ResetBits(AS_OUTPUT_STAGE_MODE_EIS_PORT, AS_OUTPUT_STAGE_MODE_EIS_PIN);
  GPIO_ResetBits(AS_OUTPUT_STAGE_MODE_VOLTAMMETRY_PORT, AS_OUTPUT_STAGE_MODE_VOLTAMMETRY_PIN);
  GPIO_ResetBits(ANALOG_ON_OFF_PORT, ANALOG_ON_OFF_PIN);
  GPIO_ResetBits(CALIBRATION_RELAY_PORT, CALIBRATION_RELAY_PIN);
  
  /* Bias DAC */
  GPIO_SetBits(DAC_BIAS_nCS_PORT, DAC_BIAS_nCS_PIN);
  GPIO_ResetBits(DAC_BIAS_nLDAC_PORT, DAC_BIAS_nLDAC_PIN);
  
  /* Signal DAC */
  GPIO_SetBits(DAC_SIGNAL_nCS_PORT, DAC_SIGNAL_nCS_PIN);
  GPIO_ResetBits(DAC_SIGNAL_nLDAC_PORT, DAC_SIGNAL_nLDAC_PIN);
  
  /* TIA */
  GPIO_SetBits(TIA_nCS_PORT, TIA_nCS_PIN);
  GPIO_SetBits(TIA_nLATCH_PORT, TIA_nLATCH_PIN);
  
  /* Bluetooth module. */
  
  /* Power indication leds. */
  GPIO_ResetBits(PWR_IND_0_PORT, PWR_IND_0_PIN);
  GPIO_ResetBits(PWR_IND_1_PORT, PWR_IND_1_PIN);
  GPIO_ResetBits(PWR_IND_2_PORT, PWR_IND_2_PIN);
  
  /* GPIO initialization -----------------------------------------------------*/
  /* Push button controller pins */
  gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
  gpioInitStruct.GPIO_OType = GPIO_OType_PP;
  gpioInitStruct.GPIO_Pin = PB_CONTROLLER_nKILL_PIN;
  gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpioInitStruct.GPIO_Speed = GPIO_Low_Speed;
  GPIO_Init(PB_CONTROLLER_nKILL_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Mode = GPIO_Mode_IN;
  gpioInitStruct.GPIO_Pin = PB_CONTROLLER_nINT_PIN;
  gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(PB_CONTROLLER_nINT_PORT, &gpioInitStruct);
  
  /* HUB SPI pins */
  gpioInitStruct.GPIO_Mode = GPIO_Mode_AF;
  gpioInitStruct.GPIO_OType = GPIO_OType_PP;
  gpioInitStruct.GPIO_Pin = HUB_SCK_PIN;
  gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpioInitStruct.GPIO_Speed = GPIO_High_Speed;
  GPIO_Init(HUB_SCK_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = HUB_MOSI_PIN;
  GPIO_Init(HUB_MOSI_PORT, &gpioInitStruct);
  
  /* ADC Convert, SCK, SDO and BUSY pins */
  gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
  gpioInitStruct.GPIO_OType = GPIO_OType_PP;
  gpioInitStruct.GPIO_Pin = ADC_CNV_PIN;
  gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpioInitStruct.GPIO_Speed = GPIO_High_Speed;
  GPIO_Init(ADC_CNV_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Mode = GPIO_Mode_AF;
  gpioInitStruct.GPIO_Pin = ADC_SCK_PIN;
  GPIO_Init(ADC_SCK_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = ADC_SDO_PIN;
  GPIO_Init(ADC_SDO_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Mode = GPIO_Mode_IN;
  gpioInitStruct.GPIO_Pin = ADC_BUSY_PIN;
  gpioInitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(ADC_BUSY_PORT, &gpioInitStruct);
  
  /* Vref sens pin. */
  gpioInitStruct.GPIO_Mode = GPIO_Mode_AN;
  gpioInitStruct.GPIO_Pin = VIN_SENS_PIN;
  gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(VIN_SENS_PORT, &gpioInitStruct);
  
  /* Analog control pins. */
  gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
  gpioInitStruct.GPIO_OType = GPIO_OType_PP;
  gpioInitStruct.GPIO_Pin = AS_SCALER_BIN_LSB_PIN;
  gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpioInitStruct.GPIO_Speed = GPIO_Low_Speed;
  GPIO_Init(AS_SCALER_BIN_LSB_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = AS_SCALER_BIN_MSB_PIN;
  GPIO_Init(AS_SCALER_BIN_MSB_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = AS_SCALER_DEC_LSB_PIN;
  GPIO_Init(AS_SCALER_DEC_LSB_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = AS_SCALER_DEC_MSB_PIN;
  GPIO_Init(AS_SCALER_DEC_MSB_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = AS_OUTPUT_STAGE_EN_OUT_PIN;
  GPIO_Init(AS_OUTPUT_STAGE_EN_OUT_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = AS_OUTPUT_STAGE_MODE_EIS_PIN;
  GPIO_Init(AS_OUTPUT_STAGE_MODE_EIS_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = AS_OUTPUT_STAGE_MODE_VOLTAMMETRY_PIN;
  GPIO_Init(AS_OUTPUT_STAGE_MODE_VOLTAMMETRY_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = ANALOG_ON_OFF_PIN;
  GPIO_Init(ANALOG_ON_OFF_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = CALIBRATION_RELAY_PIN;
  GPIO_Init(CALIBRATION_RELAY_PORT, &gpioInitStruct);
  
  /* Bias DAC pins */
  gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
  gpioInitStruct.GPIO_OType = GPIO_OType_PP;
  gpioInitStruct.GPIO_Pin = DAC_BIAS_nCS_PIN;
  gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpioInitStruct.GPIO_Speed = GPIO_High_Speed;
  
  GPIO_Init(DAC_BIAS_nCS_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = DAC_BIAS_nLDAC_PIN;
  gpioInitStruct.GPIO_Speed = GPIO_High_Speed;
  
  GPIO_Init(DAC_BIAS_nLDAC_PORT, &gpioInitStruct);
  
  /* Signal DAC pins */
  gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
  gpioInitStruct.GPIO_OType = GPIO_OType_PP;
  gpioInitStruct.GPIO_Pin = DAC_SIGNAL_nCS_PIN;
  gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpioInitStruct.GPIO_Speed = GPIO_High_Speed;
  GPIO_Init(DAC_SIGNAL_nCS_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = DAC_SIGNAL_nLDAC_PIN;
  gpioInitStruct.GPIO_Speed = GPIO_High_Speed;
  GPIO_Init(DAC_SIGNAL_nLDAC_PORT, &gpioInitStruct);
  
  /* TIA pins */
  gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
  gpioInitStruct.GPIO_OType = GPIO_OType_PP;
  gpioInitStruct.GPIO_Pin = TIA_nCS_PIN;
  gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpioInitStruct.GPIO_Speed = GPIO_High_Speed;
  GPIO_Init(TIA_nCS_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = TIA_nLATCH_PIN;
  gpioInitStruct.GPIO_Speed = GPIO_High_Speed;
  GPIO_Init(TIA_nLATCH_PORT, &gpioInitStruct);
  
  /* Bluetooth module pins. */
  // nRST pin.
  gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
  gpioInitStruct.GPIO_OType = GPIO_OType_PP;
  gpioInitStruct.GPIO_Pin = BT_MODULE_nRST_PIN;
  gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpioInitStruct.GPIO_Speed = GPIO_Low_Speed;
  GPIO_Init(BT_MODULE_nRST_PORT, &gpioInitStruct);
  
  // nCS pin
  gpioInitStruct.GPIO_Pin = BT_MODULE_nCS_PIN;
  gpioInitStruct.GPIO_Speed = GPIO_High_Speed;
  GPIO_Init(BT_MODULE_nCS_PORT, &gpioInitStruct);
  
  // IRQ pin  
  gpioInitStruct.GPIO_Pin = BT_MODULE_IRQ_PIN;
  gpioInitStruct.GPIO_Mode = GPIO_Mode_IN;
  gpioInitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(BT_MODULE_IRQ_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(BT_MODULE_IRQ_PORT, &gpioInitStruct);
  
  // MOSI
  gpioInitStruct.GPIO_Pin = BT_MODULE_MOSI_PIN;
  gpioInitStruct.GPIO_Mode = GPIO_Mode_AF;
  gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpioInitStruct.GPIO_Speed = GPIO_High_Speed;
  gpioInitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_Init(BT_MODULE_MOSI_PORT, &gpioInitStruct);
  
  // MISO
  gpioInitStruct.GPIO_Pin = BT_MODULE_MISO_PIN;
  GPIO_Init(BT_MODULE_MISO_PORT, &gpioInitStruct);
  
  // CLK
  gpioInitStruct.GPIO_Pin = BT_MODULE_CLK_PIN;
  GPIO_Init(BT_MODULE_CLK_PORT, &gpioInitStruct);
  
  /* Serial protocol UART pins. */
  gpioInitStruct.GPIO_OType = GPIO_OType_PP;
  gpioInitStruct.GPIO_Mode = GPIO_Mode_AF;
  gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpioInitStruct.GPIO_Speed = GPIO_High_Speed;
  
  gpioInitStruct.GPIO_Pin = SERIAL_PROTOCOL_UART_RX_PIN;
  GPIO_Init(SERIAL_PROTOCOL_UART_RX_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = SERIAL_PROTOCOL_UART_TX_PIN;
  GPIO_Init(SERIAL_PROTOCOL_UART_TX_PORT, &gpioInitStruct);

  /* Power indication led pins. */
  gpioInitStruct.GPIO_Pin = PWR_IND_0_PIN;
  gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
  gpioInitStruct.GPIO_Speed = GPIO_Low_Speed;
  gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
  gpioInitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_Init(PWR_IND_0_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = PWR_IND_1_PIN;
  GPIO_Init(PWR_IND_1_PORT, &gpioInitStruct);
  
  gpioInitStruct.GPIO_Pin = PWR_IND_2_PIN;
  GPIO_Init(PWR_IND_2_PORT, &gpioInitStruct);
  
  /* Alternating function configuration --------------------------------------*/
  /* Hub SPI pins. */
  GPIO_PinAFConfig(HUB_SCK_PORT, HUB_SCK_PIN_SOURCE, HUB_SPI_AF_MAPPING);
  GPIO_PinAFConfig(HUB_MOSI_PORT, HUB_MOSI_PIN_SOURCE, HUB_SPI_AF_MAPPING);
  
  /* ADC SCK and SDO pins */
  GPIO_PinAFConfig(ADC_SCK_PORT, ADC_SCK_PIN_SOURCE, ADC_SPI_AF_MAPPING);
  GPIO_PinAFConfig(ADC_SDO_PORT, ADC_SDO_PIN_SOURCE, ADC_SPI_AF_MAPPING);
  
  /* Bluetooth module alternating function configuration. */
  GPIO_PinAFConfig(BT_MODULE_CLK_PORT, BT_MODULE_CLK_PIN_SOURCE, BT_MODULE_SPI_AF_MAPPING);
  GPIO_PinAFConfig(BT_MODULE_MOSI_PORT, BT_MODULE_MOSI_PIN_SOURCE, BT_MODULE_SPI_AF_MAPPING);
  GPIO_PinAFConfig(BT_MODULE_MISO_PORT, BT_MODULE_MISO_PIN_SOURCE, BT_MODULE_SPI_AF_MAPPING);
                   
  /* Set serial protocol alternate function mapping. */
  GPIO_PinAFConfig(SERIAL_PROTOCOL_UART_TX_PORT, SERIAL_PROTOCOL_UART_TX_PIN_SOURCE, 
                   SERIAL_PROTOCOL_UART_AF_MAPPING);
  GPIO_PinAFConfig(SERIAL_PROTOCOL_UART_RX_PORT, SERIAL_PROTOCOL_UART_RX_PIN_SOURCE, 
                   SERIAL_PROTOCOL_UART_AF_MAPPING);
}

void Init_SPI(void)
{
  SPI_InitTypeDef spiInitStruct;

  /* Enable SPI clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
  
  /* Init SPI1(Hub channel for DAC Signal, DAC Bias and TIA. */
  spiInitStruct.SPI_Direction = SPI_Direction_1Line_Tx;
  spiInitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
  spiInitStruct.SPI_Mode = SPI_Mode_Master;
  spiInitStruct.SPI_CRCPolynomial = 0;
  spiInitStruct.SPI_CPHA = SPI_CPHA_2Edge;
  spiInitStruct.SPI_CPOL = SPI_CPOL_Low;
  spiInitStruct.SPI_DataSize = SPI_DataSize_8b;
  spiInitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
  spiInitStruct.SPI_NSS = SPI_NSS_Soft;
  SPI_Init(HUB_SPI, &spiInitStruct);
  
  /* Init ADC SPI */
  spiInitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
  spiInitStruct.SPI_CPHA = SPI_CPHA_1Edge;
  spiInitStruct.SPI_CPOL = SPI_CPOL_Low;
  spiInitStruct.SPI_DataSize = SPI_DataSize_16b;
  spiInitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  spiInitStruct.SPI_CRCPolynomial = 0;
  spiInitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
  spiInitStruct.SPI_Mode = SPI_Mode_Master;
  spiInitStruct.SPI_NSS = SPI_NSS_Soft;
  SPI_Init(ADC_SPI, &spiInitStruct);
  
  /* Init Bluetooth module SPI. */
  spiInitStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
  spiInitStruct.SPI_CPHA = SPI_CPHA_1Edge;
  spiInitStruct.SPI_CPOL = SPI_CPOL_Low;
  spiInitStruct.SPI_DataSize = SPI_DataSize_8b;
  spiInitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  spiInitStruct.SPI_CRCPolynomial = 0;
  spiInitStruct.SPI_FirstBit = SPI_FirstBit_MSB;
  spiInitStruct.SPI_Mode = SPI_Mode_Master;
  spiInitStruct.SPI_NSS = SPI_NSS_Soft;
  SPI_Init(BT_MODULE_SPI, &spiInitStruct);
}

void Init_TIM(void)
{
  /* Enable TIM clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
  
  TIM_TimeBaseInitTypeDef timTimeBaseInitStruct;
  
  /* Configure micro scheduler timer, and enable update interrupt. */
  timTimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  timTimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  timTimeBaseInitStruct.TIM_Period = 168;      // Period of 2us.
  timTimeBaseInitStruct.TIM_Prescaler = 0;
  timTimeBaseInitStruct.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM2, &timTimeBaseInitStruct);
  
  TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
  
  /* Configure voltammetry core timer, and enable update interrupt. */
  timTimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  timTimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  timTimeBaseInitStruct.TIM_Period = 168;      // Period of 2us.
  timTimeBaseInitStruct.TIM_Prescaler = 0;
  timTimeBaseInitStruct.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM5, &timTimeBaseInitStruct);
  
  TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
  
  /* Configure eis scheduler timer, and enable update interrupt. */
  timTimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  timTimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  timTimeBaseInitStruct.TIM_Period = 1000;     // Period of 1ms, but it will be reconfigured.
  timTimeBaseInitStruct.TIM_Prescaler = 83;
  timTimeBaseInitStruct.TIM_RepetitionCounter = 0;
  
  TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
}

void Init_USART(void)
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    
  USART_InitTypeDef uartInitStruct;
  
  uartInitStruct.USART_BaudRate = 57600;
  uartInitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  uartInitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  uartInitStruct.USART_Parity = USART_Parity_No;
  uartInitStruct.USART_StopBits = USART_StopBits_1;
  uartInitStruct.USART_WordLength = USART_WordLength_8b;
    
  USART_Init(UART4, &uartInitStruct);

  USART_ITConfig(UART4, USART_IT_TC, ENABLE);
  USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
  USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);
  USART_ITConfig(UART4, USART_IT_ERR, ENABLE);
}

void Init_SYSCFG(void)
{
  /* Enable SYSCFG block */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  /* Enable I/0 compensation cell */
  SYSCFG_CompensationCellCmd(ENABLE);

  /* Wait until it's enabled */
  while(SET != SYSCFG_GetCompensationCellStatus());
  
  /* Config EXTI line of the push button controller nINT pin */
  SYSCFG_EXTILineConfig(PB_CONTROLLER_nINT_EXTI_PORT_SOURCE, 
                        PB_CONTROLLER_nINT_EXTI_PIN_SOURCE);
  
  /* Config EXTI line of the ADC busy pin */
  SYSCFG_EXTILineConfig(ADC_BUSY_EXTI_PORT_SOURCE,
                        ADC_BUSY_EXTI_PIN_SOURCE);
  
  /* Config EXTI line of the BlueNRG IRQ pin */
  SYSCFG_EXTILineConfig(BT_MODULE_IRQ_EXTI_PORT_SOURCE,
                        BT_MODULE_IRQ_EXTI_PIN_SOURCE);
}