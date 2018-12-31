/**
  * @author     Onur Efe
  */
/* Includes ------------------------------------------------------------------*/
#include "packet_manager.h"
#include "middlewares.h"
#include "stm32f4xx_conf.h"
#include "peripheral_mapping.h"

/* Private definitions -------------------------------------------------------*/
#define SDU_OVERHEAD                            2
    
#define FRAMING_OVERHEAD                        2
    
#define WORST_ENCODING_MULTIPLIER               2
    
#define MAX_PACKET_SIZE                         (PACKET_MANAGER_MAX_SDU_SIZE + SDU_OVERHEAD)
    
#define MAX_FRAME_SIZE                          (MAX_PACKET_SIZE * WORST_ENCODING_MULTIPLIER + \
                                                 FRAMING_OVERHEAD)

#define RECEIVE_BUFFER_SIZE                     (8 * MAX_FRAME_SIZE)
#define TRANSMIT_BUFFER_SIZE                    65535

/* Private typedefs ----------------------------------------------------------*/
typedef enum
{
  START_CHARACTER                               = 0x0D,
  TERMINATE_CHARACTER                           = 0x3A,
  ESCAPE_CHARACTER                              = 0x3B
} SpecialCharacter_t;

typedef enum
{
  START_CHARACTER_CODE                          = 0x00,
  TERMINATE_CHARACTER_CODE                      = 0x01,
  ESCAPE_CHARACTER_CODE                         = 0x02
} SpecialCharacterEscapeCode_t;

/* Private function prototypes -----------------------------------------------*/
static uint8_t  *dequeueAndDispatch(uint32_t frameSize, uint32_t *pSduLength);
static void     patchAndEnqueue(uint8_t *pSdu, uint32_t sduLength);

/* Private variables ---------------------------------------------------------*/
static PacketManager_SduReceivedDelegate_t      SduReceivedDelegate;
static PacketManager_ErrorOccurredDelegate_t    ErrorOccurredDelegate;

static uint8_t                                  State = PACKET_MANAGER_STATE_UNINIT;

static uint8_t                                  ReceiveBufferContainer[RECEIVE_BUFFER_SIZE];
static uint8_t                                  TransmitBufferContainer[TRANSMIT_BUFFER_SIZE];

static Queue_Buffer_t                           ReceiveBuffer;
static Queue_Buffer_t                           TransmitBuffer;

static volatile Bool_t                          ErrorOccurredFlag;
static volatile Bool_t                          TerminateCharacterReceivedFlag;

/* Exported functions --------------------------------------------------------*/
/***
  * @Brief      Interrupt service routine for UART events.
  */
void PacketManager_UARTIsr(void)
{
  /* If the IDLE flag is set, discard the interrupt. */
  if (USART_GetITStatus(SERIAL_PROTOCOL_UART, USART_IT_IDLE) != RESET)
  {
    USART_ReceiveData(SERIAL_PROTOCOL_UART);
  }
  
  /* If overrun, noise, framing or parity errors occurred. */
  else if ((USART_GetITStatus(SERIAL_PROTOCOL_UART, USART_IT_ORE_RX) == SET)    ||\
           (USART_GetITStatus(SERIAL_PROTOCOL_UART, USART_IT_PE) == SET)        ||\
           (USART_GetITStatus(SERIAL_PROTOCOL_UART, USART_IT_NE) == SET)        ||\
           (USART_GetITStatus(SERIAL_PROTOCOL_UART, USART_IT_FE) == SET))
  {
    // Set error occurred flag.
    ErrorOccurredFlag = TRUE;
  }
  
  /* If receive buffer not empty. */
  else if (USART_GetITStatus(SERIAL_PROTOCOL_UART, USART_IT_RXNE) == SET)
  {
    uint8_t element = USART_ReceiveData(SERIAL_PROTOCOL_UART);
    
    /* If the queue is full, data will be missed. So, we can say that some packets
      will be missed. */
    if (Queue_IsFull(&ReceiveBuffer) == FALSE)
    {
      Queue_Enqueue(&ReceiveBuffer, element);
      
      if (element == TERMINATE_CHARACTER)
      {
        TerminateCharacterReceivedFlag = TRUE;
      }
    }
  }
  
  /* If transmit buffer empty. */
  else if (USART_GetFlagStatus(SERIAL_PROTOCOL_UART, USART_FLAG_TC) == SET)
  {
    if (Queue_IsEmpty(&TransmitBuffer) == FALSE)
    {
      uint8_t element = Queue_Dequeue(&TransmitBuffer);
      USART_SendData(SERIAL_PROTOCOL_UART, (uint16_t)element);
    }
    else
    {
      USART_ClearITPendingBit(SERIAL_PROTOCOL_UART, USART_IT_TC);
    }
  }
}

