/**
  * @author     Onur Efe
  */

#ifndef __PACKET_MANAGER_H
#define __PACKET_MANAGER_H

/* Include files -------------------------------------------------------------*/
#include "generic.h"

/* Exported constants --------------------------------------------------------*/
#define PACKET_MANAGER_MAX_SDU_SIZE             126
    
/* Exported types ------------------------------------------------------------*/
typedef enum
{
  PACKET_MANAGER_STATE_UNINIT                   = 0,
  PACKET_MANAGER_STATE_READY,
  PACKET_MANAGER_STATE_OPERATING,
  PACKET_MANAGER_STATE_ERROR,      
} PacketManager_State_t;

typedef void (*PacketManager_SduReceivedDelegate_t)(uint8_t *pSdu, uint32_t sduLength);
typedef void (*PacketManager_ErrorOccurredDelegate_t)(void);

typedef struct
{
  PacketManager_SduReceivedDelegate_t           sduReceivedDelegate;
  PacketManager_ErrorOccurredDelegate_t         errorOccurredDelegate;
} PacketManager_SetupParams_t;

/* Exported functions --------------------------------------------------------*/
/***
  * @Brief      Setup function for UART controller module.
  *
  * @Params     pSetupParams-> Pointer to setup parameters.
  */
extern void PacketManager_Setup(PacketManager_SetupParams_t *pSetupParams);

/***
  * @Brief      Sets start event.
  */
extern void PacketManager_Start(void);

/***
  * @Brief      Module executer function.
  */
extern void PacketManager_Execute(void);

/***
  * @Brief      Sets stop event.
  */
extern void PacketManager_Stop(void);

/***
  * @Brief      Patches and pushes sdu to the TX buffer.
  *
  * @Params     pSdu-> Pointer to the sdu.
  *             sduLength-> Length of sdu.
  */
extern void PacketManager_Send(uint8_t *pSdu, uint32_t sduLength);

/***
  * @Brief      Gets available space in the rx buffer.
  *
  * @Return     Available space.
  */
extern uint32_t PacketManager_GetAvailableSpace(void);

/***
  * @Brief      Handles module errors. Recovers uncorrupted messages and clears the 
  *             hardware errors.
  */
extern void PacketManager_ErrorHandler(void);
#endif