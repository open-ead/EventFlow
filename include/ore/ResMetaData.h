#pragma once

#include <ore/EnumUtil.h>

namespace ore {

struct ResMetaData {
    ORE_ENUM(DataType, kArgument, kContainer, kInt, kBool, kFloat, kString, kWString, kIntArray, kBoolArray, kFloatArray, kStringArray, kWStringArray, kActorIdentifier)
};

}  // namespace ore
