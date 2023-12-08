//#include "PVInverter_Variable.h"		

#define ARRAY_COMPARE_ENTRIES			4

typedef struct configVariables{
	unsigned char bPowerLevelArr[ARRAY_COMPARE_ENTRIES];
	unsigned char  bFreqWindowArr[ARRAY_COMPARE_ENTRIES];
	unsigned int wGridVoltMin[ARRAY_COMPARE_ENTRIES];
	unsigned int  wGridVoltMax[ARRAY_COMPARE_ENTRIES];
	unsigned int  wPVVoltMin[ARRAY_COMPARE_ENTRIES];
	unsigned int  wPVVoltMax[ARRAY_COMPARE_ENTRIES];
}CONFIG_VALUES;