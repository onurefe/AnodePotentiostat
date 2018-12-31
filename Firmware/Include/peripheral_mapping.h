/**
  * @author     Onur Efe
  */

#ifndef __PERIPHERAL_MAPPING_H
#define __PERIPHERAL_MAPPING_H

#include "generic.h"

/* Alternating function mapping. ---------------------------------------------*/
#define HUB_SPI_AF_MAPPING                      GPIO_AF_SPI1
#define ADC_SPI_AF_MAPPING                      GPIO_AF_SPI2
#define BT_MODULE_SPI_AF_MAPPING                GPIO_AF_SPI3
#define SERIAL_PROTOCOL_UART_AF_MAPPING         GPIO_AF_UART4

/* GPIO Pinmapping -----------------------------------------------------------*/
#define SERIAL_PROTOCOL_UART_TX_PIN             GPIO_Pin_0
#define SERIAL_PROTOCOL_UART_TX_PORT            GPIOA
#define SERIAL_PROTOCOL_UART_TX_PIN_SOURCE      GPIO_PinSource0

#define SERIAL_PROTOCOL_UART_RX_PIN             GPIO_Pin_1
#define SERIAL_PROTOCOL_UART_RX_PORT            GPIOA
#define SERIAL_PROTOCOL_UART_RX_PIN_SOURCE      GPIO_PinSource1 

#define VIN_SENS_PIN                            GPIO_Pin_2
#define VIN_SENS_PORT                           GPIOA

#define CALIBRATION_RELAY_PIN                   GPIO_Pin_3
#define CALIBRATION_RELAY_PORT                  GPIOA

#define DAC_SIGNAL_nCS_PIN                      GPIO_Pin_4
#define DAC_SIGNAL_nCS_PORT                     GPIOA
#define DAC_SIGNAL_PIN_SOURCE                   GPIO_PinSource4

#define HUB_SCK_PIN                             GPIO_Pin_5
#define HUB_SCK_PORT                            GPIOA
#define HUB_SCK_PIN_SOURCE                      GPIO_PinSource5

#define DAC_BIAS_nCS_PIN                        GPIO_Pin_6
#define DAC_BIAS_nCS_PORT                       GPIOA

#define HUB_MOSI_PIN                            GPIO_Pin_7
#define HUB_MOSI_PORT                           GPIOA
#define HUB_MOSI_PIN_SOURCE                     GPIO_PinSource7

#define BT_MODULE_IRQ_PIN                       GPIO_Pin_8
#define BT_MODULE_IRQ_PORT                      GPIOA

#define BT_MODULE_nRST_PIN                      GPIO_Pin_9
#define BT_MODULE_nRST_PORT                     GPIOA                  

#define LED_GREEN_PIN                           GPIO_Pin_10
#define LED_GREEN_PORT                          GPIOA

#define LED_RED_PIN                             GPIO_Pin_11
#define LED_RED_PORT                            GPIOA

#define BT_MODULE_nCS_PIN                       GPIO_Pin_15
#define BT_MODULE_nCS_PORT                      GPIOA

/* PORTB pinmap */
#define AS_SCALER_BIN_LSB_PIN                   GPIO_Pin_0
#define AS_SCALER_BIN_LSB_PORT                  GPIOB

#define AS_SCALER_BIN_MSB_PIN                   GPIO_Pin_1
#define AS_SCALER_BIN_MSB_PORT                  GPIOB

#define AS_SCALER_DEC_LSB_PIN                   GPIO_Pin_2
#define AS_SCALER_DEC_LSB_PORT                  GPIOB

#define AS_SCALER_DEC_MSB_PIN                   GPIO_Pin_4
#define AS_SCALER_DEC_MSB_PORT                  GPIOB

#define AS_OUTPUT_STAGE_EN_OUT_PIN              GPIO_Pin_5
#define AS_OUTPUT_STAGE_EN_OUT_PORT             GPIOB

#define AS_OUTPUT_STAGE_MODE_EIS_PIN            GPIO_Pin_6
#define AS_OUTPUT_STAGE_MODE_EIS_PORT           GPIOB

#define AS_OUTPUT_STAGE_MODE_VOLTAMMETRY_PIN    GPIO_Pin_7
#define AS_OUTPUT_STAGE_MODE_VOLTAMMETRY_PORT   GPIOB

#define ADC_SCK_PIN                             GPIO_Pin_10
#define ADC_SCK_PORT                            GPIOB
#define ADC_SCK_PIN_SOURCE                      GPIO_PinSource10

#define TIA_nLATCH_PIN                          GPIO_Pin_11
#define TIA_nLATCH_PORT                         GPIOB

#define ANALOG_ON_OFF_PIN                       GPIO_Pin_12
#define ANALOG_ON_OFF_PORT                      GPIOB

