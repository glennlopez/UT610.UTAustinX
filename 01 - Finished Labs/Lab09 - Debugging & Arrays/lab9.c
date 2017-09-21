// ***** 0. Documentation Section *****
// main.c for Lab 9
// Runs on LM4F120/TM4C123
// In this lab we are learning functional debugging by dumping
// recorded I/O data into a buffer
// github.com/glennlopez

// ***** 1. Pre-processor Directives Section *****
#include "TExaS.h"
#include "tm4c123gh6pm.h"

// ***** 2. Global Declarations Section *****

// FUNCTION PROTOTYPES: Each subroutine defined
void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts

// ***** 3. Subroutines Section *****

// PortF Bit-specific Address definitions (7|200, 6|100, 5|80, 4|40, 3|20, 2|10, 1|08, 0|04) expressed as 4*2^b (bitspecific addressing)
#define SW1											(*((volatile unsigned long *)0x40025040))			//PF4 - offset 0x040 (0000 0100 0000) | 64
#define SW2											(*((volatile unsigned long *)0x40025004))			//PF0 - offset 0x004 (0000 0000 0100) | 4
	
// Prototypes
void PortF_Init(void);
void SysTick_Init(void);
void Delay(void);

// Global vars
unsigned long Led;
unsigned long Time[50];		// first data point is wrong, the other 49 will be correct
unsigned long Data[50];		// you must leave the Data array defined exactly as it is

 
int main(void){  unsigned long i,last,now;
	
	/*********************************************************
		[x] Make LED flash at 10Hz
		[x] Make LED only flash if SW1 or SW2 is pressed
		[x] Record PF4,1,0 when input or output changes
	*********************************************************/

  TExaS_Init(SW_PIN_PF40, LED_PIN_PF1); 			 // activate grader and set system clock to 16 MHz
  PortF_Init();   														 // initialize PF1 to output
  SysTick_Init();															 // initialize SysTick, runs at 16 MHz
	
  i = 0;          														 // array index
  last = NVIC_ST_CURRENT_R;
  EnableInterrupts();          								 // enable interrupts for the grader
	
  while(1){
		
		//Toggle only when sw1 or sw2 is pressed (sw1/2 is active low)
		if((SW2 == 0x00) ||(SW1 == 0x00)){
			Led = GPIO_PORTF_DATA_R;   // read previous
			Led = Led^0x02;            // toggle red LED
			GPIO_PORTF_DATA_R = Led;   // output 
		

			/**************************************************
			PF4 PF1 PF0: Needs to be colected in a dumpfile
				0 0 0 1  0 0 1 1 = 0x13
			
				GPIO_PORTF_DATA_R				=		x x x x  x x x x
				0x13										=		0 0 0 1  0 0 1 1 
				Mask off other bits 		&		----------------	
																		0 0 0 x  0 0 x x
		
			Answer: Data = GPIO_PORTF_DATA_R & 0x13;
			****************************************************/
			
			if(i<50){
				now = NVIC_ST_CURRENT_R;
				Time[i] = (last-now)&0x00FFFFFF;  // 24-bit time difference
				Data[i] = GPIO_PORTF_DATA_R&0x13; // record PF4 PF1 PF0
				last = now;
				i++;
			}
			
			Delay();
		}
		
  }
	
}


// PortF init
void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) activate clock for Port F
  delay = SYSCTL_RCGC2_R;           // allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 in, PF3-1 out
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0
  GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital I/O on PF4-0
}

// Initialize SysTick with busy wait running at bus clock.
void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_ST_RELOAD_R = 0x00FFFFFF;        // maximum reload value
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it             
  NVIC_ST_CTRL_R = 0x00000005;          // enable SysTick with core clock
}

// Delay
void Delay(void){unsigned long volatile time;
  time = 75000; 
  while(time){
   time--;
  }
}
