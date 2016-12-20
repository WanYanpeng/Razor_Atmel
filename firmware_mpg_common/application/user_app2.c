/**********************************************************************************************************************
File: user_app2.c                                                                

Description:
Runs the LED display functions and control.  
Provides the API to create new display lists.

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserApp2Initialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserApp2RunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp2Flags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */



/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp2_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp2_StateMachine;            /* The state machine function pointer */
//static u32 UserApp2_u32Timeout;                      /* Timeout counter used across states */

static LedDisplayListHeadType UserApp2_sDemoLedCommandList;   
static u32 UserApp2_u32DemoListEndTime;

#ifdef USER_LIST_EXAMPLE
static LedDisplayListHeadType UserApp2_sUserLedCommandList;     
static u32 UserApp2_u32UserListEndTime;
#endif

static LedDisplayListHeadType* UserApp2_psActiveList;
static u32 UserApp2_u32ActiveListEndTime;
static bool UserApp2_bSystemRunning = TRUE;
static bool UserApp2_bSystemDark = FALSE;


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------
Function: LedDisplayStartList

Description:
Clears 

Requires:
  -

Promises:
  - 
*/
void LedDisplayStartList(void)
{
  LedDisplayListNodeType* psNodeParser;
  LedDisplayListNodeType* psNodeToKill;
  
  if(UserApp2_sUserLedCommandList.u8ListSize != 0)
  {
    /* Free all the list space */
    psNodeParser = UserApp2_sUserLedCommandList.psFirstCommand;
    
    while(psNodeParser != NULL)
    {
      psNodeToKill = psNodeParser;
      psNodeParser = psNodeParser->psNextNode;
      free(psNodeToKill);
    }
    
    /* Initialize the head node */
    UserApp2_sUserLedCommandList.psFirstCommand = NULL;
    UserApp2_sUserLedCommandList.u8ListSize = 0;
  }
  
} /* end LedDisplayStartList */


/*--------------------------------------------------------------------------------------------------------------------
Function: LedDisplayAddCommand

Description:
Adds a new LED commmand node.

Requires:
  -

Promises:
  - 
*/
bool LedDisplayAddCommand(LedDisplayListHeadType* psDisplayList_, LedCommandType* pCommandInfo_)
{
  LedDisplayListNodeType* psNewNode;
  LedDisplayListNodeType* psParser;
  
  /* Allocate the memory */
  psNewNode = malloc(sizeof(LedDisplayListNodeType));
  if(psNewNode == NULL)
  {
    DebugPrintf("\n\r*** Add node failed malloc***\n\r");
    return FALSE;
  }
  
  /* Copy in the LED and ON/OFF info */
  psNewNode->psNextNode = NULL;
  psNewNode->eCommand.eLED = pCommandInfo_->eLED;
  psNewNode->eCommand.bOn = pCommandInfo_->bOn;
  
  /* If the LED is on, then it needs to start at PWM0, otherwise it starts at PWM95.
  The ON time is always the actual start time.
  The OFF time is adjusted back by LED_FADE_TIME to allow the fade-out time. */
  if(pCommandInfo_->bOn)
  {
    psNewNode->eCommand.eCurrentRate = LED_PWM_0;
    psNewNode->eCommand.u32Time = pCommandInfo_->u32Time;
  }
  else
  {
    psNewNode->eCommand.eCurrentRate = LED_PWM_100;
    psNewNode->eCommand.u32Time = pCommandInfo_->u32Time - LED_FADE_TIME;
  }
  
  /* Locate the place in the list for the new node */
  if(psDisplayList_->u8ListSize == 0)
  {
    psDisplayList_->psFirstCommand = psNewNode;
  }
  else
  {
    /* Find the last node in the list */
    psParser = psDisplayList_->psFirstCommand;
    while(psParser->psNextNode != NULL)
    {
      psParser = psParser->psNextNode;
    }
    psParser->psNextNode = psNewNode;
    psParser = psNewNode;
  }
  psDisplayList_->u8ListSize++;
  
  return TRUE;
  
} /* end LedDisplayAddCommand() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: UserApp2Initialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void UserApp2Initialize(void)
{
  u8 au8UserApp1Start2[] = "LED display task started\n\r";
  
  /* Initialize the list variables */
  UserApp2_sDemoLedCommandList.psFirstCommand = NULL;
  UserApp2_sDemoLedCommandList.u8ListSize = 0;
  UserApp2_sUserLedCommandList.psFirstCommand = NULL;
  UserApp2_sUserLedCommandList.u8ListSize = 0;
    
  /* Set up the hard-coded display array */
  LedCommandType aeDemoList[] =
  { /* The ON times */
    {WHITE, 0, TRUE, LED_PWM_0},
    {PURPLE, 0, TRUE, LED_PWM_0},
    {BLUE, 2000, TRUE, LED_PWM_0},
    {CYAN, 3000, TRUE, LED_PWM_0},
    {RED, 0, TRUE, LED_PWM_0},
    {ORANGE, 1000, TRUE, LED_PWM_0},
    {YELLOW, 2000, TRUE, LED_PWM_0},
    {GREEN, 3000, TRUE, LED_PWM_0},
    {RED, 6000, TRUE, LED_PWM_0},
    {ORANGE, 6100, TRUE, LED_PWM_0},
    {YELLOW, 6200, TRUE, LED_PWM_0},
    {GREEN, 6300, TRUE, LED_PWM_0},
    {CYAN, 6400, TRUE, LED_PWM_0},
    {BLUE, 6500, TRUE, LED_PWM_0},
    {PURPLE, 6600, TRUE, LED_PWM_0},
    {WHITE, 6700, TRUE, LED_PWM_0},
    /* The OFF times */
    {WHITE, 2000, FALSE, LED_PWM_100},
    {PURPLE, 2000, FALSE, LED_PWM_100},
    {BLUE, 4000, FALSE, LED_PWM_100},
    {CYAN, 5000, FALSE, LED_PWM_100},
    {RED, 2000, FALSE, LED_PWM_100},
    {ORANGE, 3000, FALSE, LED_PWM_100},
    {YELLOW, 4000, FALSE, LED_PWM_100},
    {GREEN, 5000, FALSE, LED_PWM_100},
    {RED, 9000, FALSE, LED_PWM_100},
    {ORANGE, 9000, FALSE, LED_PWM_100},
    {YELLOW, 9000, FALSE, LED_PWM_100},
    {GREEN, 9000, FALSE, LED_PWM_100},
    {CYAN, 9000, FALSE, LED_PWM_100},
    {BLUE, 9000, FALSE, LED_PWM_100},
    {PURPLE, 9000, FALSE, LED_PWM_100},
    {WHITE, 9000, FALSE, LED_PWM_100},
  };

  /* Set the total list time and build the command array */
  UserApp2_u32DemoListEndTime = 9000;
  for(u8 i = 0; i < (sizeof(aeDemoList) / sizeof(LedCommandType)); i++)
  {
    LedDisplayAddCommand(&UserApp2_sDemoLedCommandList, &aeDemoList[i]);
  }
    
