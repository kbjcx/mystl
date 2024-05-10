#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>

#include "type_traits.h"

namespace mystl {

// 五种迭代器类型
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

/* iterator模板
 * 迭代器中包含了5种内嵌类型，其中迭代器之间的距离默认为指针距离，
 * 指针类型与引用类型默认为普通指针与引用
 */
template <typename Category, typename T, typename Distance = ptrdiff_t,
    typename Pointer = T*, typename Reference = T&>
struct iterator {
    // clang-format off
    using iterator_category = Category;
    using value_type        = T;
    using pointer           = Pointer;
    using reference         = Reference;
    using distance_type     = Distance;
    // clang-format on
};

// iterator traits

template <typename T>
struct has_iterator_cat {
private:
    struct two {
        char a;
        char b;
    };

    // 使用了可变参数来确保此版本总是可以匹配，返回值为 two
    template <typename U>
    static two test(...);

    // 如果 U 类型具有内嵌类型 iterator_category，则调用此版本，返回值为char
    template <typename U>
    static char test(typename U::iterator_category* = 0);

public:
    /*
     * 因此可以根据test返回值的大小是否等于char来判断一个类型中是否定义了iterator_category
     */

    static const bool value = sizeof(test<T>(0)) == sizeof(char);
};

template <typename Iterator, bool>
struct iterator_traits_impl {};

/*
 * 上述结构体的偏特化版本
 */
template <typename Iterator>
struct iterator_traits_impl<Iterator, true> {
    // clang-format off
    using iterator_category = typename Iterator::iterator_category;
    using value_type        = typename Iterator::value_type;
    using pointer           = typename Iterator::pointer;
    using reference         = typename Iterator::reference;
    using difference_type   = typename Iterator::difference_type;
    // clang-format on
};

template <typename Iterator, bool>
struct iterator_traits_helper {};

// 检查Iterator类型如果具有iterator_category类型，再判断是否属于上述 5 种迭代器类型
template <typename Iterator>
struct iterator_traits_helper<Iterator, true>
    : public iterator_traits_impl<Iterator,
          std::is_convertible<typename Iterator::iterator_category,
              input_iterator_tag>::value ||
              std::is_convertible<typename Iterator::iterator_category,
                  output_iterator_tag>::value> {};

// 萃取迭代器的特性

template <typename Iterator>
struct iterator_traits
    : public iterator_traits_helper<Iterator, has_iterator_cat<Iterator>::value> {};

// 针对原生指针定义偏特化版本
template <typename T>
struct iterator_traits<T*> {
    // clang-format off
    using iterator_category = random_access_iterator_tag;
    using value_type        = T;
    using pointer           = T*;
    using reference         = T&;
    using difference_type   = ptrdiff_t;
    // clang-format on
};

template <typename T>
struct iterator_traits<const T*> {
    // clang-format off
    using iterator_category = random_access_iterator_tag;
    using value_type        = T;
    using pointer           = const T*;
    using reference         = const T&;
    using difference_type   = ptrdiff_t;
    // clang-format on
};

/*
 * 当 has_iterator_cat<iterator_traits<T>>::value 为false实例化偏特化版本
 * 为true时根据iterator_traits<T>::iterator_category来判断
 */
template <typename T, typename U, bool = has_iterator_cat<iterator_traits<T>>::value>
struct has_iterator_cat_of
    : public m_bool_constant<
          std::is_convertible<typename iterator_traits<T>::iterator_category, U>::value> {
};

template <typename T, typename U>
struct has_iterator_cat_of<T, U, false> : public m_false_type {};

// 萃取某种迭代器
template <typename Iter>
struct is_input_iterator : public has_iterator_cat_of<Iter, input_iterator_tag> {};

template <typename Iter>
struct is_output_iterator : public has_iterator_cat_of<Iter, output_iterator_tag> {};

template <typename Iter>
struct is_forward_iterator : public has_iterator_cat_of<Iter, forward_iterator_tag> {};

template <typename Iter>
struct is_bidirectional_iterator
    : public has_iterator_cat_of<Iter, bidirectional_iterator_tag> {};

template <typename Iter>
struct is_random_access_iterator
    : public has_iterator_cat_of<Iter, random_access_iterator_tag> {};

template <typename Iterator>
struct is_iterator : public m_bool_constant<is_input_iterator<Iterator>::value ||
                                            is_output_iterator<Iterator>::value> {};

// 萃取某个迭代器的category
template <typename Iterator>
typename iterator_traits<Iterator>::iterator_category iterator_category(const Iterator&) {
    using category = typename iterator_traits<Iterator>::iterator_category;
    return category();
}

// 萃取某个迭代器的diffenence_type
template <typename Iterator>
typename iterator_traits<Iterator>::difference_type* distance_type(const Iterator&) {
    using distance = typename iterator_traits<Iterator>::difference_type;
    return static_cast<distance*>(0);
}

// 萃取某个迭代器的value_type
template <typename Iterator>
typename iterator_traits<Iterator>::value_type* value_type(const Iterator&) {
    using value = typename iterator_traits<Iterator>::value_type;
    return static_cast<value*>(0);
}

// 计算迭代器之间的距离
// 重载计算距离的函数模板，根据不同迭代器的特征来简化操作
template <typename InputIterator>
typename iterator_traits<InputIterator>::difference_type distance_dispatch(
    InputIterator first, InputIterator last, input_iterator_tag) {
    typename iterator_traits<InputIterator>::difference_type n = 0;
    while (first != last) {
        ++n;
        ++first;
    }
    return n;
}

template <typename RandomIterator>
typename iterator_traits<RandomIterator>::difference_type distance_dispatch(
    RandomIterator first, RandomIterator last, random_access_iterator_tag) {
    return last - first;
}

template <typename InputIterator>
typename iterator_traits<InputIterator>::difference_type distance(
    InputIterator first, InputIterator last) {
    return distance_dispatch(first, last, iterator_category(first));
}

// 用于让迭代器前进 n 个距离
template <typename InputIterator, typename Distance>
void advance_dispatch(InputIterator& i, Distance n, input_iterator_tag) {
    while (n--) {
        ++i;
    }
}

template <typename BiIter, typename Distance>
void advance_dispatch(BiIter& i, Distance n, bidirectional_iterator_tag) {
    if (n >= 0) {
        while (n-- > 0) {
            ++i;
        }
    } else {
        while (n++ < 0) {
            --i;
        }
    }
}

template <typename RandomIter, typename Distance>
void advance_dispatch(RandomIter& i, Distance n, random_access_iterator_tag) {
    i += n;
}

template <typename InputIterator, typename Distance>
void advance(InputIterator& i, Distance n) {
    advance_dispatch(i, n, iterator_category(i));
}

/******************************************************************************/
// 反向迭代器
template <typename Iterator>
class reverse_iterator {
private:
    Iterator current; // 记录对应的正向迭代器

public:
    // 反向迭代器的 5 种内嵌类型
    // clang-format off
    using iterator_category = typename iterator_traits<Iterator>::iterator_category;
    using value_type        = typename iterator_traits<Iterator>::value_type;
    using pointer           = typename iterator_traits<Iterator>::pointer;
    using reference         = typename iterator_traits<Iterator>::reference;
    using difference_type   = typename iterator_traits<Iterator>::difference_type;
    using iterator_type     = Iterator;
    using self              = reverse_iterator<Iterator>;
    // clang-format on

public:
    // 构造函数
    reverse_iterator() = default;
    explicit reverse_iterator(iterator_type i)
        : current(i) {}
    reverse_iterator(const self& rhs)
        : current(rhs.current) {}

public:
    // 取出对应的正向迭代器
    iterator_type base() const {
        return current;
    }

