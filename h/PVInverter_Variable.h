/*******************************************************************************
*
*******************************************************************************/

#include "p33FJ16GS504.h"  
#include "PVInverter_defines.h" 

typedef 	unsigned char		byte;  
typedef 	unsigned int 		word; 
typedef 	unsigned long		dword; 

enum axisState{
	CONTRACTING,
	EXPANDING,
	STOPPED,
	FAULT
};

#define MS_250		     2500
#define SECOND_0_5     5000
#define SECOND_1			10000
#define SECOND_2			20000
#define SECOND_3			30000


// Motor Variables
extern unsigned int wMotor_1_Current;
extern unsigned int wMotor_1_GND;
extern unsigned int wMotor_1_Curr;
extern unsigned int wMotor_2_Curr;
extern unsigned int wMotor_3_Curr;
extern unsigned int wMotor_4_Curr;
extern unsigned int wMotor1_Curr_Array[];
extern unsigned int wMotor2_Curr_Array[];
extern unsigned int wMotor3_Curr_Array[];
extern unsigned int wMotor4_Curr_Array[];
extern unsigned int MotorCurrent1, MotorCurrent2; 
extern unsigned int MotorCurrent3, MotorCurrent4;
extern unsigned int wAxis_1_State;
extern unsigned int wAxis_2_State;
extern unsigned int wBalanceCounter;
extern unsigned int wWhatPanel;
extern unsigned int wNewADCdata;

//Peak Counter Variables
extern unsigned int PeakCount;
extern unsigned int PeakCount2;
extern unsigned int RefractoryCount;
extern unsigned int RefractoryCount2;
extern unsigned int CurrentDeque[];
extern unsigned int CurrentDeque2[];
extern unsigned int Counting;
extern unsigned int Counting2;
extern unsigned int wTravelCmd_a;
extern unsigned int wTravelCmd_b;

extern unsigned int motorStateDataB20[];
extern unsigned int motorStateDataA12[];
extern unsigned int motorState;

void IncrementFunction(word, word *);
void IncrementFunction_2(word, word *);
void incrementPeakThreshold(void);
void incrementPeakThreshold2(void);
void incrementRefactory(void);
void incrementRefactory2(void);


// ADC Variables
extern unsigned int pvCellVoltage;
extern unsigned int pvCellCurrent1Saved, pvCellCurrent2Saved; 
extern int pvInverterOutputVoltage; 
extern int pvInverterOutputCurrent;
extern int pvInverterOutputCurrentSUM;
extern int pvInverterOutputCurrentNew;
extern int pvInverterOutputCurrentOld;
extern int pvInverterOutputCurrentRAW; 
extern int  pvInverterOutputVoltsRAW;
extern int	pvGridOutputVoltsRAW;
extern int wVGridL1_N_Raw;
extern int wVGridL2_N_Raw;

extern int gridVoltage;
extern int wVGridL1_N;
extern int wVGridL2_N;
extern unsigned int NumSamples;
extern unsigned int CurrentSamples;
extern unsigned int TestDelay;
extern unsigned int wDCGroundFault;


// Variables for angle and frequency calculation
extern unsigned int angleTurnOnSCR;
extern long integerOutput;
extern long pvFactor; 
extern long fractionOutput;
extern int fractionFlag;
extern long remain;
extern unsigned int fractionAngle;
extern int deltaAngle1;
extern int prevDeltaAngle;
extern int gridPeriodMin;				
extern int gridPeriodMax;				
extern int ACCurrentOffset;
extern int pvGridVoltOffset;
extern int pvInvVoltOffset;	
extern int wVGridL1Offset;
extern int wVGridL2Offset;
extern int ACCurrentOffset1;			
extern unsigned int deltaAngle; 
extern unsigned int sineAngle; 
extern unsigned int gridPeriod;
extern unsigned int gridPeriodPres;
extern unsigned int prevGridPeriod;
extern unsigned int delGridPeriod;
extern unsigned int globalAngle;
extern unsigned char gridZeroCross;
extern int firstQuadrantDetectedFlag;
extern int prevGridVoltage;
extern char zeroCrossCount;
extern unsigned int zcCounter;

