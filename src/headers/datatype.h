#pragma once

#include "math.h"

#define MAX_BITS 8
#define BOOL_STR(x) ((x) ? "True" : "False")

typedef enum {
    UNPACK = 1,
    REPACK = 2,
} Operation;

typedef union {
    u8 raw;
    struct {
        u8 bit0 : 1;
        u8 bit1 : 1;
        u8 bit2 : 1;
        u8 bit3 : 1;
        u8 bit4 : 1;
        u8 bit5 : 1;
        u8 bit6 : 1;
        u8 bit7 : 1;
    } bits;
} GenericBitFlag;