/*
 *      Author:     Alexander Emanuelsson
 *
 *      Brief:      A simple alarm clock made with an arduino.
 */


/*
 *      Includes
 */
#include <MD_MAX72xx.h>
#include <SPI.h>

#include "characters.h"
#include "numbers.h"


/*
 *      Defines
 */
#define CLK_PIN                     13  /* SCK */
#define DATA_PIN                    11  /* MOSI */
#define CS_PIN                      10  /* SS */
#define MODE_BUTTON_PIN             4
#define CONFIRM_BUTTON_PIN          2
#define BUZZER_PIN                  9

#define HARDWARE_TYPE               MD_MAX72XX::PAROLA_HW
#define MAX_DEVICES                 4
#define DEVICE_WIDTH_HEIGHT         8

#define HOURS_START_POS             0
#define MINUTES_START_POS           12
#define SECONDS_START_POS           24

#define SECONDS_PER_MINUTE          60
#define MINUTES_PER_HOUR            60
#define HOURS_PER_DAY               24

#define SECOND_CORRECTION_MINUTES   110
#define DEBOUNCE_DELAY              50

#define MENU_TIMEOUT                7       /* Seconds */
#define ALARM_TIMEOUT               10      /* Minutes */
#define CORRECTION_SECONDS          30
#define CORRECTION_MINUTES          28


/*
 *      Enums
 */
typedef enum {
    BIG = 0,
    SMALL
} numberType;

typedef enum {
    STANDBY = 0,
    UPDATE_CLOCK,
    SET_TIME,
    SET_ALARM,
    SET_INTENSITY
} stateMainEnum;

typedef enum {
    TIME_PRINT_TEXT = 0,
    TIME_SELECTION,
    TIME_SET_HOURS,
    TIME_SET_MINUTES,
    TIME_SET_SECONDS,
    TIME_CONFIRMATION
} stateSetTimeEnum;

typedef enum {
    ALARM_PRINT_TEXT = 0,
    ALARM_SELECTION,
    ALARM_ADD_PRINT_TEXT,
    ALARM_ADD_SELECTION,
    ALARM_REMOVE_PRINT_TEXT,
    ALARM_REMOVE_SELECTION,
    ALARM_CANCEL_PRINT_TEXT,
    ALARM_CANCEL_SELECTION,
    ALARM_CHOOSE_ALARM_ADD,
    ALARM_CHOOSE_ALARM_REMOVE,
    ALARM_REMOVE,
    ALARM_SET_HOURS,
    ALARM_SET_MINUTES,
    ALARM_SET_SECONDS,
    ALARM_CONFIRMATION
} stateSetAlarmEnum;

typedef enum {
    INTENSITY_PRINT_TEXT = 0,
    INTENSITY_SELECTION,
    INTENSITY_SET
} stateIntensityEnum;

typedef enum {
    CHECK = 0,
    ALARM_TRIGGERED
} stateCheckAlarmEnum;


/*
 *      Structs
 */
struct Alarm {
    bool active;
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
};


/*
 *      Global variables
 */
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

const char *timeText = "TIME";
const char *alarmText = "ALARM";
const char *intensityText="LIGHT";
const char *addText = "ADD";
const char *removeText = "REMOVE";
const char *cancelText = "CANCEL";

bool timeUpdated = false;
bool secondsUpdated = false;
bool minutesUpdated = false;
bool hoursUpdated = false;

uint8_t seconds = 0;
uint8_t minutes = 0;
uint8_t hours = 0;
uint8_t visualSeconds = 0;
uint8_t visualMinutes = 0;
uint8_t visualHours = 0;

struct Alarm alarm1;
struct Alarm alarm2;
struct Alarm alarm3;
static uint8_t selectedAlarm = 0;

bool inMenu = false;
uint8_t menuTimeout = 0;
uint8_t alarmTimeoutSeconds = 0;
uint8_t alarmTimeoutMinutes = 0;
bool alarmTriggered = false;
uint8_t correctionSeconds = 0;
uint8_t correctionMinutes = 0;

