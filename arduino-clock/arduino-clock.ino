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
#include "characters.h"

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

#define MENU_TIMEOUT                7

#define HARDWARE_TYPE               MD_MAX72XX::PAROLA_HW

#define BUF_SIZE                    75

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
    MODE_SET_ALARM,
    MODE_SET_INTENSITY
} stateMachineState;

typedef enum {
    PRINT_MODE = 0,
    SELECTION,
    SET_HOURS,
    SET_MINUTES,
    SET_SECONDS,
    ACCEPT
} stateMachineMode;

typedef enum {
    PRINT = 0,
    SELECT,
    SET_INTENSITY
} stateIntensity;

typedef enum {
    CHECK = 0,
    ALARM_TRIGGERED
} stateAlarmState;


/*
 *      Global variables
 */
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

const char *timeText = "TIME";
const char *alarmText = "ALARM";
const char *intensityText="LIGHT";

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

uint8_t secondCounter = 0;
uint8_t minuteCounter = 0;

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

bool inMenu = false;
uint8_t menuTimeout = 0;

uint8_t intensity = MAX_INTENSITY/2;

bool alarmSet = false;

bool toggle = false;

bool forceUpdate = false;

static stateMachineState state = STANDBY;
static stateMachineMode timeState = PRINT_MODE;
static stateMachineMode alarmState = PRINT_MODE;
static stateIntensity intensityState = PRINT;
static stateAlarmState alarmCheckState = CHECK;

/*
 *      Setup function
 */