/***
  * @Brief      Setup function for UART controller module.
  *
  * @Params     pSetupParams-> Pointer to setup parameters.
  */
void PacketManager_Setup(PacketManager_SetupParams_t *pSetupParams)
{
  /* Check the state compability. */
  if (State != PACKET_MANAGER_STATE_UNINIT)
  {
    ExceptionHandler_ThrowException(\
      "Packet manager module setup function called when initialized.");
  }
  
  // Ensure that the serial protocol interrupt is disabled.
  NVIC_DisableIRQ(SERIAL_PROTOCOL_IRQ_CHANNEL);
  
  /* Init buffers. */
  Queue_InitBuffer(&ReceiveBuffer, ReceiveBufferContainer, RECEIVE_BUFFER_SIZE);
  Queue_InitBuffer(&TransmitBuffer, TransmitBufferContainer, TRANSMIT_BUFFER_SIZE);
  
  // Set delegate function.
  SduReceivedDelegate = pSetupParams->sduReceivedDelegate;
  ErrorOccurredDelegate = pSetupParams->errorOccurredDelegate;
  
  /* Clear flags. */
  ErrorOccurredFlag = FALSE;
  TerminateCharacterReceivedFlag = FALSE;
  
  State = PACKET_MANAGER_STATE_READY;
}

/***
  * @Brief      Sets start event.
  */
void PacketManager_Start(void)
{
  /* Check state compatibility. */
  if (State != PACKET_MANAGER_STATE_READY)
  {
    ExceptionHandler_ThrowException(\
      "Packet manager start function called when not ready.");
  }
  
  // Disable serial protocol interrupts.
  NVIC_DisableIRQ(SERIAL_PROTOCOL_IRQ_CHANNEL);
 
  /* Clear buffers. */
  Queue_ClearBuffer(&ReceiveBuffer);
  Queue_ClearBuffer(&TransmitBuffer);
  
  // Enable UART.
  USART_Cmd(SERIAL_PROTOCOL_UART, ENABLE);
      
  State = PACKET_MANAGER_STATE_OPERATING;
  
  // Enable serial protocol interrupts.
  NVIC_EnableIRQ(SERIAL_PROTOCOL_IRQ_CHANNEL);
}

/***
  * @Brief      Module executer function.
  */
void PacketManager_Execute(void)
{
  // Check if the state is operating. There aren't anything to do when not operating.
  if (State != PACKET_MANAGER_STATE_OPERATING)
  {
    return;
  }
  
  /* Check if error occurred, switch state if it did. */
  if (ErrorOccurredFlag == TRUE)
  {
    // Disable serial protocol interrupts.
    NVIC_DisableIRQ(SERIAL_PROTOCOL_IRQ_CHANNEL);
    
    // Disable UART.
    USART_Cmd(SERIAL_PROTOCOL_UART, DISABLE);

    State = PACKET_MANAGER_STATE_ERROR;

    /* If error occurred callback function is set, call it. */
    if (ErrorOccurredDelegate != NULL)
    {
      ErrorOccurredDelegate();
    }
    
    ErrorOccurredFlag = FALSE;
  }
  
  /* Check if delimiter received. */
  else if (TerminateCharacterReceivedFlag == TRUE)
  {
    // Disable serial protocol interrupts in order to prevent race conditions.
    NVIC_DisableIRQ(SERIAL_PROTOCOL_IRQ_CHANNEL);

    /* Parse start and terminate index. */
    int32_t start_index = Queue_Search(&ReceiveBuffer, START_CHARACTER);
    int32_t terminate_index = Queue_Search(&ReceiveBuffer, TERMINATE_CHARACTER);
      
    /* If packet received, firstly decode it. Secondly make a CRC check. If valid,
    call callback function. */
    if ((start_index != -1) && (terminate_index != -1))
    {
      int32_t frame_size = (terminate_index - start_index) - 1;
      
      // Remove till start index.
      Queue_Remove(&ReceiveBuffer, start_index);
      
      /* If frame size is valid, dequeue and dispatch. */
      if (frame_size > 0)
      {
        uint32_t sdu_length;
        uint8_t *p_sdu;
        
        /* If dispatched properly, call the delegate. */
        p_sdu = dequeueAndDispatch(frame_size, &sdu_length);    
        if ((p_sdu != NULL) && (SduReceivedDelegate != NULL))
        {
          SduReceivedDelegate(p_sdu, sdu_length);
        }
        
        // Remove terminate character.
        (void)Queue_Dequeue(&ReceiveBuffer);
      }
    }
    
    TerminateCharacterReceivedFlag = FALSE;
    
    // Enable serial protocol interrupts.
    NVIC_EnableIRQ(SERIAL_PROTOCOL_IRQ_CHANNEL);
  }
}