uint8_t modeButtonReading;
uint8_t modeButtonState = 0;
uint8_t prevModeButtonState = 0;
unsigned long modeLastDebounceTime = 0;
bool modeButtonClicked = false;

uint8_t confirmButtonReading;
uint8_t confirmButtonState = 0;
uint8_t prevConfirmButtonState = 0;
unsigned long confirmLastDebounceTime = 0;
bool confirmButtonClicked = false;

bool toggle = false;
bool toggle2 = false;
bool forceUpdate = false;
uint8_t intensity = MAX_INTENSITY/2;

static stateMainEnum mainState = STANDBY;
static stateSetTimeEnum setTimeState = TIME_PRINT_TEXT;
static stateSetAlarmEnum setAlarmState = ALARM_PRINT_TEXT;
static stateIntensityEnum setIntensityState = INTENSITY_PRINT_TEXT;
static stateCheckAlarmEnum alarmCheckState = CHECK;


/*
 *      Setup function.
 */
void setup()
{
    initInterrupt();

    pinMode( MODE_BUTTON_PIN, INPUT );
    pinMode( CONFIRM_BUTTON_PIN, INPUT );
    pinMode( BUZZER_PIN, OUTPUT );
    noTone( BUZZER_PIN );

    /* Start the matrix display module. */
    mx.begin();

    /* Set initial time */
    printHours( hours, false );
    printMinutes( minutes, false );
    printSeconds( seconds, false );

    alarm1.active = false;
    alarm1.seconds = 0;
    alarm1.minutes = 0;
    alarm1.hours = 0;

    alarm2.active = false;
    alarm2.seconds = 0;
    alarm2.minutes = 0;
    alarm2.hours = 0;

    alarm3.active = false;
    alarm3.seconds = 0;
    alarm3.minutes = 0;
    alarm3.hours = 0;
}

/*
 *      Main loop function.
 */
void loop()
{
    buttonHandler();
    updateTime();
    updateCorrectionTime();
    checkCorrection();
    checkMenuTimeout();
    checkAlarmTimeout();
    checkAlarm();
    mainStateMachine();
}

/*
 *      Interrupt initialization.
 */
void initInterrupt()
{
    noInterrupts();     /* Disable all interrupts */
    TCCR1A = 0x00;
    TCCR1B = 0x00;
    TCNT1 = 0x0000;     /* Preload timer */
    TCCR1A = 0xA2;      /* fast 16 bit PWM */
    TCCR1B = 0x1c;      /* 256 Prescaler */
    ICR1 = 62500 - 1;
    TIMSK1 |= 0x01;     /* Enable timer overflow interrupt */
    interrupts();       /* Enable all interrupts */
}

/*
 *      Interrupt Service Routine.
 */
ISR( TIMER1_OVF_vect )
{
    seconds++;
    correctionSeconds++;
    timeUpdated = true;

    if ( inMenu )
    {
        menuTimeout++;
    }
    if ( alarmTriggered )
    {
        alarmTimeoutSeconds++;
    }

    toggle = !toggle;
}

/*
 *      Main state machine function.
 */
