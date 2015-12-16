#ifndef __KERNEL_H__
#define __KERNEL_H__

#include "Kernel_HAL.h"

// Maximum size of the heap in bytes.
#define HEAP_MAX_BYTES 512 * sizeof(unsigned)

// All error codes supported by the Kernel.
typedef enum {
  ERR_NONE,
  ERR_UNDEFINED_ERROR,
  ERR_BAD_POINTER,
  ERR_OVER_MAX,
  ERR_INVALID_STACK_SIZE,
  ERR_INVALID_SEM_TYPE,
  ERR_EVENT_OWNED,
  ERR_MEM_ALIGNMENT,
  ERR_MEM_MAXED,
  ERR_MEM_ERROR,
  ERR_TASK_INVALID,
  ERR_TASK_NO_DELETE
} kernelErrors;

// All priorities supported by the Kernel.
typedef enum {
  Realtime,
  High,
  Normal,
  Low,
  Idle
} kernelPriority;

// All objects supported by the Kernel.
typedef enum {
  MUTEX,
  COUNTING,
  MAX_KERNEL_OBJECTS
} kernelObjects;

/*******************************************************************************
 * OS_CreateTask
 ** A system call that allows a user to directly create a 
 ** task within the kernel.
 *
 * Parameters:
 ** void (*)(void)
 *** Pointer to a function that will be called when the task is executed.
 *
 ** kernelPriority
 *** Priority level to be assigned to the newly created task.
 *
 * Return Value:
 ** unsigned
 *** 0 = error
 *** 1 = success.
 ******************************************************************************/
unsigned OS_CreateTask(void (*)(void), kernelPriority);

/*******************************************************************************
 * OS_TaskDelete
 ** A system call that allows a user to remove
 ** a task from kernel scheduling.
 *
 * Parameters:
 ** unsigned ID
 *** The ID of the task to remove.
 *
 * Return Value:
 ** unsigned
 *** 0 = error
 *** 1 = success.
 ******************************************************************************/
unsigned OS_TaskDelete(unsigned ID);

/*******************************************************************************
 * OS_TaskSuspend
 ** A system call that allows a user to suspend a task
 ** from the kernel's scheduling routine.
 *
 * Parameters:
 ** unsigned ID
 *** The ID of the task to suspend.
 *
 * Return Value:
 ** unsigned
 *** 0 = error
 *** 1 = success.
 ******************************************************************************/
unsigned OS_TaskSuspend(unsigned ID);

/*******************************************************************************
 * OS_TaskResume
 ** A system call that allows a user to resume scheduling of
 ** a task.
 *
 * Parameters:
 ** unsigned ID
 *** The ID of the task to remove.
 *
 * Return Value:
 ** unsigned
 *** 0 = error
 *** 1 = success.
 ******************************************************************************/
unsigned OS_TaskResume(unsigned ID);

/*******************************************************************************
 * OS_InitKernel
 ** A system call that initializes key data structures, the scheduler,
 ** as well as the idle task. A call to this function is required before 
 ** the operating system can execute properly. 
 *
 * Parameters:
 ** unsigned char numTasks
 *** Maximum number of tasks that can be held and ran by the 
 *** kernel within its lifetime. (This value cannot exceed MAX_TASKS.)
 *
 ** unsigned stackSize
 *** Maximum size of allowable stack for each task. 
 *** This value must be a multiple of 4, cannot be smaller than MIN_STACK, 
 *** and cannot exceed MAX_STACK.
 *
 * Return Value:
 ** unsigned
 *** 0 = error
 *** 1 = success.
 ******************************************************************************/
unsigned OS_InitKernel(unsigned char numTasks, unsigned stackSize);

/*******************************************************************************
 * OS_GetError
 ** If an error code is produced from a function, use this function to 
 ** retrive more information on what happened.
 *
 * Parameters:
 ** void
 *
 * Return Value:
 ** kernelErrors
 *** Enumeration describing exact failure.
 ******************************************************************************/
kernelErrors OS_GetError(void);

