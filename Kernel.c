#include "Kernel.h"
#include "Timer.h"
#include <stdlib.h>

// Defines
// Minimum size of each TCB's stack.
#define MIN_STACK       0x18
// Maximum size of each TCB's stack.
#define MAX_STACK       0x40
// Maximum number of tasks.
#define MAX_TASKS       0x05
// Base value for stack.
#define STACK_BASE      0x11
// Task aging threshold which dictates how fast or slow it changes priority queues.
#define AGE_THRESHOLD   5
// Maximum number of priorities.
#define MAX_PRIORITIES	((unsigned)Idle) + 1
// Maximum number of Semaphores.
#define MAX_SEMAPHORES  5
// Invalid Semphore value.
#define INVALID_SEMAPHORE 0x00
// Size of a byte in bits
#define BITSPERBYTE      8
// Number of bits per unsigned.
#define EVENT_OWNERS     sizeof(unsigned) * BITSPERBYTE
  
typedef struct TCB TCB;
typedef struct Queue Queue;

typedef enum {
  BLK_FREE,
  BLK_USED
} Block_Status;

typedef struct {
  char* ptr;
  Block_Status status;
  unsigned allocID;
} Block;

// Semaphores
typedef struct {
  unsigned  id;
  // Semaphore
  unsigned  count;
  unsigned  maxCount;
  unsigned  maxConstCount;
  char      dir;
  // Mutex
  char      locked;
  TCB*      owner;
  // Blocked list
  Queue*     blocked;
  kernelObjects type;
} Semaphore;

/*******************************************************************************
 * OSp_SetError
 ** The kernel uses this to set the current error enumeration.
 *
 * Parameters:
 ** kernelErrors
 *** Sets the current error to this value.
 *
 * Return Value:
 ** void
 ******************************************************************************/
void OSp_SetError(kernelErrors);

/*******************************************************************************
 * OSp_IdleTask
 ** Guaranteed task that will run when no other tasks exist in the queue.
 *
 * Parameters:
 ** void
 *
 * Return Value:
 ** void
 ******************************************************************************/
void OSp_IdleTask(void);

/*******************************************************************************
 * OSp_InitTCBs
 ** Initializes TCB stack pointers and places the thumb bit in each stack space
 *
 * Parameters:
 ** void
 *
 * Return Value:
 ** void
 ******************************************************************************/
void OSp_InitTCBs(void);

/*******************************************************************************
 * OSp_InitScheduler
 ** Initializes the kernel's scheduler.
 *
 * Parameters:
 ** void
 *
 * Return Value:
 ** void
 ******************************************************************************/
void OSp_InitScheduler(void);

/*******************************************************************************
 * OSp_QueueTask
 ** Ensures a task is ready to be queued and places it in the queue.
 *
 * Parameters:
 ** TCB* task
 *** Task to queue for scheduling.
 *
 * Return Value:
 ** void
 ******************************************************************************/
void OSp_QueueTask(TCB* task);

/*******************************************************************************
 * OSp_CycleTasks
 ** Rotates tasks in a given queue.
 *
 * Parameters:
 ** Queue* queue
 *** Queue to rotate tasks within.
 *
 * Return Value:
 ** void
 ******************************************************************************/
void OSp_CycleTasks(Queue* queue);

/*******************************************************************************
 * OSp_UpdateScheduler
 ** Updates all queues, ages, and sets the next Task.
 *
 * Parameters:
 ** void
 *
 * Return Value:
 ** unsigned
 *** 0 - Do not context switch.
 *** 1 - Perform context switch.
 ******************************************************************************/
unsigned OSp_UpdateScheduler(void);

/*******************************************************************************
 * OSp_ScheduleTask
 ** Function to be called by kernel that performs scheduling.
 *
 * Parameters:
 ** void
 *
 * Return Value:
 ** unsigned
 *** 0 - Do not context switch.
 *** 1 - Perform context switch.
 ******************************************************************************/
unsigned OSp_ScheduleTask(void);

/*******************************************************************************
 * OSp_MoveTask
 ** Moves a task from one queue to another.
 *
 * Parameters:
 ** TCB* _task
 *** Pointer to a task to move.
 *
 ** unsigned _priority
 *** Priority of queue to place task.
 *
 * Return Value:
 ** void
 ******************************************************************************/
void OSp_MoveTask(TCB* _task, unsigned _priority);

/*******************************************************************************
 * OSp_PopQueue
 ** Pops a task from a specified scheduler priority queue.
 *
 * Parameters:
 ** Queue* _queue
 *** Queue to remove from task from.
 *
 * Return Value:
 ** TCB*
 *** Pointer to the TCB removed from the scheduler queue.
 ******************************************************************************/
TCB* OSp_PopQueue(Queue* _queue);

/*******************************************************************************
 * OSp_PushQueue
 ** Pushes a task onto a specified scheduler priority queue.
 *
 * Parameters:
 ** Queue* _queue
 *** Queue to push task onto.
 *
 ** TCB* _task
 *** Task to be pushed.
 *
 * Return Value:
 ** unsigned
 *** 0 = Error
 *** 1 = Success
 ******************************************************************************/
