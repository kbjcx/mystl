#pragma once

// 这个头文件包含两个函数 construct，destroy
// construct : 负责对象的构造
// destroy   : 负责对象的析构

#include <new>

#include "iterator.h"
#include "type_traits.h"
#include "util.h"

namespace mystl {

// 构造对象
template <typename T>
void construct(T* ptr) {
    ::new (static_cast<void*>(ptr)) T();
}

template <typename T1, typename T2>
void construct(T1* ptr, const T2& value) {
    ::new (static_cast<void*>(ptr)) T1(value);
}

template <typename T1, typename... Args>
void construct(T1* ptr, Args&&... args) {
    ::new (static_cast<void*>(ptr)) T1(mystl::forward<Args>(args)...);
}

// 析构对象
template <typename T>
void destroy_one(T*, std::true_type) {}

template <typename T>
void destroy_one(T* pointer, std::false_type) {
    if (pointer != nullptr) {
        pointer->~T();
    }
}

template <typename T>
void destroy(T* pointer) {
    destroy_one(pointer, std::is_trivially_destructible<T>{});
}

template <typename ForwardIter>
void destroy_cat(ForwardIter, ForwardIter, m_true_type) {}

template <typename ForwardIter>
void destroy_cat(ForwardIter first, ForwardIter last, m_false_type) {
    for (; first != last; ++first) {
        destroy(&(*first));
    }
}

template <typename ForwardIter>
void destroy(ForwardIter first, ForwardIter last) {
    destroy_cat(first, last,
        std::is_trivially_destructible<
            typename iterator_traits<ForwardIter>::value_type>{});
}

} // namespace mystl
