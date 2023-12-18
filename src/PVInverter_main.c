/*******************************************************************************
* © 2011 Microchip Technology Inc.
*

*
*
*******************************************************************************/


#include <p33FJ16GS504.h>  

#include "PVInverter_Variable.h"			// Include Global Variables		
#include "PVInverter_defines.h"		
#include "uart.h"
#include "Motor.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define HEXA										0
#define DECIMAL									1
#define ASCII										2
#define ASCII_1									3

void Int2Hex(unsigned int);
void Int2Dec(unsigned int);
void Convert2ASCII(unsigned char temp, unsigned char index);
void SendUART(unsigned int,unsigned char);
void Delay_17_8uS_Times(unsigned int);
void NextLine(void);
void Space(void);

// Configuration bits
_FOSCSEL(FNOSC_FRC)
_FOSC(FCKSM_CSECMD & OSCIOFNC_ON & IOL1WAY_OFF)
_FPOR(FPWRT_PWR4)																	// Power On Reset = 4 ms, a non zero enables LPRC Osc
_FICD(ICS_PGD2 & JTAGEN_OFF)
	//------------------------------
	//        FWDT
	// Watchdog timer Configuration Register 
	// FWDTEN_ON  timer is always enabled. 
	// WINDIS_OFF - WDOG window is off 
	// WDTPRE_PR128  watchdog prescaler to 1:128
	// WDTPOST_PS1024 .. Watchdog postscalar
	// WDTPRE x WDTPOST x 1/32768 = Watchdog Timeout
	//-------------------------------
// hit dog inside _ADCP0Interrupt:             
//_FWDT(FWDTEN_ON & WINDIS_OFF & WDTPRE_PR32 & WDTPOST_PS2);  	// watchdog timer = 2 Secs  (WDTPRE_PR128 & WDTPOST_PS512)
_FWDT(FWDTEN_OFF);																						// 2ms = (WDTPRE_PR32 & WDTPOST_PS2)


unsigned char readBuf[5];
unsigned int wCounter17_8uS;
extern byte bTransmit[MSG_LEN_STM32];
extern byte bReceiveBuf[];  	// Receive Buffer Solar Deploy
extern byte bAddress;
word wDelay;

const dword dwCheckSumCompare __attribute__ ((address(0x2bf0))) = 0x34b62cf7; 

#ifdef DC_GROUND_FAULT
float fPVVoltCompareLO,fPVVoltCompareHI;
word wDCGroundVar,wPVVoltTemp;
word wDCGNDFaultLO;
word wDCGNDFaultHI;
#endif

/***** RMS Current and Voltage Measurement Vars *****/
dword dwCurrSumSquares;
dword dwVoltSumSquares;
word wCurrFilter[MAX_NUM_FILTER_POINTS];
//word wTemperatureFilter[MAX_NUM_TEMP_FILTER_POINTS] = {512,512,512,512,512,512,512,512,};
float fCurr;
float fVolt;
float fTemp;
dword dwTemp;
byte bCycleCounter = 16;
byte bRescueCounter = 40;			// apprx 7 second delay on software loop

#ifdef WAVE_ANALYSIS
extern unsigned char bStore;
extern unsigned int wCounterCapture;
extern int iTest[];
#endif