/***
  * @Brief      Sets stop event.
  */
void PacketManager_Stop(void)
{
  if (State != PACKET_MANAGER_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "Packet manager tried to stop when not operating.");
  }
  
  // Disable Serial Protocol IRQ in order to prevent race conditions.
  NVIC_DisableIRQ(SERIAL_PROTOCOL_IRQ_CHANNEL);
  
  // Disable UART.
  USART_Cmd(SERIAL_PROTOCOL_UART, DISABLE);

  State = PACKET_MANAGER_STATE_READY;
  
  NVIC_EnableIRQ(SERIAL_PROTOCOL_IRQ_CHANNEL);
}

/***
  * @Brief      Patches and pushes sdu to the TX buffer.
  *
  * @Params     pPacketManagerSdu-> Pointer to the sdu.
  *             sduLength-> Length of sdu.
  */
void PacketManager_Send(uint8_t *pSdu, uint32_t sduLength)
{
  /* Check for state compatibility. */
  if (State != PACKET_MANAGER_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "Packet manager tried to send packet when not operating.");
  }

  // Disable Serial Protocol IRQ in order to prevent race conditions.
  NVIC_DisableIRQ(SERIAL_PROTOCOL_IRQ_CHANNEL);
  
  // Enqueue start character.
  Queue_Enqueue(&TransmitBuffer, START_CHARACTER);
  
  // Call the core function.
  patchAndEnqueue(pSdu, sduLength);
  
  // Enqueue terminate character.
  Queue_Enqueue(&TransmitBuffer, TERMINATE_CHARACTER);
  
  /* If the transmit buffer is not empty, and there is data on the transmit buffer, 
    push data to the transmit buffer. */
  if ((USART_GetFlagStatus(SERIAL_PROTOCOL_UART, USART_FLAG_TXE) == SET) &&\
      (Queue_IsEmpty(&TransmitBuffer) == FALSE))
  {
    uint8_t element = Queue_Dequeue(&TransmitBuffer);
    USART_SendData(SERIAL_PROTOCOL_UART, (uint16_t)element);
  }
  
  NVIC_EnableIRQ(SERIAL_PROTOCOL_IRQ_CHANNEL);
}

/***
  * @Brief      Gets available space in the rx buffer.
  *
  * @Return     Available space.
  */
uint32_t PacketManager_GetAvailableSpace(void)
{
  /* Calculate the worst case condition and return it. */
  return (((Queue_GetAvailableSpace(&TransmitBuffer) - FRAMING_OVERHEAD) / \
           WORST_ENCODING_MULTIPLIER) - SDU_OVERHEAD);
}
                                      
/***
  * @Brief      Handles module errors. Recovers uncorrupted messages and clears the 
  *             hardware errors.
  */
void PacketManager_ErrorHandler(void)
{  
  /* Check for state compatibility. */
  if (State != PACKET_MANAGER_STATE_ERROR)
  {
    ExceptionHandler_ThrowException(\
      "Packet manager tried to handle error when there aren't any error.");
  }
  
  /* Clear buffers. */
  Queue_ClearBuffer(&ReceiveBuffer);
  Queue_ClearBuffer(&TransmitBuffer);
  
  /* Clear hardware errors. */
  /* PE (Parity error), FE (Framing error), NE (Noise error), ORE (OverRun 
    error) and IDLE (Idle line detected) are cleared by the following sequence. */
  USART_GetITStatus(SERIAL_PROTOCOL_UART,
                    (USART_IT_ORE_ER | USART_IT_NE | USART_IT_FE | USART_IT_PE | USART_IT_IDLE));
  
  USART_ReceiveData(SERIAL_PROTOCOL_UART);
  
  // Set state to ready.
  State = PACKET_MANAGER_STATE_READY;
}


