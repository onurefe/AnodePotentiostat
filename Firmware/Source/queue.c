#include "generic.h"
#include "exception_handler.h"
#include "queue.h"
#include "stdlib.h"

/* Exported functions**********************************************************/
/***
  * @Brief      Creates a buffer, allocates it's memory and returns the pointer 
  *             of it.           
  * @Params     pBuff-> Pointer to buffer.
  *             pContainer-> Address of the container.
  *             size-> Size of the data container.
  *
  * @Return     None.
  */
void Queue_InitBuffer(Queue_Buffer_t *pBuff, uint8_t *pContainer, uint32_t size)
{ 
  /* Check if the pointer is valid. */
  if (pContainer == NULL)
  {
    ExceptionHandler_ThrowException("Queue invalid container address.\n");
  }
  
  /* Initialize buffer. */
  pBuff->head = 0;
  pBuff->tail = 0;
  pBuff->pContainer = pContainer;
  pBuff->capacity = size;
}

/***
  * @Brief      Clears the addressed buffer.
  *
  * @Params     pBuff-> Pointer to the buffer.
  */
void Queue_ClearBuffer(Queue_Buffer_t *pBuff)
{
  pBuff->head = 0;
  pBuff->tail = 0;
}

/***
  * @Brief      Enqueues byte to the given buffer.
  *
  * @Params     pBuff-> Pointer to the buffer.
  *             byte-> Byte to be enqueued.
  */
void Queue_Enqueue(Queue_Buffer_t *pBuff, uint8_t byte)
{
  /* Set new tail element and update the tail value. */
  pBuff->pContainer[pBuff->tail] = byte;
  if (++pBuff->tail >= pBuff->capacity)
  {
    pBuff->tail = 0;
  }
  
  /* Check if buffer overflow occurred. */
  if (pBuff->tail == pBuff->head)
  {
    ExceptionHandler_ThrowException("Queue overflowed.\n");
  }
}

/***            
  * @Brief      Dequeues byte from the given buffer.
  *
  * @Params     pBuff-> Pointer to the buffer.
  *
  * @Return     Element.
  */
uint8_t Queue_Dequeue(Queue_Buffer_t *pBuff)
{ 
  /* Check if there are any element to dequeue. If there isn't, this means that 
    a fatal error occurred. */
  if (pBuff->head == pBuff->tail)
  {
    ExceptionHandler_ThrowException("Queue underflowed.\n");
  }
  
  // Parse head element.
  uint8_t element = pBuff->pContainer[pBuff->head];
  
  /* And update the head value. */
  if (++pBuff->head >= pBuff->capacity)
  {
    pBuff->head = 0;
  }
  
  return element;
}

/***
  * @Brief      Removes elements until the indexed element(also including).
  *
  * @Params     pBuff-> Pointer to the buffer.
  *             elementIndex-> Index of the element.
  *
  * @Return     None.
  */
void Queue_Remove(Queue_Buffer_t *pBuff, uint16_t elementIndex)
{
  /* Just change head index. */
  if (elementIndex < Queue_GetElementCount(pBuff))
  {
    uint32_t new_head;
  
    new_head = pBuff->head + elementIndex + 1;
    
    if (new_head >= pBuff->capacity)
    {
      new_head -= pBuff->capacity;
    }
    
    pBuff->head = new_head;
  }
}

/***
  * @Brief      Searches an element in the buffer. Returns element index if the element
  *             exists. Returns -1 otherwise.
  *
  * @Params     pBuff-> Buffer to be searched.
  *             element-> Element value.
  *
  * @Return     Element index or -1.
  */

int32_t Queue_Search(Queue_Buffer_t *pBuff, uint8_t element)
{
  uint32_t num_of_elements;
  
  num_of_elements = Queue_GetElementCount(pBuff);
  
  // Search buffer.
  for (uint32_t i = 0; i < num_of_elements; i++)
  {
    if (Queue_Peek(pBuff, i) == element)
    {
      return i;
    }
  }
  
  return -1;
}

/***
  * @Brief      Peeks an element in the buffer. Indexing starts at the first element.
  *                       
  * @Params     pBuff-> Pointer to the buffer.
  *             elementIndex-> Index of the element.
  *
  * @Return     Element value.
  */
uint8_t Queue_Peek(Queue_Buffer_t *pBuff, uint32_t elementIndex)
{
  uint32_t element_position;
  
  element_position = pBuff->head + elementIndex;
  
  if (element_position >= pBuff->capacity)
  {
    element_position -= pBuff->capacity;
  }
  
  return (pBuff->pContainer[element_position]);
}

/***
  * @Brief      Returns available space of the buffer.
  *
  * @Params     pBuff-> Buffer pointer.
  *
  * @Return     Available space.
  */
uint32_t Queue_GetAvailableSpace(Queue_Buffer_t *pBuff)
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
  * @Brief      Returns number of elements the buffer contains.
  *
  * @Params     pBuff-> Buffer pointer.
  *
  * @Return     Element count.
  */
uint32_t Queue_GetElementCount(Queue_Buffer_t *pBuff)
{
  uint32_t element_count;

  if (pBuff->tail >= pBuff->head)
  {
    element_count = pBuff->tail - pBuff->head;
  }
  else
  {
    element_count = (pBuff->tail + pBuff->capacity) - pBuff->head;
  }
  
  return element_count;
}

/***
  * @Brief      Checks if the buffer is empty.
  *
  * @Params     pBuff-> Pointer to buffer.
  *
  * @Return     TRUE or FALSE.
  */
Bool_t Queue_IsEmpty(Queue_Buffer_t *pBuff)
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
Bool_t Queue_IsFull(Queue_Buffer_t *pBuff)
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