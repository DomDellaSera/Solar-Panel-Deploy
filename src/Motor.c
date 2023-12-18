#include "Motor.h"
#include "PVInverter_defines.h"	
#include "uart.h"
#include "PVInverter_Variable.h"	

extern byte bReceiveBuf[];  	// Receive Buffer Solar Deploy
extern byte bTransmit[];			// Transmit Buffer Solar Deploy
extern word wCurrFilter[];


void SendPanelSet_1_Axis_1_Curr(void)
{
	bTransmit[INDEX_PAYLOAD_0] = wMotor_1_Curr >> 8;
	bTransmit[INDEX_PAYLOAD_1] = wMotor_1_Curr;
	bTransmit[INDEX_PAYLOAD_2] = wMotor_2_Curr >> 8;
	bTransmit[INDEX_PAYLOAD_3] = wMotor_2_Curr;
	SendPacket(PANEL_SET_1,PANEL_1_MOTOR_CURR,bTransmit);		// It is a polled transmit, PANEL_SET_1 address of Master
}


void SendPanelSet_1_Axis_2_Curr(void)
{
	bTransmit[INDEX_PAYLOAD_0] = wMotor_3_Curr >> 8;
	bTransmit[INDEX_PAYLOAD_1] = wMotor_3_Curr;
	bTransmit[INDEX_PAYLOAD_2] = wMotor_4_Curr >> 8;
	bTransmit[INDEX_PAYLOAD_3] = wMotor_4_Curr;
	SendPacket(PANEL_SET_1,PANEL_2_MOTOR_CURR,bTransmit);				// It is a polled transmit
}


void SendMotor3Motor4PeakCounts(void)
{
	bTransmit[INDEX_PAYLOAD_0] = PeakCount >> 8;                // Motor 3
	bTransmit[INDEX_PAYLOAD_1] = PeakCount;
	bTransmit[INDEX_PAYLOAD_2] = PeakCount2 >> 8;               // Motor 4
	bTransmit[INDEX_PAYLOAD_3] = PeakCount2;
	SendPacket(PANEL_SET_1,PEAK_COUNTS_MOTORS,bTransmit);		// It is a polled transmit, PANEL_SET_1 address of Master STM32F429
}

void SendMotor3Motor4PeakSums(void)
{
	bTransmit[INDEX_PAYLOAD_0] = wTravelAccum_a >> 8;           // Motor 3
	bTransmit[INDEX_PAYLOAD_1] = wTravelAccum_a;
	bTransmit[INDEX_PAYLOAD_2] = wTravelAccum_b >> 8;           // Motor 4
	bTransmit[INDEX_PAYLOAD_3] = wTravelAccum_b;
	SendPacket(PANEL_SET_1,PEAK_SUMS_MOTORS,bTransmit);			// It is a polled transmit, PANEL_SET_1 address of Master STM32F429
}

void SendArrayIndex(void)
{
	bTransmit[INDEX_PAYLOAD_0] = motorState;                    // motorState is ArrayIndex at 429 micro
	SendPacket(PANEL_SET_1,ARRAY_INDEX,bTransmit);			// It is a polled transmit, PANEL_SET_1 address of Master STM32F429
}

void EqualizeCurrentsAxis_1(void)
{
	int wError;

	wError = wMotor_1_Curr - wMotor_2_Curr;
	
	if (wError < -125)
	{
		TurnOffMotor_1();
		wBalanceCounter = SECOND_0_5;
	}
	else if (wError > 125)
	{
		TurnOffMotor_2();
		wBalanceCounter = SECOND_0_5;
	}
}


void EqualizeCurrentsAxis_2(void)
{
	int wError;

	wError = wMotor_3_Curr - wMotor_4_Curr;
	
	if (wError < -125)
	{
		TurnOffMotor_3();
		wBalanceCounter = SECOND_0_5;
	}
	else if (wError > 125)
	{
		TurnOffMotor_4();
		wBalanceCounter = SECOND_0_5;
	}
}


// MAX_NUM_FILTER_POINTS is Base 2.. so every 0.5 Sec the filter gets filled up
void AcquireMotor_1_GND(void)
{
	byte i = MAX_NUM_FILTER_POINTS;										// stay in loop for 2 Secs
	do {
		miscCounter = 500;					// 0.05 Secs
		while (miscCounter);
		wMotor_1_GND = InputFilterPoint(MotorCurrent1,wCurrFilter);
		i--;
	}
	while (i != 0);

}

