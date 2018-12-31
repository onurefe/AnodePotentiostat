/***
  * @author     Onur Efe
  */
#ifndef __QUEUE_GENERIC_H
#define __QUEUE_GENERIC_H

/* Includes ------------------------------------------------------------------*/
#include "generic.h"

/* Typedefs ------------------------------------------------------------------*/
typedef struct
{
  uint32_t      tail;
  uint32_t      head;
  uint32_t      capacity;
  uint8_t       itemSize;
  uint8_t       *pContainer;
} QueueGeneric_Buffer_t;

/* Exported functions --------------------------------------------------------*/
/***
  * @Brief      Inits a buffer.   
  * @Params     pBuff-> Pointer to buffer.
  *             ppContailer-> Address of the container which contains object addresses.
  *             itemSize-> Size of objects in bytes.
  *             length-> Length of the container.
  *
  * @Return     None.
  */
extern void QueueGeneric_InitBuffer(QueueGeneric_Buffer_t *pBuff, uint8_t *pContainer, 
                                    uint8_t itemSize, uint32_t length);

/***
  * @Brief      Clears the addressed buffer.
  *
  * @Params     pBuff-> Pointer to the buffer.
  */
extern void QueueGeneric_ClearBuffer(QueueGeneric_Buffer_t *pBuff);

/***
  * @Brief      Enqueues object pointer to the given buffer.
  *
  * @Params     pBuff-> Pointer to the buffer.
  *             pObj-> Object pointer to be enqueued.
  *
  * @Return     None.
  */
extern void QueueGeneric_Enqueue(QueueGeneric_Buffer_t *pBuff, uint8_t *pObj);

/***            
  * @Brief      Dequeues object pointer from the given buffer.
  *
  * @Params     pBuff-> Pointer to the buffer.
  *
  * @Return     Object pointer.
  */
extern void QueueGeneric_Dequeue(QueueGeneric_Buffer_t *pBuff, uint8_t *pObj);

/***
  * @Brief      Returns available space of the buffer.
  *
  * @Params     pBuff-> Buffer pointer.
  *
  * @Return     Available space.
  */
extern uint32_t QueueGeneric_GetAvailableSpace(QueueGeneric_Buffer_t *pBuff);

/***
  * @Brief      Checks if the buffer is empty.
  *
  * @Params     pBuff-> Pointer to buffer.
  *
  * @Return     TRUE or FALSE.
  */
extern Bool_t QueueGeneric_IsEmpty(QueueGeneric_Buffer_t *pBuff);

/***
  * @Brief      Checks if the buffer is full.
  *
  * @Params     pBuff-> Pointer to buffer.
  *
  * @Return     TRUE or FALSE.
  */
extern Bool_t QueueGeneric_IsFull(QueueGeneric_Buffer_t *pBuff);
#endif