int main(void)
{
	wRCON = RCON;														// RCON gets reset inside case SYSTEM_STARTUP: 
  initClock();                            // Initialize Primary and Auxiliary Oscillators
	initIOports();										// Initialize all I/O Ports
	Configure_UART();
	initADC();
  ADCONbits.ADON = 1;				        // Enable the ADC Module early for ADC Settling Time
	T2CONbits.TON = 1;                // Enable Timer 2

	AcquireMotor_1_GND();							// this function runs for a few secs to acquire Vref

	ColorOffPanel_1();
	ColorOffPanel_2();

	wWhatPanel = NO_PANEL;

	TurnOffBothAxis_1_Motors();
	TurnOffBothAxis_2_Motors();

	while(1)
	{
		if (bReceiveBuf[INDEX_CMD_STM32] != 0)          // this if is necessary otherwise keyboards inputs not 
		{												// processed correctly. Don't know why
			switch (bReceiveBuf[INDEX_CMD_STM32])
			{
                case 'a':
                    wTravelCmd_a = bReceiveBuf[INDEX_PAYLOAD_0] << 8;
                    wTravelCmd_a |= bReceiveBuf[INDEX_PAYLOAD_1];
                    Counting = TRUE;
                    PeakCount = 0;
                    ExtendMotor_3_Panel_2();
                    break;
               case 'b':
                    wTravelCmd_b = bReceiveBuf[INDEX_PAYLOAD_0] << 8;
                    wTravelCmd_b |= bReceiveBuf[INDEX_PAYLOAD_1];
                    Counting2 = TRUE;
                    PeakCount2 = 0;
                    ExtendMotor_4_Panel_2();
                    break;
                    
                /*
                 * The main idea is there will be 3 types of actuator movement:
                 * 1) Next/Previous Stage
                 * 2) Keyed entry of stage select (1-16)
                 * 3) Manual Keyed entry of counts to move for each motor. 
                 * This last one is in the scenario of a crash in between stages
                 * 

                     
                     */
                case '['://Open
                    

                    if (motorState >= 17) {
                        /*
                         There are state variables but they don't seem to be used. They should probably be used here
                         */
                        break;
                    }

                    globalDataReset();
 
                    
                    if (wTravelCmd_a >= 0){
                        ExtendMotor_3_Panel_2();
                        wRetracting_a = FALSE;
                    }
                    else
                    {
                        wTravelCmd_a *= -1;
                        RetractMotor_3_Panel_2();
                        wRetracting_a = TRUE;
                    }
                       
                    if (wTravelCmd_b >= 0) {
                        ExtendMotor_4_Panel_2();
                        wRetracting_b = FALSE;
                    }
                    else
                    {
                        RetractMotor_4_Panel_2();
                        //wTravelCmd_b *= -1;
                        wRetracting_b = TRUE;
                    }

                    
                    motorState++; //Must increment after

                    break;

                case ']'://Close

                    
                    if (motorState < 1) {                                 
                        break;
                    }
                   
                    motorState--; //Must increment before data reset
                    globalDataReset();
                    
                    // 12 inch actuator
                    if (wTravelCmd_a >= 0)
                    {
                        RetractMotor_3_Panel_2();
                        wRetracting_a = TRUE;
                    }
                    else
                    {
                       //wTravelCmd_a *= -1;
                       ExtendMotor_3_Panel_2();
                       wRetracting_a = FALSE;
                    }

                    // 20 inch actuator
                    if (wTravelCmd_b >= 0)
                    {
                        RetractMotor_4_Panel_2();
                        wRetracting_b = TRUE;
                    }
                    else
                    {
                        RetractMotor_4_Panel_2();
                        //wTravelCmd_b *= -1;
                        wRetracting_b = FALSE;
                    }
                                        
                    break;
                
               
				case '1':
                    
					if (wWhatPanel == PANEL_1)
					{
						ExtendBothAxis_1_Motors();
						wAxis_1_State = EXPANDING;
					}
					else if (wWhatPanel == PANEL_2)
					{
						ExtendBothAxis_2_Motors();
						wAxis_2_State = EXPANDING;
					}
					wBalanceCounter = SECOND_3;
					break;
				case '2':
					if (wWhatPanel == PANEL_1)
					{
						ContractBothAxis_1_Motors();
						wAxis_1_State = CONTRACTING;
					}
					else if (wWhatPanel == PANEL_2)
					{
						ContractBothAxis_2_Motors();
						wAxis_2_State = CONTRACTING;
					}
					break;
				case '0':
                    PeakCount = 0;
                    motorState = 0;
                    wTravelAccum_a = 0;
                    wTravelAccum_b = 0;
					if (wWhatPanel == PANEL_1)
					{
						TurnOffBothAxis_1_Motors();
						wAxis_1_State = STOPPED;
					}
					else if (wWhatPanel == PANEL_2)
					{
						TurnOffBothAxis_2_Motors();
						wAxis_2_State = STOPPED;
					}
					break;
				case '4':
					if (wWhatPanel == PANEL_1)
						RetractMotorAxis_1();
					else if (wWhatPanel == PANEL_2)
						RetractMotor_3_Panel_2();
					break;
				case '5':
					if (wWhatPanel == PANEL_1)
						TurnOffMotor_1();
					else if (wWhatPanel == PANEL_2)
                    {
                        Counting = FALSE;
						TurnOffMotor_3();
                    }
					break;
				case '6':
					if (wWhatPanel == PANEL_1)
						ExtendMotorAxis_1();
					else if (wWhatPanel == PANEL_2)
                    {
						ExtendMotor_3_Panel_2();
                    }
					break;
				case '7':
					if (wWhatPanel == PANEL_1)
						RetractMotorAxis_2();
					else if (wWhatPanel == PANEL_2)
						RetractMotor_4_Panel_2();
					break;
				case '8':
					if (wWhatPanel == PANEL_1)
						TurnOffMotor_2();
					else if (wWhatPanel == PANEL_2)
						TurnOffMotor_4();
					break;
				case '9':
					if (wWhatPanel == PANEL_1)
						ExtendMotorAxis_2();
					else if (wWhatPanel == PANEL_2)
						ExtendMotor_4_Panel_2();
					break;
                case 'i':
                    SendArrayIndex();
                    break;
				case 'q':
					SendPanelSet_1_Axis_1_Curr();
					SendPanelSet_1_Axis_2_Curr();
					break;
                case 'p':
                    SendMotor3Motor4PeakCounts();
                    break;
                case 'r':
                    SendMotor3Motor4PeakSums();
                    break;
				case 'x':
					wWhatPanel = PANEL_1;
					WhitePanel_1();
					ColorOffPanel_2();
					break;
				case 'y':
					wWhatPanel = PANEL_2;
					WhitePanel_2();
					ColorOffPanel_1();
					break;
				case 'z':
					ColorOffPanel_1();	
					ColorOffPanel_2();
					wWhatPanel = NO_PANEL;
					break;	
			}
			bReceiveBuf[INDEX_CMD_STM32] = 0;
		}	

        // This happens every 102uSec per ADCP0Interrupt()
		if (wNewADCdata == TRUE)
		{
			wMotor_1_Curr = InputFilterPoint(MotorCurrent1,wMotor1_Curr_Array);
			wMotor_2_Curr = InputFilterPoint(MotorCurrent2,wMotor2_Curr_Array);
			wMotor_3_Curr = InputFilterPoint(MotorCurrent3,wMotor3_Curr_Array);
			wMotor_4_Curr = InputFilterPoint(MotorCurrent4,wMotor4_Curr_Array);
            
            //FindPeakMotor3(MotorCurrent3);
            IncrementFunction(MotorCurrent3, CurrentDeque);  
            IncrementFunction_2(MotorCurrent4, CurrentDeque2);  
            
            
            /*
             I think this is an important juncture where we have to decide, based
             * on potential failure modes how we want to end motor movement. The logic
             * below is fine for now I think, but needs to be mulled over.
             */
             if (PeakCount > abs(wTravelCmd_a))
            {
                Counting = FALSE;
                PeakCount = 0;
                if (wRetracting_a == FALSE)
                    wTravelAccum_a--;       // this counted 1 extra so fix here
                else
                    wTravelAccum_a++;
				TurnOffMotor_3();
            }
         
           if (PeakCount2 > abs(wTravelCmd_b))
            {
                Counting2 = FALSE;
                PeakCount2 = 0;
                if (wRetracting_b == FALSE)
                    wTravelAccum_b--;       // this counted 1 extra so fix here
                else
                    wTravelAccum_b++;
				TurnOffMotor_4();
            }            
            wNewADCdata = FALSE;
		}      
	}	// end of while(1)
}  // End of Main()