#define PB_CONTROLLER_nINT_PIN                  GPIO_Pin_13
#define PB_CONTROLLER_nINT_PORT                 GPIOB

#define PB_CONTROLLER_nKILL_PIN                 GPIO_Pin_14
#define PB_CONTROLLER_nKILL_PORT                GPIOB


/* PORTC pinmap */
#define PWR_IND_0_PIN                           GPIO_Pin_0
#define PWR_IND_0_PORT                          GPIOC

#define PWR_IND_1_PIN                           GPIO_Pin_1
#define PWR_IND_1_PORT                          GPIOC

#define ADC_SDO_PIN                             GPIO_Pin_2
#define ADC_SDO_PORT                            GPIOC
#define ADC_SDO_PIN_SOURCE                      GPIO_PinSource2

#define PWR_IND_2_PIN                           GPIO_Pin_3
#define PWR_IND_2_PORT                          GPIOC

#define TIA_nCS_PIN                             GPIO_Pin_4
#define TIA_nCS_PORT                            GPIOC

#define ADC_CNV_PIN                             GPIO_Pin_5
#define ADC_CNV_PORT                            GPIOC

#define ADC_BUSY_PIN                            GPIO_Pin_6
#define ADC_BUSY_PORT                           GPIOC

#define DAC_BIAS_nLDAC_PIN                      GPIO_Pin_7
#define DAC_BIAS_nLDAC_PORT                     GPIOC

#define DAC_SIGNAL_nLDAC_PIN                    GPIO_Pin_8
#define DAC_SIGNAL_nLDAC_PORT                   GPIOC

#define BT_MODULE_CLK_PIN                       GPIO_Pin_10
#define BT_MODULE_CLK_PORT                      GPIOC
#define BT_MODULE_CLK_PIN_SOURCE                GPIO_PinSource10
                
#define BT_MODULE_MISO_PIN                      GPIO_Pin_11
#define BT_MODULE_MISO_PORT                     GPIOC
#define BT_MODULE_MISO_PIN_SOURCE               GPIO_PinSource11

#define BT_MODULE_MOSI_PIN                      GPIO_Pin_12
#define BT_MODULE_MOSI_PORT                     GPIOC
#define BT_MODULE_MOSI_PIN_SOURCE               GPIO_PinSource12

/* EXTI mapping --------------------------------------------------------------*/
/* Push button controller nINT pin */
#define PB_CONTROLLER_nINT_EXTI_LINE            EXTI_Line13
#define PB_CONTROLLER_nINT_EXTI_PIN_SOURCE      EXTI_PinSource13
#define PB_CONTROLLER_nINT_EXTI_PORT_SOURCE     EXTI_PortSourceGPIOB

/* ADC busy pin */
#define ADC_BUSY_EXTI_LINE                      EXTI_Line6
#define ADC_BUSY_EXTI_PIN_SOURCE                EXTI_PinSource6
#define ADC_BUSY_EXTI_PORT_SOURCE               EXTI_PortSourceGPIOC

/* Bluetooth module IRQ pin. */
#define BT_MODULE_IRQ_EXTI_LINE                 EXTI_Line8
#define BT_MODULE_IRQ_EXTI_PIN_SOURCE           EXTI_PinSource8
#define BT_MODULE_IRQ_EXTI_PORT_SOURCE          EXTI_PortSourceGPIOA

/* TIMER mapping -------------------------------------------------------------*/
#define VOLTAMMETRY_CORE_TIMER                  TIM5
#define VOLTAMMETRY_CORE_TIMER_FREQUENCY        84000000
#define VOLTAMMETRY_CORE_TIMER_MAX_RELOAD       MAX_UINT32
#define VOLTAMMETRY_CORE_TIMER_MAX_PRESCALER    MAX_UINT16

#define EIS_CORE_TIMER                          TIM2
#define EIS_CORE_TIMER_FREQUENCY                84000000
#define EIS_CORE_TIMER_MAX_RELOAD               MAX_UINT32
#define EIS_CORE_TIMER_MAX_PRESCALER            MAX_UINT16

/* SPI mapping ---------------------------------------------------------------*/
#define HUB_SPI                                 SPI1
#define ADC_SPI                                 SPI2       
#define BT_MODULE_SPI                           SPI3

/* USART mapping -------------------------------------------------------------*/
#define SERIAL_PROTOCOL_UART                    UART4

/* IRQ mapping ---------------------------------------------------------------*/
#define PB_CONTROLLER_nINT_EXTI_IRQ_CHANNEL     EXTI15_10_IRQn
#define ADC_BUSY_EXTI_IRQ_CHANNEL               EXTI9_5_IRQn
#define BT_MODULE_IRQ_EXTI_IRQ_CHANNEL          EXTI9_5_IRQn
#define SERIAL_PROTOCOL_IRQ_CHANNEL             UART4_IRQn                    

#endif