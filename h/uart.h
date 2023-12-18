/***************************************************************************
 *
 * MODULE: uart.h
 *
 * DESCRIPTION:
 *
 *    Header file for uart.c
 *
 *
 * ENGINEER:  Aldo Della Sera
 *
 *  REV    DATE      PROGRAMMER   DETAILS
 *
 *  1.0    1-11-12	 A. Della Sera
 *
 ****************************************************************************/
#ifndef UART_H
#define UART_H

/*****************************************************************************
 * general defintions
 *****************************************************************************/
#define RS485_TX_EN						LATCbits.LATC6
#define ADDR_LO								PORTBbits.RB6
#define ADDR_HI								PORTBbits.RB7
#define BROADCAST							0xff
//#define UART_DEBUG

/*****************************************************************************
 * Equipment Addresses
 *****************************************************************************/
#define PANEL_SET_1					1
#define PANEL_SET_2   				2
#define PANEL_SET_3   				3
#define PANEL_SET_4				   	4

/*****************************************************************************
 * Parameter Info
 *****************************************************************************/
#define PANEL_1_MOTOR_CURR		1
#define PANEL_2_MOTOR_CURR		2
#define PEAK_COUNTS_MOTORS      3
#define PEAK_SUMS_MOTORS        4
#define ARRAY_INDEX             5

/***********************************************************/
/**************** STM32F429 Interface Packet ***************/
/***********************************************************/
#define INDEX_STX_STM32         0
#define INDEX_ADDR_STM32	    1
#define INDEX_CMD_STM32		    2
#define INDEX_FRAME_RATE     	3
#define PACKET_NUMBER        	4

#define INDEX_PAYLOAD_0         5
#define INDEX_PAYLOAD_1         6
#define INDEX_PAYLOAD_2         7
#define INDEX_PAYLOAD_3         8
#define INDEX_CHKSUM_STM32      9

#define MSG_LEN_STM32         (INDEX_CHKSUM_STM32 + 1)



/*****************************************************************************
 * 485 Protocol Message Definitions
 *****************************************************************************/
#define INDEX_STX      				0
#define INDEX_ADDR    				1
#define INDEX_CMD      				2
#define INDEX_SHT      				3

#define INDEX_D0     					4   // First byte in payload
#define INDEX_D1 			      	5   
#define INDEX_D2 			      	6    
#define INDEX_D3        			7    
#define INDEX_D4     					8   
#define INDEX_D5 			      	9    
#define INDEX_D6 			      	10    
#define INDEX_D7        			11    
#define INDEX_D8     					12     
#define INDEX_D9 			      	13    
#define INDEX_D10 			      14    
#define INDEX_D11        			15    
#define INDEX_D12     				16     
#define INDEX_D13 			      17    
#define INDEX_D14 			      18    
#define INDEX_D15        			19    
#define INDEX_D16     				20     
#define INDEX_D17 			      21    
#define INDEX_D18 			      22    
#define INDEX_D19        			23    
#define INDEX_D20     				24     
#define INDEX_D21 			      25    
#define INDEX_D22 			      26    
#define INDEX_D23        			27    
#define INDEX_D24     				28     
#define INDEX_D25 			      29    
#define INDEX_D26 			      30    
#define INDEX_D27        			31    

#define INDEX_CHKSUM  				32

#define STX           				0xaa
#define MSG_LEN       (INDEX_CHKSUM + 1)      
#define PAYLOAD_TOTAL					28

/*****************************************************************************
 * Command Definitions
 *****************************************************************************/
#define READ_STATUS           0x01
#define GET_ALL			          0x02
#define MESSAGE		          	0x03
#define DIM_SLAVES			  		0x04
#define BLANK_DISPLAY	      	0x05
#define ACK					  				0x06
#define NACK				  				0x07
#define SET_INVERTER_ON_OFF		0x08
#define RESET									0x09
#define PROG_VARIABLE					0x0a
#define FACTORY_MODE					0X0b

/***************************************************************************
 * FUNCTION PROTOTYPES
 ***************************************************************************/
void Transmit(void);
unsigned char DoChecksum(unsigned char *bMsg);
unsigned char DoChecksumSTM32(unsigned char *bMsg);
unsigned char ParseCommand1(void);
void Configure_UART(void);
unsigned char CheckPacketAck(void);
void SendPacket(unsigned char, unsigned char, unsigned char *);
unsigned char	ReadResistorsForAddr(void);
void SetParamaters(void);
void SetNewMaxGridVoltLimit(void);
void SetNewMinGridVoltLimit(void);

#endif
