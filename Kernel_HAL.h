#ifndef __KERNEL_HAL_H__
#define __KERNEL_HAL_H__

// Disable global interrupt enable
#define OS_CRITICAL_BEGIN __asm { cpsid i }

// Enable global interrupt enable
#define OS_CRITICAL_END __asm { cpsie i }

#define RED   1 // LED Red
#define BLUE  2 // LED BLUE
#define GREEN 3 // LED Green

/*******************************
 * OS_SetLEDs
 ** An abstraction that allows LEDs to be set in a non-platform dependent way.
 *
 * Parameters:
 ** unsigned char
 *** Value of LED user wishes to enable. 
 *** Use provided macros for this value.
 *
 * Return Value:
 ** unsigned
 *** 0 = error
 *** 1 = success
 ******************************/
unsigned OS_SetLEDs(unsigned char);

/*******************************
 * OS_Start
 ** An abstraction that starts the kernel's core operation.
 *
 * Parameters:
 ** void
 *
 * Return Value:
 ** unsigned
 *** 0 = error
 *** 1 = success.
 ******************************/
unsigned OS_Start(void);

/*******************************
 * TIMER0A_Handler
 ** An abstraction for an interrupt handler that provides platform-dependent
 **  dispatching which is used by the kernel to perform context switching.
 ** Even though this is public, do not call this manually.
 *
 * Parameters:
 ** void
 *
 * Return Value:
 ** unsigned
 *** 0 = error
 *** 1 = success.
 ******************************/
void TIMER0A_Handler(void);

/*******************************
 * SystemInit
 ** An abstraction that initializes key systems on a given platform.
 ** Even though this is public, do not call this manually.
 *
 * Parameters:
 ** void
 *
 * Return Value:
 ** void
 ******************************/
void SystemInit(void);

#endif
