#include "Kernel_HAL.h"
#include "TM4C123GH6PM.h"

extern void OSp_IdleTask(void);
extern void OSp_ScheduleTask(void);

// Prototypes
/*******************************
 * OSp_InitGPIOF
 ** Initializes platform-dependent GPIO Port F.
 *
 * Parameters:
 ** void
 *
 * Return Value:
 ** void
 ******************************/
void OSp_InitGPIOF(void);

/*******************************
 * OSp_InitTimers
 ** Initializes platform-dependent timers.
 *
 * Parameters:
 ** void
 *
 * Return Value:
 ** void
 ******************************/
void OSp_InitTimers(void);

#define BON(X)           |=(X) // Set a bit on
#define BOFF(X)          &=~(X) // Set a bit off
#define BTOG(X)          ^=(X) // Toggle a bit

#define MAX_WAIT         0xFFFF; // Maximum wait value.

// This first section is related to setting the system clock
#define	OSCSRC           0x00000030  // Enable the low-frequency oscillator
#define	XTAL             0x000007C0  // Sets the crystal value 
#define	MHz_16           0x00000540  // 16 MHz
#define	USE_RCC2         0x80000000  // Use RCC2 instead of RCC
#define	DIV400           0x40000000  // Divide by 400MHz PLL
// Specifies which Divisor generates the system clock.
#define	SYSDIV           0x1FC00000  // Specifies divisor to set system clock
#define	DIV_5            0x01000000  // Sets a divisor of 5
#define	BYPASS2          0x00000800  // PLL bypass for source 2
#define	OSCSRC2          0x00000070  // Sets the Oscillator source 2
#define	MOSCDIS          0x00000001  // Disables the main oscillator.
// This section relates to using PORTF
#define	GPIO_PORTF       0x00000020  // PORTF base

#define	PIN_1            0x00000002  // 1st PORTF pin
#define	PIN_2            0x00000004  // 2nd PORTF pin
#define	PIN_3            0x00000008  // 3rd PORTF pin

// This section covers Timer operations
#define	TAEN             0x00000001  // Timer A enable
#define	TAMODE           0x00000003  // Timer A mode
#define	PERIODIC         0x00000002  // Periodic define to set Timer mode
#define	TAMIM            0x00000010  // Timer A Match Interrupt mask
#define	TIMER0_IRQ_BIT   0x00080000  // Timer 0 IRQ bit
#define	TIMER_LD_VALA    0x00DFFB40  // Value to start counting down from.

#define	TIMER_VAL_SET(X) X->TAILR = TIMER_LD_VALA

// Public

unsigned OS_SetLEDs(unsigned char _value) {
  switch (_value) {
  case 1:
    GPIOF->DATA BOFF(PIN_2 | PIN_3);
    GPIOF->DATA BON(PIN_1);
    break;
    
  case 2:
    GPIOF->DATA BOFF(PIN_1 | PIN_3);
    GPIOF->DATA BON(PIN_2);
    break;
		
  case 3:
    GPIOF->DATA BOFF(PIN_1 | PIN_2);
    GPIOF->DATA BON(PIN_3);
		
  default:
    GPIOF->DATA BON(RED);
    GPIOF->DATA BON(BLUE);

    // If an incorrect value was passed, return as an error.
    return 0;
  }
  return 1;
}

