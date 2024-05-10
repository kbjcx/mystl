#pragma once

#include <initializer_list>

#include "exceptdef.h"
#include "iterator.h"
#include "memory.h"
#include "util.h"

namespace mystl {

#ifdef max
#pragma message("#undefing marco max")
#undef max
#endif // max

#ifdef min
#pragma message("#undefing marco min")
#undef min
#endif // min

template <typename T>
class vector {
    static_assert(!std::is_same<bool, T>::value, "vector bool is abandoned in mystl");

public:
    // clang-format off
    using allocator_type = mystl::allocator<T>;
    using data_allocator = mystl::allocator<T>;

    using value_type                = typename allocator_type::value_type;
    using pointer                   = typename allocator_type::pointer;
    using const_pointer             = typename allocator_type::const_pointer;
    using reference                 = typename allocator_type::reference;
    using const_reference           = typename allocator_type::const_reference;
    using size_type                 = typename allocator_type::size_type;
    using difference_type           = typename allocator_type::difference_type;

    using iterator                  = value_type*;
    using const_iterator            = const value_type*;
    using reverse_iterator          = mystl::reverse_iterator<iterator>;
    using const_reverse_iterator    = mystl::reverse_iterator<const_iterator>;
    // clang-format on

    allocator_type get_allocator() {
        return data_allocator();
    }

private:
    iterator begin_;
    iterator end_;
    iterator cap_;

public:
    vector() noexcept {
        try_init();
    }

    explicit vector(size_type n) {
        fill_init(n, value_type{});
    }

    vector(size_type n, const value_type& value) {
        fill_init(n, value);
    }

    template <typename Iter,
        typename std::enable_if<mystl::is_input_iterator<Iter>::value, int>::type = 0>
    vector(Iter first, Iter last) {
        MYSTL_DEBUG(!(last < first));
        range_init(first, last);
    }

    vector(const vector& rhs) {
        range_init(rhs.begin(), rhs.end());
    }

    vector(vector&& rhs) noexcept
        : begin_(rhs.begin())
        , end_(rhs.end())
        , cap_(rhs.cap_) {
        rhs.begin() = nullptr;
        rhs.end() = nullptr;
        rhs.cap_ = nullptr;
    }

    vector(std::initializer_list<value_type> ilist) {
        range_init(ilist.begin(), ilist.end());
    }

    vector& operator=(const vector& rhs) {
        if (this != &rhs) {
            const auto len = rhs.size();
            if (len > capacity()) {
                vector tmp(rhs.begin(), rhs.end());
                swap(tmp);
            } else if (size() >= len) {
                // 如果已有的数据多余赋值的数据，就需要把多余的半部分删除
                // 将前面的部分拷贝到目标位置
                auto iter = mystl::copy(rhs.begin(), rhs.end(), begin());
                // 将后续多余的数据删除掉
                data_allocator::destroy(iter, end());
                end() = begin() + len;
            } else {
                mystl::copy(rhs.begin(), rhs.begin() + size(), begin());
                mystl::uninitialized_copy(rhs.begin() + size(), rhs.end(), end());
                end() = begin() + len;
                cap_ = end();
            }
        }

        return *this;
    }

    vector& operator=(vector&& rhs) {
        destroy_and_recover(begin(), end(), capacity());
        begin() = rhs.begin();
        end() = rhs.end();
        cap_ = rhs.cap_;
        rhs.begin() = nullptr;
        rhs.end() = nullptr;
        rhs.cap_ = nullptr;
        return *this;
    }

    vector& operator=(std::initializer_list<value_type> ilist) {
        vector tmp(ilist.begin(), ilist.end());
        swap(tmp);
        return *this;
    }

