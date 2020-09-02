#include "generic.h"
#include "exception_handler.h"
#include "queue_generic.h"
#include "utils.h"
#include "stdlib.h"

/* Exported functions**********************************************************/
/***
  * @Brief      Inits a buffer.   
  * @Params     pBuff-> Pointer to buffer.
  *             ppContailer-> Address of the container which contains object addresses.
  *             itemSize-> Size of objects in bytes.
  *             length-> Length of the container.
  *
  * @Return     None.
  */
void QueueGeneric_InitBuffer(QueueGeneric_Buffer_t *pBuff, uint8_t *pContainer, 
                             uint8_t itemSize, uint32_t length)
{ 
  /* Check if the pointer is valid. */
  if (pContainer == NULL)
  {
    ExceptionHandler_ThrowException("Queue(generic) invalid container address.\n");
  }
  
  /* Initialize buffer. */
  pBuff->head = 0;
  pBuff->tail = 0;
  pBuff->pContainer = pContainer;
  pBuff->itemSize = itemSize;
  pBuff->capacity = length;
}

/***
  * @Brief      Clears the addressed buffer.
  *
  * @Params     pBuff-> Pointer to the buffer.
  */
void QueueGeneric_ClearBuffer(QueueGeneric_Buffer_t *pBuff)
{
  pBuff->head = 0;
  pBuff->tail = 0;
}

/***
  * @Brief      Enqueues object pointer to the given buffer.
  *
  * @Params     pBuff-> Pointer to the buffer.
  *             pObj-> Object pointer to be enqueued.
  *
  * @Return     None.
  */
void QueueGeneric_Enqueue(QueueGeneric_Buffer_t *pBuff, uint8_t *pObj)
{
  // Set new tail element.
  Utils_MemoryCopy(pObj, &(pBuff->pContainer[pBuff->tail * pBuff->itemSize]), 
                   (uint32_t)pBuff->itemSize);
  
  // Update the tail value. 
  if (++pBuff->tail >= pBuff->capacity)
  {
    pBuff->tail = 0;
  }
  
  /* Check if buffer overflow occurred. */
  if (pBuff->tail == pBuff->head)
  {
    ExceptionHandler_ThrowException("Queue(generic) overflowed.\n");
  }
}

/***            
  * @Brief      Dequeues object pointer from the given buffer.
  *
  * @Params     pBuff-> Pointer to the buffer.
  *
  * @Return     Object pointer.
  */
void QueueGeneric_Dequeue(QueueGeneric_Buffer_t *pBuff, uint8_t *pObj)
{ 
  /* Check if there are any element to dequeue. If there isn't, this means that 
    a fatal error occurred. */
  if (pBuff->head == pBuff->tail)
  {
    ExceptionHandler_ThrowException("Queue(generic) underflowed.\n");
  }
  
  // Parse head object.
  Utils_MemoryCopy(&(pBuff->pContainer[pBuff->head * pBuff->itemSize]), pObj, 
                   (uint32_t)pBuff->itemSize);
  
  /* And update the head value. */
  if (++pBuff->head >= pBuff->capacity)
  {
    pBuff->head = 0;
  }
}

/***
  * @Brief      Returns available space of the buffer.
  *
  * @Params     pBuff-> Buffer pointer.
  *
  * @Return     Available space.
  */
uint32_t QueueGeneric_GetAvailableSpace(QueueGeneric_Buffer_t *pBuff)
{
  uint32_t available_space;

  if (pBuff->tail >= pBuff->head)
  {
    available_space = pBuff->capacity - (pBuff->tail - pBuff->head);
  }
  else
  {
    available_space = pBuff->head - pBuff->tail;
  }
  
  return available_space;
}

/***
  * @Brief      Checks if the buffer is empty.
  *
  * @Params     pBuff-> Pointer to buffer.
  *
  * @Return     TRUE or FALSE.
  */
Bool_t QueueGeneric_IsEmpty(QueueGeneric_Buffer_t *pBuff)
{
  if (pBuff->tail == pBuff->head)
  {
    return TRUE;
  }
  
  return FALSE;
}

/***
  * @Brief      Checks if the buffer is full.
  *
  * @Params     pBuff-> Pointer to buffer.
  *
  * @Return     TRUE or FALSE.
  */
Bool_t QueueGeneric_IsFull(QueueGeneric_Buffer_t *pBuff)
{
  uint32_t next_tail = pBuff->tail + 1;
  
  /* Calculate next tail. */
  if (next_tail >= pBuff->capacity)
  {
    next_tail -= pBuff->capacity;
  }
  
  /* If it's equal to head value, this means the buffer is full. */
  if (next_tail == pBuff->head)
  {
    return TRUE;
  }
  
  return FALSE;
}