void mainStateMachine()
{
    switch ( mainState )
    {
        case STANDBY:
            mx.update();
            if ( isModeButtonClicked() )
            {
                inMenu = true;
                mainState = SET_TIME;
                return;
            }
            if ( isConfirmButtonClicked() )
            {
                /* Do nothing */
            }
            if ( ( secondsUpdated || minutesUpdated || hoursUpdated || forceUpdate ) && !alarmTriggered )
            {
                mainState = UPDATE_CLOCK;
            }
            break;

        case UPDATE_CLOCK:
            mainState = STANDBY;
            if ( forceUpdate )
            {
                mx.clear();
            }

            if ( secondsUpdated || forceUpdate )
            {
                secondsUpdated = false;
                printSeconds( seconds, false );
            }
            if ( minutesUpdated || forceUpdate )
            {
                minutesUpdated = false;
                printMinutes( minutes, false );
            }
            if ( hoursUpdated || forceUpdate )
            {
                hoursUpdated = false;
                printHours( hours, false );
            }

            /* Toggle the time colon. */
            if ( toggle )
            {
                mx.setColumn( 10, 0x14 );
            }
            else
            {
                mx.setColumn( 10, 0x00 );
            }
            forceUpdate = false;
            break;
        
        case SET_TIME:
            setTimeStateMachine();
            break;

        case SET_ALARM:
            setAlarmStateMachine();
            break;

        case SET_INTENSITY:
            setIntensityStateMachine();
            break;

        default:
            break;
    }
}

/*
 *      Set time state machine function.
 */
void setTimeStateMachine()
{
    switch ( setTimeState )
    {
        case TIME_PRINT_TEXT:
            mx.clear();
            printText( timeText );
            setTimeState = TIME_SELECTION;
            break;

        case TIME_SELECTION:
            if ( isModeButtonClicked() )
            {
                setTimeState = TIME_PRINT_TEXT;
                mainState = SET_ALARM;
                menuTimeout = 0;
            }
            else if ( isConfirmButtonClicked() )
            {
                inMenu = false;
                menuTimeout = 0;
                setTimeState = TIME_SET_HOURS;
                visualHours = 0;
                visualMinutes = 0;
                visualSeconds = 0;
                mx.clear();
                printTime( visualHours, visualMinutes, visualSeconds );
                printHours( visualHours, true );
                mx.setColumn( 4, 0x80 );
            }
            break;

        case TIME_SET_HOURS:
            if ( isModeButtonClicked() )
            {
                visualHours = ( visualHours + 1 ) % HOURS_PER_DAY;
                printHours( visualHours, true );
            }
            else if ( isConfirmButtonClicked() )
            {
                setTimeState = TIME_SET_MINUTES;
                mx.clear();
                printTime( visualHours, visualMinutes, visualSeconds );
                printMinutes( visualMinutes, true );
                mx.setColumn( 16, 0x80 );
            }
            break;

        case TIME_SET_MINUTES:
            if ( isModeButtonClicked() )
            {
                visualMinutes = ( visualMinutes + 1 ) % MINUTES_PER_HOUR;
                printMinutes( visualMinutes, true );
            }
            else if ( isConfirmButtonClicked() )
            {
                setTimeState = TIME_SET_SECONDS;
                mx.clear();
                printTime( visualHours, visualMinutes, visualSeconds );
                printSeconds( visualSeconds, true );
                mx.setColumn( 27, 0x40 );
            }
            break;

        case TIME_SET_SECONDS:
            if ( isModeButtonClicked() )
            {
                visualSeconds = ( visualSeconds + 1 ) % SECONDS_PER_MINUTE;
                printSeconds( visualSeconds, true );
            }
            else if ( isConfirmButtonClicked() )
            {
                setTimeState = TIME_CONFIRMATION;
                mx.clear();
                printTime( visualHours, visualMinutes, visualSeconds );
                mx.setRow( 7, 0xFF );
            }
            break;

        case TIME_CONFIRMATION:
            if ( isModeButtonClicked() )
            {
                setTimeState = TIME_PRINT_TEXT;
                mainState = STANDBY;
                forceUpdate = true;
            }
            else if ( isConfirmButtonClicked() )
            {
                setTimeState = TIME_PRINT_TEXT;
                mainState = STANDBY;
                forceUpdate = true;
                hours = visualHours;
                minutes = visualMinutes;
                seconds = visualSeconds;
            }
            break;

        default:
            break;
    }
}

/*
 *      Set alarm state machine function.
 */
