/*******************************************************************************
*
*
*******************************************************************************/

#include <dsp.h>

#define FLYBACK_PERIOD_VALUE 8434  		 	 // 8434 corresponds to 114kHz  
#define FLYBACK_INTERLEAVED_PHASE 4217   	 // 4217 corresponds to 180Deg Phase-shift
#define FULLBRIDGEPERIOD (FLYBACK_PERIOD_VALUE >> 1) // 228kHz switching frequency (1/228kHz/1.04ns) = 4216 

#define FLYBACK_ALTDTR_VALUE 75				 // Dead-time ~75/175ns
#define FLYBACK_DTR_VALUE 175

#define FLYBACKDUTY1 PDC1					 // Re-define PWM Duty Cycle Registers 
#define FLYBACKDUTY2 PDC2

// State for Full Bridge Drive
#define FULLBRIDGE_Q3Q4_ACTIVE 				1
#define FULLBRIDGE_INACTIVE_2ND_QUADRANT 	2
#define FULLBRIDGE_Q2Q5_ACTIVE 				3
#define FULLBRIDGE_INACTIVE_4TH_QUADRANT 	4

// 180 degree is equal to 32767 
#define ONEHUNDREDSEVENTYFIVEDEGREE 32000	// 31800 Degree to turn off the Full-Bridge unfolding circuit					 
#define ONEHUNDREDSEVENTYDEGREE 31900		// ~170 degree  (used to be 31200, got change to decrease higher harmonics to 31900)
#define ONEHUNDREDSEVENTYDEGREE33 31200  //for 33% power test
#define ONEHUNDREDTENDEGREE			20000		// 5000 (1ms SCR Gate Pulse) 20000 (4.883ms)  22000 (5.386ms) 
#define NINTYDEGREE 16383
#define ONEDEGREE 45						// ~1 degree
#define ZERODEGREE 0
#define ANTIISLANDLOWVLIMIT 1000

#define MAXDUTYCLAMPED Q15(0.55)
#define Turns 1
#define GRID_OVERVOLTAGE_LIMIT 				15700								//14800 		
#define GRID_OVERVOLTAGE_LIMIT_HYS 		15300								//14200 	
#define GRID_UNDERVOLTAGE_LIMIT 			12300   						//11500													
#define GRID_UNDERVOLTAGE_LIMIT_HYS 	12800   						//12000											
#define L1_N_OVERVOLTAGE_LIMIT 				16100							 		
#define L2_N_OVERVOLTAGE_LIMIT 				15600							 		
#define L1_N_UNDERVOLTAGE_LIMIT 				11900							 		
#define L2_N_UNDERVOLTAGE_LIMIT 				11600							 		
#define PVCELL_VOLTAGE_MIN 						10200					// 10200 (for 23.5V), 11000 for 25V			
#define PVCELL_VOLTAGE_MIN_HYS  			10500					// 10500 (for 23.5V), 11300 for 25V
#define PVCELL_VOLTAGE_MAX 						22700		
#define PVCELL_VOLTAGE_MAX_HYS 				20000			
#define PVCEL_GAIN_REDUCE             21000
#define INVERTER_OUTPUTCURRENT_MAX 		28800					// 1.18V + 1.65V     14000 = 1.4 + 1.6V = 3.0 V2.828A Fault Current Peak!!!
#define POWER_MAXIMUM 								25000					// 22500	
#define PVCELL_MAX_CURRENT 						31800					// 29000
#define MPPTFACTORMINIMUM 						800						//800
#define MININCREMENTMPPTFACTOR 				30							//30
#define MAXINCREMENTMPPTFACTOR 				40							//40
#define MINDECREMENTMPPTFACTOR 				50							//50
#define MAXDECREMENTMPPTFACTOR 				80							//80


// Coefficients for PI Controller
#define Ra Q15(0.6621)				// proportional: 	0.3621
#define Rsa Q15(0.0849)				// integral:			0.0449
#define Rsa1 Q15(0.005)

// Coefficients for Load Balancing 
#define MAX_BALANCE Q15(0.004)
#define KA_Q15 Q15(0.006)
#define KSA_Q15 Q15(0.0094)

// System State Definitions
#define SYSTEM_STARTUP 					0
#define DAY_MODE 								1
#define STANDBY_MODE 						2
#define SYSTEM_ERROR 						3

// Grid Frequency State
#define GRID_FREQUENCY_NOT_OK 	0
#define GRID_FREQUENCY_OK 			1

// Grid Voltage State
#define GRID_VOLTAGE_NOT_OK 		0
#define GRID_VOLTAGE_OK 				1

// Grid State
#define GRID_NOT_OK 						0
#define GRID_OK 								1

// PV Module State
#define PV_UNDERVOLTAGE 				0
#define PV_OVERVOLTAGE 					1
#define PV_MODULE_OK 						3

