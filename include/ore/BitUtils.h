#pragma once

#include <ore/Types.h>

namespace ore {

constexpr int PopCount(u32 x) {
    x = x - ((x >> 1) & 0x55555555);
    x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F;
    x += (x >> 8);
    x += (x >> 16);
    return int(x & 0x3f);
}

constexpr int PopCount(u64 x) {
    x = x - ((x >> 1) & 0x5555555555555555);
    x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0F;
    x += (x >> 8);
    x += (x >> 16);
    x += (x >> 32);
    return int(x & 0x7f);
}

constexpr int CountTrailingZeros(u32 x) {
    return PopCount((x & -x) - 1);
}

constexpr int CountTrailingZeros(u64 x) {
    return PopCount((x & -x) - 1);
}

class BitArray {
public:
    using Word = size_t;
    static constexpr int NumBitsPerWord = sizeof(Word) * 8;
    static constexpr int ClearMask = ~(NumBitsPerWord - 1);
    static constexpr int ShiftAmount = CountTrailingZeros(u32(NumBitsPerWord));

    class TestIter {
    public:
        TestIter(const Word* start, const Word* end);
        TestIter& operator++();

        int operator*() const { return m_bit; }
        bool operator==(const TestIter& other) const { return m_bit == other.m_bit; }
        bool operator!=(const TestIter& other) const { return !operator==(other); }

    private:
        void SetInvalid() {
            m_bit = -1;
            m_current_word = nullptr;
            m_last_word = nullptr;
            m_next = 0;
        }

        int m_bit;
        const Word* m_current_word;
        const Word* m_last_word;
        Word m_next;
    };

    /// Same as TestIter but clears bits after iterating over them.
    class TestClearIter {
    public:
        TestClearIter(Word* start, Word* end);
        TestClearIter& operator++();
        int operator*() const { return m_bit; }
        bool operator==(const TestClearIter& other) const { return m_bit == other.m_bit; }
        bool operator!=(const TestClearIter& other) const { return !operator==(other); }

    private:
        void SetInvalid() {
            m_bit = -1;
            m_current_word = nullptr;
            m_last_word = nullptr;
            m_next = 0;
        }

        int m_bit;
        Word* m_current_word;
        Word* m_last_word;
        Word m_next;
    };

    constexpr BitArray() = default;
    constexpr BitArray(Word* words, int num_bits) { SetData(words, num_bits); }

    void SetData(Word* words, int num_bits) {
        m_words = words;
        m_num_bits = num_bits;
    }

    void SetAllOn();
    void SetAllOff();
    TestIter BeginTest() const;
    TestIter EndTest() const;
    TestClearIter BeginTestClear();
    TestClearIter EndTestClear();

private:
    int GetNumWords() const { return int((m_num_bits + NumBitsPerWord - 1) >> ShiftAmount); }

    void Fill(int num, Word value) {
        auto* it = m_words;
        for (int i = num - 1; i >= 0; --i)
            *it++ = value;
    }

    Word* m_words{};
    int m_num_bits{};
};

}  // namespace ore
