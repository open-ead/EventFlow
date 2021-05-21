#pragma once

#include <ore/Types.h>

namespace ore {

struct StringPool {
    int GetLength() const;
    void SetLength(int len);

    u32 magic;
    u32 reserved_4;
    u32 reserved_8;
    u32 reserved_c;
    int length;
};

}  // namespace ore