void setAlarmStateMachine()
{
    switch ( setAlarmState )
    {
        case ALARM_PRINT_TEXT:
            mx.clear();
            printText( alarmText );
            setAlarmState = ALARM_SELECTION;
            break;

        case ALARM_SELECTION:
            if ( isModeButtonClicked() )
            {
                setAlarmState = ALARM_PRINT_TEXT;
                mainState = SET_INTENSITY;
                menuTimeout = 0;
            }
            else if ( isConfirmButtonClicked() )
            {
                setAlarmState = ALARM_ADD_PRINT_TEXT;
                menuTimeout = 0;
            }
            break;

        case ALARM_ADD_PRINT_TEXT:
            mx.clear();
            printText( addText );
            setAlarmState = ALARM_ADD_SELECTION;
            break;

        case ALARM_ADD_SELECTION:
            if ( isModeButtonClicked() )
            {
                setAlarmState = ALARM_REMOVE_PRINT_TEXT;
                menuTimeout = 0;
            }
            else if ( isConfirmButtonClicked() )
            {
                setAlarmState = ALARM_CHOOSE_ALARM_ADD;
                menuTimeout = 0;
                mx.clear();
                printAlarmSelection( selectedAlarm );
            }
            break;

        case ALARM_REMOVE_PRINT_TEXT:
            mx.clear();
            printText( removeText );
            setAlarmState = ALARM_REMOVE_SELECTION;
            break;

        case ALARM_REMOVE_SELECTION:
            if ( isModeButtonClicked() )
            {
                setAlarmState = ALARM_CANCEL_PRINT_TEXT;
                menuTimeout = 0;
            }
            else if ( isConfirmButtonClicked() )
            {
                setAlarmState = ALARM_CHOOSE_ALARM_REMOVE;
                menuTimeout = 0;
                mx.clear();
                printAlarmSelection( selectedAlarm );
            }
            break;

        case ALARM_CANCEL_PRINT_TEXT:
            mx.clear();
            printText( cancelText );
            setAlarmState = ALARM_CANCEL_SELECTION;
            break;

        case ALARM_CANCEL_SELECTION:
            if ( isModeButtonClicked() )
            {
                setAlarmState = ALARM_ADD_PRINT_TEXT;
                menuTimeout = 0;
            }
            else if ( isConfirmButtonClicked() )
            {
                inMenu = false;
                menuTimeout = 0;
                setAlarmState = ALARM_PRINT_TEXT;
                mainState = STANDBY;
                forceUpdate = true;
            }
            break;

        case ALARM_CHOOSE_ALARM_ADD:
            if ( isModeButtonClicked() )
            {
                menuTimeout = 0;
                selectedAlarm = ( selectedAlarm + 1 ) % 3;

                mx.clear();
                printAlarmSelection( selectedAlarm );
            }
            else if ( isConfirmButtonClicked() )
            {
                setAlarmState = ALARM_SET_HOURS;
                inMenu = false;
                menuTimeout = 0;
                if ( selectedAlarm == 0 )
                {
                    visualHours = alarm1.hours;
                    visualMinutes = alarm1.minutes;
                    visualSeconds = alarm1.seconds;
                }
                else if ( selectedAlarm == 1 )
                {
                    visualHours = alarm2.hours;
                    visualMinutes = alarm2.minutes;
                    visualSeconds = alarm2.seconds;
                }
                else if ( selectedAlarm == 2 )
                {
                    visualHours = alarm3.hours;
                    visualMinutes = alarm3.minutes;
                    visualSeconds = alarm3.seconds;
                }
                mx.clear();
                printTime( visualHours, visualMinutes, visualSeconds );
                printHours( visualHours, true );
                mx.setColumn( 4, 0x80 );
            }
            break;

        case ALARM_CHOOSE_ALARM_REMOVE:
            if ( isModeButtonClicked() )
            {
                menuTimeout = 0;
                selectedAlarm = ( selectedAlarm + 1 ) % 3;

                mx.clear();
                printAlarmSelection( selectedAlarm );
            }
            else if ( isConfirmButtonClicked() )
            {
                setAlarmState = ALARM_REMOVE;
            }
            break;

        case ALARM_REMOVE:
            if ( selectedAlarm == 0 )
            {
                alarm1.active = false;
                alarm1.seconds = 0;
                alarm1.minutes = 0;
                alarm1.hours = 0;
            }
            else if ( selectedAlarm == 1 )
            {
                alarm2.active = false;
                alarm2.seconds = 0;
                alarm2.minutes = 0;
                alarm2.hours = 0;
            }
            else if ( selectedAlarm == 2 )
            {
                alarm3.active = false;
                alarm3.seconds = 0;
                alarm3.minutes = 0;
                alarm3.hours = 0;
            }
            selectedAlarm = 0;
            inMenu = false;
            menuTimeout = 0;
            setAlarmState = ALARM_PRINT_TEXT;
            mainState = STANDBY;
            forceUpdate = true;
            break;

        case ALARM_SET_HOURS:
            if ( isModeButtonClicked() )
            {
                visualHours = ( visualHours + 1 ) % HOURS_PER_DAY;
                printHours( visualHours, true );
            }
            else if ( isConfirmButtonClicked() )
            {
                setAlarmState = ALARM_SET_MINUTES;
                mx.clear();
                printTime( visualHours, visualMinutes, visualSeconds );
                printMinutes( visualMinutes, true );
                mx.setColumn( 16, 0x80 );
            }
            break;

        case ALARM_SET_MINUTES:
            if ( isModeButtonClicked() )
            {
                visualMinutes = ( visualMinutes + 1 ) % MINUTES_PER_HOUR;
                printMinutes( visualMinutes, true );
            }
            else if ( isConfirmButtonClicked() )
            {
                setAlarmState = ALARM_SET_SECONDS;
                mx.clear();
                printTime( visualHours, visualMinutes, visualSeconds );
                printSeconds( visualSeconds, true );
                mx.setColumn( 27, 0x40 );
            }
            break;

        case ALARM_SET_SECONDS:
            if ( isModeButtonClicked() )
            {
                visualSeconds = ( visualSeconds + 1 ) % SECONDS_PER_MINUTE;
                printSeconds( visualSeconds, true );
            }
            else if ( isConfirmButtonClicked() )
            {
                setAlarmState = ALARM_CONFIRMATION;
                mx.clear();
                printTime( visualHours, visualMinutes, visualSeconds );
                mx.setRow( 7, 0xFF );
            }
            break;

        case ALARM_CONFIRMATION:
            if ( isModeButtonClicked() )
            {
                setAlarmState = ALARM_PRINT_TEXT;
                mainState = STANDBY;
                forceUpdate = true;
            }
            else if ( isConfirmButtonClicked() )
            {
                if ( selectedAlarm == 0 )
                {
                    alarm1.active = true;
                    alarm1.seconds = visualSeconds;
                    alarm1.minutes = visualMinutes;
                    alarm1.hours = visualHours;
                }
                else if ( selectedAlarm == 1 )
                {
                    alarm2.active = true;
                    alarm2.seconds = visualSeconds;
                    alarm2.minutes = visualMinutes;
                    alarm2.hours = visualHours;
                }
                else if ( selectedAlarm == 2 )
                {
                    alarm3.active = true;
                    alarm3.seconds = visualSeconds;
                    alarm3.minutes = visualMinutes;
                    alarm3.hours = visualHours;
                }
                selectedAlarm = 0;
                setAlarmState = ALARM_PRINT_TEXT;
                mainState = STANDBY;
                forceUpdate = true;
            }
            break;

        default:
            break;
    }

}