unsigned OSp_PushQueue(Queue* _queue, TCB* _task);

/*******************************************************************************
 * OSp_AllocateInit
 ** Initializes heap allocation variables, arrays, and anything else.
 ** Also takes a size in bytes as the chunk size for allocations.
 *
 * Parameters:
 ** unsigned blockSize
 *** Size in bytes used as chunk size.
 *
 * Return Value:
 ** unsigned
 *** 0 = Error
 *** anything else = Success
 ******************************************************************************/
unsigned OSp_AllocateInit(unsigned blockSize);

/*******************************************************************************
 * OSp_Assignblocks
 ** Assigns blocks as used.
 *
 * Parameters:
 ** Block* startBlock
 *** Starting block to assign
 *
 ** unsigned numBlocks
 *** Number of blocks to assign as used.
 *
 * Return Value:
 ** void
 ******************************************************************************/
void OSp_AssignBlocks(Block* startBlock, unsigned numBlocks);

/*******************************************************************************
 * OSp_FreeBlocks
 ** Frees all blocks of a specified ID.
 *
 * Parameters:
 ** unsigned id
 *** ID of allocated blocks that should be freed.
 *
 * Return Value:
 ** void
 ******************************************************************************/
void OSp_FreeBlocks(unsigned id);

/*******************************************************************************
 * OSp_AllocateInit
 ** Initializes a new Semaphore kernel object.
 *
 * Parameters:
 ** unsigned direction
 *** Direction of counting that should be applied.
 *
 ** unsigned maxCnt
 *** Maximum value that the Semaphore object can take on.
 *
 * Return Value:
 ** Semaphore*
 *** Address of the newly created Semaphore object.
 ******************************************************************************/
Semaphore* OSp_SemInit(unsigned direction, unsigned maxCnt);

/*******************************************************************************
 * OSp_BlockAcquire
 ** Blocks the current task until the Semaphore object is released.
 *
 * Parameters:
 ** Semaphore*
 *** Semaphore kernel object to block on.
 *
 * Return Value:
 ** void
 ******************************************************************************/
void OSp_BlockAcquire(Semaphore* sem);

/*******************************************************************************
 * OSp_AcquireCountSem
 ** Attempts to acquire a counting Semaphore object for the calling task.
 ** (Procedure can block!)
 *
 * Parameters:
 ** Semaphore*
 *** Semaphore kernel object to acquire.
 *
 * Return Value:
 ** void
 ******************************************************************************/
void OSp_AcquireCountSem(Semaphore* sem);

/*******************************************************************************
 * OSp_AcquireMutex
 ** Attempts to acquire a mutex Semaphore object for the calling task.
 ** (Procedure can block!)
 *
 * Parameters:
 ** Semaphore*
 *** Semaphore kernel object to acquire.
 *
 * Return Value:
 ** void
 ******************************************************************************/
void OSp_AcquireMutex(Semaphore* sem);

/*******************************************************************************
 * OSp_CycleBlocked
 ** Removed task from the blocked queue and set it ready.
 *
 * Parameters:
 ** Semaphore*
 *** Semaphore kernel object to modify blocked queue.
 *
 * Return Value:
 ** unsigned
 *** 0 = Task does not exist.
 *** 1 = Task exists.
 ******************************************************************************/
unsigned OSp_CycleBlocked(Semaphore* sem);

/*******************************************************************************
 * OSp_ReleaseMutex
 ** Attempts to release mutex Semaphore object.
 *
 * Parameters:
 ** Semaphore*
 *** Semaphore kernel object to attempt to release.
 *
 * Return Value:
 ** unsigned
 *** 0 = Mutex was not released.
 *** 1 = Mutex was released.
 ******************************************************************************/
unsigned OSp_ReleaseMutex(Semaphore* sem);

/*******************************************************************************
 * OSp_ReleaseSem
 ** Attempts to release Semaphore object.
 *
 * Parameters:
 ** Semaphore*
 *** Semaphore kernel object to attempt to release.
 *
 * Return Value:
 ** unsigned
 *** 0 = Semaphore was not released.
 *** 1 = Semaphore was released.
 ******************************************************************************/
unsigned OSp_ReleaseSem(Semaphore* sem);

/*******************************************************************************
 * OSp_EventCheckOwned
 ** Checks if bits are owned by a task.
 *
 * Parameters:
 ** TCB* task
 *** Task to check against bits.
 *
 ** unsigned bits
 *** Bits to check for ownership.
 *
 ** unsigned checkUnowned
 *** Flag to determine if unowned bits are perceived as an error.
 *
 * Return Value:
 ** unsigned
 *** 0 = Error
 *** 1 = Success
 ******************************************************************************/
unsigned OSp_EventCheckOwned(TCB* task, unsigned bits, unsigned checkUnowned);

