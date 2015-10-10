/***********************************************************************************************************************
File: mpgl2_board_test.h                                                                

Description:
Header file for mpgl1_board_test.c.
***********************************************************************************************************************/

#ifndef __BOARD_TEST_H
#define __BOARD_TEST_H

/**********************************************************************************************************************
Type Definitions
***********************************************************************************************************************/


/***********************************************************************************************************************
Constants / Definitions
***********************************************************************************************************************/
/* BoardTest_u32Flags */
#define _LEDS_ON                              (u32)0x000000001  /* Set when audio and ANT test is active */
#define _AUDIO_ANT_ON                         (u32)0x000000008  /* Set when audio and ANT test is active */

#define   BOARDTEST_INIT_MSG_TIMEOUT      (u32)1000   /* Timeout for init display message to send */

/* BoardTest ANT radio settings */
#define	ANT_SERIAL_LO_BOARDTEST			          (u8)0x34
#define ANT_SERIAL_HI_BOARDTEST			          (u8)0x12
#define	ANT_DEVICE_TYPE_BOARDTEST					    DEVICE_TYPE_BOARDTEST
#define	ANT_TRANSMISSION_TYPE_BOARDTEST		    (u8)0x01

/* Default channel configuration parameters */
#define ANT_CHANNEL_BOARDTEST                 (u8)0
#define ANT_CHANNEL_TYPE_BOARDTEST            CHANNEL_TYPE_MASTER
#define ANT_NETWORK_BOARDTEST                 (u8)0

#define ANT_CHANNEL_PERIOD_DEC_BOARDTEST      (u16)8192
#define ANT_CHANNEL_PERIOD_HEX_BOARDTEST      (u16)0x2000
#define ANT_CHANNEL_PERIOD_HI_BOARDTEST		    (u8)0x20
#define ANT_CHANNEL_PERIOD_LO_BOARDTEST		    (u8)0x00

#define ANT_FREQUENCY_BOARDTEST						    (50)
#define ANT_TX_POWER_BOARDTEST					  	  RADIO_TX_POWER_0DBM

/* MPG Board Test specific Device Types */
#define	DEVICE_TYPE_BOARDTEST		              (u8)0x60


/***********************************************************************************************************************
Function Declarations
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
void BoardTestInitialize(void);
void BoardTestRunActiveState(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
void BoardTestSM_Idle(void);             
void BoartTestSM_Error(void);         


#endif /* __BOARD_TEST_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