/*
 *      Set intensity state machine function.
 */
void setIntensityStateMachine()
{
    switch ( setIntensityState )
    {
        case INTENSITY_PRINT_TEXT:
            mx.clear();
            printText( intensityText );
            setIntensityState = INTENSITY_SELECTION;
            break;

        case INTENSITY_SELECTION:
            if ( isModeButtonClicked() )
            {
                setIntensityState = INTENSITY_PRINT_TEXT;
                mainState = STANDBY;
                forceUpdate = true;
                inMenu = false;
                menuTimeout = 0;
            }
            else if ( isConfirmButtonClicked() )
            {
                inMenu = false;
                menuTimeout = 0;
                setIntensityState = INTENSITY_SET;
                mx.clear();
                for ( uint8_t i = 0; i < DEVICE_WIDTH_HEIGHT; i++ )
                {
                    mx.setRow( i, 0xFF );
                }
            }
            break;

        case INTENSITY_SET:
            if ( isModeButtonClicked() )
            {
                intensity = ( intensity + 1 ) % MAX_INTENSITY;
                mx.control( MD_MAX72XX::INTENSITY, intensity );
            }
            else if ( isConfirmButtonClicked() )
            {
                setIntensityState = INTENSITY_PRINT_TEXT;
                mainState = STANDBY;
                forceUpdate = true;
            }
            break;

        default:
            break;
    }
}

