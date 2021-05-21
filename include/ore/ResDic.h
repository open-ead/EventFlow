#pragma once

#include <ore/BinaryFile.h>
#include <ore/StringView.h>
#include <ore/Types.h>

namespace ore {

struct ResEndian;

struct ResDicEntry {
    // Bits 3-7: index of the byte that should be checked
    // Bits 0-2: index of the bit in that byte
    u32 compact_bit_idx;
    u16 next_indices[2];
    BinTPtr<BinString> name;
};

struct ResDic {
    static int FindRefBit(const StringView& str1, const StringView& str2);

    ResDicEntry* GetEntries() { return &root_entry; }
    const ResDicEntry* GetEntries() const { return &root_entry; }

    u32 magic;
    int num_entries;
    ResDicEntry root_entry;
    // Followed by ResDicEntry[num_entries].
};

void SwapEndian(ResEndian* endian, ResDic* dic);

}  // namespace ore
