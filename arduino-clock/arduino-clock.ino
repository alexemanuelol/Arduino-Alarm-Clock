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
#define CLK_PIN     13 /* SCK */
#define DATA_PIN    11 /* MOSI */
#define CS_PIN      10 /* SS */

#define MAX_DEVICES     4
#define MATRIX_WIDTH    (8 * MAX_DEVICES)
#define MATRIX_HEIGHT   8

#define HOURS_START_POS     0
#define MINUTES_START_POS   12
#define SECONDS_START_POS   24

#define SECOND_CORRECTION_MINUTES   106


/*
 *      Enums
 */
typedef enum {
    SMALL = 0,
    BIG
} numberSize;

typedef enum {
    STANDBY = 0,
    UPDATE
} appState;


/*
 *      Global variables
 */
MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::PAROLA_HW, CS_PIN, MAX_DEVICES);

uint8_t seconds = 0;
uint8_t minutes = 0;
uint8_t hours = 0;
uint8_t minuteCounter = 0;
bool alternateColon = 0;

static appState state = STANDBY;

/*
 *      Main function
 */
int main(void)
{
    initInterrupt();
    mx.begin();

    set_hours(hours);
    set_minutes(minutes);
    set_seconds(seconds);

    while (true)
    {
        app();
    }

    return 0;
}

/*
 *      application function
 */
void app()
{
    switch (state)
    {
        case STANDBY:
            mx.update();
            break;

        case UPDATE:
            state = STANDBY;

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
                    set_hours(hours);
                }
                set_minutes(minutes);
            }
            set_seconds(seconds);

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
    state = UPDATE;
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
void set_number(uint8_t startPos, uint8_t number, numberSize numSize)
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
 *      Set a number at the hours position
 */
void set_hours(uint8_t hours)
{
    set_number(HOURS_START_POS, (uint8_t) hours / 10, BIG);
    set_number(HOURS_START_POS + 5, (uint8_t) hours % 10, BIG);
}

/*
 *      Set a number at the minutes position
 */
void set_minutes(uint8_t minutes)
{
    set_number(MINUTES_START_POS, (uint8_t) minutes / 10, BIG);
    set_number(MINUTES_START_POS + 5, (uint8_t) minutes % 10, BIG);
}

/*
 *      Set a number at the seconds position
 */
void set_seconds(uint8_t seconds)
{
    set_number(SECONDS_START_POS, (uint8_t) seconds / 10, SMALL);
    set_number(SECONDS_START_POS + 4, (uint8_t) seconds % 10, SMALL);
}
