#include <algorithm>
#include <ore/ResDic.h>

namespace ore {

int ResDic::FindRefBit(const StringView& str1, const StringView& str2) {
    const auto len1 = str1.size();
    const auto len2 = str2.size();
    const auto len = std::max(int(len1), int(len2));

    for (int bit_idx = 0; bit_idx < 8 * len; ++bit_idx) {
        const int idx = bit_idx >> 3;

        int bit1 = 0;
        if (len1 > idx)
            bit1 = str1[int(len1) + -(idx + 1)] >> (bit_idx % 8) & 1;

        int bit2 = 0;
        if (len2 > idx)
            bit2 = str2[int(len2) + -(idx + 1)] >> (bit_idx % 8) & 1;

        if (bit1 != bit2)
            return bit_idx;
    }

    return -1;
}

}  // namespace ore
