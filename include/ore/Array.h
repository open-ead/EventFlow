#pragma once

#include <algorithm>
#include <iterator>
#include <memory>
#include <ore/Allocator.h>
#include <type_traits>

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

template <typename T>
class ArrayListBase {
public:
    ArrayListBase() : m_data(), m_size(), m_capacity() {}
    ArrayListBase(T* data, int capacity) {
        m_size = 0;
        m_data = data;
        m_capacity = capacity;
    }
    ~ArrayListBase() { clear(); }

    ArrayListBase(const ArrayListBase&) = delete;
    auto operator=(const ArrayListBase&) = delete;

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
        auto* item = new (&m_data[m_size++]) T(std::forward<Args>(args)...);
        return *item;
    }

    void push_back(const T& item) { new (&m_data[m_size++]) T(item); }

    void pop_back() {
        std::destroy_at(&back());
        --m_size;
    }

    void clear() {
        std::destroy(begin(), end());
        m_size = 0;
    }

    T* m_data;
    int m_size;
    int m_capacity;
};

template <typename T, int N>
class FixedArrayList : public ArrayListBase<T> {
public:
    FixedArrayList() : ArrayListBase<T>(reinterpret_cast<T*>(m_storage), N) {}

private:
    std::aligned_storage_t<sizeof(T), alignof(T)> m_storage[N];
};

// This is like a std::vector.
template <typename T>
class DynArrayList : public ArrayListBase<T> {
public:
    DynArrayList() = default;
    explicit DynArrayList(Allocator* allocator) : m_allocator(allocator) {}

    ~DynArrayList() {
        clear();
        m_allocator = nullptr;
        this->m_size = 0;
    }

    DynArrayList(const DynArrayList&) = delete;
    auto operator=(const DynArrayList&) = delete;

    void Init(Allocator* allocator, int initial_capacity = 1) {
        clear();
        m_allocator = allocator;
        Reallocate(initial_capacity);
    }

    template <typename... Args>
    T& emplace_back(Args&&... args) {
        GrowIfNeeded();
        return ArrayListBase<T>::emplace_back(std::forward<Args>(args)...);
    }

    void push_back(const T& item) {
        GrowIfNeeded();
        return ArrayListBase<T>::push_back(item);
    }

    void clear() {
        std::destroy(this->begin(), this->end());
        auto* data = this->m_data;
        this->m_data = nullptr;
        this->m_size = 0;
        this->m_capacity = 0;
        if (data)
            m_allocator->FreeImpl(data);
    }

    template <typename InputIterator>
    void OverwriteWith(InputIterator src_begin, InputIterator src_end) {
        const int src_size = std::distance(src_begin, src_end);
        if (src_size > this->m_capacity) {
            this->m_size = 0;
            Reallocate(2 * src_size);
        }
        this->m_size = src_size;
        std::uninitialized_copy(src_begin, src_end, this->begin());
    }

private:
    void GrowIfNeeded() {
        if (this->m_size < this->m_capacity)
            return;
        Reallocate(2 * this->m_size + 2);
    }

    void Reallocate(int new_capacity) {
        const int num_bytes = sizeof(T) * new_capacity;
        auto* new_buffer =
            static_cast<T*>(m_allocator->AllocImpl(num_bytes, alignof(std::max_align_t)));
        auto* old_buffer = this->m_data;
        auto* capacity = &this->m_capacity;

        std::uninitialized_move(this->begin(), this->end(), new_buffer);
        this->m_data = new_buffer;
        *capacity = new_capacity;

        if (old_buffer)
            m_allocator->FreeImpl(old_buffer);
    }

    Allocator* m_allocator{};
};

}  // namespace ore