__asm unsigned OS_Start(void) {
  IMPORT OS_TaskRUNNING
    // This code is based on code from Jonathan Valvano's book,
    // Real Time Operating Systems,  Pg. 177
    LDR R0, =OS_TaskRUNNING // Store address of current task into r0
    LDR R2, [R0]        	  // R2 = OS_TaskRUNNING
    LDR SP, [R2] 	          // Store OS_TaskRUNNING->sp into the SP
	
    MOV R3, #1
    STRB R3, [R2, #8]
	
    POP {R4-R11} 	          // Restore registers r4-r11
  POP {R0-R3} 	          // Restore registers r0-r3
  POP {R12} 	          // Restore r12
  POP {LR}                // Discard LR from initial stack
  POP {LR} 	          // Start Location
  POP {R1}          	  // Discard PSR
  ALIGN 4
    MOV R0, #1              // Show we exited correctly
    CPSIE I 	          // Enable global interrupts
    BX  LR 	          // Start the first task (Branch/call address in LR)
    }

__asm void TIMER0A_Handler(void) {
  // This code was found in Jonathan Valvano's book,
  // Real Time Operating Systems, Pg. 176
  CPSID 	I               // Disable global interrupts
    PUSH	{R4-R11}        // Save Registers R4-R11
  LDR	R1, =0x40030000	// Load TIMER0 base address into R1
	
    // Clear interrupt bit
    LDR	R2, =0x40030024
    LDR    	R3, [R2]
    ORR	R4, R3, #1
    STR    	R4, [R2]
	
    ALIGN 4
	
    // Reload TAILR with correct value (TIMER0)
    LDR    	R0, =TIMER_LD_VALA // Move VALA value to R0
    STR   	R0, [R1, #28]	   // Store value in R0 to R1+0x28 offset (TAILR)
	
    PRESERVE8		   // Guarantee we preserve 8-byte alignment
    IMPORT 	OSp_ScheduleTask
    LDR   	R0, =OSp_ScheduleTask
	
    PUSH	{LR}
  BLX   	R0
    POP	{LR}

  CBZ     R0, SKIP    
	
    LDR    	R0, =OS_TaskRUNNING	// Store address of current task into r0
    LDR    	R1, [R0]        		// R1 = OS_TaskRUNNING
    STR	    SP, [R1]	       		// Store current Stack Pointer into TCB

    IMPORT 	OS_TaskNEXT
    LDR	    R2, =OS_TaskNEXT		// Load address OS_TaskNEXT into R2
    LDR	    R3, [R2]        		// Load value at OS_TaskNEXT into R3

    STR	 R3, [R0]        		// Load OS_TaskNEXT into RUNNING

    LDR	SP, [R3]

    SKIP 
    ALIGN 4
    POP	{R4-R11}
	
  CPSIE	I
    BX	LR
    }

void SystemInit(void) {
  volatile unsigned int wait = MAX_WAIT;

  // Define the primary osc. as 16MHz
  SYSCTL->RCC BOFF(XTAL);
  SYSCTL->RCC BON(MHz_16);

  // Enable primary osc.
  SYSCTL->RCC BOFF(MOSCDIS);

  // Toggle the ability of RCC2 to override RCC1 (required to get 80MHz)
  SYSCTL->RCC2 BON(USE_RCC2);

  // Use the system divisor on the 400MHz PLL source
  SYSCTL->RCC2 BON(DIV400);

  // Enable dividing PLL source by 5 (result: 80MHz)
  SYSCTL->RCC2 BOFF(SYSDIV);
  SYSCTL->RCC2 BON(DIV_5);

  // Bypass primary OSC source (enables PLL)
  SYSCTL->RCC2 BOFF(BYPASS2);

  // Primary Osc. is external source
  SYSCTL->RCC2 BOFF(OSCSRC2);

  while (wait) {
    wait--;
  }
	
  // Enable the interrupt for Timer 0
  NVIC->ISER[0]	= TIMER0_IRQ_BIT;
}


// Privates

void OSp_InitGPIOF(void) {
  volatile unsigned int wait = MAX_WAIT;
	
  // Enable system clock to Port F peripheral
  SYSCTL->RCGCGPIO BON(GPIO_PORTF);

  // Allow clock to stabilize
  wait = MAX_WAIT;
  while (wait) {
    wait--;
  } // end while	
  
  // Set the Port F pins to outputs (for lights)
  GPIOF->DIR BON(PIN_1 | PIN_2 | PIN_3);
  
  // Enable digital I/O function on the Port F pins
  GPIOF->DEN BON(PIN_1 | PIN_2 | PIN_3);
  
  // Disable analog & other alternate functions on PORTF
  GPIOF->AMSEL = 0x00;
  GPIOF->AFSEL = 0x00;

  // Turn off pins
  GPIOF->DATA    &= ~0x15; // ~(1111)
}


void OSp_InitTimers(void) {
  volatile unsigned int wait = MAX_WAIT;
	
  // Provide clock source to timer module
  SYSCTL->RCGCTIMER BON(TAEN);

  // Allow clock to stabilize
  wait = MAX_WAIT;
  while (wait) {
    wait--;
  } // end while	

  // Make sure timer is off before changing settings
  TIMER0->CTL = 0x00;
	
  // Reset configuration
  TIMER0->CFG = 0x00;
	
  // Set timer mode (one-shot, periodic, etc.)
  TIMER0->TAMR = PERIODIC;
	
  // Set starting timer value (timer counts down from this)
  TIMER_VAL_SET(TIMER0);
	
  // Allow Timer0 interrupts (NOTE: The end of the timer sequence is a 
  //                                `timeout`, not a `rollover`)
  TIMER0->IMR = 0x1;
	
  // Enable timer and start counting
  TIMER0->CTL BON(TAEN);
  SYSCTL->RCGCTIMER BON(TAEN);

  wait = MAX_WAIT;
  while (wait) {
    --wait;
  }
}
