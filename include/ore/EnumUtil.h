#pragma once

#include <iterator>
#include <ore/IterRange.h>
#include <ore/StringView.h>

namespace ore {

namespace detail::EnumUtil {

int FindIndex(int value, const IterRange<const int*>& values);
void Parse(const IterRange<StringView*>& out, StringView definition);

constexpr int CountValues(const char* text_all, size_t text_all_len) {
    int count = 1;
    for (size_t i = 0; i < text_all_len; ++i) {
        if (text_all[i] == ',')
            ++count;
    }
    return count;
}

}  // namespace detail::EnumUtil

template <class T>
struct Enum {
public:
    Enum() { T::Init(); }

    static Enum<T>& Info() { return s_Info; }

    StringView name{};
    IterRange<StringView*> members{};

private:
    static inline Enum<T> s_Info{};
};

#define ORE_ENUM(NAME, ...)                                                                        \
    class NAME {                                                                                   \
    public:                                                                                        \
        enum ValueType { __VA_ARGS__ };                                                            \
                                                                                                   \
        static void Init() {                                                                       \
            static ore::StringView names[cCount];                                                  \
            ore::detail::EnumUtil::Parse(ore::IterRange<ore::StringView*>(names), cTextAll);       \
            ore::Enum<NAME>::Info().name = #NAME;                                                  \
            ore::Enum<NAME>::Info().members = ore::IterRange<ore::StringView*>(names);             \
        }                                                                                          \
                                                                                                   \
    private:                                                                                       \
        static constexpr const char* cTextAll = #__VA_ARGS__;                                      \
        static constexpr size_t cTextAllLen = sizeof(#__VA_ARGS__);                                \
        static constexpr int cCount = ore::detail::EnumUtil::CountValues(cTextAll, cTextAllLen);   \
    };

// FIXME
template <class T>
class ValuedEnum;

}  // namespace ore