/*
 *      Check alarm function.
 */
void checkAlarm()
{
    switch ( alarmCheckState )
    {
        case CHECK:
            if ( alarm1.active )
            {
                if ( seconds == alarm1.seconds && minutes == alarm1.minutes && hours == alarm1.hours )
                {
                    alarmCheckState = ALARM_TRIGGERED;
                    alarmTriggered = true;
                    mx.control( MD_MAX72XX::INTENSITY, MAX_INTENSITY );
                }
            }
            if ( alarm2.active )
            {
                if ( seconds == alarm2.seconds && minutes == alarm2.minutes && hours == alarm2.hours )
                {
                    alarmCheckState = ALARM_TRIGGERED;
                    alarmTriggered = true;
                    mx.control( MD_MAX72XX::INTENSITY, MAX_INTENSITY );
                }
            }
            if ( alarm3.active )
            {
                if ( seconds == alarm3.seconds && minutes == alarm3.minutes && hours == alarm3.hours )
                {
                    alarmCheckState = ALARM_TRIGGERED;
                    alarmTriggered = true;
                    mx.control( MD_MAX72XX::INTENSITY, MAX_INTENSITY );
                }
            }
            break;

        case ALARM_TRIGGERED:
            if ( modeButtonClicked || confirmButtonClicked || !alarmTriggered )
            {
                alarmTimeoutMinutes = 0;
                alarmTimeoutSeconds = 0;
                modeButtonClicked = false;
                confirmButtonClicked = false;
                alarmTriggered = false;
                alarmCheckState = CHECK;
                noTone( BUZZER_PIN );
                forceUpdate = true;
                mx.control( MD_MAX72XX::INTENSITY, intensity );
                return;
            }
            
            if ( toggle )
            {
                if ( toggle != toggle2 )
                {
                    tone( BUZZER_PIN, 500 );
                    for ( uint8_t i = 0; i < DEVICE_WIDTH_HEIGHT; i++ )
                    {
                        mx.setRow( i, 0xFF );
                    }
                    toggle2 = toggle;
                }
            }
            else
            {
                if ( toggle != toggle2 )
                {
                    noTone( BUZZER_PIN );
                    mx.clear();
                    printTime( hours, minutes, seconds );
                    toggle2 = toggle;
                }
            }
            break;
        
        default:
            break;
    }
}

/*
 *      Check the menu timeout.
 */
void checkMenuTimeout()
{
    if ( inMenu )
    {
        if ( menuTimeout == MENU_TIMEOUT )
        {
            inMenu = false;
            menuTimeout = 0;
            mainState = STANDBY;
            setTimeState = TIME_PRINT_TEXT;
            setAlarmState = ALARM_PRINT_TEXT;
            selectedAlarm = 0;
            forceUpdate = true;
        }
    }
}

/*
 *      Check the alarm timeout
 */
