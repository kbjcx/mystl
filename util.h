#pragma once

// 工具头文件，包含 move、forward、swap等函数，以及 pair 结构

#include <cstddef>
#include <type_traits>

#include "type_traits.h"

namespace mystl {

// move
template <typename T>
typename std::remove_reference<T>::type&& move(T&& arg) noexcept {
    return static_cast<typename std::remove_reference<T>::type&&>(arg);
}

// forward
template <typename T>
T&& forward(typename std::remove_reference<T>::type& arg) noexcept {
    return static_cast<T&&>(arg);
}

template <typename T>
T&& forward(typename std::remove_reference<T>::type&& arg) noexcept {
    static_assert(!std::is_lvalue_reference<T>::value, "bad forward");
    return static_cast<T&&>(arg);
}

// swap
template <typename T>
void swap(T& lhs, T& rhs) {
    auto tmp(mystl::move(lhs));
    lhs = mystl::move(rhs);
    rhs = mystl::move(tmp);
}

template <typename ForwardIter1, typename ForwardIter2>
ForwardIter2 swap_range(ForwardIter1 first1, ForwardIter1 last1, ForwardIter2 first2) {
    for (; first1 != last1; ++first1, ++first2) {
        mystl::swap(*first1, *first2);
    }

    return first2;
}

template <typename T, size_t N>
void swap(T (&a)[N], T (&b)[N]) {
    mystl::swap_range(a, a + N, b);
}

/******************************************************************************/
// pair
template <typename T1, typename T2>
struct pair {
    using first_type = T1;
    using second_type = T2;

    first_type first;
    second_type second;

    // 默认构造函数
    template <typename Other1 = T1, typename Other2 = T2,
        typename =
            typename std::enable_if<std::is_default_constructible<Other1>::value &&
                                        std::is_default_constructible<Other2>::value,
                void>::value>
    constexpr pair()
        : first()
        , second() {}

    template <typename U1 = T1, typename U2 = T2,
        typename std::enable_if<std::is_copy_constructible<U1>::value &&
                                    std::is_copy_constructible<U2>::value &&
                                    std::is_convertible<const U1&, T1>::value &&
                                    std::is_convertible<const U2&, T2>::value,
            int>::type = 0>
    constexpr pair(const T1& a, const T2& b)
        : first(a)
        , second(b) {}

    template <typename U1 = T1, typename U2 = T2,
        typename std::enable_if<std::is_copy_constructible<U1>::value &&
                                    std::is_copy_constructible<U2>::value &&
                                    (!std::is_convertible<const U1&, T1>::value ||
                                        !std::is_convertible<const U2&, T2>::value),
            int>::type = 0>
    explicit constexpr pair(const T1& a, const T2& b)
        : first(a)
        , second(b) {}

    pair(const pair&) = default;
    pair(pair&&) = default;
};

} // namespace mystl
