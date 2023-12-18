/***************************************************************************
 *
 * MODULE: uart.c
 *
 * DESCRIPTION:
 *
 *    Firmware for the UART portion of the module.
 *
 *
 * ENGINEER:  A. Della Sera
 *
 *  REV    DATE      PROGRAMMER   DETAILS
 *
 *  1.0    1-11-12   A. Della Sera
 *
 ****************************************************************************/
  
#include <p33FJ16GS504.h>  
#include "uart.h"
#include "PVInverter_defines.h"			
#include "PVInverter_Variable.h"		
#include "Config.h"	
#include "InverterSettings.h"


/*****************************************************************************
 * USART Macro
 *****************************************************************************/
//int __C30_UART ;

/***************************************************************************
* GLOBAL VARIABLES
***************************************************************************/  

byte bReceive[MSG_LEN_STM32];  			// Receive Buffer Solar Deploy
byte bTransmit[MSG_LEN_STM32];
byte bReceiveBuf[MSG_LEN_STM32];  	// Receive Buffer Solar Deploy
//byte bReceiveBuf[MSG_LEN];  	// Receive Buffer
byte bPacketData[PAYLOAD_TOTAL];
byte bPixelInfo[PAYLOAD_TOTAL];
byte bParity;
byte bAddress;
int iTotalOutput;
CONFIG_VALUES ConfigValues;
byte bPowerLevel[ARRAY_COMPARE_ENTRIES];
byte bBandWidth[ARRAY_COMPARE_ENTRIES];
byte bMaxGridVoltLimit[ARRAY_COMPARE_ENTRIES];
byte bMinGridVoltLimit[ARRAY_COMPARE_ENTRIES];
byte bMinPVVoltLimit[ARRAY_COMPARE_ENTRIES];
byte bIgnoreGridFaultTimer[ARRAY_COMPARE_ENTRIES];
byte bDisableAntiIslanding[ARRAY_COMPARE_ENTRIES];


/***************************************************************************
* EXTERNAL VARIABLES
***************************************************************************/
extern word wDelay;


void SendPacket(byte equipment, byte parameter, byte *value)
{
  bTransmit[INDEX_ADDR] = equipment;
  bTransmit[INDEX_CMD] = parameter;  

	Transmit();
}


byte CheckPacketAck(void)
{
	byte bPacketResponse;

	if (bNewPacketArrived)
	{
		Delay_17_8uS_Times(200);			// let's wait 3.4 ms before transmiting
		LED = 1;	
		wLedPowerOutputTimer = LED_FAST_BLINK;
			
		bPacketResponse = ParseCommand1();
		bNewPacketArrived = FALSE;
	}

	// Blink Faster when producing power!!!!  LED_FAST_BLINK nneds to be lot shorter than 1/4 Sec
	// since each master packet gets comes every 1/4, which causes LED to turn off
	if ( (wLedPowerOutputTimer == FALSE) && (wCurrentACMax>100) )
	{
		LED = 0;	
	}

	return bPacketResponse;
}


/*****************************************************************************
 *
 *    FUNCTION: 
 *    ISR_ReceiveChar()
 *
 *    DESCRIPTION:
 *    Interrupt service routine for the receive character on UART.
 *    For packets from Solar Deploy effort
 *    
 *    INTERRUPT VECTOR:
 *    
 *    INPUTS:
 *    none
 *    
 *    OUTPUTS:
 *    none
 *
 *****************************************************************************/