/***** Filter points can only be based of 2... like 16, 32, 64 *****/
word InputFilterPoint(word wInputPoint, word *pFilterPts)

{
	byte i;
	dword lAccumulator;

	// shift and add the points
	lAccumulator = 0;
	for (i=0; i<MAX_NUM_FILTER_POINTS-1; i++)
	{
		pFilterPts[i] = pFilterPts[i+1];
		lAccumulator += pFilterPts[i+1]; 
	}
	pFilterPts[MAX_NUM_FILTER_POINTS-1] = wInputPoint;
	lAccumulator += wInputPoint;
	lAccumulator >>= 5;										// the divisor must be base of 2: divide by 16 -> 4, 32 -> 5
	
	return lAccumulator;
}

/*
 * This function's aim is to create an upper bound for a threshold as to 
 * prevent overflow from interrupts no motor movement occurs
 */
void incrementRefactory(void)
{
    if (RefractoryCount < 16) 
    {
        RefractoryCount++;
    }
}

/*
 * This function's aim is to create an upper bound for a threshold as to 
 * prevent overflow from interrupts no motor movement occurs
 */
void incrementRefactory2(void)
{
    if (RefractoryCount2 < 16) 
    {
        RefractoryCount2++;
    }
}


//The point of this function is to increment the peak count until a threshold and reset the refactory counter.
//This is fixing the issue of counting two peaks where two adjacent values are equal at a peak.
void incrementPeakThreshold(void) 
{
    if (RefractoryCount > 3 && Counting == TRUE) 
    {
        PeakCount++;
        
        if (wRetracting_a == FALSE)
            wTravelAccum_a++;
        else
            wTravelAccum_a--;
        
        RefractoryCount = 0;
    }
}

