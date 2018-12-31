/**
  * @author     Onur Efe
  */

#ifndef __PERIPHERAL_INIT_H
#define __PERIPHERAL_INIT_H

/* Exported functions --------------------------------------------------------*/
extern void Init_CRC(void);
extern void Init_EXTI(void);
extern void Init_GPIO(void);
extern void Init_SPI(void);
extern void Init_TIM(void);
extern void Init_USART(void);
extern void Init_ADC(void);
extern void Init_DMA(void);
extern void Init_SYSCFG(void);
extern void Init_NVIC(void);

#endif