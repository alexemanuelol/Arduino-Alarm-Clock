/*
 *      Author:     Alexander Emanuelsson
 *
 *      Brief:
 *
 *      Usage:
 */

/*
 *      Includes
 */
#include <MD_MAX72xx.h>
#include <SPI.h>

#include "numbers.h"

/*
 *      Defines
 */
#define CLK_PIN                     13 /* SCK */
#define DATA_PIN                    11 /* MOSI */
#define CS_PIN                      10 /* SS */
#define MODE_BUTTON_PIN             4
#define CONFIRM_BUTTON_PIN          2
#define BUZZER_PIN                  9

#define MAX_DEVICES                 4
#define MATRIX_WIDTH                (8 * MAX_DEVICES)
#define MATRIX_HEIGHT               8

#define HOURS_START_POS             0
#define MINUTES_START_POS           12
#define SECONDS_START_POS           24

#define SECOND_CORRECTION_MINUTES   110
#define DEBOUNCE_DELAY              50


/*
 *      Enums
 */
typedef enum {
    SMALL = 0,
    BIG
} numberSize;

typedef enum {
    STANDBY = 0,
    UPDATE_CLOCK,
    MODE_SET_TIME,
    MODE_SET_ALARM
} stateMachineState;

typedef enum {
    PRINT_MODE = 0,
    SELECTION,
    SET_HOURS,
    SET_MINUTES,
    SET_SECONDS,
    ACCEPT
} stateMachineMode;


/*
 *      Global variables
 */
MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::PAROLA_HW, CS_PIN, MAX_DEVICES);

uint8_t seconds = 0;
uint8_t minutes = 0;
uint8_t hours = 0;

uint8_t prevSeconds = 0;
uint8_t prevMinutes = 0;
uint8_t prevHours = 0;

uint8_t visualSeconds = 0;
uint8_t visualMinutes = 0;
uint8_t visualHours = 0;

uint8_t alarmSeconds = 0;
uint8_t alarmMinutes = 0;
uint8_t alarmHours = 0;

uint8_t minuteCounter = 0;
bool alternateColon = 0;

uint8_t modeButtonReading;
uint8_t confirmButtonReading;
uint8_t modeButtonState = 0;
uint8_t prevModeButtonState = 0;
uint8_t confirmButtonState = 0;
uint8_t prevConfirmButtonState = 0;
bool modeButtonClicked = false;
bool confirmButtonClicked = false;
unsigned long modeLastDebounceTime = 0;
unsigned long confirmLastDebounceTime = 0;

bool forceUpdate = false;

static stateMachineState state = STANDBY;

static stateMachineMode timeState = PRINT_MODE;
static stateMachineMode alarmState = PRINT_MODE;

/*
 *      Setup function
 */
void setup()
{
    initInterrupt();

    pinMode(MODE_BUTTON_PIN, INPUT);
    pinMode(CONFIRM_BUTTON_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    mx.begin();

    printHours(hours);
    printMinutes(minutes);
    printSeconds(seconds);
}

/*
 *      Main loop function
 */
void loop()
{
    buttonHandler();

    /*if (modeButtonState == HIGH)
    {
        tone(BUZZER_PIN, 500);
    }
    else
    {
        noTone(BUZZER_PIN);
    }*/


    mainStateMachine();
}

/*
 *      Main state machine function
 */
void mainStateMachine()
{
    switch (state)
    {
        case STANDBY:
            mx.update();
            if (modeButtonClicked == true)
            {
                modeButtonClicked = false;
                state = MODE_SET_TIME;
            }
            if (confirmButtonClicked == true)
            {
                confirmButtonClicked = false;
                /* Nothing */
            }
            if ( ((seconds != prevSeconds) || (minutes != prevMinutes) || (hours != prevHours)) && state != MODE_SET_TIME )
            {
                state = UPDATE_CLOCK;
            }
            break;

        case UPDATE_CLOCK:
            state = STANDBY;
            if (forceUpdate)
            {
                mx.clear();
            }

            if (seconds != prevSeconds || forceUpdate)
            {
                prevSeconds = seconds;
                printSeconds(seconds);
            }
            if (minutes != prevMinutes || forceUpdate)
            {
                prevMinutes = minutes;
                printMinutes(minutes);
            }
            if (hours != prevHours || forceUpdate)
            {
                prevHours = hours;
                printHours(hours);
            }

            if (alternateColon)
            {
                alternateColon = 0;
                mx.setColumn(10, 0x14);
            }
            else
            {
                alternateColon = 1;
                mx.setColumn(10, 0x00);
            }
            forceUpdate = false;
            break;
        
        case MODE_SET_TIME:
            switch (timeState)
            {
                case PRINT_MODE:
                    mx.clear();
                    mx.setColumn(0, 0xFF);
                    // print "Set Time"
                    timeState = SELECTION;
                    break;

                case SELECTION:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        timeState = PRINT_MODE;
                        state = MODE_SET_ALARM;
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        timeState = SET_HOURS;
                        visualHours = hours;
                        visualMinutes = minutes;
                        visualSeconds = seconds;
                        printHours(visualHours);
                        printMinutes(visualMinutes);
                        printSeconds(visualSeconds);
                    }
                    break;

                case SET_HOURS:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        visualHours = (visualHours + 1) % 24;
                        printHours(visualHours);
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        timeState = SET_MINUTES;
                    }
                    break;

                case SET_MINUTES:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        visualMinutes = (visualMinutes + 1) % 60;
                        printMinutes(visualMinutes);
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        timeState = SET_SECONDS;
                    }
                    break;

                case SET_SECONDS:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        visualSeconds = (visualSeconds + 1) % 60;
                        printSeconds(visualSeconds);
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        timeState = ACCEPT;
                    }
                    break;

                case ACCEPT:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        timeState = PRINT_MODE;
                        state = STANDBY;
                        forceUpdate = true;
                        hours = visualHours;
                        minutes = visualMinutes;
                        hours = visualHours;
                    }
                    break;

                default:
                    break;
            }
            break;

        case MODE_SET_ALARM:
            switch (alarmState)
            {
                case PRINT_MODE:
                    mx.clear();
                    mx.setColumn(0, 0xFF);
                    mx.setColumn(1, 0xFF);
                    // print "Set Alarm"
                    alarmState = SELECTION;
                    break;

                case SELECTION:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        alarmState = PRINT_MODE;
                        state = MODE_SET_TIME;
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        alarmState = SET_HOURS;
                        visualHours = hours;
                        visualMinutes = minutes;
                        visualSeconds = seconds;
                        printHours(visualHours);
                        printMinutes(visualMinutes);
                        printSeconds(visualSeconds);
                    }
                    break;

                case SET_HOURS:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        visualHours = (visualHours + 1) % 24;
                        printHours(visualHours);
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        alarmState = SET_MINUTES;
                    }
                    break;

                case SET_MINUTES:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        visualMinutes = (visualMinutes + 1) % 60;
                        printMinutes(visualMinutes);
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        alarmState = SET_SECONDS;
                    }
                    break;

                case SET_SECONDS:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        visualSeconds = (visualSeconds + 1) % 60;
                        printSeconds(visualSeconds);
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        alarmState = ACCEPT;
                    }
                    break;

                case ACCEPT:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        alarmState = PRINT_MODE;
                        state = STANDBY;
                        forceUpdate = true;
                        alarmHours = visualHours;
                        alarmMinutes = visualMinutes;
                        alarmHours = visualHours;
                    }
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }
}

