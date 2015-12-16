# MWOS
A small ARM-M RTOS.

## Supports
Kernel Preemption, Semaphores, Mutexes, Events, Dynamic Memory

## Theory of Operation
Beginning from the topmost level, the user should view the kernel as a set of system calls one could perform tasks through. This interface is designed to be as minimal yet straightforward as possible. If a user wishes to create a task, they must provide a priority level the task should run at. This informs the kernel how to prioritize the task against others.

### Kernel
The kernel is in charge of all task management and controls the actual context switching of tasks. It itself, is split into two major components, the Hardware Abstraction Layer (HAL) and the Core. It is pre-emptive  

#### HAL
The HAL is designed to ensure hardware independence and abstract away anything that might be platform dependent. Within the HAL, resides timer functionality, I/O interaction routines, and the dispatcher which aids the core in performing the context switch when necessary.

## Scheduling
**Multi-Level Feedback Queue**

For each priority within the kernel, a queue is created for it. Once a task is added to a priority queue, it will be susceptible to aging. If a task runs too little or too often, it will age negatively or positively, respectively. Once the age has reached a given threshold, it will be moved into a lower or higher priority queue. This process will continue until the task is able to run, up which it will be reset to its original priority and the cycle will repeat.
