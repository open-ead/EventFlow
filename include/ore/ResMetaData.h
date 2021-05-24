#pragma once

#include <ore/BinaryFile.h>
#include <ore/EnumUtil.h>
#include <ore/Types.h>

namespace ore {

struct ResDic;
struct ResEndian;

struct ResMetaData {
    union Value {
        BinTPtr<ResMetaData> container;
        // Also used for booleans. Anything that is != 0 is treated as true.
        int i;
        float f;
        BinTPtr<BinString> str;
        BinTPtr<BinWString> wstr;
    };

    ORE_ENUM(DataType, kArgument, kContainer, kInt, kBool, kFloat, kString, kWString, kIntArray, kBoolArray, kFloatArray, kStringArray, kWStringArray, kActorIdentifier)

    SizedEnum<DataType::Type, u8> type;
    u16 num_items;
    BinTPtr<ResDic> dictionary;
    Value value;
};

// FIXME
struct ResUserData {
    ORE_ENUM(DataType, kInt, kFloat, kString, kWString, kStream)
};

void SwapEndian(ResEndian* endian, ResMetaData* res);

}  // namespace ore
