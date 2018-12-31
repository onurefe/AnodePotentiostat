/***
  * @author     Onur Efe
  */
#ifndef __CHARACTERISTIC_PROTOCOL_H
#define __CHARACTERISTIC_PROTOCOL_H

/* Include files -------------------------------------------------------------*/
#include "generic.h"

/* Exported constants --------------------------------------------------------*/ 
#define CHARACTERISTIC_PROTOCOL_PDU_SIZE                PACKET_MANAGER_MAX_SDU_SIZE
#define CHARACTERISTIC_PROTOCOL_MAX_DATA_OVERHEAD       3
#define CHARACTERISTIC_PROTOCOL_MAX_DATA_SIZE           (PACKET_MANAGER_MAX_SDU_SIZE \
                                                         - CHARACTERISTIC_PROTOCOL_MAX_DATA_OVERHEAD)

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  CHARACTERISTIC_PROTOCOL_STATE_UNINIT = 0,
  CHARACTERISTIC_PROTOCOL_STATE_READY,
  CHARACTERISTIC_PROTOCOL_STATE_OPERATING
} CharacteristicProtocol_State_t;

// Delegates.
typedef void (*DisconnectionRequestReceivedDelegate_t)(void);
typedef void (*ConnectionRequestReceivedDelegate_t)(void);
typedef void (*ReadRequestReceivedDelegate_t)(uint16_t charId);
typedef void (*WriteRequestReceivedDelegate_t)(uint16_t charId, uint8_t *pData, uint32_t dataLength);
typedef void (*RegisterRequestReceivedDelegate_t)(uint16_t charId);
typedef void (*NotificationAcknowledgeReceivedDelegate_t)(void);
typedef void (*TimeoutOccurredDelegate_t)(void);

// Setup params structure.
typedef struct
{
  DisconnectionRequestReceivedDelegate_t        disconnectionRequestReceivedDelegate;
  ConnectionRequestReceivedDelegate_t           connectionRequestReceivedDelegate;
  ReadRequestReceivedDelegate_t                 readRequestReceivedDelegate;
  WriteRequestReceivedDelegate_t                writeRequestReceivedDelegate;
  RegisterRequestReceivedDelegate_t             registerRequestReceivedDelegate;
  NotificationAcknowledgeReceivedDelegate_t     notificationAckReceivedDelegate;
  TimeoutOccurredDelegate_t                     timeoutOccurredDelegate;
} CharacteristicProtocol_SetupParams_t;

/* Exported functions --------------------------------------------------------*/
extern void CharacteristicProtocol_Setup(CharacteristicProtocol_SetupParams_t *pSetupParams);
extern void CharacteristicProtocol_Start(void);
extern void CharacteristicProtocol_Execute(void);
extern void CharacteristicProtocol_Stop(void);
extern void CharacteristicProtocol_SendConnectionResp(OperationResult_t operationResult);
extern void CharacteristicProtocol_SendDisconnectionResp(OperationResult_t operationResult);
extern void CharacteristicProtocol_SendNotification(uint16_t charId, uint8_t *pData, 
                                                    uint32_t dataLength);
extern void CharacteristicProtocol_SendReadResp(OperationResult_t operationResult,
                                                uint8_t *pData, uint32_t dataLength);
extern void CharacteristicProtocol_SendWriteResp(OperationResult_t operationResult);
extern void CharacteristicProtocol_SendRegisterResp(OperationResult_t operationResult);

#endif