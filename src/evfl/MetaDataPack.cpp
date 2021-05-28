#include <algorithm>
#include <evfl/MetaDataPack.h>
#include <ore/BinaryFile.h>

namespace evfl {

void MetaDataPack::AddInt(const char* key, int value) {
    auto& entry = AddEntry();
    entry.key = key;
    entry.value.i = value;
    entry.type = DataType::kInt;
}

// NON_MATCHING: ???
void MetaDataPack::AddBool(const char* key, bool value) {
    auto& entry = AddEntry();
    entry.key = key;
    entry.value.i = value;
    entry.type = DataType::kBool;
}

void MetaDataPack::AddFloat(const char* key, float value) {
    auto& entry = AddEntry();
    entry.key = key;
    entry.value.f = value;
    entry.type = DataType::kFloat;
}

void MetaDataPack::AddStringPtr(const char* key, const char* value) {
    auto& entry = AddEntry();
    entry.key = key;
    entry.value.str = value;
    entry.type = DataType::kString;
}

void MetaDataPack::AddWStringPtr(const char* key, const wchar_t* value) {
    auto& entry = AddEntry();
    entry.key = key;
    entry.value.wstr = value;
    entry.type = DataType::kWString;
}

MetaDataPack::Entry* MetaDataPack::Find(const ore::StringView& key) const {
    for (auto& entry : GetEntries()) {
        if (entry.IsKey(key))
            return &entry;
    }
    return nullptr;
}

bool MetaDataPack::FindInt(int* value, const ore::StringView& key) const {
    auto* entry = Find(key);
    if (!entry || entry->type != DataType::kInt)
        return false;
    *value = entry->value.i;
    return true;
}

bool MetaDataPack::FindBool(bool* value, const ore::StringView& key) const {
    auto* entry = Find(key);
    if (!entry || entry->type != DataType::kBool)
        return false;
    *value = entry->value.i == 1;
    return true;
}

bool MetaDataPack::FindFloat(float* value, const ore::StringView& key) const {
    auto* entry = Find(key);
    if (!entry || entry->type != DataType::kFloat)
        return false;
    *value = entry->value.f;
    return true;
}

bool MetaDataPack::FindString(ore::StringView* value, const ore::StringView& key) const {
    auto* entry = Find(key);
    if (!entry || entry->type != DataType::kString)
        return false;
    *value = entry->value.str;
    return true;
}

bool MetaDataPack::FindWString(ore::WStringView* value, const ore::StringView& key) const {
    auto* entry = Find(key);
    if (!entry || entry->type != DataType::kWString)
        return false;
    *value = entry->value.wstr;
    return true;
}

int MetaDataPack::GetType(const ore::StringView& key) const {
    auto* entry = Find(key);
    if (entry && u32(entry->type) <= DataType::kWString)
        return entry->type + 2;

#ifdef MATCHING_HACK_NX_CLANG
    asm("");  // Force a branch to be generated (instead of CSEL)
#endif
    return 13;
}

// NON_MATCHING: two add operands swapped
void MetaDataPack::Builder::CalcMemSize() {
    m_entries_byte_size = sizeof(Entry) * m_num_entries;
    m_required_size = 0;
    m_alignment_real = 16;
    if (m_num_entries != 0) {
        m_alignment_real = std::max(16, m_alignment);
        m_buffer_offset = ore::AlignUpToPowerOf2(_14, m_alignment) - _14;
        m_required_size = m_buffer_offset + m_entries_byte_size;
    }
}

bool MetaDataPack::Builder::Build(MetaDataPack* pack, ore::Buffer buffer) {
    if (m_alignment_real <= 0)
        return false;
    if (m_required_size > buffer.size)
        return false;
    pack->m_buffer.data = buffer.data;
    pack->m_buffer.size = buffer.size;
    const int byte_size = m_entries_byte_size;
    pack->m_entries = reinterpret_cast<Entry*>(buffer.data + m_buffer_offset);
    pack->m_entries_capacity = byte_size / int(sizeof(Entry));
    pack->m_entries_num = 0;
    return true;
}

}  // namespace evfl
