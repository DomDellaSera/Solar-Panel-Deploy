/*******************************************************************************

*
*******************************************************************************/

#include <p33FJ16GS504.h> 
#include "PVInverter_defines.h"				// Include #defines 
#include "PVInverter_Variable.h"			// Include Variables 
	
void LoadBalance(void);
void MPPTRoutine(void);

extern byte bAddress;
extern unsigned int wAntiIslandingTimer;
unsigned long lowVoltCounter = 0;		

byte bDir = INCREMENT;	
//word wTestCounter;		

void LoadBalance(void) 
{
	// Difference of the two Mosfet currents

	loadBalError =   0 - diffPVCurrent;

	// Error * Proportional Gain
	loadBalPoutput = ( (__builtin_mulss(loadBalError,(int)KA_Q15)) >> 15);	
	loadBalIoutputTempLong = loadBalIoutputTempLong + ( (__builtin_mulss(loadBalError,(int)KSA_Q15)) >> 15);
	
	// Check for Integral term exceeding MAX_BALANCE, If true, saturate the integral term to MAX_BALANCE
	// Check for Integral term going below -MAX_BALANCE, If true, saturate the integral term to -MAX_BALANCE
	if(loadBalIoutputTempLong > MAX_BALANCE)
	{
		loadBalIoutputTempLong = MAX_BALANCE;
	}
	else if(loadBalIoutputTempLong < -MAX_BALANCE)
	{
		loadBalIoutputTempLong = -MAX_BALANCE;
	}

	loadBalIoutput = (int)loadBalIoutputTempLong;

	// PI Output = Proportional Term + Integral Term
	loadBalPIoutputTempLong = (long) loadBalPoutput + (long) loadBalIoutput;
	
	// Check for PI Output exceeding MAX_BALANCE, If true, saturate PI Output to MAX_BALANCE 
	// Check for PI Output going below -MAX_BALANCE, If true, saturate PI Output to -MAX_BALANCE
	if(loadBalPIoutputTempLong > MAX_BALANCE)
	{
		loadBalPIoutputTempLong = MAX_BALANCE;
	}
	else if(loadBalPIoutputTempLong < -MAX_BALANCE)
	{
		loadBalPIoutputTempLong = -MAX_BALANCE;
	}
	
	loadBalPIoutput = (int)loadBalPIoutputTempLong;

	// Delta to correct the two duty cycles

	deltaDuty = ( (__builtin_mulss((int)loadBalPIoutput,(int)FLYBACK_PERIOD_VALUE)) >> 15);
}