/*******************************************************************************
 * OSp_CheckReady
 ** Sets a task to the ready state.
 *
 * Parameters:
 ** TCB* task
 *** Task to set to ready.
 *
 * Return Value:
 ** void
 ******************************************************************************/
void OSp_CheckReady(TCB* task);

/*******************************************************************************
 * OSp_EventBlock
 ** Blocks the calling task until the event is ready.
 *
 * Parameters:
 ** unsigned anyEvent
 *** Bitwise OR of set bits.
 *
 ** unsigned allEvent
 *** Bitwise AND of set bits.
 *
 * Return Value:
 ** void
 ******************************************************************************/
void OSp_EventBlock(unsigned anyEvent, unsigned allEvents);

//Timer* timResponse;

// Memory
typedef struct {
  char data[HEAP_MAX_BYTES];
  unsigned short blockSize;
  Block blocks[HEAP_MAX_BYTES];
  unsigned freeBlocks,
    allocID;
} Alloc_Table_t;

Alloc_Table_t alloc_table;

// Types
typedef void (*OS_TaskAddress)(void);

typedef enum {
  READY,
  RUNNING,
  SUSPENDED,
  EVENTBLOCKED,
  SEMBLOCKED,
  DELETED
} taskState_t;

// TCB
struct TCB {
  unsigned 	 *sp;
  OS_TaskAddress taskAddress;
  taskState_t	 taskState;
  kernelPriority taskPriority,
    taskAgedPriority;
  int            taskAge;
  unsigned       ID,
    slot;
};

// Queue
struct Queue {
  unsigned numTasks;
  TCB* tasks[MAX_TASKS];
};

// Events
typedef struct {
  unsigned record;
} Event;

// Variables
Event events;
TCB* EventOwners[sizeof(unsigned) * BITSPERBYTE];
static unsigned Stacks[MAX_TASKS*MAX_STACK];
static Queue FeedbackQueues[MAX_PRIORITIES];
static unsigned NumTasks;

static unsigned MaxTasks;
static unsigned StackSize;

unsigned TaskIDs;
// Booleans that tell us if a TCB slot is in use.
unsigned TaskSlots[MAX_TASKS];

// Semaphores
Queue BlockedQueue[MAX_SEMAPHORES];
static unsigned SemaphoreID;
static unsigned SemaphoreCount;

static kernelErrors currentError;

TCB Tasks[MAX_TASKS];
TCB *OS_TaskNEXT;
TCB *OS_TaskRUNNING;

Semaphore Semaphores[MAX_SEMAPHORES];

// Extern'd functions
extern void OSp_InitGPIOF(void);
extern void OSp_InitTimers(void);

// Default task
void OSp_IdleTask(void) {
  while (1) {
    ;
  }
}

// Scheduler Stuff
void Swap(TCB** taskA, TCB** taskB) {
  TCB* temp = *taskA;
  *taskA = *taskB;
  *taskB = temp;
}

void OSp_InitScheduler(void) {
  unsigned i;
	
  // Initialize our queues.
  for (i = 0; i < MAX_PRIORITIES; ++i)
    FeedbackQueues[i].numTasks = 0;
}

void OSp_QueueTask(TCB* task) {
  Queue* queue = FeedbackQueues + task->taskPriority;
	
  task->taskAge = 0;
  task->taskAgedPriority = task->taskPriority;
	
  queue->tasks[queue->numTasks++] = task;
}

// This will move a task from one queue, to the end of the new one.
void OSp_MoveTask(TCB* _task, unsigned _priority) {
  kernelPriority priority = (kernelPriority)_priority;
  Queue* queue = 0;
  int i;
	
  // First, requeue the task, we wanted to reset the age anyway.
  OSp_QueueTask(_task);
  _task->taskAgedPriority = priority;

  queue = &FeedbackQueues[_task->taskPriority];
	
  // Find the task so we can remove it.
  for (i = 0; i < queue->numTasks; ++i) {
    TCB* runningTask = queue->tasks[i];
		
    if (runningTask == _task) {
      // Found it.
      //runningTask = 0;
      break;
    }
  }
	
  // i should be at the position we found the task.
  // we need to shift other tasks left by one.
  for (; i < queue->numTasks; ++i) {
    queue[i-1] = queue[i];
  }
}

// This is a dumb swap for all tasks in a given queue.
void OSp_CycleTasks(Queue* queue) {
  OS_CRITICAL_BEGIN
    int i;
	
  // We only need to swap NUM_TASKS - 1 times.
  for (i = 0; i < queue->numTasks; ++i) {
    TCB* task1 = queue->tasks[i];
    TCB* task2 = queue->tasks[i + 1];
    
    Swap(&task1, &task2);
  }
	
  OS_CRITICAL_END
    }