// Control loop variables
extern unsigned int decoupleTerm, prevDecoupleTerm;
extern int flybackDuty;
extern int finalFlybackDutyCycle1,finalFlybackDutyCycle2;
extern unsigned int currentReferenceDynamic;
extern long Ioutput ;
extern long totalOutput;
extern long Duty;
extern int IRdrop;
extern int IoRef;
extern int Poutput;
extern int currentError;
extern int currentErrorOld;
extern long Dinput;
extern long Doutput;
extern int rectifiedVacQ15;
extern int rectifiedIacQ15;
extern long rectifiedVacQ15long;
extern unsigned char SCRState;
extern unsigned char OC_Flag;

// AC Current Variables and Moving Avg Variables/Array
extern unsigned int averageRectifiedCurrent, maxInverterOutputCurrent; 
extern unsigned int rectifiedInverterOutputCurrentArray[8];
extern long unsigned int rectifiedInverterOutputCurrentSum;
extern unsigned char currentArrayCnt;
extern unsigned char ShortCircuit;
extern unsigned int CYCLES;

// Variables for peak detection 
extern int tempVacQ15,peakVacQ15,maxVacQ15, maxVacQ15Old;
extern int peakGridVac,maxGridVacQ15, peakGridVacOld,maxGridVGridL1_N,maxGridVGridL2_N;
extern int peakGridL1_N, peakGridL2_N;

extern int rectifiedGridVoltage;
extern int rectifiedVGridL1_N;
extern int rectifiedVGridL2_N;

// Variable used for MPPT Software 
extern unsigned int prevInputVoltageAverage;
extern int inputPower;
extern int prevInputPower, mpptFactor;
extern int prevPVVoltage;
extern unsigned char delMPPT;
extern int mpptFactorMaximum;
extern long unsigned int inputVoltageSum;
extern unsigned int inputVoltageAverage;
extern long unsigned int inputCurrentSum;
extern unsigned int inputCurrentAverage;
extern unsigned char avgInputDataReadyFlag;
extern unsigned int numberofSamples;
extern long unsigned int finalInputVoltageSum;		
extern long unsigned int finalInputCurrentSum;
extern long pvInverterOutputCurrentACCUM;
extern long pvInverterOutputVoltsACCUM;
extern long	pvGridOutputVoltsACCUM;
extern long lVGridL1_ACCUM;
extern long lVGridL2_ACCUM;

#ifdef BENCHTESTING
extern int count1;
extern int mpptFactor1;
extern unsigned char mpptIncrement;
#endif


// Define all Flags
extern unsigned char underVoltageFlag;
extern unsigned char overVoltageFlag;
extern unsigned char pvOverVoltageFlag;
extern unsigned char pvUnderVoltageFlag;
extern unsigned char mpptStartFlag;
extern unsigned char saturationFlag;
extern unsigned char systemStartFlag;
extern unsigned char nintyDegreeDetectFlag;
extern unsigned char gridFrequencyErrorFlag;
extern unsigned char firstQuadrantFlag;
extern unsigned char systemErrorFirstPassFlag;
extern unsigned char ODD_EVEN;

// New MOSFET block bridge variables
extern unsigned char prevFullBridgeState;
extern unsigned char fullBridgeState;
extern unsigned char thirdQuadrantFlag;
extern unsigned char zeroCrossDelay;		// nominal for 60Hz.  There is a min, max also depends on Peak Vac
extern unsigned char startFullBridgeFlag;

// Define system states
extern unsigned char systemState;
extern unsigned char errorState;
extern unsigned char pvModuleState;
extern unsigned char gridFrequencyState;
extern unsigned char gridVoltageState;
extern unsigned char gridState;

