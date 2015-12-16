#ifndef TIMER__H__
#define TIMER__H__

#define NUM_TIMERS 10

typedef enum {
  Go,
  Stop
} timerStatus_t;

typedef struct Timer {
  unsigned rollOvers;
  unsigned count;
  timerStatus_t status;
} Timer;

static Timer gTimers[NUM_TIMERS];
static unsigned gNumTimers;

// Global Timers Stuff
void Timers_Init(void);
Timer* Timer_Create(void);
void Timers_Update(void);

// Specific Timer stuff
void Timer_Start(Timer* t);
void Timer_Pause(Timer* t);
void Timer_Resume(Timer* t);
void Timer_Reset(Timer* t);

#endif