// Motor 1
void ExtendMotorAxis_1(void)
{
	MOTOR1_OUT = 1;			
	MOTOR1_IN = 0;
	miscCounter = 2500;
	while (miscCounter);
	MOTOR1_OUT = 1;			
	MOTOR1_IN= 1;
}

void RetractMotorAxis_1(void)
{
	MOTOR1_OUT = 0;			
	MOTOR1_IN = 1;
	miscCounter = 2500;
	while (miscCounter);
	MOTOR1_OUT = 0;			
	MOTOR1_IN = 0;
}

// Motor 2
void ExtendMotorAxis_2(void)
{
	MOTOR2_OUT = 1;			
	MOTOR2_IN = 0;
	miscCounter = 2500;
	while (miscCounter);
	MOTOR2_OUT = 1;			
	MOTOR2_IN= 1;
}

void RetractMotorAxis_2(void)
{
	MOTOR2_OUT = 0;			
	MOTOR2_IN = 1;
	miscCounter = 2500;
	while (miscCounter);
	MOTOR2_OUT = 0;			
	MOTOR2_IN = 0;
}


// Motor 3
void ExtendMotor_3_Panel_2(void)
{
	MOTOR3_OUT = 1;			
	MOTOR3_IN = 0;
	miscCounter = 2500;
	while (miscCounter);
	MOTOR3_OUT = 1;			
	MOTOR3_IN= 1;
}

//Motor 3
void RetractMotor_3_Panel_2(void)
{
	MOTOR3_OUT = 0;			
	MOTOR3_IN = 1;
	miscCounter = 2500;
	while (miscCounter);
	MOTOR3_OUT = 0;			
	MOTOR3_IN = 0;
}

// Motor 4
void ExtendMotor_4_Panel_2(void)
{
	MOTOR4_OUT = 1;			
	MOTOR4_IN = 0;
	miscCounter = 2500;
	while (miscCounter);
	MOTOR4_OUT = 1;			
	MOTOR4_IN= 1;
}

//Motor 4
void RetractMotor_4_Panel_2(void)
{
	MOTOR4_OUT = 0;			
	MOTOR4_IN = 1;
	miscCounter = 2500;
	while (miscCounter);
	MOTOR4_OUT = 0;			
	MOTOR4_IN = 0;
}

void ExtendBothAxis_1_Motors(void)
{
	MOTOR1_OUT = 1;			
	MOTOR1_IN = 0;
	MOTOR2_OUT = 1;			
	MOTOR2_IN = 0;
	miscCounter = 2500;
	while (miscCounter);
	MOTOR1_OUT = 1;			
	MOTOR1_IN= 1;
	MOTOR2_OUT = 1;			
	MOTOR2_IN = 1;
	GreenPanel_1();
}

void ExtendBothAxis_2_Motors(void)
{
	MOTOR3_OUT = 1;			
	MOTOR3_IN = 0;
	MOTOR4_OUT = 1;			
	MOTOR4_IN = 0;
	miscCounter = 2500;
	while (miscCounter);
	MOTOR3_OUT = 1;			
	MOTOR3_IN= 1;
	MOTOR4_OUT = 1;			
	MOTOR4_IN = 1;
	GreenPanel_2();
}

void ContractBothAxis_1_Motors(void)
{
	MOTOR1_OUT = 0;			
	MOTOR1_IN = 1;
	MOTOR2_OUT = 0;			
	MOTOR2_IN = 1;
	miscCounter = 2500;
	while (miscCounter);
	MOTOR1_OUT = 0;			
	MOTOR1_IN = 0;
	MOTOR2_OUT = 0;			
	MOTOR2_IN = 0;
	RedPanel_1();
}

void ContractBothAxis_2_Motors(void)
{
	MOTOR3_OUT = 0;			
	MOTOR3_IN = 1;
	MOTOR4_OUT = 0;			
	MOTOR4_IN = 1;
	miscCounter = 2500;
	while (miscCounter);
	MOTOR3_OUT = 0;			
	MOTOR3_IN = 0;
	MOTOR4_OUT = 0;			
	MOTOR4_IN = 0;
	RedPanel_2();
}

