#pragma once

#include <ore/Types.h>

namespace ore {

class Allocator {
public:
    Allocator() = default;
    virtual ~Allocator() = default;

    virtual void* AllocImpl(size_t size, size_t alignment) = 0;
    virtual void FreeImpl(void* ptr) = 0;
};

}  // namespace ore
