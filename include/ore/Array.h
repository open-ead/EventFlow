#pragma once

#include <memory>
#include <ore/Allocator.h>

namespace ore {

// This is like a std::span, not a fixed-size array like std::array.
// Elements will NOT be automatically freed.
template <typename T>
class Array {
public:
    Array() = default;
    Array(T* data, int size) : m_data(data), m_size(size) {}

    T* data() const { return m_data; }
    int size() const { return m_size; }

    auto begin() const { return data(); }
    auto end() const { return data() + size(); }

    T& operator[](int idx) { return m_data[idx]; }
    const T& operator[](int idx) const { return m_data[idx]; }

    T& front() { return m_data[0]; }
    const T& front() const { return m_data[0]; }

    T& back() { return m_data[m_size - 1]; }
    const T& back() const { return m_data[m_size - 1]; }

    void AllocateElements(int num, Allocator* allocator) {
        auto* new_buffer = allocator->AllocImpl(num * int(sizeof(T)), alignof(std::max_align_t));
        DestructElements();
        m_data = static_cast<T*>(new_buffer);
        m_size = num;
        DefaultConstructElements();
    }

    void DestructElements() { std::destroy(begin(), end()); }

private:
    void DefaultConstructElements() {
        for (auto it = begin(), e = end(); it != e;)
            new (it++) T;
    }

    T* m_data{};
    int m_size{};
};

// This is like a std::vector.
template <typename T>
class Vector {
public:
    Vector() = default;
    explicit Vector(Allocator* allocator) : m_allocator(allocator) {}

    ~Vector() {
        clear();
        m_allocator = nullptr;
        m_size = 0;
    }

    Vector(const Vector&) = delete;
    auto operator=(const Vector&) = delete;

    void Init(Allocator* allocator) {
        clear();
        m_allocator = allocator;
        Reallocate(1);
    }

    T* begin() { return m_data; }
    const T* begin() const { return m_data; }

    T* end() { return m_data + m_size; }
    const T* end() const { return m_data + m_size; }

    T* data() { return m_data; }
    const T* data() const { return m_data; }

    int size() const { return m_size; }
    int capacity() const { return m_capacity; }

    T& operator[](int idx) { return m_data[idx]; }
    const T& operator[](int idx) const { return m_data[idx]; }

    T& front() { return m_data[0]; }
    const T& front() const { return m_data[0]; }

    T& back() { return m_data[m_size - 1]; }
    const T& back() const { return m_data[m_size - 1]; }

    template <typename... Args>
    T& emplace_back(Args&&... args) {
        GrowIfNeeded();
        auto* item = new (&m_data[m_size++]) T(std::forward<Args>(args)...);
        return *item;
    }

    void push_back(const T& item) {
        GrowIfNeeded();
        new (&m_data[m_size++]) T(item);
    }

    void pop_back() {
        std::destroy_at(&back());
        --m_size;
    }

    void clear() {
        std::destroy(begin(), end());
        auto* data = m_data;
        m_data = nullptr;
        m_size = 0;
        m_capacity = 0;
        if (data)
            m_allocator->FreeImpl(data);
    }

private:
    void GrowIfNeeded() {
        if (m_size < m_capacity)
            return;
        Reallocate(2 * m_size + 2);
    }

    void Reallocate(int new_capacity) {
        const int num_bytes = sizeof(T) * new_capacity;
        auto* new_buffer =
            static_cast<T*>(m_allocator->AllocImpl(num_bytes, alignof(std::max_align_t)));
        auto* old_buffer = m_data;
        auto* capacity = &m_capacity;

        std::uninitialized_move(begin(), end(), new_buffer);
        m_data = new_buffer;
        *capacity = new_capacity;

        if (old_buffer)
            m_allocator->FreeImpl(old_buffer);
    }

    T* m_data{};
    int m_size{};
    int m_capacity{};
    Allocator* m_allocator{};
};

}  // namespace ore
