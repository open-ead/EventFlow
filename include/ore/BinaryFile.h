#pragma once

#include <ore/StringView.h>
#include <ore/Types.h>

namespace ore {

struct RelocationTable;

struct BinaryBlockHeader {
    BinaryBlockHeader* FindNextBlock(int type);
    const BinaryBlockHeader* FindNextBlock(int type) const;
    BinaryBlockHeader* GetNextBlock();
    const BinaryBlockHeader* GetNextBlock() const;
    void SetNextBlock(BinaryBlockHeader* block);

    u32 magic;
    int next_block_offset;
};

struct BinaryFileHeader {
    bool IsValid(s64 magic_, int ver_major_, int ver_minor_, int ver_patch_, int ver_sub_) const;
    bool IsSignatureValid(s64 magic_) const;
    bool IsVersionValid(int major, int minor, int patch, int sub) const;
    bool IsEndianReverse() const;
    bool IsEndianValid() const;

    bool IsAlignmentValid() const;
    int GetAlignment() const;
    void SetAlignment(int alignment_);

    bool IsRelocated() const;
    void SetRelocated(bool relocated);

    void SetByteOrderMark();

    int GetFileSize() const;
    void SetFileSize(int size);

    StringView GetFileName() const;
    void SetFileName(const StringView& name);

    RelocationTable* GetRelocationTable();
    void SetRelocationTable(RelocationTable* table);

    BinaryBlockHeader* GetFirstBlock();
    const BinaryBlockHeader* GetFirstBlock() const;
    void SetFirstBlock(BinaryBlockHeader* block);

    BinaryBlockHeader* FindFirstBlock(int type);
    const BinaryBlockHeader* FindFirstBlock(int type) const;

    u64 magic;
    u8 ver_major;
    u8 ver_minor;
    u8 ver_patch;
    u8 ver_sub;
    s16 bom;
    u8 alignment;
    u8 _f;
    int file_name_offset;
    u16 relocation_flags;
    u16 first_block_offset;
    int relocation_table_offset;
    int file_size;
};

template <typename T>
struct BinTString {
    const T* data() const { return reinterpret_cast<const T*>(this + 1); }
    const T& operator[](size_t idx) const { return data()[idx]; }

    u16 length;
};

using BinString = BinTString<char>;
using BinWString = BinTString<wchar_t>;

template <typename T>
struct BinTPtr {
    void Clear();
    void Set(T* ptr);
    T* Get();
    const T* Get() const;
    void SetOffset(void* base, void* ptr);
    u64 GetOffset() const;
    const T* ToPtr(void* base) const;
    void Relocate(void* base);
    void Unrelocate(void* base);

    u64 offset_or_ptr;
};

static_assert(sizeof(u64) >= sizeof(void*));

}  // namespace ore
