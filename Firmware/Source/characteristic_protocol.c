/***
  * @author     Onur Efe
  */
/* Includes ------------------------------------------------------------------*/
#include "characteristic_protocol.h"
#include "packet_manager.h"
#include "middlewares.h"

/* Private constants ---------------------------------------------------------*/
// Protocol PDU's
// All pdus have type value at offset 0.
#define PDU_TYPE_OFFSET                                 0

// Connection request fields.
#define CONNECTION_REQ_SIZE                             1

// Connection response fields.
#define CONNECTION_RESP_OPERATION_RESULT_OFFSET         1
#define CONNECTION_RESP_SIZE                            2
    
// Disconnection request fields.
#define DISCONNECTION_REQ_SIZE                          1

// Disconnection response fields.
#define DISCONNECTION_RESP_OPERATION_RESULT_OFFSET      1
#define DISCONNECTION_RESP_SIZE                         2

// Read request fields.
#define READ_REQ_CHAR_ID_OFFSET                         1
#define READ_REQ_SIZE                                   3

// Read response fields.
#define READ_RESP_OPERATION_RESULT_OFFSET               1
#define READ_RESP_DATA_OFFSET                           2
#define READ_RESP_MIN_SIZE                              2

// Write request fields.
#define WRITE_REQ_CHAR_ID_OFFSET                        1
#define WRITE_REQ_DATA_OFFSET                           3
#define WRITE_REQ_MIN_SIZE                              3

// Write response fields.
#define WRITE_RESP_OPERATION_RESULT_OFFSET              1
#define WRITE_RESP_SIZE                                 2

// Register request fields.
#define REGISTER_REQ_CHAR_ID_OFFSET                     1
#define REGISTER_REQ_SIZE                               3

// Register response fields.
#define REGISTER_RESP_OPERATION_RESULT_OFFSET           1
#define REGISTER_RESP_SIZE                              2

// Notification fields.
#define NOTIFICATION_CHAR_ID_OFFSET                     1
#define NOTIFICATION_DATA_OFFSET                        3
#define NOTIFICATION_MIN_SIZE                           3

/* Private typedefs ----------------------------------------------------------*/
typedef enum
{
  CONNECTION_REQ_PDU = 0,
  CONNECTION_RESP_PDU,
  DISCONNECTION_REQ_PDU,
  DISCONNECTION_RESP_PDU,
  READ_REQ_PDU,
  READ_RESP_PDU,
  WRITE_REQ_PDU,
  WRITE_RESP_PDU,
  REGISTER_REQ_PDU,
  REGISTER_RESP_PDU,
  NOTIFICATION_PDU
} PduType_t;

/* Private function prototypes -----------------------------------------------*/
static void errorOccurredEventHandler(void);
static void pduReceivedEventHandler(uint8_t *pData, uint32_t dataLength);

/* Private variable declerations ---------------------------------------------*/
// State variables.
static CharacteristicProtocol_State_t   State = CHARACTERISTIC_PROTOCOL_STATE_UNINIT;

// Variables to store some setup data.
static ConnectionRequestReceivedDelegate_t              ConnectionRequestReceivedDelegate;
static DisconnectionRequestReceivedDelegate_t           DisconnectionRequestReceivedDelegate;
static ReadRequestReceivedDelegate_t                    ReadRequestReceivedDelegate;
static WriteRequestReceivedDelegate_t                   WriteRequestReceivedDelegate;
static RegisterRequestReceivedDelegate_t                RegisterRequestReceivedDelegate;

// Send buffer to hold last pdu(for resend).
static uint8_t  SendBuffer[CHARACTERISTIC_PROTOCOL_PDU_SIZE];

