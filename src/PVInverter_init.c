/*******************************************************************************
*
*******************************************************************************/


#include <p33FJ16GS504.h>  
#include "PVInverter_defines.h"

void initClock(void)
{
  	// Configure Oscillator to operate the device at 40 MIPS
	//   Fosc = Fin*M/(N1*N2), Fcy = Fosc/2
 	//   Fosc = 7.37*(43)/(2*2) = 80MHz for Fosc, Fcy = 40MHz

	// Configure PLL prescaler, PLL postscaler, PLL divisor
	PLLFBD = 41; 			                // M = PLLFBD + 2
	CLKDIVbits.PLLPOST = 0;                 // N1 = 2 
	CLKDIVbits.PLLPRE = 0;                  // N2 = 2

    // Change oscillator to FRC + PLL 
    __builtin_write_OSCCONH(0x01);			// New Oscillator FRC w/ PLL
    __builtin_write_OSCCONL(0x01);  		// Enable Clock Switch
    
    while(OSCCONbits.COSC != 0b001) 		// Wait for new Oscillator to become FRC w/ PLL    
		{
			ClrWdt();
		}
		while(OSCCONbits.LOCK != 1)					// Wait for Pll to Lock 
		{
			ClrWdt();
		}

	// Now setup the ADC and PWM clock for 120MHz
	//   ((FRC * 16) / APSTSCLR ) = (7.37 * 16) / 1 = ~ 120MHz
	
	ACLKCONbits.FRCSEL = 1;					// FRC provides input for Auxiliary PLL (x16)
	ACLKCONbits.SELACLK = 1;				// Auxiliary Ocillator provides clock source for PWM & ADC
	ACLKCONbits.APSTSCLR = 7;				// Divide Auxiliary clock by 1 
	ACLKCONbits.ENAPLL = 1;					// Enable Auxiliary PLL 

	while(ACLKCONbits.APLLCK != 1)			// Wait for Aux. PLL to Lock
	{
		ClrWdt();
	}
    
}