unsigned OSp_QueueRemoveTask(Queue* queue, TCB* task) {
  OS_CRITICAL_BEGIN
    unsigned i;
  //TCB* foundTask;
  
  for (i = 0; i < queue->numTasks; ++i) {
    if (queue->tasks[i] == task) {
      // Shift forward everything behind the found task.
      for (; i < queue->numTasks; ++i) {
        queue[i-1] = queue[i];
      }
      
      return 1;
    }
  }
  
  OS_CRITICAL_END
    return 0;
}

void OSp_SchedulerRemoveTask(TCB* task) {
  OS_CRITICAL_BEGIN
    unsigned i;
  
  // Loop through highest->lowest priority queues and find the task.
  for (i = 0; i < MAX_PRIORITIES; ++i) {
    Queue* queue = FeedbackQueues + i;
    
    if (OSp_QueueRemoveTask(queue, task))
      break;
  }
  
  OS_CRITICAL_END
    }

unsigned OSp_UpdateScheduler(void) {
  // MULTILEVEL FEEDBACK QUEUE IMPLEMENTATION
  // We're here to switch to a new task.
  // Loop through highest->lowest priority queues and find the next task.
  // Move the current one to the end of its priority queue.
  // If we find a task, we'll set that to be the next task.
  // Once we find a task (idle or not), look for ones of 
  // lower priority and age them.
  // Once a task is aged up to AGE_THRESHOLD.
  // Move it to the next higher priority and reset its age.
	
  // Only cycle a queue when you put a task back into it.
	
  int i;
  TCB* nextTask = 0;
	
  // Loop through highest->lowest priority queues and find the next task.
  for (i = 0; i < MAX_PRIORITIES; ++i) {
    Queue* queue = FeedbackQueues + i;
		
    int j;
    // If there is a task, get a pointer to it.
    for (j = 0; j < queue->numTasks; ++j) {
      TCB* task = queue->tasks[j];
			
      // Ensure we actually found a new task.
      // Make sure the task isn't blocked.
      if (task != OS_TaskRUNNING && task->taskState != EVENTBLOCKED
	  && task->taskState != SEMBLOCKED && task->taskState != SUSPENDED && task->taskState != DELETED) {
        nextTask = task;
        goto GotNextTask;
      }
    }
  }

  // If we land here, that means we didn't find any tasks 
  // that could replaced what's running. I think we're done here.
  return 0;
  
 GotNextTask:

  if (OS_TaskRUNNING->taskPriority != Idle) {
    // We need to get the current task to the end of its own queue.
    OSp_CycleTasks(FeedbackQueues + OS_TaskRUNNING->taskPriority);
    
    // For the running task, negate its priority by 1.
    OS_TaskRUNNING->taskAge--;
  }
	
  // Set OS_RUNNING to new task.
  OS_TaskNEXT = nextTask;

  // Age all lower priorities and move tasks above threshold.
  // remember to reset their ages.
  // Also if a task has run too much, we should move it down a priority queue.
  for (i = 0; i < MAX_PRIORITIES-1; ++i) {
    Queue* queue = FeedbackQueues + i;
		
    int j;
    // If there is a task, get a pointer to it.
    for (j = 0; j < queue->numTasks; ++j) {
      TCB* task = queue->tasks[j];
			
      // Increase all task ages of lower priorities by 1.
      if (task != OS_TaskNEXT)
        task->taskAge++;
    }
  }
		
  // Check for age threshold violations.
  // Depending on the sign of the age, we'll raise
  // or lower the temporary priority.
  // We don't want to age the Idle queue.
  for (i = 0; i < MAX_PRIORITIES-1; ++i) {
    Queue* queue = FeedbackQueues + i;
		
    int j;
    // If there is a task, get a pointer to it.
    for (j = 0; j < queue->numTasks; ++j) {
      TCB* task = queue->tasks[j];
			
      // Test for the age threshold here, also ensure we're within the bounds
      // so we don't break the system.
      if (task->taskAge > AGE_THRESHOLD && task->taskPriority != Realtime) {
        OSp_MoveTask(task, ((unsigned)task->taskPriority)+1);
      } else if (task->taskAge < -AGE_THRESHOLD && task->taskPriority != Low) {
        OSp_MoveTask(task, ((unsigned)task->taskPriority)-1);
      }
    }
  }
  
  Timers_Update();
  
  return 1;
}

unsigned OS_CreateTask(void (*_task)(void), kernelPriority _priority) {
  unsigned offset = 0;
  TCB* task = Tasks;

  if (!_task) {
    OSp_SetError(ERR_BAD_POINTER);
    return 0;
  }

  // Guarantee IdleTask is at task[0] and has Idle priority.
  if (_priority == Idle) {
    if (_task != OSp_IdleTask) {
      // This is an error, a user task cannot be Idle priority.
      OSp_SetError(ERR_UNDEFINED_ERROR);
      return 0;
    }
  } else {
    unsigned i;
    
    // Loop through our TCB Slots to find an empty slot.
    for (i = 0; i < MAX_TASKS; ++i) {
      if (TaskSlots[i] == 0) {
        offset = i;
        TaskSlots[i] = 1;
        break;
      }
    }
    
    // If offset == 0, we didn't find an open slot.
    if (offset >= MAX_TASKS) {
      OSp_SetError(ERR_OVER_MAX);
      return 0;
    }
    
    //offset = NumTasks;
  }

  // Ensure we're looking at the right task, based off the offset
  task = (Tasks + offset);

  // Assign the task its callback.
  task->taskAddress = _task;
  
  // Assign the priority
  task->taskPriority = _priority;
	
  task->taskState = READY;
	
  task->ID = TaskIDs++;

  task->slot = offset;
  
  // Set task callback address
  Stacks[(offset + 1) * (StackSize) - 0x03] = (unsigned)task->taskAddress;

  OSp_QueueTask(task);
	
  ++NumTasks;

  return task->ID;
}