//The point of this function is to increment the peak count until a threshold and reset the refactory counter.
//This is fixing the issue of counting two peaks where two adjacent values are equal at a peak.
void incrementPeakThreshold2(void) 
{
    if (RefractoryCount2 > 3 && Counting2 == TRUE) 
    {
        PeakCount2++;
        if (wRetracting_b == FALSE)
            wTravelAccum_b++;
        else
            wTravelAccum_b--;
        RefractoryCount2 = 0;
    }
}

void IncrementFunction(word curVal, word *peakDeque)
{  
    incrementRefactory();
    /*
    Set first index to second value
    Set Second index to third value
    Set third index to new value
    */
    peakDeque[0] = peakDeque[1];
    peakDeque[1] = peakDeque[2];
    peakDeque[2] = curVal;

    //Initialization
    //	Must account for errored peak when initial values such as {0, 2, 1} where 0 is the initial function 
    if(peakDeque[0] != 0)
    {
        if ((peakDeque[1] >= peakDeque[0] && peakDeque[1] > peakDeque[2]) || 
            (peakDeque[1] > peakDeque[0] && peakDeque[1] >= peakDeque[2])) 
        {
            //if(peakDeque[0] < peakDeque[1] && peakDeque[1] > peakDeque[2]) {

            incrementPeakThreshold();
            if(RefractoryCount == 0) 
            {
                //printf("Peak at %d\n", peakDeque[1]);
            }
        }
    }
}

void IncrementFunction_2(word curVal, word *peakDeque)
{  
    incrementRefactory2();
    /*
    Set first index to second value
    Set Second index to third value
    Set third index to new value
    */
    peakDeque[0] = peakDeque[1];
    peakDeque[1] = peakDeque[2];
    peakDeque[2] = curVal;

    //Initialization
    //	Must account for errored peak when initial values such as {0, 2, 1} where 0 is the initial function 
    if(peakDeque[0] != 0)
    {
        if ((peakDeque[1] >= peakDeque[0] && peakDeque[1] > peakDeque[2]) || 
            (peakDeque[1] > peakDeque[0] && peakDeque[1] >= peakDeque[2])) 
        {
            //if(peakDeque[0] < peakDeque[1] && peakDeque[1] > peakDeque[2]) {

            incrementPeakThreshold2();
            if(RefractoryCount2 == 0) 
            {
                //printf("Peak at %d\n", peakDeque[1]);
            }
        }
    }
}

void globalDataReset() {
    wTravelCmd_a = motorStateDataA12[motorState];
    wTravelCmd_b = motorStateDataB20[motorState];
    Counting = TRUE;
    Counting2 = TRUE;
    PeakCount = 0;
    PeakCount2 = 0;
    
}




#if 0
		ClrWdt();
		dwCheckSum = dwCheckSumCompare;
		// Make sure flash is not corrupted!!! 
		FlashMemoryCheckSum();									//do a checksum of entire flash except the last 16 bytes (checksum store here)
		while(dwCheckSum != dwCheckSumCompare)
		{
			ClrWdt();
			if (bNewPacketArrived)
			{
				LED = 1;	
			
				ParseCommand1();
				bNewPacketArrived = FALSE;
				bError = CHECK_SUM_ERROR;
			}
		}	
#endif

/****************************************************************************/
/*** Function that does actuall reading, must implement table read in asm ***/
/****************************************************************************/
dword ReadProgramMemDouble(word page,word addrlo)
{
	asm ("mov		W0,TBLPAG");	
	asm ("tblrdl 	[W1],W0");
	asm ("tblrdh 	[W1],W1");
}

