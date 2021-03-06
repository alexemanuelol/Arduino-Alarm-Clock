#ifndef CHARACTERS_H
#define CHARACTERS_H

#include <Arduino.h>

#ifdef __cplusplus
extern "C"{
#endif

#define CHARACTERS              95
#define CHARACTER_WIDTH         5
#define SPACING                 1

const uint8_t characters[CHARACTERS][CHARACTER_WIDTH] =
{
    { 0x00, 0x00, 0x00, 0x00, 0x00 },   /* SPACE */
    { 0x5F, 0x00, 0x00, 0x00, 0x00 },   /* ! */
    { 0x07, 0x00, 0x07, 0x00, 0x00 },   /* " */ // space between
    { 0x14, 0x7F, 0x14, 0x7F, 0x14 },   /* # */
    { 0x44, 0x4A, 0xFF, 0x4A, 0x32 },   /* $ */
    { 0x63, 0x13, 0x08, 0x64, 0x63 },   /* % */
    { 0x36, 0x49, 0x49, 0x36, 0x48 },   /* & */
    { 0x07, 0x00, 0x00, 0x00, 0x00 },   /* ' */
    { 0x3E, 0x41, 0x41, 0x00, 0x00 },   /* ( */
    { 0x41, 0x41, 0x3E, 0x00, 0x00 },   /* ) */
    { 0x08, 0x2A, 0x1C, 0x2A, 0x08 },   /* * */
    { 0x08, 0x08, 0x3E, 0x08, 0x08 },   /* + */
    { 0x60, 0xE0, 0x00, 0x00, 0x00 },   /* , */
    { 0x08, 0x08, 0x08, 0x08, 0x00 },   /* - */
    { 0x60, 0x60, 0x00, 0x00, 0x00 },   /* . */
    { 0x60, 0x10, 0x08, 0x04, 0x03 },   /* / */
    { 0x3E, 0x51, 0x49, 0x45, 0x3E },   /* 0 */
    { 0x42, 0x7F, 0x40, 0x00, 0x00 },   /* 1 */
    { 0x71, 0x49, 0x49, 0x49, 0x46 },   /* 2 */
    { 0x41, 0x49, 0x49, 0x49, 0x36 },   /* 3 */
    { 0x0F, 0x08, 0x08, 0x08, 0x7F },   /* 4 */
    { 0x4F, 0x49, 0x49, 0x49, 0x79 },   /* 5 */
    { 0x3E, 0x49, 0x49, 0x49, 0x30 },   /* 6 */
    { 0x03, 0x01, 0x01, 0x01, 0x7F },   /* 7 */
    { 0x36, 0x49, 0x49, 0x49, 0x36 },   /* 8 */
    { 0x06, 0x49, 0x49, 0x49, 0x3E },   /* 9 */
    { 0x6C, 0x6C, 0x00, 0x00, 0x00 },   /* : */
    { 0x6C, 0xEC, 0x00, 0x00, 0x00 },   /* ; */
    { 0x08, 0x14, 0x22, 0x00, 0x00 },   /* < */
    { 0x14, 0x14, 0x14, 0x14, 0x00 },   /* = */
    { 0x22, 0x14, 0x08, 0x00, 0x00 },   /* > */
    { 0x01, 0x59, 0x09, 0x09, 0x06 },   /* ? */
    { 0x3E, 0x41, 0x5D, 0x59, 0x4E },   /* @ */
    { 0x7E, 0x09, 0x09, 0x09, 0x7E },   /* A */
    { 0x7F, 0x49, 0x49, 0x49, 0x36 },   /* B */
    { 0x3E, 0x41, 0x41, 0x41, 0x41 },   /* C */
    { 0x7F, 0x41, 0x41, 0x41, 0x3E },   /* D */
    { 0x7F, 0x49, 0x49, 0x49, 0x49 },   /* E */
    { 0x7F, 0x09, 0x09, 0x09, 0x01 },   /* F */
    { 0x3E, 0x41, 0x41, 0x49, 0x79 },   /* G */
    { 0x7F, 0x08, 0x08, 0x08, 0x7F },   /* H */
    { 0x41, 0x7F, 0x41, 0x00, 0x00 },   /* I */
    { 0x30, 0x41, 0x41, 0x41, 0x3F },   /* J */
    { 0x7F, 0x08, 0x14, 0x22, 0x41 },   /* K */
    { 0x7F, 0x40, 0x40, 0x40, 0x40 },   /* L */
    { 0x7F, 0x02, 0x0C, 0x02, 0x7F },   /* M */
    { 0x7F, 0x04, 0x08, 0x10, 0x7F },   /* N */
    { 0x3E, 0x41, 0x41, 0x41, 0x3E },   /* O */
    { 0x7F, 0x09, 0x09, 0x09, 0x06 },   /* P */
    { 0x3E, 0x41, 0x41, 0x61, 0x7E },   /* Q */
    { 0x7F, 0x09, 0x19, 0x29, 0x46 },   /* R */
    { 0x46, 0x49, 0x49, 0x49, 0x31 },   /* S */
    { 0x01, 0x01, 0x7F, 0x01, 0x01 },   /* T */
    { 0x3F, 0x40, 0x40, 0x40, 0x3F },   /* U */
    { 0x1F, 0x20, 0x40, 0x20, 0x1F },   /* V */
    { 0x3F, 0x40, 0x38, 0x40, 0x3F },   /* W */
    { 0x63, 0x14, 0x08, 0x14, 0x63 },   /* X */
    { 0x03, 0x04, 0x78, 0x04, 0x03 },   /* Y */
    { 0x61, 0x51, 0x49, 0x45, 0x43 },   /* Z */
    { 0x7F, 0x41, 0x41, 0x00, 0x00 },   /* [ */
    { 0x03, 0x04, 0x08, 0x10, 0x60 },   /* \ */
    { 0x41, 0x41, 0x7F, 0x00, 0x00 },   /* ] */
    { 0x04, 0x02, 0x01, 0x02, 0x04 },   /* ^ */
    { 0x80, 0x80, 0x80, 0x80, 0x00 },   /* _ */
    { 0x01, 0x02, 0x04, 0x00, 0x00 },   /* ` */
    { 0x38, 0x44, 0x44, 0x7C, 0x00 },   /* a */
    { 0x7F, 0x44, 0x44, 0x38, 0x00 },   /* b */
    { 0x38, 0x44, 0x44, 0x44, 0x00 },   /* c */
    { 0x38, 0x44, 0x44, 0x7F, 0x00 },   /* d */
    { 0x38, 0x54, 0x54, 0x58, 0x00 },   /* e */
    { 0x04, 0x7E, 0x05, 0x01, 0x00 },   /* f */
    { 0x18, 0xA4, 0xA4, 0x7C, 0x00 },   /* g */
    { 0x7F, 0x04, 0x04, 0x78, 0x00 },   /* h */
    { 0x7D, 0x00, 0x00, 0x00, 0x00 },   /* i */
    { 0x84, 0x85, 0x7C, 0x00, 0x00 },   /* j */
    { 0x7F, 0x10, 0x28, 0x44, 0x00 },   /* k */
    { 0x7F, 0x00, 0x00, 0x00, 0x00 },   /* l */
    { 0x7C, 0x04, 0x78, 0x04, 0x78 },   /* m */
    { 0x7C, 0x04, 0x04, 0x78, 0x00 },   /* n */
    { 0x38, 0x44, 0x44, 0x38, 0x00 },   /* o */
    { 0xFC, 0x24, 0x24, 0x18, 0x00 },   /* p */
    { 0x18, 0x24, 0x24, 0xFC, 0x00 },   /* q */
    { 0x7C, 0x04, 0x04, 0x08, 0x00 },   /* r */
    { 0x58, 0x54, 0x54, 0x34, 0x00 },   /* s */
    { 0x04, 0x7F, 0x04, 0x00, 0x00 },   /* t */
    { 0x3C, 0x40, 0x40, 0x7C, 0x00 },   /* u */
    { 0x1C, 0x20, 0x40, 0x7C, 0x00 },   /* v */
    { 0x3C, 0x40, 0x30, 0x40, 0x3C },   /* w */
    { 0x6C, 0x10, 0x10, 0x6C, 0x00 },   /* x */
    { 0x1C, 0xA0, 0xA0, 0x7C, 0x00 },   /* y */
    { 0x64, 0x54, 0x54, 0x4C, 0x00 },   /* z */
    { 0x08, 0x36, 0x41, 0x41, 0x00 },   /* { */
    { 0x7F, 0x00, 0x00, 0x00, 0x00 },   /* | */
    { 0x41, 0x41, 0x36, 0x08, 0x00 },   /* } */
    { 0x10, 0x08, 0x10, 0x08, 0x00 },   /* ~ */
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif
