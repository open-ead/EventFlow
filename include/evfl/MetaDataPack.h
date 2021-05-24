#pragma once

#include <ore/EnumUtil.h>

namespace evfl {

// FIXME
struct MetaDataPack {
    ORE_ENUM(DataType, kInt, kBool, kFloat, kString, kWString)
};

}  // namespace evfl