void __attribute__((interrupt,auto_psv)) _U1RXInterrupt(void)
{
  static int iIndx,i;

   /*    Character has been received.
    *    Packet lenght = 10 bytes 
    *    +----+----+----+----+----+----+----+----+----+----+
    *    |STX |ADR |CMD | FR | P# | D0 | D1 | D2 | D3 | CHK|
    *    +----+----+----+----+----+----+----+----+----+----+
    *
    * Byte 0:  STX
    * Byte 1:  Address
    * Byte 2:  Command
    * Byte 3:  Frame Rate
    * Byte 4:  Packet Number
    * Byte 5:  D0 
    * Byte 6:  D1
    * Byte 7:  D2
    * Byte 8:  D3
    * Byte 9:  Checksum
    *
    */

		IFS0bits.U1RXIF = 0; 					// clear RX interrupt flag
				
    /* transfer data from UART receive register to buffer */
    bReceive[iIndx] = U1RXREG;		// Get Received character
    if (bReceive[INDEX_STX] != STX)
    {
        iIndx = 0;    						// resync - possible trouble
        return;
    }

    iIndx++;

    if (iIndx < MSG_LEN_STM32)
    {
        return;
    }


    /* Do parity check */
    bParity = DoChecksumSTM32(bReceive);
    
    iIndx = 0;

    /* check for end of message or parity error */
    if (bParity != bReceive[INDEX_CHKSUM_STM32]) 
    {         
        return;  
    }

    /* if address does not match, disregard message */
    if (bReceive[INDEX_ADDR] != bAddress /*|| */)
    {           
			LED = 0;         
    	if (bReceive[INDEX_ADDR] == BROADCAST)
    	{
				bNewPacketArrived = TRUE;
    	}
    	else
        return;
    }                
      
		// copy buffer            
    for (i=0; i<MSG_LEN; i++) 
    	bReceiveBuf[i] = bReceive[i];
    	
    bNewPacketArrived = TRUE;
}


#if 0
/*****************************************************************************
 *
 *    FUNCTION: 
 *    ISR_ReceiveChar()
 *
 *    DESCRIPTION:
 *    Interrupt service routine for the receive character on UART.
 *    
 *    INTERRUPT VECTOR:
 *    
 *    
 *    INPUTS:
 *    none
 *    
 *    OUTPUTS:
 *    none
 *
 *****************************************************************************/
void __attribute__((interrupt,auto_psv)) _U1RXInterrupt(void)
{
  static int iIndx,i;

   /*    Character has been received.
    *    Packet lenght = 33 bytes 
    *    +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
    *    |STX |ADR |CMD | AUX| D0 | D1 | D2 | D3 | D4 | D5 | D6 | D7 | ...| D24| D25| D26| D27| CHK|
    *    +----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+----+
    *
    * Byte 0:  STX
    * Byte 1:  Address
    * Byte 2:  Command
    * Byte 3:  AUX
    * Byte 4:  D0
    * Byte 5:  D1 
    * Byte 6:  D2
    * Byte 7:  D3
    * Byte 8:  D4
    * Byte 9:  D5
    * Byte 10: D6
    * Byte 11: D7
    *     .
    *     .
    *     .
    * Byte 28: 
    * Byte 29: 
    * Byte 30: 
    * Byte 31: D27 
    * Byte 32: Checksum
    *
    */

		IFS0bits.U1RXIF = 0; 					// clear RX interrupt flag
				
    /* transfer data from UART receive register to buffer */
    bReceive[iIndx] = U1RXREG;		// Get Received character
    if (bReceive[INDEX_STX] != STX)
    {
        iIndx = 0;    						// resync - possible trouble
        return;
    }

    iIndx++;

    if (iIndx < MSG_LEN)
    {
        return;
    }


    /* Do parity check */
    bParity = DoChecksumSTM32(bReceive);
    
    iIndx = 0;

    /* check for end of message or parity error */
    if (bParity != bReceive[INDEX_CHKSUM_STM32]) 
    {         
        return;  
    }

    /* if address does not match, disregard message */
    if (bReceive[INDEX_ADDR] != bAddress /*|| */)
    {           
			LED = 0;         
    	if (bReceive[INDEX_ADDR] == BROADCAST)
    	{
				bNewPacketArrived = TRUE;
    	}
    	else
        return;
    }                
      
		// copy buffer            
    for (i=0; i<MSG_LEN; i++) 
    	bReceiveBuf[i] = bReceive[i];
    	
    bNewPacketArrived = TRUE;
}
#endif