void initPWM(void)
{
    // PWM setup for Interleaved Flyback Inverter primary section
	
	PTPER = FLYBACK_PERIOD_VALUE;			// Flyback inverter period value 
    
	PHASE2 = FLYBACK_INTERLEAVED_PHASE;		// 180Deg phase-shift for second converter 
    
	PTCON2bits.PCLKDIV = 0;					// Maximum (1.04nsec) timing resolution  

	PWMCON1bits.ITB = 0;					// PTPER register provides the timing for PWM1, PWM2 generator 
	PWMCON2bits.ITB = 0;					

	PWMCON1bits.IUE = 1;					// Enable immediate updates
	PWMCON2bits.IUE = 1;
	
	PWMCON1bits.DTC = 0;                    // Positive dead-time enabled 
	PWMCON1bits.DTC = 0;                    
  
	// Workaround for Errata #32
 	IOCON1bits.PENH = 0;                    // GPIO controls PWM pins @ startup
  	IOCON1bits.PENL = 0;                    
   	IOCON2bits.PENH = 0;                    
   	IOCON2bits.PENL = 0;                    

    IOCON1bits.PMOD = 0;                    // PWM1H and PWM1L is in complementary output mode 
    IOCON2bits.PMOD = 0;                    // PWM2H and PWM2L is in complementary output mode 
	
    IOCON1bits.OVRDAT = 0;                  // Inverter IGBTs will be turned OFF during override
    IOCON1bits.OVRENH = 1;                  // Keep Inverter IGBTs OFF during SYSTEM_STARTUP
    IOCON1bits.OVRENL = 1;                  // Keep Inverter IGBTs OFF during SYSTEM_STARTUP
    IOCON2bits.OVRDAT = 0;                  // Inverter IGBTs will be turned OFF during override
    IOCON2bits.OVRENH = 1;                  // Keep Inverter IGBTs OFF during SYSTEM_STARTUP
    IOCON2bits.OVRENL = 1;                  // Keep Inverter IGBTs OFF during SYSTEM_STARTUP

    ALTDTR1 = FLYBACK_ALTDTR_VALUE;			// Setup dead-time
	DTR1 = FLYBACK_DTR_VALUE;
    ALTDTR2 = FLYBACK_ALTDTR_VALUE;
	DTR2 = FLYBACK_DTR_VALUE;
                                                                                                                          
    PDC1 = 100;								// Initialize Flyback duty cycle 
	PDC2 = 100;
                                                                                                                                            
   	TRGCON2bits.TRGDIV = 1;					// Trigger every 2nd PWM Period ~17us             
	TRGCON2bits.TRGSTRT = 0;                // Wait 0 PWM cycles before generating first PWM trigger

    TRIG1 = 1000;							// PWM1 used to trigger ADCP0
	TRIG2 = 1000;


	// PWM3 Configuration for Full-Bridge Drive
	TRISCbits.TRISC0 = 0;				// OPTO_DRV1 is an output
	TRISCbits.TRISC13 = 0;				// OPTO_DRV2

	PWMCON3bits.ITB = 1;					// Independent Time Base

	PHASE3 = FULLBRIDGEPERIOD;				// 2x the switching frequency of the flyback
	SPHASE3 = FULLBRIDGEPERIOD;
	
	PDC3 = PHASE3 >> 1;						// 50% duty cycle
	SDC3 = SPHASE3 >> 1;

	PWMCON3bits.DTC = 3;					// Dead-time disabled

	IOCON3bits.PMOD = 1;					// Redundant Output Mode

	IOCON3bits.PENH = 1;     				// PWM controls I/O pins              
	IOCON3bits.PENL = 1;

	IOCON3bits.OVRDAT = 0;                  // Full-Bridge override data (Safe State)  
	IOCON3bits.OVRENH = 1;                  // Enable override (Disable MOSFETs)
	IOCON3bits.OVRENL = 1;

	// PWM3 Fault/Current Limit Setup
	FCLCON3bits.FLTMOD = 3; 				// Disable Faults


#if 0
	// Initialize PWM3 for SCR bridge

	PWMCON3bits.ITB = 0;					// MASTER time base for PWM3 
	PWMCON3bits.IUE = 1;					// Enable immediate updates

	IOCON3bits.PENH = 0;					// GPIO controls the PWM3H pin @ startup
	IOCON3bits.PENL = 0;					

	IOCON3bits.PMOD = 3;					// True independent output Mode 		
	IOCON3bits.OVRDAT = 0;					// SCR will be turned off at start-up
	IOCON3bits.OVRENH = 1;					// Enable the over ride at startup 
	IOCON3bits.OVRENL = 1;					// Enable the over ride at startup 

	PDC3 = 0;
	SDC3 = 0;
#endif
}

void initADC(void)
{
	// The port pins that are to function as analog inputs must have their
	//  corresponding TRIS bit set (input)
	TRISAbits.TRISA0 = 1;
	TRISAbits.TRISA1 = 1;
	TRISAbits.TRISA2 = 1;
	TRISBbits.TRISB0 = 1;

  ADCONbits.SLOWCLK = 1;          // Requirement from ADC Errata
  ADCONbits.FORM = 0;             // Output in Integer Format 
	ADCONbits.EIE = 0;              // Disable Early Interrupt 
	ADCONbits.ORDER = 0;            // Even channel first 
	ADCONbits.SEQSAMP = 0;          // Simultaneus Sampling Enabled 
	ADCONbits.ASYNCSAMP = 1;				// Asynchronous Sampling Enabled 
	ADCONbits.ADCS = 5;             // Clock Divider is set up for Fadc/6, TAD = 50ns
																	// For simultaneus sampling total conversion time for one pair is 0.7us
	
	ADSTAT = 0;                     // Clear the ADSTAT register 
	ADPCFG = 0xF700;                // AN0 - AN7, AN11 are analog inputs 
   
  ADPCFGbits.PCFG0 = 0;           // AN0 is Motor 1 Current Input
  ADPCFGbits.PCFG1 = 0;           // AN1 is Motor 2 Current Input
	ADPCFGbits.PCFG2 = 0;						// AN2 is Motor 3 Current Input
  ADPCFGbits.PCFG3 = 0;           // AN3 is Motor 4 Current Input
	  
  ADCPC0bits.TRGSRC0 = 0x1f;		  // AN0 and AN1 triggered by Timer 2 Period Match 
	ADCPC0bits.TRGSRC1 = 0x1f;		  // AN2 and AN3 triggered by Timer 2 Period Match 
	
	IPC27bits.ADCP0IP = 5;          // Set ADC Pair0 Interrupt Priority 
	IFS6bits.ADCP0IF = 0;	    			// Clear ADC Pair0 Interrupt Flag 
	IEC6bits.ADCP0IE = 1;           // Enable ADC Pair0 Interrupt at start 
}