/*******************************************************************************
 * OS_SemCreate
 ** Creates a new semaphore kernel object.
 *
 * Parameters:
 ** kernelObjects type
 *** The type of kernel object to create.
 *
 ** unsigned direction
 *** Direction of counting to be performed.
 *
 ** unsigned maxCnt
 *** Maximum value for the count.
 *
 * Return Value:
 ** unsigned
 *** 0 = error
 *** anything else = ID of the semaphore.
 ******************************************************************************/
unsigned OS_SemCreate(kernelObjects type, unsigned direction, unsigned maxCnt);

/*******************************************************************************
 * OS_SemAcquire
 ** Attempts to acquire a semaphore. (Call is blocking!)
 *
 * Parameters:
 ** unsigned ID
 *** ID of the semaphore to acquire.
 *
 * Return Value:
 ** unsigned
 *** 0 = error
 *** 1 = success.
 ******************************************************************************/
unsigned OS_SemAcquire(unsigned ID);

/*******************************************************************************
 * OS_SemRelease
 ** Attempts to release a semaphore. (Call is blocking!)
 *
 * Parameters:
 ** unsigned ID
 *** ID of the semaphore to release.
 *
 * Return Value:
 ** unsigned
 *** 0 = error
 *** 1 = success.
 ******************************************************************************/
unsigned OS_SemRelease(unsigned ID);

/*******************************************************************************
 * OS_EventLock
 ** If all bits are unowned, they will become owned by calling task.
 *
 * Parameters:
 ** unsigned bits
 *** Bits to assign ownership.
 *
 * Return Value:
 ** unsigned
 *** 0 = error
 *** bits = bits are now owned.
 ******************************************************************************/
unsigned OS_EventLock(unsigned bits);

/*******************************************************************************
 * OS_EventUnlock
 ** If all bits are owned, they will become unowned by calling task.
 *
 * Parameters:
 ** unsigned bits
 *** Bits to remove ownership.
 *
 * Return Value:
 ** unsigned
 *** 0 = error
 *** bits = bits are now unowned.
 ******************************************************************************/
unsigned OS_EventUnlock(unsigned bits);

/*******************************************************************************
 * OS_EventSet
 ** If all bits are unset, they will become set.
 *
 * Parameters:
 ** unsigned bits
 *** Bits to set.
 *
 * Return Value:
 ** unsigned
 *** 0 = error
 *** bits = bits are now set.
 ******************************************************************************/
unsigned OS_EventSet(unsigned bits);

/*******************************************************************************
 * OS_EventSet
 ** If all bits are set, they will become unset.
 *
 * Parameters:
 ** unsigned bits
 *** Bits to unset.
 *
 * Return Value:
 ** unsigned
 *** 0 = error. 
 *** bits = bits are now unset.
 ******************************************************************************/
unsigned OS_EventClear(unsigned bits);

/*******************************************************************************
 * OS_EventWait
 ** If bits are bitwise OR of first param and bitwise AND of second param.
 ** (Call can block!)
 *
 * Parameters:
 ** unsigned anyEvent
 *** Bitwise OR of set bits.
 *
 ** unsigned allEvent
 *** Bitwise AND of set bits.
 *
 * Return Value:
 ** unsigned 
 *** 1 = success.
 ******************************************************************************/
unsigned OS_EventWait(unsigned anyEvent, unsigned allEvent);

/*******************************************************************************
 * OS_Malloc
 ** Attempts to allocate size in bytes on the heap and returns it to caller.
 *
 * Parameters:
 ** unsigned blockSize
 *** Size in bytes to attempt to allocate.
 *
 * Return Value:
 ** void*
 *** 0 - Failure.
 *** else - Pointer to position in memory allocated block starts.
 ******************************************************************************/
void* OS_Malloc(unsigned blockSize);

/*******************************************************************************
 * OS_Free
 ** Attempts to free block of allocated bytes.
 *
 * Parameters:
 ** void* ptr
 *** Pointer to location to attempt to free.
 *
 * Return Value:
 ** void*
 *** 0 - Failure
 *** anything else - Success
 ******************************************************************************/
unsigned OS_Free(void* ptr);

#endif