/***************************************************************************
 *
 *    FUNCTION: 
 *    Transmit()
 *
 *    DESCRIPTION:
 *    Transmits the 16-byte reply message.  Calculates the checksum.
 *    
 *    INPUTS:
 *    none
 *    
 *    OUTPUTS:
 *    none
 *
 ***************************************************************************/
void Transmit(void)
{
    static byte bCntr;

// 	 	IEC0bits.U1RXIE = 0;			// Disable UART RX interrupt while transmitting
   	RS485_TX_EN = TRUE;  						// disable rx, enable tx

    bTransmit[INDEX_STX] = STX; 
    
    /* calculate the checksum */
    bTransmit[INDEX_CHKSUM_STM32] = DoChecksumSTM32(bTransmit);
        
    // transmit message
    for (bCntr = 0; bCntr < MSG_LEN_STM32; bCntr++)
    {
        while(!U1STAbits.TRMT)   			/* transmit register is not empty and transmission in progress */
				{
					ClrWdt();
				}

        U1TXREG = bTransmit[bCntr];   /* transfer data byte to TX reg */
    }
    
    while(!U1STAbits.TRMT)  					/* transmit register is not empty and transmission in progress */
		{
			ClrWdt();
		}

    RS485_TX_EN = FALSE;  						// disable tx, enable rx
//		IEC0bits.U1RXIE = 1;			// Enable UART RX interrupt
}



/***************************************************************************
 *
 *    FUNCTION: 
 *    DoChecksumSTM32()
 *
 *    DESCRIPTION:
 *    Calculates the checksum for Command type messages or for STM32.
 * 
 *    INPUTS:
 *    byte - pointer to Command Message
 *    
 *    OUTPUTS:
 *    byte - checksum
 *
 ***************************************************************************/
unsigned char DoChecksumSTM32(unsigned char *bMsg)
{
    byte bChksum;
  
    bChksum = bMsg[INDEX_STX_STM32]       + 
              bMsg[INDEX_ADDR_STM32]      + 
              bMsg[INDEX_CMD_STM32]       +
              bMsg[INDEX_FRAME_RATE]      +  
              bMsg[PACKET_NUMBER]         +  
             	bMsg[INDEX_PAYLOAD_0]       +               
             	bMsg[INDEX_PAYLOAD_1]       +               
             	bMsg[INDEX_PAYLOAD_2]       +               
             	bMsg[INDEX_PAYLOAD_3]; 
    
    if (bChksum == STX)
        bChksum -= 0x01;

    return(bChksum);
}


/***************************************************************************
 *
 *    FUNCTION: 
 *    DoChecksum()
 *
 *    DESCRIPTION:
 *    Calculates the checksum for the message.
 * 
 *    INPUTS:
 *    byte - pointer to message
 *    
 *    OUTPUTS:
 *    byte - checksum
 *
 ***************************************************************************/
unsigned char DoChecksum(unsigned char *bMsg)
{
    byte bChksum;
  
    bChksum = bMsg[INDEX_STX] + 
              bMsg[INDEX_ADDR] + 
              bMsg[INDEX_CMD] +
              bMsg[INDEX_SHT] +  

             	bMsg[INDEX_D0] +               
             	bMsg[INDEX_D1] +               
             	bMsg[INDEX_D2] +               
             	bMsg[INDEX_D3] +
                
             	bMsg[INDEX_D4] +               
             	bMsg[INDEX_D5] +               
             	bMsg[INDEX_D6] +               
             	bMsg[INDEX_D7] +               
                
             	bMsg[INDEX_D8] +               
             	bMsg[INDEX_D9] +                                          
             	bMsg[INDEX_D10] +               
             	bMsg[INDEX_D11] +
                
             	bMsg[INDEX_D12] +               
             	bMsg[INDEX_D13] +               
             	bMsg[INDEX_D14] +               
             	bMsg[INDEX_D15] +
                
             	bMsg[INDEX_D16] +               
             	bMsg[INDEX_D17] +               
             	bMsg[INDEX_D18] +               
             	bMsg[INDEX_D19] +
                
             	bMsg[INDEX_D20] +               
             	bMsg[INDEX_D21] +               
             	bMsg[INDEX_D22] +               
             	bMsg[INDEX_D23] +
                
             	bMsg[INDEX_D24] +               
             	bMsg[INDEX_D25] +               
             	bMsg[INDEX_D26] +               
             	bMsg[INDEX_D27];

    if (bChksum == STX)
        bChksum -= 0x01;

    return(bChksum);
}