void checkAlarmTimeout()
{
    if ( alarmTriggered )
    {
        if ( alarmTimeoutSeconds == SECONDS_PER_MINUTE )
        {
            alarmTimeoutSeconds = 0;
            alarmTimeoutMinutes++;
            if ( alarmTimeoutMinutes == ALARM_TIMEOUT )
            {
                alarmTriggered = false;
            }
        }
    }
}

/*
 *      Check the correction time.
 */
void checkCorrection()
{
    if ( correctionSeconds == CORRECTION_SECONDS && correctionMinutes == CORRECTION_MINUTES )
    {
        correctionSeconds = 0;
        correctionMinutes = 0;
        timeUpdated = true;
        seconds++;
        updateTime();
    }
}

/*
 *      Update the correction time variables.
 */
void updateCorrectionTime()
{
    if ( correctionSeconds == SECONDS_PER_MINUTE )
    {
        correctionSeconds = 0;
        correctionMinutes++;
    }
}

/*
 *      Update the time variables.
 */
void updateTime()
{
    /* Check if time has been updated. */
    if ( timeUpdated )
    {
        timeUpdated = false;
        if ( seconds == SECONDS_PER_MINUTE )
        {
            seconds = 0;
            minutes++;
            if ( minutes == MINUTES_PER_HOUR )
            {
                minutes = 0;
                hours++;
                if ( hours == HOURS_PER_DAY )
                {
                    hours = 0;
                }
                hoursUpdated = true;
            }
            minutesUpdated = true;
        }
        secondsUpdated = true;
    }
}

/*
 *      Button handler.
 */
void buttonHandler()
{
    modeButtonReading = digitalRead( MODE_BUTTON_PIN );
    confirmButtonReading = digitalRead( CONFIRM_BUTTON_PIN );

    if ( modeButtonReading != prevModeButtonState )
    {
        modeLastDebounceTime = millis();
    }
    if ( confirmButtonReading != prevConfirmButtonState )
    {
        confirmLastDebounceTime = millis();
    }

    if ( ( millis() - modeLastDebounceTime ) > DEBOUNCE_DELAY )
    {
        if ( modeButtonReading != modeButtonState )
        {
            modeButtonState = modeButtonReading;

            if ( modeButtonState == HIGH )
            {
                modeButtonClicked = true;
            }
        }
    }

    if ( ( millis() - confirmLastDebounceTime ) > DEBOUNCE_DELAY )
    {
        if ( confirmButtonReading != confirmButtonState )
        {
            confirmButtonState = confirmButtonReading;

            if ( confirmButtonState == HIGH )
            {
                confirmButtonClicked = true;
            }
        }
    }

    prevModeButtonState = modeButtonReading;
    prevConfirmButtonState = confirmButtonReading;
}

/*
 *      Returns true if the button has been clicked.
 */
bool isModeButtonClicked()
{
    if ( modeButtonClicked == true )
    {
        modeButtonClicked = false;
        return true;
    }
    return false;
}

/*
 *      Returns true if the button has been clicked.
 */
bool isConfirmButtonClicked()
{
    if ( confirmButtonClicked == true )
    {
        confirmButtonClicked = false;
        return true;
    }
    return false;
}

/*
 *      Print a number at the hours position on the matrix display.
 */
void printHours( uint8_t hours, bool underscore )
{
    printNumber( hours / 10, HOURS_START_POS, BIG, underscore );
    printNumber( hours % 10, HOURS_START_POS + 5, BIG, underscore );
}

/*
 *      Print a number at the minutes position on the matrix display.
 */
void printMinutes( uint8_t minutes, bool underscore )
{
    printNumber( minutes / 10, MINUTES_START_POS, BIG, underscore );
    printNumber( minutes % 10, MINUTES_START_POS + 5, BIG, underscore );
}

/*
 *      Print a number at the seconds position on the matrix display.
 */