void TurnOffBothAxis_1_Motors(void)
{
	MOTOR1_OUT = 1;			
	MOTOR1_IN = 0;
	MOTOR2_OUT = 1;			
	MOTOR2_IN = 0;
	ColorOffPanel_1();
}

void TurnOffBothAxis_2_Motors(void)
{
	MOTOR3_OUT = 1;			
	MOTOR3_IN = 0;
	MOTOR4_OUT = 1;			
	MOTOR4_IN = 0;
	ColorOffPanel_2();
}

void TurnOffMotor_1(void)
{
	PurplePanel_1();
	MOTOR1_OUT = 1;			
	MOTOR1_IN = 0;
}

void TurnOffMotor_2(void)
{
	OrangePanel_1();
	MOTOR2_OUT = 1;			
	MOTOR2_IN = 0;
}

void TurnOffMotor_3(void)
{
	PurplePanel_2();
	MOTOR3_OUT = 1;			
	MOTOR3_IN = 0;
}

void TurnOffMotor_4(void)
{
	OrangePanel_2();
	MOTOR4_OUT = 1;			
	MOTOR4_IN = 0;
}

void TurnOnBothMotorsAxis_1(void)
{
	GreenPanel_1();
	MOTOR1_OUT = 1;			
	MOTOR1_IN= 1;
	MOTOR2_OUT = 1;			
	MOTOR2_IN = 1;
}

void TurnOnBothMotorsAxis_2(void)
{
	GreenPanel_2();
	MOTOR3_OUT = 1;			
	MOTOR3_IN= 1;
	MOTOR4_OUT = 1;			
	MOTOR4_IN = 1;
}


/******* RGB LED for Panel 1 = U8 in schematic *******/
void WhitePanel_1(void)
{
	GREEN_AXIS_1	= LED_ON;
	RED_AXIS_1		= LED_ON;	
	BLUE_AXIS_1		= LED_ON;
}

void PurplePanel_1(void)
{
	GREEN_AXIS_1	= LED_OFF;
	RED_AXIS_1		= LED_ON;	
	BLUE_AXIS_1		= LED_ON;
}

void OrangePanel_1(void)
{
	GREEN_AXIS_1	= LED_ON;
	RED_AXIS_1		= LED_ON;	
	BLUE_AXIS_1		= LED_OFF;
}

void GreenPanel_1(void)
{
	GREEN_AXIS_1	= LED_ON;
	RED_AXIS_1		= LED_OFF;	
	BLUE_AXIS_1		= LED_OFF;	
}

void RedPanel_1(void)
{
	GREEN_AXIS_1	= LED_OFF;
	RED_AXIS_1		= LED_ON;	
	BLUE_AXIS_1		= LED_OFF;	
}

void ColorOffPanel_1(void)
{
	GREEN_AXIS_1	= LED_OFF;
	RED_AXIS_1		= LED_OFF;	
	BLUE_AXIS_1		= LED_OFF;	
}



/******* RGB LED for Panel 2 = U7 in schematic *******/
void WhitePanel_2(void)
{
	GREEN_AXIS_2	= LED_ON;
	RED_AXIS_2		= LED_ON;	
	BLUE_AXIS_2		= LED_ON;
}

void PurplePanel_2(void)
{
	GREEN_AXIS_2	= LED_OFF;
	RED_AXIS_2		= LED_ON;	
	BLUE_AXIS_2		= LED_ON;
}

void OrangePanel_2(void)
{
	GREEN_AXIS_2	= LED_ON;
	RED_AXIS_2		= LED_ON;	
	BLUE_AXIS_2		= LED_OFF;
}

void GreenPanel_2(void)
{
	GREEN_AXIS_2	= LED_ON;
	RED_AXIS_2		= LED_OFF;	
	BLUE_AXIS_2		= LED_OFF;	
}

void RedPanel_2(void)
{
	GREEN_AXIS_2	= LED_OFF;
	RED_AXIS_2		= LED_ON;	
	BLUE_AXIS_2		= LED_OFF;	
}

void ColorOffPanel_2(void)
{
	GREEN_AXIS_2	= LED_OFF;
	RED_AXIS_2		= LED_OFF;	
	BLUE_AXIS_2		= LED_OFF;	
}

