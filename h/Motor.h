
#include "p33FJ16GS504.h"

// Axis 1 Motors
#define	MOTOR1_OUT  LATBbits.LATB5 
#define	MOTOR1_IN   LATBbits.LATB6 
#define	MOTOR2_OUT  LATBbits.LATB15 
#define	MOTOR2_IN   LATBbits.LATB8 

//Axis 2 Motors
#define	MOTOR3_OUT  LATBbits.LATB2 
#define	MOTOR3_IN   LATCbits.LATC2 
#define	MOTOR4_OUT  LATCbits.LATC7 
#define	MOTOR4_IN   LATCbits.LATC8 

#define TestPin1    	LATCbits.LATC12 

// RGB LED for motors Panel 1
#define RED_AXIS_1		LATBbits.LATB11
#define BLUE_AXIS_1		LATBbits.LATB12
#define GREEN_AXIS_1	LATBbits.LATB13

// RGB LED for motors Panel 2
#define RED_AXIS_2		LATAbits.LATA3
#define BLUE_AXIS_2		LATAbits.LATA4
#define GREEN_AXIS_2	LATBbits.LATB14

enum panelNumber{
	NO_PANEL,
	PANEL_1,
	PANEL_2
};


#define LED_ON				0
#define LED_OFF				1

void ExtendBothAxis_1_Motors(void);
void ExtendBothAxis_2_Motors(void);
void ContractBothAxis_1_Motors(void);
void ContractBothAxis_2_Motors(void);
void TurnOffBothAxis_1_Motors(void);
void TurnOffBothAxis_2_Motors(void);
void ExtendMotorAxis_1(void);
void ExtendMotorAxis_2(void);
void ExtendMotor_3_Panel_2(void);
void ExtendMotor_4_Panel_2(void);
void RetractMotorAxis_1(void);
void RetractMotorAxis_2(void);
void RetractMotor_3_Panel_2(void);
void RetractMotor_4_Panel_2(void);
void AcquireMotor_1_GND(void);
void TurnOffMotor_1(void);
void TurnOffMotor_2(void);
void TurnOffMotor_3(void);
void TurnOffMotor_4(void);
void TurnOnBothMotorsAxis_1(void);
void TurnOnBothMotorsAxis_2(void);
void SendPanelSet_1_Axis_1_Curr(void);
void SendPanelSet_1_Axis_2_Curr(void);
void EqualizeCurrentsAxis_1(void);
void EqualizeCurrentsAxis_2(void);

void WhitePanel_1(void);
void PurplePanel_1(void);
void OrangePanel_1(void);
void RedPanel_1(void);
void GreenPanel_1(void);
void ColorOffPanel_1(void);

void WhitePanel_2(void);
void PurplePanel_2(void);
void OrangePanel_2(void);
void RedPanel_2(void);
void GreenPanel_2(void);
void ColorOffPanel_2(void);