    ~vector() {
        destroy_and_recover(begin(), end(), capacity());
        begin() = nullptr;
        end() = nullptr;
        cap_ = nullptr;
    }

public:
    // 迭代器相关操作
    iterator begin() {
        return begin_;
    }
    const_iterator begin() const {
        return begin_;
    }
    iterator end() {
        return end_;
    }
    const_iterator end() const {
        return end_;
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const {
        return const_reverse_iterator(end());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const {
        return const_reverse_iterator(begin());
    }

    const_iterator cbegin() {
        return begin();
    }

    const_iterator cend() {
        return end();
    }

    const_reverse_iterator crbegin() {
        return rbegin();
    }

    const_reverse_iterator crend() {
        return rend();
    }

    // 容量相关操作
    bool empty() const {
        return begin() == end();
    }

    size_type size() const {
        return static_cast<size_type>(end() - begin());
    }

    size_type max_size() const {
        return static_cast<size_type>(-1) / sizeof(T);
    }

    size_type capacity() const {
        return static_cast<size_type>(cap_ - begin());
    }

    void reserve(size_type n) {
        if (capacity() < n) {
            THROW_LENGTH_ERROR_IF(n > max_size(),
                "n can not larger than max_size() in vector<T>::reverse(n)");
            const auto old_size = size();
            auto tmp = data_allocator::allocate(n);
            mystl::uninitialized_move(begin(), end(), tmp);
            data_allocator::deallocate(begin(), capacity());
            begin() = tmp;
            end() = tmp + old_size;
            cap_ = begin() + n;
        }
    }

    void shrink_to_fit() {
        if (end() < cap_) {
            reinsert(size());
        }
    }

    // 访问元素相关操作
    reference operator[](size_type n) {
        MYSTL_DEBUG(n < size());
        return *(begin() + n);
    }

    const_reference operator[](size_type n) const {
        MYSTL_DEBUG(n < size());
        return *(begin() + n);
    }

    reference at(size_type n) {
        THROW_LENGTH_ERROR_IF(!(n < size()), "vertor<T>::at() subscript out of range");
        return (*this)[n];
    }

    const_reference at(size_type n) const {
        THROW_LENGTH_ERROR_IF(!(n < size()), "vertor<T>::at() subscript out of range");
        return (*this)[n];
    }

    reference front() {
        MYSTL_DEBUG(!empty());
        return *begin();
    }

    const_reference front() const {
        MYSTL_DEBUG(!empty());
        return *begin();
    }

    reference back() {
        MYSTL_DEBUG(!empty());
        return *(end() - 1);
    }

    const_reference back() const {
        MYSTL_DEBUG(!empty());
        return *(end() - 1);
    }

    pointer data() {
        return begin();
    }

    const_pointer data() const {
        return begin();
    }

    // 修改容器相关操作

    // asign
    void assign(size_type n, const value_type& value) {
        fill_assign(n, value);
    }

    template <typename Iter,
        typename std::enable_if<mystl::is_input_iterator<Iter>::value, int>::value = 0>
    void assign(Iter first, Iter last) {
        MYSTL_DEBUG(!(last < first));
        copy_assign(first, last, iterator_category(first));
    }

    void assign(std::initializer_list<value_type> ilist) {
        copy_assign(ilist.first, ilist.end(), mystl::forward_iterator_tag{});
    }

    // emplace / emplace_back
    template <typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        MYSTL_DEBUG(pos >= begin() && pos <= end());
        iterator xpos = const_cast<iterator>(pos);
        const size_type n = xpos - begin();
        if (end() != cap_ && xpos == end()) {
            data_allocator::construct(
                mystl::address_of(*end()), mystl::forward<Args>(args)...);
            ++end();
        } else if (end() != cap_) {
            auto new_end = end();
            data_allocator::construct(mystl::address_of(*end()), *(end() - 1));
            ++new_end;
            mystl::copy_backward(xpos, end() - 1, end());
            *xpos = value_type(mystl::forward<Args>(args)...);
        } else {
            reallocate_emplace(xpos, mystl::forward<Args>(args)...);
        }

        return begin() + n;
    }

    template <typename... Args>
    iterator emplace_back(Args&&... args) {
        if (end() < cap_) {
            data_allocator::construct(
                mystl::address_of(*end()), mystl::forward<Args>(args)...);
            ++end();
        } else {
            reallocate_emplace(end(), mystl::forward<Args>(args)...);
        }
    }

    // push_back / pop_back
    void push_back(const value_type& value) {
        if (end() != cap_) {
            data_allocator::construct(mystl::address_of(*end()), value);
            ++end();
        } else {
            reallocate_insert(end(), value);
        }
    }
    void push_back(value_type&& value) {
        emplace_back(mystl::move(value));
    }

    void pop_back() {
        MYSTL_DEBUG(!empty());
        data_allocator::destroy(end() - 1);
        --end();
    }

    // insert
    iterator insert(const_iterator pos, const value_type& value) {
        MYSTL_DEBUG(pos >= begin() && pos <= end());
        iterator xpos = const_cast<iterator>(pos);
        const size_type n = pos - begin();
        if (end() != cap_ && xpos == end()) {
            data_allocator::construct(mystl::address_of(*end()), value);
            ++end();
        } else if (end() != cap_) {
            auto new_end = end();
            data_allocator::construct(mystl::address_of(*end()), *(end() - 1));
            ++new_end;
            auto value_copy = value; // 避免元素被下面的复制操作改变
            mystl::copy_backward(xpos, end() - 1, end());
            *xpos = mystl::move(value_copy);
        } else {
            reallocate_insert(xpos, value);
        }

        return begin() + n;
    }

    iterator insert(const_iterator pos, value_type&& value) {
        return emplace(pos, mystl::move(value));
    }

    iterator insert(const_iterator pos, size_type n, const value_type& value) {
        MYSTL_DEBUG(pos >= begin() && pos <= end());
        return fill_insert(const_cast<iterator>(pos), n, value);
    }

    template <typename Iter,
        typename std::enable_if<mystl::is_input_iterator<Iter>::value, int>::value = 0>
    void insert(const_iterator pos, Iter first, Iter last) {
        MYSTL_DEBUG(pos >= begin() && pos <= end() && !(last < first));
        copy_insert(const_cast<iterator>(pos), first, last);
    }

    // erase / clear
    iterator erase(const_iterator pos);
    iterator erase(const_iterator first, const_iterator last);

    void clear();

    // resize / reverse
    void resize(size_type new_size);
    void resize(size_type new_size, const value_type& value);

    void reverse();

    // swap
    void swap(vector& rhs);

private:
    // helper functions

    // initialize / destroy
    void try_init();

    void init_space(size_type n, size_type cap);

    void fill_init(size_type n, const value_type& value);

    template <typename Iter>
    void range_init(Iter first, Iter last);

    void destroy_and_recover(iterator first, iterator last, size_type n);

    // calculate the growth size
    size_type get_new_cap(size_type add_size);

    // assign
    void fill_assign(size_type n, const value_type& value);

    template <typename Iter>
    void copy_assign(Iter first, Iter last, input_iterator_tag);

    template <typename Iter>
    void copy_assign(Iter first, Iter last, forward_iterator_tag);

    // reallocate
    template <typename... Args>
    void reallocate_emplace(iterator pos, Args&&... args);

    void reallocate_insert(iterator pos, const value_type& value);

    // insert
    iterator fill_insert(iterator pos, size_type n, const value_type& value);

    template <typename Iter>
    void copy_insert(iterator pos, Iter first, Iter last);

    // shrink to fit
    void reinsert(size_type n);
};

} // namespace mystl