// Define all count variables 
extern int pvInverterStartupCount;
extern unsigned int miscCounter;
extern unsigned int systemRestartCounter;
extern unsigned char pvVoltageCounter;
extern unsigned int ZCDcount;
extern unsigned char inputAverageCompletedFlag;
extern unsigned char loadBalanceCount;
extern unsigned int gridUnderVoltageCounter;
extern unsigned int L1_N_UnderVoltageCounter;
extern unsigned int L2_N_UnderVoltageCounter;
extern unsigned int pvUnderVoltageCounter;
extern unsigned int startupCheckCount;
extern unsigned int gridPeriodCounter;
extern unsigned char antiIslandingCounter;
extern unsigned char gridZeroStateCounter;


// Load Balance Variables 
extern int diffPVCurrent;
extern int loadBalError;
extern int loadBalPoutput;
extern int loadBalIoutput;
extern int loadBalPIoutput;
extern long loadBalIoutputTempLong;
extern long loadBalPIoutputTempLong;
extern int deltaDuty;

extern signed int deltaVac;
extern signed int prevRectifiedGridVoltage;

extern int iRa,iRa2, iRsa,iRsa2,iRsa33, iRsa233, iRa50,iRa250,iRsa50,iRsa250, iRa33,iRa233;

// Variable Declaration for DMCI Tool (for debugging purposes) 
#ifdef DMCI
extern int array1[];
extern int array2[];
extern int array3[];
extern unsigned char ind;
extern int count;
#endif


extern byte bZeroXsing;
extern word wCurrentACMax;
extern word wCurrentAC;
extern word wGridVolts;
extern word wVar1,wVar2;
extern word wSetMPPTfactorHiLimit;
extern byte bArmUnderVoltageShutDown;
extern word wUnderVoltageShutDownCounter;
extern word wRCON;
extern byte bError;
extern word wGridFaultTimer;
extern byte bDisableGridFaultTimer;
extern byte bAntiIslandOFF;
extern byte bNewPacketArrived;
extern word wStartInverterCompare;
extern word wPVTemperature;
extern word wBoardTemperature;
extern byte bTemperatureSign;
extern dword dwCheckSum;
extern word wIMI;


/******************* Temperature Throttle ***********/
extern byte bThrottleBackTemp;
extern byte bThrottleReduction;
extern byte bThrottleBackState;
extern word wMPPTFactorReduction;
extern word wThrottleBackTimer;
extern word wLedPowerOutputTimer; 
extern word wIMITimer;

/******************* SetPoints **********************/
// Bandwidth
extern word wUpperPeriodLimit60Hz[BANDWIDTH_ENTRIES];
extern word wLowerPeriodLimit60Hz[BANDWIDTH_ENTRIES];
extern byte bBandWidthPointer;
// Grid OverVoltage Limit	
extern word wGridOverVoltageLim[40];
extern byte bGridOverVoltageLimptr;
extern word wGridOverVoltageLimHys;
extern word wGridOverVoltageLimit;
// Grid UnderVoltage Limit	
extern word wGridUnderVoltageLim[40];
extern byte bGridUnderVoltageLimptr;
extern word wGridUnderVoltageLimHys;
extern word wGridUnderVoltageLimit;
// Grid UnderVoltage Limit	
extern word wPVUnderVoltageLim[40];
extern byte bPVUnderVoltageLimptr;
extern word wPVUnderVoltageLimHys;
extern word wPVUnderVoltageLimit;
// Factory MODE
extern byte bFactoryMode;
// Set Power Level
//extern byte bSkip_SYSTEM_ERROR_Wdg_Reset;


// Function Declaration
void initClock(void);
void initADC(void);
void initPWM(void);
void initTimer2(void);
void initIOports(void);
void initStateMachineTimer(void);
void initCMP(void);
void InitDAC(void);
word InputFilterPoint(word,word *);
word TemperatureFilterPoint(word,word *);
void fullBridgeDrive(void);
void TurnFlyBackMOSFET_ON(void);
void TurnFlyBackMOSFET_OFF(void);
void SelfRescueAndFaults(void);
void FlashMemoryCheckSum(void);
dword ReadProgramMemDouble(word,word);
void IMI_Test(void);