// Initialize each TCB's SP, Thumb Bit, and other initializations.
void OSp_InitTCBs(void) {
  int i;
  
  for (i = 0; i < MaxTasks; ++i) {
    unsigned int* stack_space = &Stacks[i * StackSize];
    
    Tasks[i].sp = (stack_space + StackSize - STACK_BASE);
    stack_space[StackSize - 0x02] = 0x01000000;
  }
}

unsigned OS_InitKernel(unsigned char numTasks, unsigned stackSize) {
  unsigned i;
  
  OS_CRITICAL_BEGIN
	
    // Initialize Statics
    NumTasks  = 0;
  MaxTasks    = numTasks ? numTasks + 1 : MAX_TASKS;
  SemaphoreCount = 0;
  for (i = 0; i < EVENT_OWNERS; ++i) EventOwners[i] = 0;
  TaskIDs = 1;
  for (i = 0; i < MAX_TASKS; ++i) TaskSlots[i] = 0;
  
  events.record = 0;
  // Start semaphore IDs at 1
  SemaphoreID = 1;
  
  // Ensure the stack size is a multiple of 4 and larger than the minimum.
  if (stackSize == 0)
    StackSize = MAX_STACK;
  else if (stackSize % 4 == 0 && stackSize > MIN_STACK)
    StackSize = stackSize;
  else {
    OSp_SetError(ERR_INVALID_STACK_SIZE);
    return 0;
  }
  
  if (NumTasks >= MAX_TASKS || StackSize > MAX_STACK) {
    OSp_SetError(ERR_OVER_MAX);
    return 0;
  }
	
  OSp_SetError(ERR_NONE);
  
  OSp_InitGPIOF();
  OSp_InitTimers();
  
  OSp_InitTCBs();
  OSp_InitScheduler();
	
  OS_CreateTask(&OSp_IdleTask, Idle);
  
  OS_TaskNEXT    = Tasks + 1;
  OS_TaskRUNNING = Tasks;
  
  OSp_AllocateInit(8);
	
  Timers_Init();
  
  OS_CRITICAL_END
  
    return 1;
}

unsigned OSp_ScheduleTask(void) {
  unsigned ret = OSp_UpdateScheduler();

  if (ret != 0) {
    if (OS_TaskRUNNING->taskState != SEMBLOCKED
	&& OS_TaskRUNNING->taskState != EVENTBLOCKED) {
      OS_TaskRUNNING->taskState = READY;
      OS_TaskNEXT->taskState = RUNNING;
    }
  }

  return ret;
}

void OSp_SetError(kernelErrors err) {
  currentError = err;
}

// NOTE: Resets the current error on read back to a good state.
kernelErrors OS_GetError(void) {
  kernelErrors err = currentError;
	
  currentError = ERR_NONE;
  return err;
}

Semaphore* OSp_SemInit(unsigned direction, unsigned maxCnt) {
  // Grab & reset the Semaphore we're gonna use.
  Semaphore* sem = Semaphores + SemaphoreCount++;
  
  sem->id = SemaphoreID++;
  sem->locked = 0;
  sem->blocked = &BlockedQueue[sem->id % MAX_SEMAPHORES];
  
  if (direction == 0) {
    sem->dir      = 0;
    sem->maxCount = 0;
    sem->count    = sem->maxConstCount = maxCnt;
  } else {
    sem->dir      = 1;
    sem->maxCount = maxCnt;
    sem->count    = sem->maxConstCount = 0;
  }

  return sem;
}

unsigned OS_SemCreate(kernelObjects type, 
                      unsigned direction, unsigned maxCnt) {
  Semaphore* currSem;
  
  if (type > MAX_KERNEL_OBJECTS) {
    OSp_SetError(ERR_INVALID_SEM_TYPE);
    return 0;
  }

  if (maxCnt == 0 || SemaphoreCount >= MAX_SEMAPHORES) {
    OSp_SetError(ERR_OVER_MAX);
    return 0;
  }

  currSem = OSp_SemInit(direction, maxCnt);

  // Do Mutex things.
  if (type == MUTEX) {
    currSem->locked = 0;
    currSem->owner = OS_TaskRUNNING;
    currSem->type = MUTEX;
  } else
    currSem->type = COUNTING;
  
  //timResponse = Timer_Create();
  
  return currSem->id;
}