#ifdef USER_LIST_EXAMPLE
  LedCommandType aeUserList[] =
  { /* The ON times */
    {WHITE, 0, TRUE, LED_PWM_0},
    {RED, 500, TRUE, LED_PWM_0},
    {YELLOW, 0, TRUE, LED_PWM_0},
    {YELLOW, 1000, TRUE, LED_PWM_0},
    /* The OFF times */    
    {WHITE, 2000, FALSE, LED_PWM_100},
    {RED, 2000, FALSE, LED_PWM_100},
    {YELLOW, 1000, FALSE, LED_PWM_100},
    {YELLOW, 2000, FALSE, LED_PWM_100}
  };

  /* Set the total list time and build the command array */
  UserApp2_u32UserListEndTime = 2000;
  for(u8 i = 0; i < (sizeof(aeUserList) / sizeof(LedCommandType)); i++)
  {
    LedDisplayAddCommand(&UserApp2_sUserLedCommandList, &aeUserList[i]);
  }
#endif  

  UserApp2_psActiveList = &UserApp2_sDemoLedCommandList;
  UserApp2_u32ActiveListEndTime = UserApp2_u32DemoListEndTime;

  /* Load the LCD and complete the initialization */
  LoadLcdScreen();
  DebugPrintf(au8UserApp1Start2);
  
  /* If good initialization, set state to Idle */
  if( 1 )
  {
    UserApp2_StateMachine = UserApp2SM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp2_StateMachine = UserApp2SM_FailedInit;
  }

} /* end UserApp2Initialize() */

  
/*----------------------------------------------------------------------------------------------------------------------
Function UserApp2RunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserApp2RunActiveState(void)
{
  UserApp2_StateMachine();

} /* end UserApp2RunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function LoadLcdScreen()

Description:
Clears the LCD screen and writes the default text.
The display is ON without a cursor.

Requires:
  - None

Promises:
- The LCD is updated to the main screen:
EiE Competition
DMO   USR    ||    X
0123456789ABCDEF0123  (reference character addresses)
*/
void LoadLcdScreen(void)
{
                             /*0123456789ABCDEF0123*/
  u8 au8UserApp2LcdStart1[] = "EiE Competition";
  u8 au8UserApp2LcdStart2[] = "DMO   USR    ||    X";

  LCDCommand(LCD_CLEAR_CMD);
  LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON);
  
  LCDMessage(LINE1_START_ADDR, au8UserApp2LcdStart1);
  LCDMessage(LINE2_START_ADDR, au8UserApp2LcdStart2);
  
} /* end LoadLcdScreen() */

