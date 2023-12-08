/*******************************************************************************

*
*******************************************************************************/

#include "PVInverter_Variable.h"
#include "PVInverter_defines.h"
#include "InverterSettings.h"

// Motor Variables
unsigned int wMotor_1_Current;
unsigned int wMotor_1_GND;
unsigned int wMotor_1_Curr;
unsigned int wMotor_2_Curr;
unsigned int wMotor_3_Curr;
unsigned int wMotor_4_Curr;
unsigned int wMotor1_Curr_Array[MAX_NUM_FILTER_POINTS];
unsigned int wMotor2_Curr_Array[MAX_NUM_FILTER_POINTS];
unsigned int wMotor3_Curr_Array[MAX_NUM_FILTER_POINTS];
unsigned int wMotor4_Curr_Array[MAX_NUM_FILTER_POINTS];
unsigned int MotorCurrent1 = 0, MotorCurrent2 = 0;
unsigned int MotorCurrent3 = 0, MotorCurrent4 = 0;
unsigned int wAxis_1_State = STOPPED;
unsigned int wAxis_2_State = STOPPED;
unsigned int wBalanceCounter = SECOND_2;
unsigned int wWhatPanel;
unsigned int wNewADCdata=FALSE;

//Peak Counter Variables
unsigned int PeakCount = 0;
unsigned int PeakCount2 = 0;
unsigned int RefractoryCount = 0;
unsigned int RefractoryCount2 = 0;
unsigned int CurrentDeque[3];
unsigned int CurrentDeque2[3];
unsigned int Counting=FALSE;
unsigned int Counting2=FALSE;
unsigned int wTravelCmd_a;
unsigned int wTravelCmd_b;

// ADC Variables
int pvInverterOutputVoltage = 0; 
unsigned int pvCellVoltage = 0;
unsigned int pvCellCurrent1Saved = 0, pvCellCurrent2Saved = 0; 
int gridVoltage = 0;
int wVGridL1_N = 0;
int wVGridL2_N = 0;
int pvInverterOutputCurrent = 0;
int pvInverterOutputCurrentSUM = 0;
int pvInverterOutputCurrentNew = 0;
int pvInverterOutputCurrentOld = 0;
int pvInverterOutputCurrentRAW = 0;
int pvInverterOutputVoltsRAW = 0;
int wVGridL1_N_Raw = 0;
int wVGridL2_N_Raw = 0;
int	pvGridOutputVoltsRAW = 0;
unsigned int wDCGroundFault = 0;

// Variables for angle and frequency calculation 
unsigned int angleTurnOnSCR = 0;
long integerOutput = 0; 
long pvFactor = 0;
long fractionOutput = 0;
int fractionFlag = 0;
long remain = 0;
unsigned int fractionAngle = 0;
int deltaAngle1 = 57;
int prevDeltaAngle = 0;
int gridPeriodMin = UPPER_FREQ_LIMIT;					// At startup frequency is unknown. Since we operate from 47Hz to 63Hz
int gridPeriodMax = LOWER_FREQ_LIMIT;					// we need to set Min and Max appropriately (# of ADC sample counts in one half cycle)
int ACCurrentOffset = 16384;				// Initial Offset value 
int pvGridVoltOffset = 16383;
int	pvInvVoltOffset = 16383;
int wVGridL1Offset = 16383;
int wVGridL2Offset = 16383;
unsigned int deltaAngle = 0, sineAngle = 0;
unsigned char gridZeroCross = 0;
unsigned int gridPeriod = 520;
unsigned int gridPeriodPres = 520;
unsigned int prevGridPeriod = 500;
unsigned int delGridPeriod = 0;
unsigned int gridPeriodCounter = 520;
unsigned int globalAngle = 0;
int firstQuadrantDetectedFlag = 0;
int prevGridVoltage = 0;
char zeroCrossCount = 0;
unsigned int zcCounter = 0;

// Control loop variable
unsigned int decoupleTerm = 0, prevDecoupleTerm = 0;
int flybackDuty = 0;
int finalFlybackDutyCycle1 = 0, finalFlybackDutyCycle2 = 0;
int Poutput = 0;
long Ioutput = 0;
long totalOutput = 0;
long Duty = 0;
long Doutput =0;
int IRdrop = 0;
int IoRef = 0;
long Dinput =0;
int currentError = 0;
int currentErrorOld =0;
unsigned int currentReferenceDynamic = 0;
int rectifiedVacQ15 = 0;
int rectifiedIacQ15 = 0;
long rectifiedVacQ15long = 0;

// AC Current Variables and Moving Avg Variables/Array
unsigned int averageRectifiedCurrent = 0, maxInverterOutputCurrent = 0; 
unsigned int rectifiedInverterOutputCurrentArray[8] = {0,0,0,0,0,0,0,0};
long unsigned int rectifiedInverterOutputCurrentSum = 0;
unsigned char currentArrayCnt = 0;

