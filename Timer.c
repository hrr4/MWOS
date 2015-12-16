#include "Timer.h"

void Timers_Init(void) {
  gNumTimers = 0;
}

Timer* Timer_Create(void) {
  Timer* t = gTimers + gNumTimers++;
  
  // Ensure the timer is initialized
  Timer_Reset(t);
  
  return t;
}

void Timers_Update(void) {
  volatile unsigned i;
  
  for (i = 0; i < gNumTimers; ++i) {
    Timer* t = gTimers + i;
    
    if (t->status == Go) {
      unsigned oldCount = t->count;
      t->count++;
      
      if (oldCount > t->count)
        t->rollOvers++;
    }
  }
}

void Timer_Reset(Timer* t) {
  t->count     = 0;
  t->rollOvers = 0;
  t->status    = Stop;
}

void Timer_Start(Timer* t) {
  t->status = Go;
}

void Timer_Pause(Timer* t) {
  t->status = Stop;
}

void Timer_Resume(Timer* t) {
  t->status = Go;
}