void OSp_BlockAcquire(Semaphore* sem) {
  // Failure, send the task to the blocked queue.
  OSp_PushQueue(sem->blocked, OS_TaskRUNNING);
  OS_TaskRUNNING->taskState = SEMBLOCKED;

  OS_CRITICAL_END
    
    // Give up our quantum, we're done.
    while (OS_TaskRUNNING->taskState == SEMBLOCKED) { ; }

  OS_CRITICAL_BEGIN
    }

void OSp_AcquireCountSem(Semaphore* sem) {
  switch (sem->dir) {
  case 0:
    if (sem->count > sem->maxCount)
      sem->count--;
    else {
      // Once unblocked, acquire.
      OSp_BlockAcquire(sem);
      sem->count--;
    }
    
    break;

  case 1:
    if (sem->count < sem->maxCount)
      sem->count++;
    else {
      // Once we are unblocked, we'll acquire.
      OSp_BlockAcquire(sem);
      sem->count++;
    }

    break;
  }
}

void OSp_AcquireMutex(Semaphore* sem) {
  if (sem->locked) {
    // Check for Recursive
    if (OS_TaskRUNNING == sem->owner)
      return;

    // Time to block.
    OSp_BlockAcquire(sem);
  }

  // Acquire the mutex here.
  sem->locked = 1;
  sem->owner  = OS_TaskRUNNING;
}

unsigned OS_SemAcquire(unsigned ID) {
  Semaphore* sem;
  
  if (ID > SemaphoreID || ID == INVALID_SEMAPHORE) {
    OSp_SetError(ERR_INVALID_SEM_TYPE);
    return 0;
  }
  
  OS_CRITICAL_BEGIN

    sem = Semaphores + ((ID-1) % MAX_SEMAPHORES);

  switch(sem->type) {
  case COUNTING: {
    OSp_AcquireCountSem(sem);

    break;
  }

  case MUTEX: {
    OSp_AcquireMutex(sem);

    break;
  }
  }
  
  //Timer_Pause(timResponse);
  
  OS_CRITICAL_END

    return 1;
}

unsigned OSp_CycleBlocked(Semaphore* sem) {
  TCB* task = OSp_PopQueue(sem->blocked);

  task->taskState = READY;
  
  // Might do something with this in the future?
  return task ? 1 : 0;
}

unsigned OSp_ReleaseMutex(Semaphore* sem) {
  if (OS_TaskRUNNING != sem->owner)
    return 0;

  sem->owner  = 0;
  sem->locked = 0;

  return 1;
}

unsigned OSp_ReleaseSem(Semaphore* sem) {
  switch(sem->dir) {
  case 0:
    if (sem->count < sem->maxConstCount)
      ++sem->count;

    break;

  case 1:
    if (sem->count > sem->maxConstCount)
      --sem->count;

    break;
  }

  return 1;
}

unsigned OS_SemRelease(unsigned ID) {
  Semaphore* sem;
  unsigned status;
  
  if (ID == INVALID_SEMAPHORE) {
    OSp_SetError(ERR_INVALID_SEM_TYPE);
    return 0;
  }
  
  OS_CRITICAL_BEGIN

    sem = Semaphores + ((ID-1) % MAX_SEMAPHORES);
  
  switch(sem->type) {
  case MUTEX:
    status = OSp_ReleaseMutex(sem);

    break;

  case COUNTING:
    status = OSp_ReleaseSem(sem);

    break;
  }

  // Status of 1 == released.
  if (status != 0) {
    OSp_CycleBlocked(sem);
  }
  
  //Timer_Start(timResponse);
  
  OS_CRITICAL_END
  
    return status;
}

// Pops item off front of queue and shifts all forward.
TCB* OSp_PopQueue(Queue* _queue) {
  TCB* task;
  unsigned i;
  
  if (_queue->numTasks > 0)
    task = _queue->tasks[0];
  
  // Shift tasks
  for (i = 0; i < _queue->numTasks; ++i) {
    _queue->tasks[i] = _queue->tasks[i+1];
  }

  if (_queue->numTasks > 0)
    --_queue->numTasks;

  return task;
}

// Inserts task at end of queue.
unsigned OSp_PushQueue(Queue* _queue, TCB* _task) {
  if (_queue->numTasks >= MAX_TASKS) {
    OSp_SetError(ERR_OVER_MAX);
    return 0;
  }

  _queue->tasks[_queue->numTasks++] = _task;
  return 1;
}

unsigned OSp_EventCheckOwned(TCB* task, unsigned bits, unsigned checkUnowned) {
  unsigned i;

  // Loop through and ensure we can set all bits.
  for (i = 0; i < EVENT_OWNERS; ++i) {
    unsigned bit = (bits >> i) & 0x01;
    if (!checkUnowned) {
      if (bit && EventOwners[i] == task)
        continue;
    } else {
      if ((bit && EventOwners[i] == task) || EventOwners[i] == 0)
        continue;
    }
    
    OSp_SetError(ERR_EVENT_OWNED);
    return 0;
  }
  
  return 1;
}

