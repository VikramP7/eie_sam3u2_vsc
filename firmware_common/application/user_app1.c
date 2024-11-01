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
  PWMAudioOn(BUZZER1);
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
  static u8 u8BPM = 60;
  static u16 u16notes[] = {F4, G4S, F4, F4, A4S, F4, D4S, F4, C5, F4, F4, C5S, C5, G4S, F4, C5, F5, F4, D4S, D4S, C4, G4, F4};
  static u8 u8noteLengths[] = {4, 3, 2, 1, 2, 2, 2, 4, 3, 2, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 1, 2, 6};
  // static u16 u16notes[] = {C4, D4, E4, F4, G4, A4, B4, C5};
  // static u8 u8noteLengths[] = {8, 8, 8, 8, 8, 8, 8, 8};
  static u8 u8noteIndex = 0;
  static u16 u16timeOnNotems = 0;
  static bool bmute = FALSE;

  // checks if we've played that note for long enough now
  if (u16timeOnNotems >= ((7500 * u8noteLengths[u8noteIndex]) / u8BPM))
  {
    u16timeOnNotems = 0;
    PWMAudioOff(BUZZER1);
    u8noteIndex++;
    u8noteIndex %= (sizeof(u16notes) / sizeof(u16));
  }
  // set freq for current note if note switch
  if (u16timeOnNotems == 10)
  {
    PWMAudioSetFrequency(BUZZER1, u16notes[u8noteIndex]);
    if (!bmute)
    {
      PWMAudioOn(BUZZER1);
    }
  }

  // check is the mute button was toggled
  if (WasButtonPressed(BUTTON0))
  {
    ButtonAcknowledge(BUTTON0);
    if (bmute)
    {
      PWMAudioOn(BUZZER1);
      bmute = FALSE;
    }
    else
    {
      PWMAudioOff(BUZZER1);
      bmute = TRUE;
    }
  }

  // check is the mute button was toggled
  if (WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    u8BPM += 20;
    if (u8BPM >= 130)
    {
      u8BPM = 40;
    }
  }
  // increases the total time the note has been on
  u16timeOnNotems++;
} /* end UserApp1SM_Idle() */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)
{

} /* end UserApp1SM_Error() */

/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
