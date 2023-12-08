/*******************************************************************************
*
*******************************************************************************/

#include <p33FJ16GS504.h> 
#include "PVInverter_defines.h"				// Include #defines 
#include "PVInverter_Variable.h"			// Include Variables 
#include "SineTable512.h"	
#include "Motor.h"

#define BLOCK_START												14000
#define BLOCK_END													16000

#ifdef WAVE_ANALYSIS

#define STABILIZER_TRIGGER_WINDOW_START 	9500
#define STABILIZER_TRIGGER_WINDOW_END 		9600
#define STABILIZER_END_WINDOW_START 			9300			// used when #define SUPPRESS
#define STABILIZER_END_WINDOW_END 				9400			// used when #define SUPPRESS
#define CURRENT_ERROR_LIMIT								270


/***** Test Vars ********/
int iTest[MAX_TEST_ARRAY];
unsigned char bIndex;
unsigned char bStore = FALSE;
unsigned int wCounterCapture;
#endif

/***** Anti Islanding Variables *****/
unsigned int wAntiIslandingTimer = TIME_TO_ANTI_ISLAND;		// let's start anti island scheme 1 second or so latter
unsigned char bAntiIslandZCCounter = 0;										// how many zero crossings i am going to shift frequency
unsigned char bAntiIslandTriggerSet = ANTI_ISLAND_END;
unsigned int wgridPeriodAltered = 100;
unsigned int wIslandTemp;


extern unsigned int wCounter17_8uS;
extern int iTotalOutput;
extern word wCurrFilter[];

/**** This interrupt is happening every 102uS triggered by Timer 2 ISR ****/
/**** 32 point filter will get filled up in 3.2ms at 100uS rate        ****/
void __attribute__((interrupt, no_auto_psv)) _ADCP0Interrupt()
{	

	ClrWdt();

	MotorCurrent1 = ADCBUF0;
	MotorCurrent2 = ADCBUF1;
	MotorCurrent3 = ADCBUF2;
	MotorCurrent4 = ADCBUF3;

	wNewADCdata = TRUE;

	TestPin1 ^= 1;

	ADSTATbits.P0RDY = 0;                   /* Clear ADC pair ready bit */
	IFS6bits.ADCP0IF = 0;                   /* Clear ADC Interrupt Flag */
}


// Timer 2 Interrupt: 102 uS measured
void __attribute__((__interrupt__, no_auto_psv)) _T2Interrupt()
{

	if (miscCounter)
		miscCounter--;

	if (wBalanceCounter)
		wBalanceCounter--;


  TMR2 = 0;
  IFS0bits.T2IF = 0;   
}