/*
 *      Button handler
 */
void buttonHandler()
{
    modeButtonReading = digitalRead(MODE_BUTTON_PIN);
    confirmButtonReading = digitalRead(CONFIRM_BUTTON_PIN);

    if (modeButtonReading != prevModeButtonState)
    {
        modeLastDebounceTime = millis();
    }
    if (confirmButtonReading != prevConfirmButtonState)
    {
        confirmLastDebounceTime = millis();
    }

    if ( (millis() - modeLastDebounceTime) > DEBOUNCE_DELAY )
    {
        if (modeButtonReading != modeButtonState)
        {
            modeButtonState = modeButtonReading;

            if (modeButtonState == HIGH)
            {
                modeButtonClicked = true;
            }
        }
    }

    if ( (millis() - confirmLastDebounceTime) > DEBOUNCE_DELAY )
    {
        if (confirmButtonReading != confirmButtonState)
        {
            confirmButtonState = confirmButtonReading;

            if (confirmButtonState == HIGH)
            {
                confirmButtonClicked = true;
            }
        }
    }

    prevModeButtonState = modeButtonReading;
    prevConfirmButtonState = confirmButtonReading;
}

/*
 *      Interrupt Service Routine
 */
ISR(TIMER1_OVF_vect)
{
    /* Update time */
    seconds = seconds + 1;
    if (seconds == 60)
    {
        seconds = 0;
        minutes = minutes + 1;
        minuteCounter = minuteCounter + 1;
        if (minuteCounter == SECOND_CORRECTION_MINUTES)
        {
            minuteCounter = 0;
            seconds = seconds + 1;
        }
        if (minutes == 60)
        {
            minutes = 0;
            hours = hours + 1;
            if (hours == 24)
            {
                hours = 0;
            }
        }
    }
}

/*
 *      Interrupt initialization
 */
void initInterrupt()
{
    noInterrupts();     /* Disable all interrupts */
    TCCR1A = 0x00;
    TCCR1B = 0x00;
    TCNT1 = 0x0000;     /* Preload timer */
    TCCR1A = 0xA2;      /* fast 16 bit PWM */
    TCCR1B = 0x1c;      /* 256 Prescaler */
    ICR1 = 62500-1;
    TIMSK1 |= 0x01;     /* Enable timer overflow interrupt */
    interrupts();       /* Enable all interrupts */
}

/*
 *      Print a number on the display
 */
void printNumber(uint8_t startPos, uint8_t number, numberSize numSize)
{
    switch (numSize)
    {
        case SMALL:
            mx.setColumn(startPos, numbers_small[number][0]);
            mx.setColumn(startPos+1, numbers_small[number][1]);
            mx.setColumn(startPos+2, numbers_small[number][2]);
            break;
        case BIG:
            mx.setColumn(startPos, numbers_big[number][0]);
            mx.setColumn(startPos+1, numbers_big[number][1]);
            mx.setColumn(startPos+2, numbers_big[number][2]);
            mx.setColumn(startPos+3, numbers_big[number][3]);
            break;
        default:
            break;
    }
}

/*
 *      Print a number at the hours position
 */
void printHours(uint8_t hours)
{
    printNumber(HOURS_START_POS, (uint8_t) hours / 10, BIG);
    printNumber(HOURS_START_POS + 5, (uint8_t) hours % 10, BIG);
}

/*
 *      Print a number at the minutes position
 */
void printMinutes(uint8_t minutes)
{
    printNumber(MINUTES_START_POS, (uint8_t) minutes / 10, BIG);
    printNumber(MINUTES_START_POS + 5, (uint8_t) minutes % 10, BIG);
}

/*
 *      Print a number at the seconds position
 */
void printSeconds(uint8_t seconds)
{
    printNumber(SECONDS_START_POS, (uint8_t) seconds / 10, SMALL);
    printNumber(SECONDS_START_POS + 4, (uint8_t) seconds % 10, SMALL);
}
