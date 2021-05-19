#pragma once

#include <ore/Types.h>

namespace ore {

constexpr int PopCount(u32 x) {
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x += (x >> 8);
    x += (x >> 16);
    return int(x & 0x3f);
}

constexpr int Log2ForPowerOf2(u32 x) {
    return PopCount((x & -x) - 1);
}

}  // namespace ore
