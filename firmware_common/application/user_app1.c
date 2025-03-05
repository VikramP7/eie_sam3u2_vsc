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

static u32 UserApp1_u32DataMsgCount = 0; /*ANT_DATA packet counter*/
static u32 UserApp1_u32TickMsgCount = 0; /*ANT_TICK packet counter*/
static u32 UserApp1_u32TimeOut = 0;

/* New variables */
volatile u32 G_u32UserApp1Flags; /*!< @brief Global state flags */

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;    /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;     /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;      /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags; /*!< @brief From main.c */

extern u32 G_u32AntApiCurrentMessageTimeStamp;                           // From ant_api.c
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;           // From ant_api.c
extern u8 G_au8AntApiCurrentMessageBytes[ANT_APPLICATION_MESSAGE_BYTES]; // From ant_api.c
extern AntExtendedDataType G_sAntApiCurrentMessageExtData;               // From ant_api.c

/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_<type>" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_pfStateMachine; /*!< @brief The state machine function pointer */
// static u32 UserApp1_u32Timeout;                           /*!< @brief Timeout counter used across states */

/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/
static void UserApp1SM_WaitAntReady();
static void UserApp1SM_ChannelOpen();
static void UserApp1SM_WaitChannelOpen();
static void UserApp1SM_WaitChannelClose();

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
  PixelAddressType sStringLocation;
  u8 au8WelcomeMessage[] = "ANT Slave Demo";

  LedOn(RED0); /*Set Status LED to red as ant is unconfigured*/

  AntAssignChannelInfoType sChannelInfo;
  if (AntRadioStatusChannel(U8_ANT_CHANNEL_USERAPP) == ANT_UNCONFIGURED)
  {
    sChannelInfo.AntChannel = U8_ANT_CHANNEL_USERAPP;
    sChannelInfo.AntChannelType = CHANNEL_TYPE_SLAVE;
    sChannelInfo.AntChannelPeriodHi = U8_ANT_CHANNEL_PERIOD_HI_USERAPP;
    sChannelInfo.AntChannelPeriodLo = U8_ANT_CHANNEL_PERIOD_LO_USERAPP;

    sChannelInfo.AntDeviceIdHi = U8_ANT_DEVICE_ID_HI_USERAPP;
    sChannelInfo.AntDeviceIdLo = U8_ANT_DEVICE_ID_LO_USERAPP;
    sChannelInfo.AntDeviceType = U8_ANT_DEVICE_TYPE_USERAPP;
    sChannelInfo.AntTransmissionType = U8_ANT_TRANSMISSION_TYPE_USERAPP;

    sChannelInfo.AntFrequency = U8_ANT_FREQUENCY_USERAPP;
    sChannelInfo.AntTxPower = U8_ANT_TX_POWER_USERAPP;

    sChannelInfo.AntNetwork = ANT_NETWORK_DEFAULT;
    for (u8 i = 0; i < ANT_NETWORK_NUMBER_BYTES; i++)
    {
      sChannelInfo.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
    }
  } /*END radio initilization*/

  sStringLocation.u16PixelColumnAddress = U16_LCD_CENTER_COLUMN - (strlen((char const *)au8WelcomeMessage) * (U8_LCD_SMALL_FONT_COLUMNS + U8_LCD_SMALL_FONT_SPACE) / 2);
  sStringLocation.u16PixelRowAddress = U8_LCD_SMALL_FONT_LINE7;

  static PixelBlockType G_sLcdClearLine7Mi =
      {
          .u16RowStart = U8_LCD_SMALL_FONT_LINE7,
          .u16ColumnStart = 0,
          .u16RowSize = U8_LCD_SMALL_FONT_ROWS,
          .u16ColumnSize = U16_LCD_COLUMNS};

  LcdClearPixels(&G_sLcdClearLine7Mi);
  LcdLoadString(&au8WelcomeMessage, LCD_FONT_SMALL, &sStringLocation);

  /* If good initialization, set state to Idle */
  if (AntAssignChannel(&sChannelInfo))
  {
    LedOn(RED0);
    LedOn(GREEN0);
    LedOff(BLUE0);
    UserApp1_pfStateMachine = UserApp1SM_WaitAntReady;
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
static void UserApp1SM_WaitAntReady(void)
{
  if (AntRadioStatusChannel(U8_ANT_CHANNEL_USERAPP) == ANT_CONFIGURED)
  {
    UserApp1_pfStateMachine = UserApp1SM_Idle;
  }
} /* end UserApp1SM_WaitAntReady() */

/* What does this state do? */
static void UserApp1SM_Idle(void)
{
  if (WasButtonPressed(BUTTON0))
  {
    ButtonAcknowledge(BUTTON0);

    AntOpenChannelNumber(U8_ANT_CHANNEL_USERAPP);
    LedOff(RED0);
    LedOff(GREEN0);
    LedOff(BLUE0);
    LedBlink(GREEN0, LED_2HZ);

    UserApp1_u32TimeOut = G_u32SystemTime1ms;
    UserApp1_pfStateMachine = UserApp1SM_WaitChannelOpen;
  }
} /* end UserApp1SM_Idle() */

static void UserApp1SM_WaitChannelOpen()
{
  if (AntRadioStatusChannel(U8_ANT_CHANNEL_USERAPP) == ANT_OPEN)
  {
    LedOn(GREEN0);
    UserApp1_pfStateMachine = UserApp1SM_ChannelOpen;
  }

  if (IsTimeUp(&UserApp1_u32TimeOut, U32_TIMEOUT_OPEN_CHANNEL))
  {
    AntCloseChannelNumber(U8_ANT_CHANNEL_USERAPP);
    // no longer green
    LedOff(GREEN0);

    // turn on yellow led
    LedOn(RED0);
    LedOn(GREEN0);

    UserApp1_pfStateMachine = UserApp1SM_Idle;
  }
} /* end UserApp1SM_WaitChannelOpen() */

static void UserApp1SM_ChannelOpen()
{
  static u8 u8LastState = 0xff;
  static u8 au8TickMessage[] = "Event x\n\r";

  static u8 au8TestMessage[] = {0, 0, 0, 0, 0xA5, 0, 0, 0};
  static u8 au8LastAntData[ANT_APPLICATION_MESSAGE_BYTES] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  static PixelAddressType sStringLocation;
  u8 au8DataContent[] = "xxxxxxxxxxxxxxxx";
  bool bGotNewData;

  /*Check if button 0 is pressed to close channel*/
  if (WasButtonPressed(BUTTON0))
  {
    ButtonAcknowledge(BUTTON0);

    AntCloseChannelNumber(U8_ANT_CHANNEL_USERAPP);

    LedOff(BLUE0);
    LedBlink(GREEN0, LED_2HZ);

    /*Start a timer for channel close, and change states*/
    UserApp1_u32TimeOut = G_u32SystemTime1ms;
    UserApp1_pfStateMachine = UserApp1SM_WaitChannelClose;
  }

  /*Check if search for master time out has occured*/
  if (AntRadioStatusChannel(U8_ANT_CHANNEL_USERAPP) != ANT_OPEN)
  {
    u8LastState = 0xff;
    LedBlink(GREEN0, LED_2HZ);
    LedOff(BLUE0);

    UserApp1_u32TimeOut = G_u32SystemTime1ms;
    UserApp1_pfStateMachine = UserApp1SM_WaitChannelClose;
  }

  if (AntReadAppMessageBuffer())
  {
    if (G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      // we have data
      LedOff(GREEN0);
      LedOn(BLUE0);
      u8LastState = 0xff;
      UserApp1_u32DataMsgCount++;

      bGotNewData = FALSE;
      for (u8 i = 0; i < ANT_APPLICATION_MESSAGE_BYTES; i++)
      {
        if (G_au8AntApiCurrentMessageBytes[i] != au8LastAntData[i])
        {
          bGotNewData = TRUE;
          au8LastAntData[i] = G_au8AntApiCurrentMessageBytes;

          au8DataContent[2 * i] = HexToASCIICharUpper(G_au8AntApiCurrentMessageBytes[i] / 16);
          au8DataContent[2 * i + 1] = HexToASCIICharUpper(G_au8AntApiCurrentMessageBytes[i] % 16);
        }
      }

      if (bGotNewData)
      {
        // display to screen
        sStringLocation.u16PixelColumnAddress = U16_LCD_CENTER_COLUMN - (strlen((char const *)au8DataContent) * (U8_LCD_SMALL_FONT_COLUMNS + U8_LCD_SMALL_FONT_SPACE) / 2);
        sStringLocation.u16PixelRowAddress = U8_LCD_SMALL_FONT_LINE7;

        static PixelBlockType G_sLcdClearLine7Mi =
            {
                .u16RowStart = U8_LCD_SMALL_FONT_LINE7,
                .u16ColumnStart = 0,
                .u16RowSize = U8_LCD_SMALL_FONT_ROWS,
                .u16ColumnSize = U16_LCD_COLUMNS};

        LcdClearPixels(&G_sLcdClearLine7Mi);
        LcdLoadString(&au8DataContent, LCD_FONT_SMALL, &sStringLocation);

        // update our local message counter and send the message back
        au8TestMessage[7]++;
        if (au8TestMessage[7] == 0)
        {
          au8TestMessage[6]++;
          if (au8TestMessage[6] == 0)
          {
            au8TestMessage[5]++;
          }
        }
        // AntQueueBroadcastMessage(U8_ANT_CHANNEL_USERAPP, au8TestMessage);
      }
    } /*end if ant data*/
    else if (G_eAntApiCurrentMessageClass == ANT_TICK)
    {
      // update tick counter
      UserApp1_u32TickMsgCount++;

      // check if the state is new and therfore worth responding to
      if (u8LastState != G_au8AntApiCurrentMessageBytes[ANT_TICK_MSG_EVENT_CODE_INDEX])
      {

        // new state therefore update last state
        u8LastState = G_au8AntApiCurrentMessageBytes[ANT_TICK_MSG_EVENT_CODE_INDEX];
        au8TickMessage[6] = HexToASCIICharUpper(u8LastState);
        DebugPrintf(au8TickMessage);

        // parse last state event code:
        switch (u8LastState)
        {
        case RESPONSE_NO_ERROR:
          // no need to do anthing
          LedOn(RED1);
          break;
        // paired but missing messages blue blinks
        case EVENT_RX_FAIL:
          LedOff(GREEN0);
          LedOff(RED0);
          LedBlink(BLUE0, LED_2HZ);
          break;
        // Drop to search LED is green
        case EVENT_RX_FAIL_GO_TO_SEARCH:
          LedOff(RED0);
          LedOff(BLUE0);
          LedOn(GREEN0);
          break;
        case EVENT_RX_SEARCH_TIMEOUT:
          DebugPrintf("Search Timeout\r\n");
          break;
        default:
          DebugPrintf("unexpected event\r\n");
          break;
        } /*END switch(au8LastState)*/
      } /*end if */
    }
  } /* end AntReadAppMessageBuffer()*/
}

static void UserApp1SM_WaitChannelClose()
{
  if (AntRadioStatusChannel(U8_ANT_CHANNEL_USERAPP) == ANT_CLOSED)
  {
    LedOff(GREEN0);
    // turn on yellow
    LedOn(GREEN0);
    LedOn(RED0);
    // go back to idle state after closing channel
    UserApp1_pfStateMachine = UserApp1SM_Idle;
  }

  if (IsTimeUp(&UserApp1_u32TimeOut, U32_TIMEOUT_CLOSE_CHANNEL))
  {
    LedOff(GREEN0);
    LedOff(BLUE0);
    LedBlink(RED0, LED_4HZ);
    UserApp1_pfStateMachine = UserApp1SM_Error;
  }
}

static void UserApp1SM_ChannelOpenzzz()
{
  static u8 au8TestMessage[] = {0, 0, 0, 0, 0xA5, 0, 0, 0};

  static PixelAddressType sStringLocation;
  u8 au8DataContent[] = "xxxxxxxxxxxxxxxx";

  extern PixelBlockType G_sLcdClearLine7; /* from lcd-NHD-C12864LZ.c*/

  if (AntReadAppMessageBuffer())
  {
    if (G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      // we have data
      for (u8 i = 0; i < ANT_DATA_BYTES; i++)
      {
        au8DataContent[i * 2] = HexToASCIICharUpper(G_au8AntApiCurrentMessageBytes[i] / 16);
        au8DataContent[(i * 2) + 1] = HexToASCIICharUpper(G_au8AntApiCurrentMessageBytes[i] % 16);
      }

      sStringLocation.u16PixelColumnAddress = U16_LCD_CENTER_COLUMN - (strlen((char const *)au8DataContent) * (U8_LCD_SMALL_FONT_COLUMNS + U8_LCD_SMALL_FONT_SPACE) / 2);
      sStringLocation.u16PixelRowAddress = U8_LCD_SMALL_FONT_LINE7;
      LcdClearPixels(&G_sLcdClearLine7);
      LcdLoadString(au8DataContent, LCD_FONT_SMALL, &sStringLocation);
    }
    else if (G_eAntApiCurrentMessageClass == ANT_TICK)
    {
      // Channel period has occured time to send new data
      // handles button pressing
      au8TestMessage[0] = 0x00;
      au8TestMessage[1] = 0x00;
      au8TestMessage[2] = 0x00;
      au8TestMessage[3] = 0x00;
      if (IsButtonPressed(BUTTON0))
      {
        au8TestMessage[0] = 0xff;
      }
      if (IsButtonPressed(BUTTON1))
      {
        au8TestMessage[1] = 0xff;
      }

      // does mesage counter
      au8TestMessage[7]++;
      au8TestMessage[6] += au8TestMessage[7] == 0;
      au8TestMessage[5] += (au8TestMessage[7] == 0) && (au8TestMessage[6] == 0);
      // broadcast prepared message
      AntQueueBroadcastMessage(U8_ANT_CHANNEL_USERAPP, au8TestMessage);
    }
  } /* end AntReadAppMessageBuffer()*/
} /* end UserApp1SM_ChannelOpen() */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)
{
  LedBlink(RED0, LED_4HZ);
} /* end UserApp1SM_Error() */

/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
