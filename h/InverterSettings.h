
/**********************************************************************/
/***** These settings are common for both Master and Channels 		*****/
/***** This .h file needs to be the same for inverter and Master 	*****/
/**********************************************************************/

/*** Inverters Channel Only Settings ***/
#define	DEFAULT_POWER						100				// 100%
#define	FREQUENCY_BANDWIDTH 			2				// +-2 Hz
#define GRID_VOLT_MIN						 33				// pointer value to a table of Min Grid SetPoints
#define GRID_VOLT_MAX						 33				// pointer value to a table of Max Grid SetPoints
#define PV_VOLT_MIN							 10				// pointer value to a table of Under DC Voltage SetPoints
#define PV_VOLT_MAX							500				// 50.0 Vdc
#define INVERTER_TEST_MODE				0				// 1 =  TRUE
#define WHAT_SETTING_BIT				0	

/*** Master Processor Only Settings ***/
#define QUAD_ADDRESS						1
#define SERIAL_NUMBER_HI				100
#define	SERIAL_NUMBER_LO				102		


#define	POWER_LEVEL							0x8000
#define FREQ_WINDOW							0x4000
#define GRID_MIN								0x2000
#define GRID_MAX								0x1000							
#define PV_MIN									0x0800
#define PV_MAX									0x0400
#define TEST_MODE_FACTORY				0x0200

typedef struct tagQuadSettings{
	/*** Inverters Channel Only Settings ***/
	unsigned int wInverterPower;
	unsigned int wFrequencyBandwidth;
	unsigned int wVoltMin;
	unsigned int wVoltMax;
	unsigned int wPVVoltMin;
	unsigned int wPVVoltMax;
	unsigned int wInverterTesMode;
	unsigned int wWhatSettingBits;
	unsigned int wQuadAddress;					// Master Setting Only
	unsigned int wSerialNumberHI;				// Master Setting Only
	unsigned int wSerialNumberLO;				// Master Setting Only
	unsigned int wFactoryModeDisable;
	unsigned int wIgnoreGridFaultRecoveryTimer;
	unsigned int wAntiIslandingDisable;
}QUAD_SETTINGS;

