#define NUMBERS             10
#define NUMBER_BIG_WIDTH    4
#define NUMBER_SMALL_WIDTH  3

const uint8_t numbers_big[NUMBERS][NUMBER_BIG_WIDTH] = 
{
    { 0x7F, 0x41, 0x41, 0x7F },
    { 0x00, 0x42, 0x7F, 0x40 },
    { 0x79, 0x49, 0x49, 0x4F },
    { 0x63, 0x49, 0x49, 0x77 },
    { 0x0F, 0x08, 0x08, 0x7F },
    { 0x4F, 0x49, 0x49, 0x79 },
    { 0x7F, 0x49, 0x49, 0x79 },
    { 0x03, 0x01, 0x7D, 0x03 },
    { 0x77, 0x49, 0x49, 0x77 },
    { 0x4F, 0x49, 0x49, 0x7F },
};

const uint8_t numbers_small[NUMBERS][NUMBER_SMALL_WIDTH] = 
{
    { 0x3E, 0x22, 0x3E },
    { 0x24, 0x3E, 0x20 },
    { 0x3A, 0x2A, 0x2E },
    { 0x2A, 0x2A, 0x3E },
    { 0x0E, 0x08, 0x3E },
    { 0x2E, 0x2A, 0x3A },
    { 0x3E, 0x2A, 0x3A },
    { 0x06, 0x02, 0x3E },
    { 0x3E, 0x2A, 0x3E },
    { 0x2E, 0x2A, 0x3E },
};