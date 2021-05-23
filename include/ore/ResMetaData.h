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
        int i;
        bool b;
        float f;
        BinTPtr<BinString> str;
        BinTPtr<BinWString> wstr;
    };

    ORE_ENUM(DataType, kArgument, kContainer, kInt, kBool, kFloat, kString, kWString, kIntArray, kBoolArray, kFloatArray, kStringArray, kWStringArray, kActorIdentifier)

    SizedEnum<DataType::ValueType, u8> type;
    u16 num_items;
    BinTPtr<ResDic> dictionary;
    Value value;
};

void SwapEndian(ResEndian* endian, ResMetaData* res);

}  // namespace ore
