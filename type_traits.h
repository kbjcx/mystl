#pragma once

namespace mystl {

template <typename T, T v>
struct m_integral_constant {
    static constexpr T value = v;
};

template <bool b>
using m_bool_constant = m_integral_constant<bool, b>;

using m_true_type = m_bool_constant<true>;
using m_false_type = m_bool_constant<false>;

template <typename T1, typename T2>
struct pair;

template <typename T>
struct is_pair : mystl::m_false_type {};

template <typename T1, typename T2>
struct is_pair<mystl::pair<T1, T2>> : mystl::m_true_type {};

} // namespace mystl
