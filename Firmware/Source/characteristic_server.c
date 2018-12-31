#include "characteristic_server.h"
#include "characteristic_protocol.h"
#include "middlewares.h"

/* Private constants ---------------------------------------------------------*/
// Abbrevations.
#define PROPERTY_READABLE                       CHARACTERISTIC_SERVER_CHAR_PROP_READABLE
#define PROPERTY_WRITABLE                       CHARACTERISTIC_SERVER_CHAR_PROP_WRITABLE
#define PROPERTY_VARIABLE_LENGTH                CHARACTERISTIC_SERVER_CHAR_PROP_VARIABLE_LENGTH
#define PROPERTY_REGISTERED                     CHARACTERISTIC_SERVER_CHAR_PROP_REGISTERED

/* Private typedefs ----------------------------------------------------------*/
// Short name for characteristic.
typedef CharacteristicServer_Characteristic_t   Characteristic_t;

/* Private function declerations ---------------------------------------------*/
static void disconnectionRequestReceivedEventHandler(void);
static void connectionRequestReceivedEventHandler(void);
static void readRequestReceivedEventHandler(uint16_t charId);
static void writeRequestReceivedEventHandler(uint16_t charId, uint8_t *pData, 
                                             uint32_t dataLength);

static void registerRequestReceivedEventHandler(uint16_t charId);
static Characteristic_t *getChar(uint16_t charId);

/* Private variables ---------------------------------------------------------*/
// Variables to store module control data.
static CharacteristicServer_State_t     State = CHARACTERISTIC_SERVER_STATE_UNINIT;
static Bool_t                           IsConnected;
static Characteristic_t                 *pCharacteristicTable;
static uint16_t                         NumOfChars;

// Delegates.
static CharacteristicServer_WriteDelegate_t                     WriteDelegate;
static CharacteristicServer_ConnectionStateChangedDelegate_t    ConnectionStateChangedDelegate;

/* Public function implementations. ------------------------------------------*/
// TODO: NOTHING.
void CharacteristicServer_Setup(CharacteristicServer_SetupParams_t *pSetupParams)
{
  // Check the state compability.
  if (State == CHARACTERISTIC_SERVER_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "Characteristic server module Setup function called when operating");
  }
  
  // Make the setup of characteristic protocol.
  {
    CharacteristicProtocol_SetupParams_t params;
  
    params.disconnectionRequestReceivedDelegate = disconnectionRequestReceivedEventHandler;
    params.connectionRequestReceivedDelegate = connectionRequestReceivedEventHandler;
    params.readRequestReceivedDelegate = readRequestReceivedEventHandler;
    params.writeRequestReceivedDelegate = writeRequestReceivedEventHandler;
    params.registerRequestReceivedDelegate = registerRequestReceivedEventHandler;
  
    CharacteristicProtocol_Setup(&params);
  }
  
  // Set delegates.
  ConnectionStateChangedDelegate = pSetupParams->connectionStateChangedDelegate;
  WriteDelegate = pSetupParams->writeDelegate;
  
  // Set params.
  pCharacteristicTable = pSetupParams->pCharTable;
  NumOfChars = pSetupParams->numOfChars;
  
  // Set state to ready.
  State = CHARACTERISTIC_SERVER_STATE_READY;
}

// TODO: NOTHING.
void CharacteristicServer_Start(void)
{
  // Check the state compability.
  if (State != CHARACTERISTIC_SERVER_STATE_READY)
  {
    ExceptionHandler_ThrowException(\
      "Characteristic server module start function called when not ready.");
  }
  
  // Start characteristic protocol. 
  CharacteristicProtocol_Start();
  
  // Set not connected.
  IsConnected = FALSE;
  
  // Set state to operating.
  State = CHARACTERISTIC_SERVER_STATE_OPERATING;
}

// TODO: NOTHING.
void CharacteristicServer_Execute(void)
{
  // Check the state compability.
  if (State != CHARACTERISTIC_SERVER_STATE_OPERATING)
  {
    return;
  }
  
  // Call submodule's executer.
  CharacteristicProtocol_Execute();
}

// TODO: NOTHING.
void CharacteristicServer_Stop(void)
{
  // Check the state compability.
  if (State != CHARACTERISTIC_SERVER_STATE_OPERATING)
  {
    ExceptionHandler_ThrowException(\
      "Characteristic server module Stop function called when not operating.");
  }
  
  // Stop characteristic protocol.
  CharacteristicProtocol_Stop();
  
  // Set state to ready.
  State = CHARACTERISTIC_SERVER_STATE_READY;
}