// All bits, hopefully, are unowned.
unsigned OS_EventLock(unsigned bits) {
  unsigned i;

  // Ensure we can set all bits before setting them.
  if (!OSp_EventCheckOwned(OS_TaskRUNNING, bits, 0))
    return 0;
  
  // We should be able to lock all the bits if we're here.
  for (i = 0; i < EVENT_OWNERS; ++i) {
    if ((bits >> i) & 0x01) {
      EventOwners[i] = OS_TaskRUNNING;
    }
  }
  
  return bits;
}

unsigned OS_EventUnlock(unsigned bits) {
  unsigned i;

  // Ensure we can set all bits before setting them.
  if (!OSp_EventCheckOwned(OS_TaskRUNNING, bits, 0))
    return 0;
  
  // We should be able to unlock all the bits if we're here.
  for (i = 0; i < EVENT_OWNERS; ++i) {
    if ((bits >> i) & 0x01) {
      EventOwners[i] = 0;
    }
  }
  
  return bits;
}

void OSp_CheckReady(TCB* task) {
  task->taskState = READY;
}

unsigned OS_EventSet(unsigned bits) {
  unsigned i;
  
  if (!OSp_EventCheckOwned(OS_TaskRUNNING, bits, 1))
    return 0;
  
  events.record |= bits;
  
  // Loop through tasks, Check if they're BLOCKED.
  // If they are, see if they can be set to READY.
  for (i = 0; i < EVENT_OWNERS; ++i) {
    TCB* task = EventOwners[i];
    
    if (task->taskState == EVENTBLOCKED) {
      // Somehow try to move the taskState to READY here.
      //task->taskState = READY;
      OSp_CheckReady(task);
    }
  }
  
  return bits;
}

unsigned OS_EventClear(unsigned bits) {
  if (!OSp_EventCheckOwned(OS_TaskRUNNING, bits, 1))
    return 0;
  
  events.record &= bits;

  return bits;
}

void OSp_EventBlock(unsigned anyEvent, unsigned allEvent) {
  OS_TaskRUNNING->taskState = EVENTBLOCKED;
  
  while (!((!(anyEvent | events.record) && !(allEvent & events.record)) || 
	   OS_TaskRUNNING->taskState == EVENTBLOCKED)) {;}
              
  OS_TaskRUNNING->taskState = READY;
}

unsigned OS_EventWait(unsigned anyEvent, unsigned allEvent) {
  OSp_EventBlock(anyEvent, allEvent);
  
  return 1;
}

unsigned OSp_AllocateInit(unsigned blockSize) {
  unsigned i;
  
  // Ensure blockSize is aligned and usable.
  if (blockSize == 0 || blockSize > HEAP_MAX_BYTES / 2 || blockSize % 8 != 0
      || HEAP_MAX_BYTES % blockSize != 0) {
    OSp_SetError(ERR_MEM_ALIGNMENT);
    return 0;
  }

  // allocID: 0 == unassigned ID.
  alloc_table.allocID    = 1;
  alloc_table.blockSize  = blockSize;
  alloc_table.freeBlocks = HEAP_MAX_BYTES;

  // Initialize table blocks
  for (i = 0; i < HEAP_MAX_BYTES; ++i) {
    Block* currBlock = alloc_table.blocks + i;

    currBlock->status  = BLK_FREE;
    currBlock->allocID = 0;
    currBlock->ptr     = alloc_table.data + i * sizeof(blockSize);
  }

  return sizeof(alloc_table.blocks);
}

void OSp_AssignBlocks(Block* startBlock, unsigned numBlocks) {
  unsigned i;
  
  for (i= 0; i < numBlocks; ++i) {
    Block* currBlock = startBlock + i;

    currBlock->status  = BLK_USED;
    currBlock->allocID = alloc_table.allocID;
  }
  
  alloc_table.allocID++;
}

void* OS_Malloc(unsigned blockSize) {
  OS_CRITICAL_BEGIN
    unsigned i, j;
  Block* freeBlock = 0;
  
  if (blockSize == 0 || HEAP_MAX_BYTES % blockSize != 0) {
    OSp_SetError(ERR_MEM_ALIGNMENT);
    return 0;
  }

  if (alloc_table.freeBlocks == 0) {
    OSp_SetError(ERR_MEM_MAXED);
    return 0;
  }

  // Find next free block.
  // Need to also check, if blockSize is larger than 1 block, all other
  //  blocks it would use.
  for (i = 0; i < HEAP_MAX_BYTES; ++i) {
    // Determine the amount of contiguous blocks we need.
    unsigned contigBlocks = blockSize / alloc_table.blockSize;
    // We need a head block to store
    Block* headBlock = alloc_table.blocks + i;
      
    for (j = 0; j < contigBlocks; ++j) {
      Block* currBlock = headBlock + j;

      // If its either used or the ID isn't 0, we'll assume its in use.
      if (currBlock->status != BLK_FREE || currBlock->allocID != 0)
	break;
    }

    // We've ensured all the contiguous blocks are found, lets assign them.
    if (j == contigBlocks) {
      // We were able to find an allocation space in the table.
      OSp_AssignBlocks(headBlock, contigBlocks);
        
      freeBlock = headBlock;
      alloc_table.freeBlocks -= contigBlocks;
        
      break;
    }
  }
  
  OS_CRITICAL_END

    return freeBlock->ptr;
}