/* Private functions ---------------------------------------------------------*/
/***
  * @Brief      Dequeues the next frame and dispatches(decodes and applies crc
  *             check) it.
  *
  * @Params     frameSize-> Size of the frame(in bytes).
  *             pSduLength-> Length of the sdu.
  * 
  * @Retval     Pointer to dispatched data(NULL if dispatch failed).
  */
uint8_t *dequeueAndDispatch(uint32_t frameSize, uint32_t *pSduLength)
{
  static uint8_t container[MAX_PACKET_SIZE];
  Bool_t escape_mode = FALSE;
  uint8_t *p_sdu = NULL;
  uint32_t packet_length = 0;
  
  /* Parse data with decoding. */
  for (uint32_t i = 0; i < frameSize; i++)
  {
    uint8_t element = Queue_Dequeue(&ReceiveBuffer);
          
    if (element == ESCAPE_CHARACTER)
    {
      escape_mode = TRUE;
      continue;
    }
          
    /* If escape mode */
    if (escape_mode)
    {
      /* Decode character. */
      switch (element)
      {
      case ESCAPE_CHARACTER_CODE:
        element = ESCAPE_CHARACTER;
        break;
              
      case START_CHARACTER_CODE:
        element = START_CHARACTER;
        break;
              
      default:
      case TERMINATE_CHARACTER_CODE:
        element = TERMINATE_CHARACTER;
        break;
      }
            
      // Reset escape mode.
      escape_mode = FALSE;
    }

    /* If packet length is smaller than maximum packet size, copy element to container.
      Break the loop if not. */
    if (packet_length < MAX_PACKET_SIZE)
    {
      container[packet_length++] = element;
    }
    else
    {
      // Discard the packet by setting it's length to zero.
      packet_length = 0;
      break;
    }
    
  }
        
  /* If packet length is bigger than crc size, parse crc. */
  if (packet_length > sizeof(uint32_t))
  {
    uint32_t crc_code;
    uint32_t sdu_length = packet_length - sizeof(crc_code);
    uint32_t crc_offset = sdu_length;
          
    Utils_MemoryCopy(&container[crc_offset], (uint8_t *)&crc_code,
                     sizeof(crc_code));
          
    /* Check CRC code. */
    if (crc_code == CRC32_Calculate(container, sdu_length))
    {
      p_sdu = container;
      *pSduLength = sdu_length;
    }
  }
  
  return p_sdu;
}

/***
  * @Brief      Patches(addes crc code and encodes) the data and enqueues to the
  *             transmit buffer.
  *
  * @Params     pSdu-> Pointer to the sdu.
  *             sduLength-> Length of the sdu.
  *
  * @Return     None.
  */
void patchAndEnqueue(uint8_t *pSdu, uint32_t sduLength)
{
  uint8_t container[MAX_PACKET_SIZE];
  
  // Calculate CRC code over the sdu.
  uint32_t crc_code = CRC32_Calculate(pSdu, sduLength);
  
  /* Merge in the temporary container. */
  Utils_MemoryCopy(pSdu, container, sduLength);
  Utils_MemoryCopy((uint8_t *)&crc_code, &container[sduLength], sizeof(crc_code));
  
  uint32_t packet_length = sduLength + sizeof(crc_code);
  
  /* Encode and enqueue all the elements to transmit buffer. */
  for (uint32_t i = 0; i < packet_length; i++)
  {
    uint8_t element = container[i];
    
    switch (element)
    {
    case START_CHARACTER:
      Queue_Enqueue(&TransmitBuffer, ESCAPE_CHARACTER);
      Queue_Enqueue(&TransmitBuffer, START_CHARACTER_CODE);
      break;
      
    case TERMINATE_CHARACTER:
      Queue_Enqueue(&TransmitBuffer, ESCAPE_CHARACTER);
      Queue_Enqueue(&TransmitBuffer, TERMINATE_CHARACTER_CODE);
      break;
      
    case ESCAPE_CHARACTER:
      Queue_Enqueue(&TransmitBuffer, ESCAPE_CHARACTER);
      Queue_Enqueue(&TransmitBuffer, ESCAPE_CHARACTER_CODE);
      break;
      
    default:
      Queue_Enqueue(&TransmitBuffer, element);
      break;
    }
  }
}