/*****************************************************************************
 *
 *    FUNCTION: 
 *    void ParseCommand1()  
 *    
 *    DESCRIPTION:
 *    Parse the command specified in the COMMAND byte of the 
 *    received message.
 *
 *    INPUTS:
 *    none
 *    
 *    OUTPUTS:
 *    none
 *
 *****************************************************************************/
byte ParseCommand1(void)
{               
	byte bResponse;
	CONFIG_VALUES *pConfig;
	pConfig = &ConfigValues;
	  
	bResponse = NACK;
     
		switch (bReceiveBuf[INDEX_CMD])
    {

        case GET_ALL:  
            break;

				case RESET:				// Change the MPPT mpptFactorMaximum Limit so power will be limited
					break;

				case PROG_VARIABLE:
						break;
				case FACTORY_MODE:
						bFactoryMode = bReceiveBuf[INDEX_D0];
						break;
				case ACK:
						if (bTransmit[INDEX_CMD] == bReceiveBuf[INDEX_SHT])
							bResponse = ACK;	
						break;

	}
	
	return bResponse;
}


void Configure_UART(void)
{
/****************************************************************************/
/************ UART Setup: The TX and RX pins are analog pins.  **************/
/************ Making them digital pins happens in 						 **************/
/************ InitializeBoard()																 **************/
/****************************************************************************/	

	 bAddress = 2;		
	
	// Transmit Pin Set up
	TRISCbits.TRISC5 = 0;			// data direction register, 0 = output
	RPOR10bits.RP21R = 3; 		// Assign U1TX (= 3) to RP21 (pin 3)

	// Receive Pin Set up
	TRISCbits.TRISC4 = 1;			// data direction register, 1 = input
	RPINR18bits.U1RXR = 20; 		// Assign U1RX to RP20 = 20 (pin 2)
		
	// RTS Pin Set up
	TRISCbits.TRISC6 = 0;     // DDR, 0 = output: RS-485 TXEN
	RS485_TX_EN	= FALSE;

	U1MODEbits.STSEL = 0; 		// 1-stop bit
	U1MODEbits.PDSEL = 0; 		// No Parity, 8-data bits
	U1MODEbits.ABAUD = 0; 		// Auto-Baud Disabled
	U1MODEbits.BRGH = 0; 			// Low Speed mode
	// Baud Rate 
	// U1BRG = (Fcy/16*Baud Rate) - 1
	// U1BRG = 10 for 230400, 21 for 115200, 129 for 19200, 2082 for 1200 baud
	U1BRG = 10;                 // BAUD Rate Setting for 115200 (Fcy = Fosc/2 = 80MHz/2 = 40 Mhz)
	U1STAbits.UTXISEL0 = 0; 	// Interrupt after one TX character is transmitted
	U1STAbits.UTXISEL1 = 0;

	IPC2bits.U1RXIP = 3;			// Make UART receive the lowest priority, ADC Pair0 = 5, T2 State Machine = 4
	U1STAbits.URXISEL1 = 0;		//Interrupt is set when any character is received and transferred from the UxRSR to the receive
														//buffer. Receive buffer has one or more characters
	//IEC0bits.U1TXIE = 1; 		// Enable UART TX interrupt

	IEC0bits.U1RXIE = 1;			// Enable UART RX interrupt
	U1MODEbits.UARTEN = 1; 		// Enable UART
	U1STAbits.UTXEN = 1; 			// Enable UART TX
}