// Variables for peak detection 
int tempVacQ15 = 0,peakVacQ15 = 0,maxVacQ15 = 0;
int peakGridVac = 0,peakGridVacOld =0, maxGridVacQ15 = 0, maxGridVGridL1_N = 0,maxGridVGridL2_N = 0;
int peakGridL1_N = 0, peakGridL2_N = 0;
int rectifiedGridVoltage = 0;
int rectifiedVGridL1_N;
int rectifiedVGridL2_N;

// Variables used for MPPT Software
unsigned int prevInputVoltageAverage = 0;
int inputPower = 0;
int prevInputPower = 0,mpptFactor = 2000;							// 800 aldo for mpptFactor
int prevPVVoltage = 0;
unsigned char delMPPT = 255;
long unsigned int inputCurrentSum = 0;
unsigned int inputCurrentAverage = 0;
long unsigned int inputVoltageSum = 0;
unsigned int inputVoltageAverage = 0;
unsigned char avgInputDataReadyFlag = 0;
unsigned int numberofSamples = 0;
unsigned int CurrentSamples = 0;
long unsigned int finalInputVoltageSum = 0;		
long unsigned int finalInputCurrentSum = 0;

#ifdef BENCHTESTING
int mpptFactorMaximum = DEFAULT_MPPT;			// This value of 15000 gives us 303 Watts output, little distortion at
int count1 = 0;											// 12000 give us 6% THD
int mpptFactor1 = 800;
unsigned char mpptIncrement = 100;
#else
int mpptFactorMaximum = DEFAULT_MPPT;
#endif

word wSetMPPTfactorHiLimit __attribute__((persistent));

// System State Variables
unsigned char systemState = SYSTEM_STARTUP;
unsigned char errorState = NO_FAULT;
unsigned int startupCheckCount = 0;
unsigned char pvModuleState = PV_MODULE_OK;
unsigned char gridFrequencyState = GRID_FREQUENCY_OK;
unsigned char gridVoltageState = GRID_VOLTAGE_OK;
unsigned char gridState = GRID_NOT_OK;
unsigned char SCRState = SCR_INACTIVE_4TH_QUADRANT;
unsigned char OC_Flag =0;

// Define all counter variables 
int pvInverterStartupCount = 0;
unsigned int miscCounter = 0;
unsigned int systemRestartCounter = 0;
unsigned char pvVoltageCounter = 0;
unsigned int ZCDcount = 0;
unsigned char loadBalanceCount = 0;
unsigned int gridUnderVoltageCounter = 0;
unsigned int L1_N_UnderVoltageCounter = 0;
unsigned int L2_N_UnderVoltageCounter = 0;
unsigned int pvUnderVoltageCounter = 0;
unsigned char antiIslandingCounter = 0;
unsigned char gridZeroStateCounter = 0;
unsigned int NumSamples=1;
unsigned int CYCLES=0;
unsigned int TestDelay=0;

// Define all Flag Variables
unsigned char underVoltageFlag = 0;
unsigned char overVoltageFlag = 0;
unsigned char pvOverVoltageFlag = 0;
unsigned char pvUnderVoltageFlag = 0;
unsigned char mpptStartFlag = 0;
unsigned char saturationFlag = 0;
unsigned char inputAverageCompletedFlag = 0;
unsigned char nintyDegreeDetectFlag;
unsigned char systemStartFlag = 2;
unsigned char gridFrequencyErrorFlag = 0;
unsigned char firstQuadrantFlag = 0;
unsigned char systemErrorFirstPassFlag = 0;
unsigned char ODD_EVEN;
unsigned char ShortCircuit=0;

// New MOSFET block variables
unsigned char prevFullBridgeState = 0;
unsigned char fullBridgeState = FULLBRIDGE_INACTIVE_4TH_QUADRANT;
unsigned char thirdQuadrantFlag = 0;
unsigned char zeroCrossDelay = 5;		// 20 nominal for 60Hz.  There is a min, max also depends on Peak Vac
										// need to address this later, lets start with a nominal value
unsigned char startFullBridgeFlag = 0;

// Load Balance Variables 
int diffPVCurrent = 0;				// Current difference between both legs of the Interleaved Flyback
int loadBalError = 0;				
int loadBalPoutput = 0;			
int loadBalIoutput = 0;				
int loadBalPIoutput = 0;			// Load balance output of combined P and I portions after limitation. This value
									// is used to convert to PWM registers range
long loadBalIoutputTempLong = 0;	
long loadBalPIoutputTempLong = 0;
long pvInverterOutputCurrentACCUM =0;	
long pvInverterOutputVoltsACCUM=0;
long pvGridOutputVoltsACCUM =0;
long lVGridL1_ACCUM = 0;
long lVGridL2_ACCUM = 0;
									
