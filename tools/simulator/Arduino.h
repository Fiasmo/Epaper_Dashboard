#pragma once
#include <algorithm>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "Print.h"

#include <cstdlib>
#include <string>

#define __builtin_bswap16(x) _byteswap_ushort(x)

using String = std::string;
class __FlashStringHelper;

#define PROGMEM
#define PI 3.14159265358979323846
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define radians(deg) ((deg)*DEG_TO_RAD)
#define D0 0
#define D1 1
#define D2 2
#define D3 3

inline void delay(unsigned long) {}

struct SerialShim : public Print
{
    void begin(unsigned long) {}

    size_t write(uint8_t c) override
    {
        fputc(c, stdout);
        return 1;
    }

    int printf(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        const int result = vprintf(format, args);
        va_end(args);
        return result;
    }
};

extern SerialShim Serial;