// Error State Definitions
#define NO_FAULT 								0
#define PV_UNDERVOLTAGE_FAULT 	1
#define PV_OVERVOLTAGE_FAULT 		2
#define GRID_FREQUENCY_ERROR 		3	
#define GRID_STARTUP_FAULT 			4
#define GRID_FAULT 							5
#define GRID_OVERCURRENT 				6
#define FLYBACK_OVERCURRENT 		7
#define HARDWAREZEROCROSS				8
#define DC_GND_FAULT						9
#define TRAP_FAULT							10
#define GRID_OV									11
#define GRID_UV									12
#define L1_N_OV_FAULT						13
#define L2_N_OV_FAULT						14
#define L1_N_UV_FAULT						15
#define L2_N_UV_FAULT						16
#define CHECK_SUM_ERROR					17
#define TEMPERATURE_FAULT				18


// State for SCR Bridge
#define SCR_D4D5_ACTIVE 					1
#define SCR_INACTIVE_2ND_QUADRANT 2
#define SCR_D3D6_ACTIVE 					3
#define SCR_INACTIVE_4TH_QUADRANT 4

// Include the following #define in the source code to run system on the bench. This will remove MPP-tracking so we need to define
// MPPTFactorMaximum ~10000 (in Variable.c) and create another way for a controlled softstart

//#define BENCHTESTING
#define ANTI_ISLANDING
#define DEFAULT_MPPT						9550  //3160 for 33%  (use 36VDC) 6319 for 66% 9550 for 100%   (use 39 VDC)
#define CONFIG_MPPT_SETPOINT			(DEFAULT_MPPT/100)

// Include the following #define to use array1, array2, and array3 for DMCI debug purposes

// Include the following #define to use array1, array2, and array3 for DMCI debug purposes
//#define DMCI

#define TRUE 1
#define FALSE 0

#define	OPTO_DRV1 LATBbits.LATB6 
#define	OPTO_DRV2 LATCbits.LATC13
 
#define LED					LATBbits.LATB8
#define IMI_OUTPUT  LATCbits.LATC7
#define IMI_DISABLE 0
#define IMI_ENABLE	1
#define LED_FAST_BLINK				700		// 1/8Sec per 130uS State Machine Interrupt

#define	TX_Temp  LATCbits.LATC5 
#define DACout   CMPDAC1bits.CMREF

#define SUPERLOOP_DELAY				18000		// approx 200 ms
#define MS_15									123			// 15 ms for 5 Min Grid Fault timer, based on 123uS _T2Interrupt (state machine)
#define MIN_4								  34			// based on 7 Sec time slice	


#define CURRENT_AC_HI					5
#define CURRENT_AC_LO					4

#define MAX_TEST_ARRAY				25

//#define WAVE_ANALYSIS

/****** AC Current Sensor Math Defines ******/
#define mV_PER_COUNT				(3.32/1024)
#define I_SENSE_AMP_GAIN		2.302

// IIR filter constants
#define MAX_NUM_FILTER_POINTS				32
#define MAX_NUM_TEMP_FILTER_POINTS	8


#define TIME_15_SEC									560000
#define MS_2_7											157					// 2.8 ms try shutdown inverter undervoltage at zero xsing
#define IMMIDIATE										2
#define MIN_5												20000				// 1000 = 15Sec, 2000 = 30Sec, 20000 = 5Min 
#define MAX_INSTRUCTIONS_IN_FLASH		(0x2BF0/2)	// 0x2c00 = Max flash, however using 0x2BF0 since using last bytes for dwCheckSum var

/********* Temperature Throttle *********/
#define TRIGGER_TEMPERATURE					90
#define THROTTLE_BACK_TEMPERATURE		1						
#define THROTTLE_PERCENT_STEP				20

#define THROTTLE_BACK_INIT_STATE    0
#define THROTTLE_BACK_WAIT_STATE    1
#define THROTTLE_BACK_LAST_STATE    2
#define MIN_1												12

#define ABOVE_FREEZE								0
#define BELOW_FREEZE								1

// For Testing Purposes only
#define SEC_2											18181
#define INCREMENT									0
#define DECREMENT									1

#define TIME_TO_ANTI_ISLAND		4545			// 4545 every half second 9090 - every second
#define BANDWIDTH_ENTRIES			4
#define MAX_BANDWIDTH					3
#define MAX_GRID_VOLTAGE_SET	39				// there are 40 entries on the table for max grid volts
#define MIN_GRID_VOLTAGE_SET	39				// there are 40 entries on the table for max grid volts
#define MIN_PV_VOLTAGE_SET	  39				// there are 40 entries on the table for max grid volts

#define DC_GROUND_FAULT

#define UPPER_FREQ_LIMIT			440				// 62 Hz
#define LOWER_FREQ_LIMIT			485				// 58 Hz


/***** Anti-Islanding Defines *****/
#define ANTI_ISLANDING_PERIOD_OFFSET  55		// 35
#define ISLANDING_NUMBER_ZC						4			// 3

#define	ANTI_ISLAND_SET  			0
#define	ANTI_ISLAND_EXECUTE  	1
#define	ANTI_ISLAND_END  			2

/******** Version # 1.01.08 ********/
#define VERSION_MAIN					1
#define VERSION_SECOND				1
#define VERSION_THIRD					8								

/*Peak Counting Defines*/
#define MIN_PEAK_THRESHOLD  3