int deltaDuty = 0;					// This variable has the duty cycle difference to be added or subtracted due to balance compensation


signed int deltaVac = 0;
signed int prevRectifiedGridVoltage = 0;


// Variable Declaration for DMCI Tool (for debugging purposes)
#ifdef DMCI
int array1[150];
int array2[150];
int array3[150];
unsigned char ind = 0;
int count = 0;
#endif

byte bZeroXsing = FALSE;
word wCurrentACMax;
word wGridVolts;
word wCurrentAC = 0;
word wVar1,wVar2;
byte bArmUnderoltageShutDown = 0;
word wUnderVoltageShutDownCounter = 0;
word wRCON;
byte bError __attribute__((persistent));
word wGridFaultTimer __attribute__((persistent));
byte bDisableGridFaultTimer __attribute__((persistent));
byte bAntiIslandOFF;
byte bNewPacketArrived;
word wStartInverterCompare;
word wPVTemperature;
word wBoardTemperature = 32;
byte bTemperatureSign;
word wIMI;

/*************** Throttle Back *****************/
byte bThrottleBackTemp = TRIGGER_TEMPERATURE;
byte bThrottleReduction = 1;
byte bThrottleBackState = THROTTLE_BACK_INIT_STATE;
word wMPPTFactorReduction = CONFIG_MPPT_SETPOINT;
word wThrottleBackTimer = MIN_1;

int iRa= Q15(.2321);					//0.2321 proportional
int iRa2= Q15(.4991);         //.4991 proportional around 0
int iRsa= Q15(0.1149);				//0.1649 33%,  .1189 for 66%, 100% (master at 22.5)
int iRsa2= Q15(0.1949);       //.1549  Integral around At above 50V, change to .1049
int iRa50 = Q15(.0921);       //.0921
int iRa250 = Q15(.3991);      //.3991
int iRsa50= Q15(0.0449);      //.0449
int iRsa250= Q15(0.0549);     //.0549
int iRa33=Q15(.3321);         //.3321
int iRa233=Q15(.9921);        //.9921
int iRsa33=Q15(.1649);         //.3321
int iRsa233=Q15(.1549);        //.9921

/******************* Bandwidth SetPoint Table ********************/
//  																						 1Hz,2Hz,3Hz,4Hz
word wUpperPeriodLimit60Hz[BANDWIDTH_ENTRIES] = {487,LOWER_FREQ_LIMIT,503,535};
word wLowerPeriodLimit60Hz[BANDWIDTH_ENTRIES] = {450,UPPER_FREQ_LIMIT,437,405};
byte bBandWidthPointer = FREQUENCY_BANDWIDTH-1;		


#if 0
/******************* Grid Over Voltage Limit Table ***********************/
word wGridOverVoltageLim[40] = {13050,13100,13150,13200,13250,13300,13350,13400,13450,13500,
																13550,13600,13650,13700,13750,13800,13850,13900,13950,14000,
																14050,14100,14150,14200,14250,14300,14350,14400,14550,14600,
																14700,14800,14900,15700,15100,15200,15300,15400,15500,15600};
byte bGridOverVoltageLimptr = GRID_VOLT_MAX;
word wGridOverVoltageLimit;
word wGridOverVoltageLimHys;

/******************* Grid Under Voltage Limit Table ***********************/
word wGridUnderVoltageLim[40] = {9500,9550,9600,9650,9700,9750,9800,9850,9900,9950,
																10000,10050,10100,10150,10200,10250,10300,10350,10400,10450,
																10500,10550,10600,10650,10700,10750,10800,10850,10900,10950,
																11000,11050,11100,12300,11200,11250,11300,11350,11400,11450};
byte bGridUnderVoltageLimptr = GRID_VOLT_MIN;
word wGridUnderVoltageLimit;
word wGridUnderVoltageLimHys;
#endif


/******************* PV Under Voltage Limit Table ***********************/
word wPVUnderVoltageLim[40] = 	{8200,8400,8600,8800,9000,9200,9400,9600,9800,10000,
																10200,10400,10600,10800,11000,11200,11400,11600,11800,12000,
																12200,12400,12600,12800,13000,13200,13400,13600,13800,14000,
																14200,14400,14600,14800,15000,15200,15400,15600,15800,16000};
byte bPVUnderVoltageLimptr = PV_VOLT_MIN;
word wPVUnderVoltageLimit;
word wPVUnderVoltageLimHys;

/************************* Factory MODE *****************************/
byte bFactoryMode	= 0;				// 1 = we are in factory mode !!

dword dwCheckSum;
word wLedPowerOutputTimer;
word wIMITimer = MIN_4;



