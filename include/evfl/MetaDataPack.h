#pragma once

#include <ore/Array.h>
#include <ore/Buffer.h>
#include <ore/EnumUtil.h>
#include <ore/StringView.h>

namespace evfl {

class MetaDataPack {
public:
    ORE_ENUM(DataType, kInt, kBool, kFloat, kString, kWString)

    struct Entry {
        union Value {
            bool b;
            int i;
            float f;
            const char* str;
            const wchar_t* wstr;
        };

        bool IsKey(const ore::StringView& other) const { return ore::StringView(key) == other; }

        const char* key;
        Value value;
        DataType::Type type;
    };

    class Builder {
    public:
        void CalcMemSize();
        bool Build(MetaDataPack* pack, ore::Buffer buffer);
        void SetNumEntries(int num) { m_num_entries = num; }
        int GetRequiredSize() const { return m_required_size; }

    private:
        int m_num_entries = 16;
        int m_required_size = 0;
        int m_alignment_real = 16;
        int m_entries_byte_size = 0;
        int m_alignment = 16;
        int _14 = 0;
        int m_buffer_offset = -1;
    };

    void AddInt(const char* key, int value);
    void AddBool(const char* key, bool value);
    void AddFloat(const char* key, float value);
    void AddStringPtr(const char* key, const char* value);
    void AddWStringPtr(const char* key, const wchar_t* value);

    bool FindInt(int* value, const ore::StringView& key) const;
    bool FindBool(bool* value, const ore::StringView& key) const;
    bool FindFloat(float* value, const ore::StringView& key) const;
    bool FindString(ore::StringView* value, const ore::StringView& key) const;
    bool FindWString(ore::WStringView* value, const ore::StringView& key) const;

    int GetType(const ore::StringView& key) const;

private:
    Entry* Find(const ore::StringView& key) const;
    Entry& AddEntry() { return m_entries[m_entries_num++]; }

    ore::Array<Entry> GetEntries() const { return {m_entries, m_entries_num}; }

    ore::Buffer m_buffer{};
    Entry* m_entries{};
    int m_entries_num{};
    int m_entries_capacity{};
};

}  // namespace evfl