/*----------------------------------------------------------------------------------------------------------------------
Function AllLedsOff()

Description:
Sets all LEDs to LED_PWM_0. 

Requires:
  - None

Promises:
  - All discrete LEDs are set to PWM mode with LED_PWM_0 (OFF)
*/
void AllLedsOff(void)
{
  LedPWM(RED, LED_PWM_0);
  LedPWM(ORANGE, LED_PWM_0);
  LedPWM(YELLOW, LED_PWM_0);
  LedPWM(GREEN, LED_PWM_0);
  LedPWM(CYAN, LED_PWM_0);
  LedPWM(BLUE, LED_PWM_0);
  LedPWM(PURPLE, LED_PWM_0);
  LedPWM(WHITE, LED_PWM_0);
  
} /* end AllLedsOff() */


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for first button to be pressed to start the program
BUTTON0 starts the DEMO code.
BUTTON1 starts the USER code.
*/
static void UserApp2SM_Idle(void)
{
   if(WasButtonPressed(BUTTON0))
   {
     ButtonAcknowledge(BUTTON0);
     
     UserApp2_psActiveList = &UserApp2_sDemoLedCommandList;
     UserApp2_u32ActiveListEndTime = UserApp2_u32DemoListEndTime
     UserApp2_StateMachine = UserApp2SM_RunCommandList;
   }
   
   if(WasButtonPressed(BUTTON1))
   {
     ButtonAcknowledge(BUTTON1);
     
     /* If the user list is empty, display a message for 2 seconds to tell the user this.
     Otherwise, start the USER code */
     if(UserApp2_sUserLedCommandList.u8ListSize == 0)
     {
       /* Update the LCD with the message and then go to a wait state */
        LCDCommand(LCD_CLEAR_CMD);
     }
     else
     {
       UserApp2_psActiveList = &UserApp2_sUserLedCommandList;
       UserApp2_StateMachine = UserApp2SM_RunCommandList;
     }
   }

} /* end UserApp2SM_Idle() */
    

/*-------------------------------------------------------------------------------------------------------------------*/

static void UserApp2SM_RunCommandList(void)
{
  static u32 u32SystemTime = 0;
  LedDisplayListNodeType* psListParser = UserApp2_psActiveList->psFirstCommand;

   if(WasButtonPressed(BUTTON0))
   {
     ButtonAcknowledge(BUTTON0);
     UserApp2_psActiveList = &UserApp2_sDemoLedCommandList;
     u32SystemTime = 0;
     AllLedsOff();
   }

   if(WasButtonPressed(BUTTON1))
   {
     ButtonAcknowledge(BUTTON1);
     UserApp2_psActiveList = &UserApp2_sUserLedCommandList;
     u32SystemTime = 0;
     AllLedsOff();
   }

  
  if(WasButtonPressed(BUTTON2))
  {
    UserApp2_bSystemRunning = !UserApp2_bSystemRunning;
    ButtonAcknowledge(BUTTON2);
  }

  if(WasButtonPressed(BUTTON3))
  {
    UserApp2_bSystemDark = !UserApp2_bSystemDark;
    ButtonAcknowledge(BUTTON3);
  }
  
  if(u32SystemTime > UserApp2_u32DemoListEndTime)
  {
    u32SystemTime = 0;
  }
  
  /* Handle an empty list */
  if( (UserApp2_psActiveList->u8ListSize == 0) ||
      (UserApp2_bSystemDark == TRUE) )
  {
    AllLedsOff();
  }
  else
  {
    /* Check for any LED that must change */
    while(psListParser != NULL)
    {
      if(psListParser->eCommand.u32Time == u32SystemTime)
      {
        /* Check for adjustment */
        if( (psListParser->eCommand.bOn == TRUE) )
        { 
          if(psListParser->eCommand.eCurrentRate != LED_PWM_100)
          {
            psListParser->eCommand.eCurrentRate += 2;
            psListParser->eCommand.u32Time += 20;

            /* Set the current rate */
           LedPWM(psListParser->eCommand.eLED, psListParser->eCommand.eCurrentRate);
          }
          else
          {
             psListParser->eCommand.u32Time -= 200;
             psListParser->eCommand.eCurrentRate = LED_PWM_0;
          }
        }

        if( (psListParser->eCommand.bOn == FALSE))
        {
         if(psListParser->eCommand.eCurrentRate != LED_PWM_0)
          {
            psListParser->eCommand.eCurrentRate -= 2;
            psListParser->eCommand.u32Time += 20;

            /* Set the current rate */
           LedPWM(psListParser->eCommand.eLED, psListParser->eCommand.eCurrentRate);
          }
          else
          {
            psListParser->eCommand.u32Time -= 200;
            psListParser->eCommand.eCurrentRate = LED_PWM_100;
          }
        }
       
      }

      psListParser = psListParser->psNextNode;
    }
  }

  if(UserApp2_bSystemRunning)
  {
    u32SystemTime++;
  }

} /* end UserApp2SM_Idle() */
 


#if 0
/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp2SM_Error(void)          
{
  
} /* end UserApp2SM_Error() */
#endif


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void UserApp2SM_FailedInit(void)          
{
    
} /* end UserApp2SM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/