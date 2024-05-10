#pragma once

#include "construct.h"
#include "util.h"

namespace mystl {

template <typename T>
class allocator {
public:
    // clang-format off
    using value_type        = T;
    using pointer           = T*;
    using const_pointer     = const T*;
    using reference         = T&;
    using const_reference   = const T&;
    using size_type         = size_t;
    using difference_type   = ptrdiff_t;
    // clang-format on

public:
    static T* allocate();
    static T* allocate(size_type n);

    static void deallocate(T* ptr);
    static void deallocate(T* ptr, size_type n);

    static void construct(T* ptr);
    static void construct(T* ptr, const_reference value);
    static void construct(T* ptr, T&& value);
    template <typename... Args>
    static void construct(T* ptr, Args&&... args);

    static void destroy(T* ptr);
    static void destroy(T* first, T* last);
};

template <typename T>
T* allocator<T>::allocate() {
    return static_cast<T*>(::operator new(sizeof(T)));
}

template <typename T>
T* allocator<T>::allocate(size_type n) {
    if (n == 0)
        return nullptr;
    return static_cast<T*>(::operator new(n * sizeof(T)));
}

template <typename T>
void allocator<T>::deallocate(T* ptr) {
    if (ptr == nullptr)
        return;
    ::operator delete(ptr);
}

template <typename T>
void allocator<T>::deallocate(T* ptr, size_type n) {
    if (ptr == nullptr)
        return;
    ::operator delete(ptr);
}

template <typename T>
void allocator<T>::construct(T* ptr) {
    mystl::construct(ptr);
}

template <typename T>
void allocator<T>::construct(T* ptr, const T& value) {
    mystl::construct(ptr, value);
}

template <typename T>
void allocator<T>::construct(T* ptr, T&& value) {
    mystl::construct(ptr, mystl::move(value));
}

template <typename T>
template <typename... Args>
void allocator<T>::construct(T* ptr, Args&&... args) {
    mystl::construct(ptr, mystl::forward<args>(args)...);
}

template <typename T>
void allocator<T>::destroy(T* ptr) {
    mystl::destroy(ptr);
}

template <typename T>
void allocator<T>::destroy(T* first, T* last) {
    mystl::destroy(first, last);
}

} // namespace mystl
