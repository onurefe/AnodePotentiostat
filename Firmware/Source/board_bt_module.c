/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_conf.h"
#include "peripheral_mapping.h"
#include "middlewares.h"
#include "board_bt_module.h"


#define HEADER_SIZE 5
#define MAX_BUFFER_SIZE 255

/* Private function prototypes. ----------------------------------------------*/
static int32_t transmit(uint8_t* data1, uint8_t* data2, uint8_t Nb_bytes1, uint8_t Nb_bytes2);
static void transmit_receive(const uint8_t *pTxData, uint8_t *pRxData, uint8_t size);
static void us150Delay(void);


void Board_BTModule_Open(void)
{
  SPI_Cmd(BT_MODULE_SPI, ENABLE);                               /* Enable Bluetooth module SPI. */
  
  GPIO_ResetBitsOpt(BT_MODULE_nRST_PORT, BT_MODULE_nRST_PIN);
  Utils_DelayMs(5);
  GPIO_SetBitsOpt(BT_MODULE_nRST_PORT, BT_MODULE_nRST_PIN);
  Utils_DelayMs(5);
}

void Board_BTModule_Close(void)
{
  while ((SPI_I2S_GetFlagStatusOpt(BT_MODULE_SPI, SPI_I2S_FLAG_TXE) == RESET));
  while ((SPI_I2S_GetFlagStatusOpt(BT_MODULE_SPI, SPI_I2S_FLAG_BSY) == SET));
  SPI_Cmd(BT_MODULE_SPI, DISABLE);
}

/**
 * @brief  Writes data to a serial interface.
 * @param  data1   :  1st buffer
 * @param  data2   :  2nd buffer
 * @param  n_bytes1: number of bytes in 1st buffer
 * @param  n_bytes2: number of bytes in 2nd buffer
 * @retval None
 */
void Board_BTModule_WriteSerial(const void* data1, const void* data2, int32_t n_bytes1,
                                int32_t n_bytes2)
{
  while(1)
  {
    if (transmit((uint8_t *)data1, (uint8_t *)data2, n_bytes1, n_bytes2) == 0U)
    {
      break;
    }
  }
}

/**
 * @brief  Resets the BlueNRG.
 * @param  None
 * @retval None
 */
void Board_BTModule_Reset(void)
{
  GPIO_ResetBitsOpt(BT_MODULE_nRST_PORT, BT_MODULE_nRST_PIN);
  Utils_DelayMs(5);
  GPIO_SetBitsOpt(BT_MODULE_nRST_PORT, BT_MODULE_nRST_PIN);
  Utils_DelayMs(5);
}

/**
 * @brief  Reports if the BlueNRG has data for the host micro.
 * @param  None
 * @retval 1 if data are present, 0 otherwise
 */
// FIXME: find a better way to handle this return value (bool type? TRUE and FALSE)
uint8_t Board_BTModule_IsDataPresent(void)
{
  if (GPIO_ReadInputDataBit(BT_MODULE_IRQ_PORT, BT_MODULE_IRQ_PIN) != 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }   
} /* end Board_BTModule_IsDataPresent() */

/**
 * @brief  Reads from BlueNRG SPI buffer and store data into local buffer.
 * @param  buffer   : Buffer where data from SPI are stored
 * @param  buff_size: Buffer size
 * @retval int32_t  : Number of read bytes
 */
int32_t Board_BTModule_ReadAll(uint8_t *buffer, uint8_t buff_size)
{
  uint16_t byte_count;
  uint8_t len = 0;
  uint8_t char_ff = 0xff;
  volatile uint8_t read_char;

  uint8_t header_master[HEADER_SIZE] = {0x0b, 0x00, 0x00, 0x00, 0x00};
  uint8_t header_slave[HEADER_SIZE];

  /* CS reset */
  GPIO_ResetBitsOpt(BT_MODULE_nCS_PORT, BT_MODULE_nCS_PIN);

  /* Read the header */
  transmit_receive(header_master, header_slave, HEADER_SIZE);
  	
  if (header_slave[0] == 0x02) 
  {
    /* device is ready */
    byte_count = (header_slave[4] << 8) | header_slave[3];
  
    if (byte_count > 0) 
    {
      /* avoid to read more data that size of the buffer */
      if (byte_count > buff_size)
      {
        byte_count = buff_size;
      }
  
      for (len = 0; len < byte_count; len++)
      {                                               
        transmit_receive(&char_ff, (uint8_t*)&read_char, 1); 
        buffer[len] = read_char;                                                            
      }                                                                                     
      
    }    
  }
  
  /* Release CS line */
  GPIO_SetBitsOpt(BT_MODULE_nCS_PORT, BT_MODULE_nCS_PIN);
  
  // Add a small delay to give time to the BlueNRG to set the IRQ pin low
  // to avoid a useless SPI read at the end of the transaction.
  for(volatile int i = 0; i < 2; i++)__NOP();
  
  return len;  
}