/* Exported functions --------------------------------------------------------*/
void CharacteristicProtocol_Setup(CharacteristicProtocol_SetupParams_t *pSetupParams)
{ 
  // Check the state compability.
  if (State == CHARACTERISTIC_PROTOCOL_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "Characteristic protocol module Setup function called when operating.");
  }
  
  // Packet manager setup.
  {
    PacketManager_SetupParams_t packet_setup;
  
    packet_setup.sduReceivedDelegate = pduReceivedEventHandler;
    packet_setup.errorOccurredDelegate = errorOccurredEventHandler;
    
    PacketManager_Setup(&packet_setup);
  }
  
  // Store setup data(delegates).
  ConnectionRequestReceivedDelegate = pSetupParams->connectionRequestReceivedDelegate;
  DisconnectionRequestReceivedDelegate = pSetupParams->disconnectionRequestReceivedDelegate;
  ReadRequestReceivedDelegate = pSetupParams->readRequestReceivedDelegate;
  WriteRequestReceivedDelegate = pSetupParams->writeRequestReceivedDelegate;
  RegisterRequestReceivedDelegate = pSetupParams->registerRequestReceivedDelegate;
  
  // Set state.
  State = CHARACTERISTIC_PROTOCOL_STATE_READY;
}

void CharacteristicProtocol_Start(void)
{
  // Check the state compability.
  if (State != CHARACTERISTIC_PROTOCOL_STATE_READY)
  {
    ExceptionHandler_ThrowException(\
      "Characteristic protocol module start function called when not ready.");
  }
  
  // Start packet manager.
  PacketManager_Start();
  
  // Set state.
  State = CHARACTERISTIC_PROTOCOL_STATE_OPERATING;
}

void CharacteristicProtocol_Execute(void)
{
  // Check the state compability.
  if (State != CHARACTERISTIC_PROTOCOL_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "Characteristic protocol module Execute function called when not operating.");
  }
  
  // Execute sub-module.
  PacketManager_Execute();
}

void CharacteristicProtocol_Stop(void)
{
  // Check the state compability.
  if (State != CHARACTERISTIC_PROTOCOL_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "Characteristic protocol module Stop function called when not operating.");
  }
  
  // Stop submodule.
  PacketManager_Stop();
}

void CharacteristicProtocol_SendNotification(uint16_t charId, uint8_t *pData, 
                                             uint32_t dataLength)
{
  // Check the state compability.
  if (State != CHARACTERISTIC_PROTOCOL_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "Characteristic protocol module SendNotification function called when not operating.");
  }
  
  uint32_t pdu_length;
    
  // Serialize notification.
  SendBuffer[PDU_TYPE_OFFSET] = NOTIFICATION_PDU;
  SendBuffer[NOTIFICATION_CHAR_ID_OFFSET] = ((uint8_t *)&charId)[0];
  SendBuffer[NOTIFICATION_CHAR_ID_OFFSET + 1] = ((uint8_t *)&charId)[1];
  
  Utils_MemoryCopy(pData, &SendBuffer[NOTIFICATION_DATA_OFFSET], dataLength);
  
  pdu_length = NOTIFICATION_DATA_OFFSET + dataLength;
  
  // Send it.
  PacketManager_Send(SendBuffer, pdu_length);
}

void CharacteristicProtocol_SendConnectionResp(OperationResult_t operationResult)
{
  SendBuffer[PDU_TYPE_OFFSET] = CONNECTION_RESP_PDU;
  SendBuffer[CONNECTION_RESP_OPERATION_RESULT_OFFSET] = operationResult;
  
  PacketManager_Send(SendBuffer, CONNECTION_RESP_SIZE);
}

void CharacteristicProtocol_SendDisconnectionResp(OperationResult_t operationResult)
{
  SendBuffer[PDU_TYPE_OFFSET] = DISCONNECTION_RESP_PDU;
  SendBuffer[DISCONNECTION_RESP_OPERATION_RESULT_OFFSET] = operationResult;
  
  PacketManager_Send(SendBuffer, DISCONNECTION_RESP_SIZE);
}

void CharacteristicProtocol_SendReadResp(OperationResult_t operationResult,
                                         uint8_t *pData, uint32_t dataLength)
{
  // Check the state compability.
  if (State != CHARACTERISTIC_PROTOCOL_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "Characteristic protocol module SendReadResp function called when not operating.");
  }
  
  uint32_t pdu_length;
    
  // Serialize notification.
  SendBuffer[PDU_TYPE_OFFSET] = READ_RESP_PDU;
  SendBuffer[READ_RESP_OPERATION_RESULT_OFFSET] = (uint8_t)operationResult;
  
  Utils_MemoryCopy(pData, &SendBuffer[READ_RESP_DATA_OFFSET], dataLength);
  
  pdu_length = READ_RESP_DATA_OFFSET + dataLength;
  
  // Send it.
  PacketManager_Send(SendBuffer, pdu_length);
}