void printSeconds( uint8_t seconds, bool underscore )
{
    printNumber( seconds / 10, SECONDS_START_POS, SMALL, underscore );
    printNumber( seconds % 10, SECONDS_START_POS + 4, SMALL, underscore );
}

/*
 *      Print time to the matrix display.
 */
void printTime( uint8_t hours, uint8_t minutes, uint8_t seconds )
{
    printHours( hours, false );
    printMinutes( minutes, false );
    printSeconds( seconds, false );
}

/*
 *      Print alarm selection display.
 */
void printAlarmSelection( uint8_t highlight )
{
    switch ( highlight )
    {
        case 0:
            printNumber( 1, 5, BIG, true );
            printNumber( 2, 14, BIG, false );
            printNumber( 3, 23, BIG, false );
            break;
        case 1:
            printNumber( 1, 5, BIG, false );
            printNumber( 2, 14, BIG, true );
            printNumber( 3, 23, BIG, false );
            break;
        case 2:
            printNumber( 1, 5, BIG, false );
            printNumber( 2, 14, BIG, false );
            printNumber( 3, 23, BIG, true );
            break;
        default:
            break;
    }
}

/*
 *      Print a number on the matrix display.
 */
void printNumber( uint8_t number, uint8_t position, numberType numberType, bool underscore )
{
    switch ( numberType )
    {
        case BIG:
            if ( underscore )
            {
                mx.setColumn( position, numbersBig[ number ][ 0 ] | 0x80 );
                mx.setColumn( position + 1, numbersBig[ number ][ 1 ] | 0x80 );
                mx.setColumn( position + 2, numbersBig[ number ][ 2 ] | 0x80 );
                mx.setColumn( position + 3, numbersBig[ number ][ 3 ] | 0x80 );
            }
            else
            {
                mx.setColumn( position, numbersBig[ number ][ 0 ] );
                mx.setColumn( position + 1, numbersBig[ number ][ 1 ] );
                mx.setColumn( position + 2, numbersBig[ number ][ 2 ] );
                mx.setColumn( position + 3, numbersBig[ number ][ 3 ] );
            }
            break;

        case SMALL:
            if ( underscore )
            {
                mx.setColumn( position, numbersSmall[ number ][ 0 ] | 0x40 );
                mx.setColumn( position + 1, numbersSmall[ number ][ 1 ] | 0x40 );
                mx.setColumn( position + 2, numbersSmall[ number ][ 2 ] | 0x40 );
            }
            else
            {
                mx.setColumn( position, numbersSmall[ number ][ 0 ] );
                mx.setColumn( position + 1, numbersSmall[ number ][ 1 ] );
                mx.setColumn( position + 2, numbersSmall[ number ][ 2 ] );
            }
            break;

        default:
            break;
    }
}

/*
 *      Print a text on the matrix display.
 */
void printText( const char *pMsg )
{
    mx.clear();
    uint8_t cursor = 0;
    uint8_t index;

    for ( uint8_t i = 0; i < strlen( pMsg ); i++ )
    {
        if ( pMsg[ i ] >= 0x20 && pMsg[ i ] <= 0x7E )
        {
            index = pMsg[ i ] - 0x20;

            if ( pMsg[ i ] == 0x20 )      /* SPACE */
            {
                mx.setColumn( cursor++, characters[ index ][ 0 ] );
                mx.setColumn( cursor++, characters[ index ][ 1 ] );
                cursor++;
                continue;
            }
            else if ( pMsg[ i ] == 0x22 ) /* " */
            {
                mx.setColumn( cursor++, characters[index][0] );
                mx.setColumn( cursor++, characters[index][1] );
                mx.setColumn( cursor++, characters[index][2] );
                cursor++;
                continue;
            }

            for ( uint8_t j = 0; j < CHARACTER_WIDTH; j++ )
            {
                if ( characters[ index ][ j ]  != 0x00 )
                {
                    mx.setColumn( cursor++, characters[ index ][ j ] );
                }
            }
            cursor++;
        }
    }
}
