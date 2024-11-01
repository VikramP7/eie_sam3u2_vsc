/*!*********************************************************************************************************************
@file user_app1.c
@brief User's tasks / applications are written here.  This description
should be replaced by something specific to the task.

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app1 as a template:
 1. Copy both user_app1.c and user_app1.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app1" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp1" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP1" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

------------------------------------------------------------------------------------------------------------------------
GLOBALS
- NONE

CONSTANTS
- NONE

TYPES
- NONE

PUBLIC FUNCTIONS
- NONE

PROTECTED FUNCTIONS
- void UserApp1Initialize(void)
- void UserApp1RunActiveState(void)


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>UserApp1"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags; /*!< @brief Global state flags */

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;    /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;     /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;      /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags; /*!< @brief From main.c */

/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_<type>" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_pfStateMachine; /*!< @brief The state machine function pointer */
// static u32 UserApp1_u32Timeout;                           /*!< @brief Timeout counter used across states */

/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn void UserApp1Initialize(void)

@brief
Initializes the State Machine and its variables.

Should only be called once in main init section.

Requires:
- NONE

Promises:
- NONE

*/
void UserApp1Initialize(void)
{
  HEARTBEAT_OFF();

  /* If good initialization, set state to Idle */
  if (1)
  {
    UserApp1_pfStateMachine = UserApp1SM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp1_pfStateMachine = UserApp1SM_Error;
  }

} /* end UserApp1Initialize() */

/*!----------------------------------------------------------------------------------------------------------------------
@fn void UserApp1RunActiveState(void)

@brief Selects and runs one iteration of the current state in the state machine.

All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
- State machine function pointer points at current state

Promises:
- Calls the function to pointed by the state machine function pointer

*/
void UserApp1RunActiveState(void)
{
  UserApp1_pfStateMachine();

} /* end UserApp1RunActiveState */

/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */
/*--------------------------------------------------------------------------------------------------------------------*/

/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/
/*-------------------------------------------------------------------------------------------------------------------*/
/* What does this state do? */
static void UserApp1SM_Idle(void)
{
  static u8 aau8Colour[][3] = {{RED0, 0xff, 0xff},     /*red*/
                               {RED0, GREEN0, 0xff},   /*yellow*/
                               {0xff, GREEN0, 0xff},   /*green*/
                               {0xff, GREEN0, BLUE0},  /*cyan*/
                               {0xff, 0xff, BLUE0},    /*blue*/
                               {RED0, 0xff, BLUE0},    /*purple*/
                               {RED0, GREEN0, BLUE0}}; /*white*/
  static u8 u8ButtonClickCounter = 0;
  static bool bLed1Flashing = FALSE;
  static LedRateType aeBlinkRate[] = {LED_1HZ, LED_2HZ, LED_4HZ, LED_8HZ};
  static u8 u8BlinkRateIndex = 0;

  // Button 0 controlling 3-bit binary counter in yellow lights
  if (WasButtonPressed(BUTTON0))
  {
    ButtonAcknowledge(BUTTON0);
    u8ButtonClickCounter++;
    for (u8 i = 0; i < (U8_TOTAL_LEDS - 1); i++)
    {
      LedOff((LedNameType)i);
    }
    for (u8 ledPos = 0; ledPos < 3; ledPos++)
    {
      if (u8ButtonClickCounter & (0x01 << ledPos))
      {
        for (u8 i = 0; i < 3; i++) // for rbg
        {
          if (aau8Colour[6][i]) // 6 is for white
          {
            LedOn(aau8Colour[6][i] + 2 - ledPos);
          }
        }
      }
    }
    u8ButtonClickCounter %= 8;
  }

  // button 1 held for two seconds starts flashing
  if (IsButtonHeld(BUTTON1, 2000))
  {
    if (!bLed1Flashing)
    {
      u8BlinkRateIndex = 0;
      LedBlink(RED0, aeBlinkRate[u8BlinkRateIndex]);
      bLed1Flashing = TRUE;
    }
    else
    {
      LedOff(RED0);
      bLed1Flashing = FALSE;
    }
  }

  if (WasButtonPressed(BUTTON1) && bLed1Flashing)
  {
    ButtonAcknowledge(BUTTON1);
    u8BlinkRateIndex++;
    u8BlinkRateIndex %= sizeof(aeBlinkRate) / sizeof(LedRateType);
    LedBlink(RED0, aeBlinkRate[u8BlinkRateIndex]);
  }

} /* end UserApp1SM_Idle() */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)
{

} /* end UserApp1SM_Error() */

/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
