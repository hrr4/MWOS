
/*
		This file starts the kernel and uses two tasks to change the colors
	on the Launchpad's tricolor LED.
*/

/******************************************************************************
*******************************************************************************
	Includes
*******************************************************************************
******************************************************************************/
#include "Kernel.h"
#include "Timer.h"

/******************************************************************************
*******************************************************************************
	Definitions
*******************************************************************************
******************************************************************************/
#define		EVENT_RED			0x01
#define		EVENT_BLUE		0x02
#define		EVENT_GREEN		0x04
#define		NO_ANDs				0x00
#define		NO_ORs				0x00

/******************************************************************************
*******************************************************************************
	Prototypes
*******************************************************************************
******************************************************************************/

void
TaskBlue (void);

void
TaskRed (void);

void
TaskGreen (void);

void TaskStart(void);
void TaskFinish(void);
//void TaskTimer(void);

/******************************************************************************
*******************************************************************************
	Globals
*******************************************************************************
******************************************************************************/

Timer* timResponse;

unsigned int semResponse;

/******************************************************************************
*******************************************************************************
	Main.  Note that `startup_TM4C123.s` is looking for __main
*******************************************************************************
******************************************************************************/
int __main (void) {
	volatile unsigned char i;
	unsigned int numTasks 	= 0x03;
	unsigned int stackSize 	= 0x40;

  // Responsiveness test
  // Create two tasks that share one semaphore.
  // Responsiveness measures how long it takes for a semaphore signaled by one task
  // to wake up another.
  
  if (!OS_InitKernel(numTasks, stackSize))
    while (1) {;}
  
   semResponse = OS_SemCreate(MUTEX, 0x00, 0x01);
/*      
  if (!OS_CreateTask(&TaskTimer, Normal))
    while (1) {;}
  */    
      
  timResponse = Timer_Create();
      
  if (!OS_CreateTask(&TaskStart, 0x00))
    while (1) {;}
      
  if (!OS_CreateTask(&TaskFinish, 0x00))
    while (1) {;}
      
  OS_Start();    
  
  while (1) { 
    i++;
  }
  
  /*
	if (OS_InitKernel(numTasks, stackSize)) {
		if (OS_CreateTask(TaskBlue, 0x00)) {
			OS_Start();
		} // end if 
	} // end if

	while (1) {
		i++;
	} // end while
	*/
} // end main

/******************************************************************************
*******************************************************************************
	Helper Functions
*******************************************************************************
******************************************************************************/
void TaskStart(void) {
  while (1) {
    OS_SemAcquire(semResponse);
    
    Timer_Pause(timResponse);

    OS_SemRelease(semResponse);
    
    Timer_Start(timResponse);
  }
}

void TaskFinish(void) {
  while (1) {
    OS_SemAcquire(semResponse);
    
    Timer_Pause(timResponse);

    OS_SemRelease(semResponse);
    
    Timer_Start(timResponse);
  }
}

/*
void TaskTimer(void) {
  while (1) {
    Timers_Update();
  }
}
*/
/*
void
TaskBlue (void) {
	unsigned int taskRed, taskGreen;
	
	while (0x01) {
		OS_SetLEDs(BLUE);		
		taskRed = OS_CreateTask(TaskRed, 0x00);
		taskGreen = OS_CreateTask(TaskGreen, 0x00);
		
		OS_EventClear(EVENT_RED | EVENT_GREEN);
		OS_EventWait(NO_ORs, EVENT_RED | EVENT_GREEN);
		
		OS_SetLEDs(BLUE);
		OS_TaskSuspend(taskRed);
		OS_EventClear(EVENT_RED | EVENT_GREEN);
		OS_EventWait(EVENT_RED | EVENT_GREEN, NO_ANDs);
		
		OS_SetLEDs(BLUE);
		OS_TaskDelete(taskGreen);
		OS_TaskResume(taskRed);
		OS_EventClear(EVENT_RED | EVENT_GREEN);
		OS_EventWait(EVENT_RED | EVENT_GREEN, NO_ANDs);		
		OS_TaskDelete(taskRed);
	} // end while
} // end TaskBlue

void
TaskRed (void) {
	
	while (0x01) {
		OS_SetLEDs(RED);
		OS_EventSet(EVENT_RED);
	} // end while	
} // end TaskRed

void
TaskGreen (void) {
	
	while (0x01) {
		OS_SetLEDs(GREEN);
		OS_EventSet(EVENT_GREEN);
	} // end while
} // end TaskGreen
*/