void initIOports(void)
{
	// Axis 1 Motors
	TRISBbits.TRISB6 = 0;			// Motor 1 Relay Control: set as output
	TRISBbits.TRISB5 = 0;			// Motor 1 Relay Control: set as output
	TRISBbits.TRISB8 = 0;			// Motor 2 Relay Control: set as output	
	TRISBbits.TRISB15 = 0;		// Motor 2 Relay Control: set as output

	// Axis 2 Motors
	TRISBbits.TRISB2 = 0;			// Motor 3 Relay Control: set as output
	TRISCbits.TRISC2 = 0;			// Motor 3 Relay Control: set as output
	TRISCbits.TRISC7 = 0;			// Motor 4 Relay Control: set as output	
	TRISCbits.TRISC8 = 0;			// Motor 4 Relay Control: set as output
	
	TRISCbits.TRISC12 = 0;		// TestPin1: set as output				

  initStateMachineTimer();	// Initialize Timer 2 for MPPT Execution Rate

// GPIO setup as an output for tri color LED.  Motor Axis 1, Panel 1
	TRISBbits.TRISB11 = 0;				// red		
	TRISBbits.TRISB12 = 0;				// blue
	TRISBbits.TRISB13 = 0;				// green

// GPIO setup as an output for tri color LED.  Motor Axis 2, Panel 2
	TRISAbits.TRISA3 = 0;					// red		
	TRISAbits.TRISA4 = 0;					// blue
	TRISBbits.TRISB14 = 0;				// green


#if 0
	TRISCbits.TRISC7 = 0;			// Configure I/O for output for IMI test output 
	LATCbits.LATC7 = 0; 			// IMI Test output pin. 0=disable fault.  1=enable fault

	TRISAbits.TRISA4 = 0;			// GPIO setup for override of PWM1
	TRISAbits.TRISA3 = 0;
	LATAbits.LATA4 = 0;
	LATAbits.LATA3 = 0;

	
	LATBbits.LATB11 = 0;			
	LATBbits.LATB12 = 0; 
	
	TRISBbits.TRISB8 = 0;				// Pin 41, RB8 for LED, this will change to a 16ma pin

	TRISCbits.TRISC3 = 0;				// Test Pin
	TRISCbits.TRISC11 = 0;				// Test Pin2
#endif
}

void IniDAC(void)
{
	CMPCON1bits.CMPON = 1;					// enable comparator module in order to enable dac
	CMPCON1bits.DACOE = 1;
	CMPCON1bits.EXTREF = 0;
	CMPCON1bits.RANGE =1;			// AVdd/2	
	CMPDAC1bits.CMREF = 500;
}


/***** 102uS measured *****/
void initStateMachineTimer(void)
{
	T2CONbits.TCKPS = 0;			// Prescaler of 1:1 
	PR2 = 4000;								// (100us /25ns) = 4000 
	IPC1bits.T2IP = 4;				// Set up Timer interrupt priority 
	IEC0bits.T2IE = 1;				// Enable Timer2 interrupt 
}