/**
 * @brief  Transmits data from local buffer to SPI.
 * @param  data1    : First data buffer to be written
 * @param  data2    : Second data buffer to be written
 * @param  Nb_bytes1: Size of first data buffer to be written
 * @param  Nb_bytes2: Size of second data buffer to be written
 * @retval Number of read bytes
 */
int32_t transmit(uint8_t* data1, uint8_t* data2, uint8_t Nb_bytes1, uint8_t Nb_bytes2)
{  
  int32_t result = 0;  
  int32_t spi_fix_enabled = 0;
  
#ifdef ENABLE_SPI_FIX
  spi_fix_enabled = 1;
#endif //ENABLE_SPI_FIX
  
  unsigned char header_master[HEADER_SIZE] = {0x0a, 0x00, 0x00, 0x00, 0x00};
  unsigned char header_slave[HEADER_SIZE]  = {0xaa, 0x00, 0x00, 0x00, 0x00};
  
  unsigned char read_char_buf[MAX_BUFFER_SIZE];

  /* Disable IRQ channel interrupt. */
  NVIC_DisableIRQ(BT_MODULE_IRQ_EXTI_IRQ_CHANNEL);
  
  /*
   If the SPI_FIX is enabled the IRQ is set in Output mode, then it is pulled
   high and, after a delay of at least 112us, the CS line is asserted and the
   header transmit/receive operations are started.
   After these transmit/receive operations the IRQ is reset in input mode.
 */
  if (spi_fix_enabled) 
  {
    GPIO_InitTypeDef gpioInitStruct;
  
    /* Pull IRQ high */
    gpioInitStruct.GPIO_Pin = BT_MODULE_IRQ_PIN;
    gpioInitStruct.GPIO_Mode = GPIO_Mode_OUT;
    gpioInitStruct.GPIO_OType = GPIO_OType_PP;
    gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpioInitStruct.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(BT_MODULE_IRQ_PORT, &gpioInitStruct);
    
    GPIO_SetBitsOpt(BT_MODULE_IRQ_PORT, BT_MODULE_IRQ_PIN);

    /* Assert CS line after at least 112us */
    us150Delay();
  }

  /* CS reset */
  GPIO_ResetBitsOpt(BT_MODULE_nCS_PORT, BT_MODULE_nCS_PIN);
  
  /* Exchange header */  
  transmit_receive(header_master, header_slave, HEADER_SIZE);
  
  if (spi_fix_enabled) 
  {
    GPIO_InitTypeDef gpioInitStruct;
  
    /* IRQ input */
    gpioInitStruct.GPIO_Pin = BT_MODULE_IRQ_PIN;
    gpioInitStruct.GPIO_Mode = GPIO_Mode_IN;
    gpioInitStruct.GPIO_OType = GPIO_OType_PP;
    gpioInitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    gpioInitStruct.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(BT_MODULE_IRQ_PORT, &gpioInitStruct);
    
    gpioInitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(BT_MODULE_IRQ_PORT, &gpioInitStruct);
  }

  if (header_slave[0] == 0x02) 
  {
    /* SPI is ready */
    if (header_slave[1] >= (Nb_bytes1+Nb_bytes2)) 
    {
      /*  Buffer is big enough */
      if (Nb_bytes1 > 0) 
      {
        transmit_receive(data1, read_char_buf, Nb_bytes1);
      }
      if (Nb_bytes2 > 0) 
      {
        transmit_receive(data2, read_char_buf, Nb_bytes2);
      }
    } 
    else 
    {
      /* Buffer is too small */
      result = -2;
    }
  } 
  else 
  {
    /* SPI is not ready */
    result = -1;
  }
	
  /* Release CS line */
  GPIO_SetBitsOpt(BT_MODULE_nCS_PORT, BT_MODULE_nCS_PIN);
  
  /* Enable IRQ channel interrupt. */
  NVIC_EnableIRQ(BT_MODULE_IRQ_EXTI_IRQ_CHANNEL);
    
  return result;
}

/**
 * @brief Transmits and receives data simultaneously in blocking mode.
 */
static void transmit_receive(const uint8_t *pTxData, uint8_t *pRxData, uint8_t size)
{
  uint8_t i;
  
  for (i = 0; i < size; i++)
  {
    SPI_I2S_SendDataOpt(BT_MODULE_SPI, *pTxData++);
    while ((SPI_I2S_GetFlagStatus(BT_MODULE_SPI, SPI_I2S_FLAG_TXE) == RESET) || \
           (SPI_I2S_GetFlagStatus(BT_MODULE_SPI, SPI_I2S_FLAG_BSY) == SET));
    *pRxData++ = (uint8_t)SPI_I2S_ReceiveDataOpt(BT_MODULE_SPI);
  }
}

/**
 * @brief  Utility function for delay
 * @param  None
 * @retval None
 * NOTE: TODO: implement with clock-independent function.
 */
static void us150Delay(void)
{
#if SYSCLK_FREQ == 168000000
  for(volatile int i = 0; i < 2400; i++)__NOP();
#else
#error Implement delay function.
#endif    
}