#ifdef UART_DEBUG
void SendUART(unsigned int wValue, unsigned char bNumType)
{
	unsigned char x;


    RS485_TX_EN = TRUE;  						// disable Rx, enable Tx
	if (bNumType == DECIMAL)
		Int2Dec(wValue);
	else if (bNumType == HEXA)
		Int2Hex(wValue);
//	We have ASCII, just use
		
	for (x=0; x<5; x++)
	{
		U1TXREG = readBuf[x];
		Delay_17_8uS_Times(6);
	}
	
	if (bNumType == DECIMAL)
	{
		U1TXREG = ' ';
		Delay_17_8uS_Times(6);
	}

    while(!U1STAbits.TRMT);  					/* transmit register is not empty and transmission in progress */
    RS485_TX_EN = FALSE;  						// disable tx, enable rx
}


// Ready digits of a decimal 16 bit number to be displayed in ascii.  Load readBuf[] with
// MSBit in readBuf[0], next significant bit in readBuf[1]... and so on
void Int2Dec(unsigned int wValue)
{
  unsigned char temp;
	unsigned int wTemp;
  
	temp = wValue/10000;
	if (temp != 0)
	{
		readBuf[0] = temp + '0';
		wTemp = (unsigned int)temp * 10000;	// manipulate number to ready next digit
		wValue = wValue - wTemp;
	}
	else
		readBuf[0] = ' ';

	temp = wValue/1000;
	if (temp != 0)
	{
		readBuf[1] = temp + '0';
		wTemp = (unsigned int)temp * 1000;
		wValue = wValue - wTemp;
	}
	else
		readBuf[1] = '0';
		
	temp = (unsigned char)(wValue/100);
	if (temp != 0)
	{
		readBuf[2] = temp + '0';
		wTemp = (unsigned int)temp * 100;
		wValue = wValue - wTemp;
	}
	else
		readBuf[2] = '0';

	temp = (unsigned char)(wValue/10);
	if (temp != 0)
	{
		readBuf[3] = temp + '0';
		wTemp = (unsigned int)temp * 10;
		wValue = wValue - wTemp;
	}
	else
		readBuf[3] = '0';

	
	readBuf[4] = wValue + '0';		
}

/********************************************************************/
/*** Int2Hex converts a 16 bit unsigned integer into ASCII digits ***/
/*** This function is called from Teraterm and web page apps      ***/
/********************************************************************/
void Int2Hex(unsigned int value)
{
  unsigned char temp;
  
  readBuf[0] = ' ';             //put a space
  
  temp = (unsigned char)(value >> 12);
  Convert2ASCII(temp,1);
  temp = (unsigned char)(value >> 8);
  temp &= 0xf;
  Convert2ASCII(temp,2);
  temp = (unsigned char)(value >> 4);
  temp &= 0xf;
  Convert2ASCII(temp,3);
  temp = (unsigned char)(value & 0xf);
  temp &= 0xf;
  Convert2ASCII(temp,4); 
}

/*********************************************************************/
/*** This function converts for Teraterm and webserver             ***/ 
/*** It is called from Int2Hex().  It converts one digit at a time ***/
/*********************************************************************/
void Convert2ASCII(unsigned char temp, unsigned char index)
{
  if (temp>9)
  {
    readBuf[index] = (temp - 10) + 'a';
  }  
  else
  {
    readBuf[index] = temp + '0';
  }

  //lcd_text2[index] = readBuf[index];        // variable for web page
}  




void NextLine(void)
{
 	RS485_TX_EN = TRUE;  						// disable tx, enable rx
	U1TXREG = 0x0a; 
	U1TXREG	= 0x0d;
  while(!U1STAbits.TRMT);  					/* transmit register is not empty and transmission in progress */
  RS485_TX_EN = FALSE;  						// disable tx, enable rx
}


void Space(void)
{
 	RS485_TX_EN = TRUE;  						// disable tx, enable rx 
	U1TXREG	= 0x20;

	Delay_17_8uS_Times(6);
  while(!U1STAbits.TRMT);  					/* transmit register is not empty and transmission in progress */
  RS485_TX_EN = FALSE;  						// disable tx, enable rx
}
#endif



/******************************************/
/*** Based on _ADCP0Interrupt() or 40uS ***/
/***        Used in SendUART();         ***/
/******************************************/
void Delay_17_8uS_Times(unsigned int wTick17_8uS)
{

	wCounter17_8uS = wTick17_8uS;		
	while(wCounter17_8uS);	

}