    // 重载操作符
    reference operator*() const {
        // 实际上对应正向迭代器的前一个位置
        Iterator tmp = current;
        return *(--tmp);
    }

    pointer operator->() const {
        return &(operator*());
    }

    // 前进++变成后退
    self& operator++() {
        --current;
        return *this;
    }

    self operator++(int) {
        self tmp = *this;
        --current;
        return tmp;
    }

    // 后退变为前进
    self& operator--() {
        ++current;
        return *this;
    }

    self operator--(int) {
        self tmp = *this;
        ++current;
        return tmp;
    }

    self& operator+=(difference_type n) {
        current -= n;
        return *this;
    }

    self operator+(difference_type n) const {
        return self(current - n);
    }

    self& operator-=(difference_type n) {
        current += n;
        return *this;
    }

    self operator-(difference_type n) const {
        return self(current + n);
    }

    reference operator[](difference_type n) const {
        return *(*this + n);
    }
};

template <typename Iterator>
typename reverse_iterator<Iterator>::difference_type operator-(
    const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
    return rhs.base() - lhs.base();
}

// 重载比较操作符
template <typename Iterator>
bool operator==(
    const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
    return lhs.base() == rhs.base();
}

template <typename Iterator>
bool operator<(
    const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
    return lhs.base() > rhs.base();
}

template <typename Iterator>
bool operator!=(
    const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
    return !(lhs == rhs);
}

template <typename Iterator>
bool operator>(
    const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
    return rhs < lhs;
}

template <typename Iterator>
bool operator<=(
    const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
    return !(rhs < lhs);
}

template <typename Iterator>
bool operator>=(
    const reverse_iterator<Iterator>& lhs, const reverse_iterator<Iterator>& rhs) {
    return !(lhs < rhs);
}

} // namespace mystl
