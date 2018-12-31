/***
  * @author     Onur Efe
  */
#ifndef __QUEUE_H
#define __QUEUE_H

/* Includes ------------------------------------------------------------------*/
#include "generic.h"

/* Typedefs ------------------------------------------------------------------*/
typedef struct
{
  uint32_t      tail;
  uint32_t      head;
  uint32_t      capacity;
  uint8_t       *pContainer;
} Queue_Buffer_t;

/* Exported functions --------------------------------------------------------*/
/***
  * @Brief      Creates a buffer, allocates it's memory and returns the pointer 
  *             of it.           
  * @Params     pBuff-> Pointer to buffer.
  *             pContainer-> Pointer of the data container.
  *             size-> Size of the data container.
  *
  * @Return     None.
  */
extern void Queue_InitBuffer(Queue_Buffer_t *pBuff, uint8_t *pContainer, uint32_t size);

/***
  * @Brief      Clears the addressed buffer.
  *
  * @Params     pBuff-> Pointer to the buffer.
  */
extern void Queue_ClearBuffer(Queue_Buffer_t *pBuff);

/***
  * @Brief      Enqueues byte to the given buffer.
  *
  * @Params     pBuff-> Pointer to the buffer.
  *             byte-> Byte to be enqueued.
  */
extern void Queue_Enqueue(Queue_Buffer_t *pBuff, uint8_t byte);

/***            
  * @Brief      Dequeues byte from the given buffer.
  *
  * @Params     pBuff-> Pointer to the buffer.
  *
  * @Return     Byte.
  */
extern uint8_t Queue_Dequeue(Queue_Buffer_t *pBuff);

/***
  * @Brief      Removes elements until the indexed element(also including).
  *
  * @Params     pBuff-> Pointer to the buffer.
  *             elementIndex-> Index of the element.
  *
  * @Return     None.
  */
extern void Queue_Remove(Queue_Buffer_t *pBuff, uint16_t elementIndex);

/***
  * @Brief      Searches an element in the buffer. Returns element index if the element
  *             exists. Returns -1 otherwise.
  *
  * @Params     pBuff-> Buffer to be searched.
  *             element-> Element value.
  *
  * @Return     Element index or -1.
  */

extern int32_t Queue_Search(Queue_Buffer_t *pBuff, uint8_t element);

/***
  * @Brief      Peeks element in the buffer. Indexing starts at the first element.
  *                       
  * @Params     pBuff-> Pointer to the buffer.
  *             elementIndex-> Index of the element.
  *
  * @Return     Element value.
  */
extern uint8_t Queue_Peek(Queue_Buffer_t *pBuff, uint32_t elementIndex);

/***
  * @Brief      Returns number of elements the buffer contains.
  *
  * @Params     pBuff-> Buffer pointer.
  *
  * @Return     Element count.
  */
extern uint32_t Queue_GetElementCount(Queue_Buffer_t *pBuff);

/***
  * @Brief      Returns available space of the buffer.
  *
  * @Params     pBuff-> Buffer pointer.
  *
  * @Return     Available space.
  */
extern uint32_t Queue_GetAvailableSpace(Queue_Buffer_t *pBuff);

/***
  * @Brief      Checks if the buffer is empty.
  *
  * @Params     pBuff-> Pointer to buffer.
  *
  * @Return     TRUE or FALSE.
  */
extern Bool_t Queue_IsEmpty(Queue_Buffer_t *pBuff);

/***
  * @Brief      Checks if the buffer is full.
  *
  * @Params     pBuff-> Pointer to buffer.
  *
  * @Return     TRUE or FALSE.
  */
extern Bool_t Queue_IsFull(Queue_Buffer_t *pBuff);
#endif