void CharacteristicProtocol_SendWriteResp(OperationResult_t operationResult)
{
  // Check the state compability.
  if (State != CHARACTERISTIC_PROTOCOL_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "Characteristic protocol module SendWriteResp function called when not operating.");
  }
  
  // Serialize notification.
  SendBuffer[PDU_TYPE_OFFSET] = WRITE_RESP_PDU;
  SendBuffer[WRITE_RESP_OPERATION_RESULT_OFFSET] = (uint8_t)operationResult;
  
  // Send it.
  PacketManager_Send(SendBuffer, WRITE_RESP_SIZE);
}

void CharacteristicProtocol_SendRegisterResp(OperationResult_t operationResult)
{
  // Check the state compability.
  if (State != CHARACTERISTIC_PROTOCOL_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "Characteristic protocol module SendRegisterResp function called when not operating.");
  }
  
  // Serialize notification.
  SendBuffer[PDU_TYPE_OFFSET] = REGISTER_RESP_PDU;
  SendBuffer[REGISTER_RESP_OPERATION_RESULT_OFFSET] = (uint8_t)operationResult;
  
  // Send it.
  PacketManager_Send(SendBuffer, REGISTER_RESP_SIZE);
}

static void errorOccurredEventHandler(void)
{
  // Call packet manager error handler.
  PacketManager_ErrorHandler();
}

static void pduReceivedEventHandler(uint8_t *pData, uint32_t dataLength)
{
  switch (pData[PDU_TYPE_OFFSET])
  {
  case DISCONNECTION_REQ_PDU:
    {
      // If the data length equals connection request size;
      if (dataLength == DISCONNECTION_REQ_SIZE)
      {
        if (DisconnectionRequestReceivedDelegate)
        {
          DisconnectionRequestReceivedDelegate();
        }
      }
    }
    break;
    break;
    
  case CONNECTION_REQ_PDU:
    {
      // If the data length equals connection request size;
      if (dataLength == CONNECTION_REQ_SIZE)
      {
        if (ConnectionRequestReceivedDelegate)
        {
          ConnectionRequestReceivedDelegate();
        }
      }
    }
    break;
    
  case READ_REQ_PDU:
    {
      // Check data length.
      if (dataLength == READ_REQ_SIZE)
      {
        uint16_t char_id;
        
        ((uint8_t *)&char_id)[0] = pData[READ_REQ_CHAR_ID_OFFSET];
        ((uint8_t *)&char_id)[1] = pData[READ_REQ_CHAR_ID_OFFSET + 1];
        
        if (ReadRequestReceivedDelegate)
        {
          ReadRequestReceivedDelegate(char_id);
        }
      }
    }
    break;
    
  case WRITE_REQ_PDU:
    {
      // Check data length.
      if (dataLength >= WRITE_REQ_MIN_SIZE)
      {
        uint16_t char_id;
        
        ((uint8_t *)&char_id)[0] = pData[WRITE_REQ_CHAR_ID_OFFSET];
        ((uint8_t *)&char_id)[1] = pData[WRITE_REQ_CHAR_ID_OFFSET + 1];
        
        if (WriteRequestReceivedDelegate)
        {
          WriteRequestReceivedDelegate(char_id, &pData[WRITE_REQ_DATA_OFFSET], 
                                       (dataLength - WRITE_REQ_MIN_SIZE));
        }
      }
    }
    break;
    
  case REGISTER_REQ_PDU:
    {
      // Check data length.
      if (dataLength == REGISTER_REQ_SIZE)
      {
        uint16_t char_id;
        
        ((uint8_t *)&char_id)[0] = pData[REGISTER_REQ_CHAR_ID_OFFSET];
        ((uint8_t *)&char_id)[1] = pData[REGISTER_REQ_CHAR_ID_OFFSET + 1];
        
        if (RegisterRequestReceivedDelegate)
        {
          RegisterRequestReceivedDelegate(char_id);
        }
      }
    }
    break; 
  }
}