// Will free all blocks of an ID.
void OSp_FreeBlocks(unsigned id) {
  unsigned i;
  
  for (i = 0; i < HEAP_MAX_BYTES; ++i) {
    Block* currBlock = alloc_table.blocks + i;

    if (currBlock->allocID == id && currBlock->status == BLK_USED) {
      currBlock->status = BLK_FREE;
      currBlock->allocID = 0;
      alloc_table.freeBlocks++;
    }
  }
}

unsigned OS_Free(void* ptr) {
  OS_CRITICAL_BEGIN
  
    unsigned i, offset;
  char* blkPtr = 0;
  char* tempPtr = 0;
  Block* actualBlock = 0;
  
  if (ptr == 0) {
    OSp_SetError(ERR_MEM_ERROR);
    return 0;
  }
  
  offset = ((char *)ptr - alloc_table.data) % alloc_table.blockSize;
  tempPtr = (char *)ptr;
  blkPtr = (char *)(tempPtr - offset);
  
  // blkPtr should be head data ptr for a block.
  for (i = 0; i < HEAP_MAX_BYTES; ++i) {
    Block* currBlock = alloc_table.blocks + i;
    
    // Look for matching pointers
    if (currBlock->ptr == blkPtr) {
      // Found it.
      actualBlock = currBlock;

      break;
    }
  }

  for (i = 0; i < HEAP_MAX_BYTES; ++i) {
    Block* headBlock = alloc_table.blocks + i;

    // Look for used blocks
    if (headBlock->status == BLK_USED 
	&& headBlock->allocID == actualBlock->allocID) {
      // We found the block.
      // Erase all blocks that match the ID.
      OSp_FreeBlocks(headBlock->allocID);
      break;
    }
  }
  
  OS_CRITICAL_END

    return *(unsigned *)ptr;
}

unsigned OS_TaskDelete(unsigned ID) {
  TCB* task = 0;
  unsigned i;
  unsigned slot;
    
  OS_CRITICAL_BEGIN
  
    // Find the task
    for (i = 0; i < MAX_TASKS; ++i) {
      TCB* currTask = Tasks + i;
    
      if (currTask->ID == ID) {
	task = currTask;
	break;
      }
    }
  
  if (task) {
    if (task != OS_TaskRUNNING) {
      TCB* modTask = &Tasks[task->slot];
      
      slot = task->slot;
      TaskSlots[task->slot] = 0;
      modTask->taskState = DELETED;
      
      // Make sure we remove the task from the scheduler.
      OSp_SchedulerRemoveTask(modTask);
      
      // If it's the next task that we removed, we need to reschedule.
      if (modTask == OS_TaskNEXT)
        OSp_UpdateScheduler();

    } else {
      OSp_SetError(ERR_TASK_NO_DELETE);
      return 0;
    }
  } else {
    OSp_SetError(ERR_TASK_INVALID);
    return 0;
  }
  
  NumTasks--;
  
  OS_CRITICAL_END
  
    return slot;
}

unsigned OS_TaskSuspend(unsigned ID) {
  TCB* task = 0;
  unsigned i;
  
  OS_CRITICAL_BEGIN
  
    // Find the task
    for (i = 0; i < MAX_TASKS; ++i) {
      TCB* currTask = Tasks + i;
    
      if (currTask->ID == ID) {
	task = currTask;
	break;
      }
    }
  
  if (task) {
    task->taskState = SUSPENDED;
    
    // If it's the next task that we removed, we need to reschedule.
    if (task == OS_TaskNEXT)
      OSp_UpdateScheduler();
  } else {
    OSp_SetError(ERR_TASK_INVALID);
    return 0;
  }
  
  OS_CRITICAL_END
  
    return 1;
}

unsigned OS_TaskResume(unsigned ID) {
  TCB* task = 0;
  unsigned i;
  
  OS_CRITICAL_BEGIN
  
    // Find the task
    for (i = 0; i < MAX_TASKS; ++i) {
      TCB* currTask = Tasks + i;
    
      if (currTask->ID == ID) {
	task = currTask;
	break;
      }
    }
  
  if (task) {
    if (task->taskState == SUSPENDED)
      task->taskState = READY;
  } else {
    OSp_SetError(ERR_TASK_INVALID);
    return 0;
  }
  
  OS_CRITICAL_END
  
    return 1;
}
