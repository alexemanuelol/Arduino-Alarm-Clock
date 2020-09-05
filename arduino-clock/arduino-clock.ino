#include <MD_MAX72xx.h>
#include <SPI.h>

#include "numbers.h"

#define CLK_PIN     13 /* SCK */
#define DATA_PIN    11 /* MOSI */
#define CS_PIN      10 /* SS */

#define MAX_DEVICES     4
#define MATRIX_WIDTH    (8 * MAX_DEVICES)
#define MATRIX_HEIGHT   8
#define CHAT_SPACING    1

#define HOURS_START_POS     0
#define MINUTES_START_POS   12
#define SECONDS_START_POS   24

int timer1_counter;

enum numberSize {
    SMALL = 0,
    BIG
};

typedef enum {
    STANDBY = 0,
    UPDATE
} appState;

MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::PAROLA_HW, CS_PIN, MAX_DEVICES);

uint8_t seconds = 0;
uint8_t minutes = 0;
uint8_t hours = 0;
static appState state = STANDBY;
bool alternate = 0;


/* Interrupt Service Routine */
ISR(TIMER1_OVF_vect)
{
    //TCNT1 = timer1_counter;
    state = UPDATE;
}

int main(void)
{
    initInterrupts();
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
                set_minutes(minutes);
            }
            if (minutes == 60)
            {
                minutes = 0;
                hours = hours + 1;
                set_hours(hours);
            }
            if (hours == 24)
            {
                hours = 0;
            }
            
            if (alternate)
            {
                alternate = 0;
                mx.setColumn(10, 0x14);
            }
            else
            {
                alternate = 1;
                mx.setColumn(10, 0x00);
            }
            set_seconds(seconds);
            break;
            
        default:
            break;
    }
}

void initInterrupts()
{
    noInterrupts();     /* Disable all interrupts */
    TCCR1A = 0x00;
    TCCR1B = 0x00;

    //timer1_counter = 34286;     /* Preload timer 65536-16MHz/256/2Hz */

    TCNT1 = 0x0000;//timer1_counter;     /* Preload timer */
    TCCR1A = 0xA2; // fast 16 bit PWM
    TCCR1B = 0x1c;//|= (1 << CS12);      /* 256 Prescaler */
    ICR1 = 62500-1;
    TIMSK1 |= 0x01;//(1 << TOIE1);     /* Enable timer overflow interrupt */
    interrupts();               /* Enable all interrupts */
}

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

void set_hours(uint8_t hours)
{
    set_number(HOURS_START_POS, (uint8_t) hours / 10, BIG);
    set_number(HOURS_START_POS + 5, (uint8_t) hours % 10, BIG);
}

void set_minutes(uint8_t minutes)
{
    set_number(MINUTES_START_POS, (uint8_t) minutes / 10, BIG);
    set_number(MINUTES_START_POS + 5, (uint8_t) minutes % 10, BIG);
}

void set_seconds(uint8_t seconds)
{
    set_number(SECONDS_START_POS, (uint8_t) seconds / 10, SMALL);
    set_number(SECONDS_START_POS + 4, (uint8_t) seconds % 10, SMALL);
}