void setup()
{
    initInterrupt();

    pinMode(MODE_BUTTON_PIN, INPUT);
    pinMode(CONFIRM_BUTTON_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    noTone(BUZZER_PIN);

    mx.begin();

    printHours(hours, false);
    printMinutes(minutes, false);
    printSeconds(seconds, false);
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
 *      Main loop function
 */
void loop()
{
    buttonHandler();
    checkAlarm();
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
                inMenu = true;
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
                printSeconds(seconds, false);
            }
            if (minutes != prevMinutes || forceUpdate)
            {
                prevMinutes = minutes;
                printMinutes(minutes, false);
            }
            if (hours != prevHours || forceUpdate)
            {
                prevHours = hours;
                printHours(hours, false);
            }

            if (toggle)
            {
                mx.setColumn(10, 0x14);
            }
            else
            {
                mx.setColumn(10, 0x00);
            }
            forceUpdate = false;
            break;
        
        case MODE_SET_TIME:
            switch (timeState)
            {
                case PRINT_MODE:
                    mx.clear();
                    printText(timeText);
                    timeState = SELECTION;
                    break;

                case SELECTION:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        timeState = PRINT_MODE;
                        state = MODE_SET_ALARM;
                        menuTimeout = 0;
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        inMenu = false;
                        menuTimeout = 0;
                        timeState = SET_HOURS;
                        visualHours = hours;
                        visualMinutes = minutes;
                        visualSeconds = seconds;
                        mx.clear();
                        printTime(visualHours, visualMinutes, visualSeconds);
                        printHours(visualHours, true);
                        mx.setColumn(4, 0x80);
                    }
                    break;

                case SET_HOURS:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        visualHours = (visualHours + 1) % 24;
                        printHours(visualHours, true);
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        timeState = SET_MINUTES;
                        mx.clear();
                        printTime(visualHours, visualMinutes, visualSeconds);
                        printMinutes(visualMinutes, true);
                        mx.setColumn(16, 0x80);
                    }
                    break;

                case SET_MINUTES:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        visualMinutes = (visualMinutes + 1) % 60;
                        printMinutes(visualMinutes, true);
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        timeState = SET_SECONDS;
                        mx.clear();
                        printTime(visualHours, visualMinutes, visualSeconds);
                        printSeconds(visualSeconds, true);
                        mx.setColumn(27, 0x40);
                    }
                    break;

                case SET_SECONDS:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        visualSeconds = (visualSeconds + 1) % 60;
                        printSeconds(visualSeconds, true);
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        timeState = ACCEPT;
                        mx.clear();
                        printTime(visualHours, visualMinutes, visualSeconds);
                    }
                    break;

                case ACCEPT:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        timeState = PRINT_MODE;
                        state = STANDBY;
                        forceUpdate = true;
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        timeState = PRINT_MODE;
                        state = STANDBY;
                        forceUpdate = true;
                        hours = visualHours;
                        minutes = visualMinutes;
                        seconds = visualSeconds;
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
                    printText(alarmText);
                    alarmState = SELECTION;
                    break;

                case SELECTION:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        alarmState = PRINT_MODE;
                        state = MODE_SET_INTENSITY;
                        menuTimeout = 0;
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        inMenu = false;
                        menuTimeout = 0;
                        alarmState = SET_HOURS;
                        visualHours = hours;
                        visualMinutes = minutes;
                        visualSeconds = seconds;
                        mx.clear();
                        printTime(visualHours, visualMinutes, visualSeconds);
                        printHours(visualHours, true);
                        mx.setColumn(4, 0x80);
                    }
                    break;

                case SET_HOURS:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        visualHours = (visualHours + 1) % 24;
                        printHours(visualHours, true);
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        alarmState = SET_MINUTES;
                        mx.clear();
                        printTime(visualHours, visualMinutes, visualSeconds);
                        printMinutes(visualMinutes, true);
                        mx.setColumn(16, 0x80);
                    }
                    break;

                case SET_MINUTES:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        visualMinutes = (visualMinutes + 1) % 60;
                        printMinutes(visualMinutes, true);
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        alarmState = SET_SECONDS;
                        mx.clear();
                        printTime(visualHours, visualMinutes, visualSeconds);
                        printSeconds(visualSeconds, true);
                        mx.setColumn(27, 0x40);
                    }
                    break;

                case SET_SECONDS:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        visualSeconds = (visualSeconds + 1) % 60;
                        printSeconds(visualSeconds, true);
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        alarmState = ACCEPT;
                        mx.clear();
                        printTime(visualHours, visualMinutes, visualSeconds);
                    }
                    break;

                case ACCEPT:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        alarmState = PRINT_MODE;
                        state = STANDBY;
                        forceUpdate = true;
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        alarmState = PRINT_MODE;
                        state = STANDBY;
                        forceUpdate = true;
                        alarmHours = visualHours;
                        alarmMinutes = visualMinutes;
                        alarmSeconds = visualSeconds;
                        alarmSet = true;
                    }
                    break;

                default:
                    break;
            }
            break;

        case MODE_SET_INTENSITY:
            switch (intensityState)
            {
                case PRINT:
                    mx.clear();
                    printText(intensityText);
                    intensityState = SELECT;
                    break;

                case SELECT:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        intensityState = PRINT;
                        state = STANDBY;
                        forceUpdate = true;
                        inMenu = false;
                        menuTimeout = 0;
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        inMenu = false;
                        menuTimeout = 0;
                        intensityState = SET_INTENSITY;
                        mx.clear();
                        for (uint8_t i = 0; i < MATRIX_HEIGHT; i++)
                        {
                            mx.setRow(i, 0xFF);
                        }
                    }
                    break;

                case SET_INTENSITY:
                    if (modeButtonClicked == true)
                    {
                        modeButtonClicked = false;
                        intensity = (intensity + 1) % MAX_INTENSITY;
                        mx.control(MD_MAX72XX::INTENSITY, intensity);
                    }
                    else if (confirmButtonClicked == true)
                    {
                        confirmButtonClicked = false;
                        intensityState = PRINT;
                        state = STANDBY;
                        forceUpdate = true;
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
 *      Check alarm function
 */
void checkAlarm()
{
    switch (alarmCheckState)
    {
        case CHECK:
            if (alarmSet)
            {
                if (seconds == alarmSeconds && minutes == alarmMinutes && hours == alarmHours)
                {
                    alarmCheckState = ALARM_TRIGGERED;
                }
            }
            break;

        case ALARM_TRIGGERED:
            if (modeButtonClicked || confirmButtonClicked)
            {
                modeButtonClicked = false;
                confirmButtonClicked = false;
                alarmCheckState = CHECK;
                noTone(BUZZER_PIN);
                forceUpdate = true;
                return;
            }
            
            if (toggle)
            {
                tone(BUZZER_PIN, 500);
                for (uint8_t i = 0; i < MATRIX_HEIGHT; i++)
                {
                    mx.setRow(i, 0xFF);
                }
            }
            else
            {
                noTone(BUZZER_PIN);
                forceUpdate = true;
            }
            break;
        
        default:
            break;
    }

}

/*
 *      Interrupt Service Routine
 */
ISR(TIMER1_OVF_vect)
{
    if (inMenu)
    {
        if (menuTimeout == MENU_TIMEOUT)
        {
            menuTimeout = 0;
            state = STANDBY;
            timeState = PRINT_MODE;
            alarmState = PRINT_MODE;
            forceUpdate = true;
            inMenu = false;
        }
        else
        {
            menuTimeout++;
        }
    }

    /* Update time */
    secondCounter++;
    seconds++;
    if (seconds == 60)
    {
        seconds = 0;
        minutes = minutes + 1;
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
    if (secondCounter == 60)
    {
        secondCounter = 0;
        minuteCounter++;
        if (minuteCounter == SECOND_CORRECTION_MINUTES)
        {
            minuteCounter = 0;
            seconds++;
        }

    }

    toggle = !toggle;
}

/*
 *      Print a number on the display
 */
void printNumber(uint8_t startPos, uint8_t number, numberSize numSize, bool underscore)
{
    switch (numSize)
    {
        case SMALL:
            if (underscore)
            {
                mx.setColumn(startPos, numbers_small[number][0] | 0x40);
                mx.setColumn(startPos+1, numbers_small[number][1] | 0x40);
                mx.setColumn(startPos+2, numbers_small[number][2] | 0x40);
            }
            else
            {
                mx.setColumn(startPos, numbers_small[number][0]);
                mx.setColumn(startPos+1, numbers_small[number][1]);
                mx.setColumn(startPos+2, numbers_small[number][2]);
            }
            break;
        case BIG:
            if (underscore)
            {
                mx.setColumn(startPos, numbers_big[number][0] | 0x80);
                mx.setColumn(startPos+1, numbers_big[number][1] | 0x80);
                mx.setColumn(startPos+2, numbers_big[number][2] | 0x80);
                mx.setColumn(startPos+3, numbers_big[number][3] | 0x80);
            }
            else
            {
                mx.setColumn(startPos, numbers_big[number][0]);
                mx.setColumn(startPos+1, numbers_big[number][1]);
                mx.setColumn(startPos+2, numbers_big[number][2]);
                mx.setColumn(startPos+3, numbers_big[number][3]);
            }
            
            break;
        default:
            break;
    }
}

/*
 *      Print time
 */
void printTime(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    printHours(hours, false);
    printMinutes(minutes, false);
    printSeconds(seconds, false);
}

/*
 *      Print a number at the hours position
 */
void printHours(uint8_t hours, bool underscore)
{


    printNumber(HOURS_START_POS, (uint8_t) hours / 10, BIG, underscore);
    printNumber(HOURS_START_POS + 5, (uint8_t) hours % 10, BIG, underscore);
}

/*
 *      Print a number at the minutes position
 */
void printMinutes(uint8_t minutes, bool underscore)
{
    printNumber(MINUTES_START_POS, (uint8_t) minutes / 10, BIG, underscore);
    printNumber(MINUTES_START_POS + 5, (uint8_t) minutes % 10, BIG, underscore);
}

/*
 *      Print a number at the seconds position
 */
void printSeconds(uint8_t seconds, bool underscore)
{
    printNumber(SECONDS_START_POS, (uint8_t) seconds / 10, SMALL, underscore);
    printNumber(SECONDS_START_POS + 4, (uint8_t) seconds % 10, SMALL, underscore);
}

/*
 *      Print a text to the matrix.
 */
void printText(const char *pMsg)
{
    uint8_t cursor = 0;
    uint8_t index;
    uint8_t i, j;

    for (i = 0; i < strlen(pMsg); i++)
    {
        if (pMsg[i] >= 0x20 && pMsg[i] <= 0x7E)
        {
            index = pMsg[i] - 0x20;

            if (pMsg[i] == 0x20)
            {
                mx.setColumn(cursor++, characters[index][0]);
                mx.setColumn(cursor++, characters[index][1]);
                cursor++;
                continue;
            }
            else if (pMsg[i] == 0x22)
            {
                mx.setColumn(cursor++, characters[index][0]);
                mx.setColumn(cursor++, characters[index][1]);
                mx.setColumn(cursor++, characters[index][2]);
                cursor++;
                continue;
            }

            for (j = 0; j < CHARACTER_WIDTH; j++)
            {
                if (characters[index][j] != 0x00)
                {
                    mx.setColumn(cursor++, characters[index][j]);
                }
            }
            cursor++;
        }
        else
        {
        }
    }
}