// TODO: NOTHING.
void CharacteristicServer_UpdateCharacteristic(uint16_t charId, uint8_t *pData, uint32_t dataLength)
{
  // Check state.
  if (State == CHARACTERISTIC_SERVER_STATE_UNINIT)
  {
    ExceptionHandler_ThrowException(\
      "Characteristic server module Update characteristic function called when uninitialized.");
  }

  Characteristic_t *p_char;
  
  // Get characteristic pointer.
  p_char = getChar(charId);
  
  // Validate lengths.
  if (p_char)
  {
    if (p_char->length >= dataLength)
    {
      // Write to char.
      Utils_MemoryCopy(pData, p_char->pData, dataLength);
      
      // If registered and connected, send notification.
      if (IsConnected && (p_char->properties & PROPERTY_REGISTERED))
      {
        CharacteristicProtocol_SendNotification(charId, p_char->pData, p_char->length);
      }
    }
  }
}

/* Private function implementations ------------------------------------------*/
static void connectionRequestReceivedEventHandler(void)
{
  // If the state isn't operating; can't connect. Inform this.
  if (State != CHARACTERISTIC_SERVER_STATE_OPERATING)
  {
    CharacteristicProtocol_SendConnectionResp(OPERATION_RESULT_FAILURE);
    return;
  }

  if (!IsConnected)
  {
    // Set connected.
    IsConnected = TRUE;
    
    // Call connection state changed delegate(if set).
    if (ConnectionStateChangedDelegate)
    {
      ConnectionStateChangedDelegate(IsConnected);
    }
  }
  
  // Send response.
  CharacteristicProtocol_SendConnectionResp(OPERATION_RESULT_SUCCESS);
}

static void disconnectionRequestReceivedEventHandler(void)
{
  // If the state is operating, disconnect and call delegate.
  if ((State == CHARACTERISTIC_SERVER_STATE_OPERATING))
  {
    if (IsConnected)
    {
      // Set disconnected.
      IsConnected = FALSE;
    
      // Call connection state changed delegate(if set).
      if (ConnectionStateChangedDelegate)
      {
        ConnectionStateChangedDelegate(IsConnected);
      }
    }
  }
  
  // Send response anyway.
  CharacteristicProtocol_SendDisconnectionResp(OPERATION_RESULT_SUCCESS);
}

// TODO: NOTHING.                                    
static void readRequestReceivedEventHandler(uint16_t charId)
{
  Bool_t validated = FALSE;
  Characteristic_t *p_char;
  
  // If connected;
  if (IsConnected)
  {
    // Get characteristic pointer.
    p_char = getChar(charId);
    
    // If found, check if it's readable.
    if (p_char)
    {
      if (p_char->properties & PROPERTY_READABLE)
      {
        validated = TRUE;
      }
    }
  }
  
  if (validated)
  {
    // Send read response with success.
    CharacteristicProtocol_SendReadResp(OPERATION_RESULT_SUCCESS, p_char->pData, 
                                        p_char->length);
  }
  else
  {
    // Send failure message.
    CharacteristicProtocol_SendReadResp(OPERATION_RESULT_FAILURE, NULL, 0);
  }
  
}

// TODO: NOTHING.
static void writeRequestReceivedEventHandler(uint16_t charId, uint8_t *pData, 
                                             uint32_t dataLength)
{
  Characteristic_t *p_char;
  Bool_t validated = FALSE;
  
  if (IsConnected)
  {
    // Get characteristic pointer.
    p_char = getChar(charId);
    
    // If found, check if it's writable.
    if (p_char)
    {
      // If writable and the length values are appropriate, write and validate.
      if ((p_char->properties & PROPERTY_WRITABLE) && (p_char->length >= dataLength))
      {
        Utils_MemoryCopy(pData, p_char->pData, dataLength);
        validated = TRUE;
      }
    }
  }
  
  if (validated)
  {
    // Call the delegate, if it's set.
    if (WriteDelegate)
    {
      WriteDelegate(charId);
    }
    
    CharacteristicProtocol_SendWriteResp(OPERATION_RESULT_SUCCESS);
  }
  else
  {
    // Send failure otherwise.
    CharacteristicProtocol_SendWriteResp(OPERATION_RESULT_FAILURE);
  }
}

// TODO: NOTHING.
static void registerRequestReceivedEventHandler(uint16_t charId)
{
  Characteristic_t *p_char;
  OperationResult_t operation_result = OPERATION_RESULT_FAILURE;
  
  if (IsConnected)
  {
    // Get characteristic pointer.
    p_char = getChar(charId);
    
    // If found, check if it's readable.
    if (p_char)
    {
      if (p_char->properties & PROPERTY_READABLE)
      {
        // Register and set the operation result success.
        p_char->properties |= PROPERTY_REGISTERED;
        operation_result = OPERATION_RESULT_SUCCESS;
      }
    }
  }
  
  // Send register response.
  CharacteristicProtocol_SendRegisterResp(operation_result);
}

// TODO: NOTHING.
static Characteristic_t *getChar(uint16_t charId)
{
  Characteristic_t *p_retval = NULL;
  
  // Search characteristics.
  for (uint16_t i = 0; i < NumOfChars; i++)
  {
    // If characteristic ID's match. Set table index and break the loop.
    if (pCharacteristicTable[i].charId == charId)
    {
      p_retval = &pCharacteristicTable[i];
      break;
    }
  }
  
  return p_retval;
}