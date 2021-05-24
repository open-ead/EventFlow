#pragma once

#include <memory>

namespace ore {

// This is like a std::span, not a fixed-size array like std::array.
template <typename T>
class Array {
public:
    Array() = default;
    Array(T* data, int size) { SetData(data, size); }

    T* data() const { return m_data; }
    int size() const { return m_size; }

    auto begin() const { return data(); }
    auto end() const { return data() + size(); }

    void SetData(T* data, int size) {
        m_data = data;
        m_size = size;
    }

    void DestructElements() { std::destroy(begin(), end()); }

private:
    T* m_data{};
    int m_size{};
};

}  // namespace ore
