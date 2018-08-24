// Copyright 2018 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// -----------------------------------------------------------------------------
// File: dense_set.h
// -----------------------------------------------------------------------------
//
// A `DenseSet<T, C, A>` represents container adaptor of a std::vector, that mimics
// functionality of a std::set. It utilizes spans, that segment the data into a
// balanced binary trie.

#ifndef ABSL_CONTAINER_DENSE_SET_H_
#define ABSL_CONTAINER_DENSE_SET_H_

#include <algorithm>
#include <vector>

#include "absl/algorithm/algorithm.h"
#include "absl/memory/memory.h"
#include "absl/meta/type_traits.h"

namespace absl {

// -----------------------------------------------------------------------------
// `DenseSet`
// -----------------------------------------------------------------------------
//
//
template <typename T,
          typename C = std::less<T>,
          typename A = std::allocator<T>>
class DenseSet : private std::vector<T, A> {
    static constexpr bool NoexceptMoveConstructible() {
      return std::is_move_constructible<value_type>::value &&
             std::is_move_constructible<C>::value &&
             absl::allocator_is_nothrow<allocator_type>::value;
    }
    static constexpr bool NoexceptMoveAssignable() {
      return absl::is_move_assignable<value_type>::value &&
             absl::is_move_assignable<C>::value &&
             absl::is_move_assignable<allocator_type>::value;
    }

   public:
    using Base = std::vector<T,A>;
    using allocator_type = typename Base::allocator_type;
    using key_type = typename Base::value_type;
    using value_type = typename Base::value_type;
    using pointer = typename Base::pointer;
    using const_pointer = typename Base::const_pointer;
    using reference = typename Base::reference;
    using const_reference = typename Base::const_reference;
    using size_type = typename Base::size_type;
    using difference_type = typename Base::difference_type;
    using iterator = typename Base::const_iterator;
    using const_iterator = typename Base::const_iterator;
    using reverse_iterator = typename Base::const_reverse_iterator;
    using const_reverse_iterator = typename Base::const_reverse_iterator;

    DenseSet() = default;

    explicit DenseSet(const allocator_type& alloc)
        : Base(alloc) {}

    explicit DenseSet(const C& comp, const allocator_type& alloc = allocator_type())
        : Base(alloc), comp_{comp} {}

    DenseSet(const DenseSet& other) = default;
    DenseSet(const DenseSet& other, const allocator_type& alloc)
        : Base(other, alloc), comp_{other.comp_} {}

    DenseSet(DenseSet&& other) noexcept(NoexceptMoveConstructible())
        : DenseSet(std::forward<DenseSet>(other), allocator_type())
    {
        std::swap(comp_, other.comp_);
    }

    DenseSet(DenseSet&& other, const allocator_type& alloc) noexcept(NoexceptMoveConstructible())
        : Base(std::forward<Base>(other), alloc)
    {
        std::swap(comp_, other.comp_);
    }

    template<typename InputIt >
    DenseSet(
         InputIt first, InputIt last,
         const C& comp = C(),
         const allocator_type& alloc = allocator_type())
        : Base(alloc), comp_{comp}
    {
        insert(first, last);
    }

    template<typename InputIt >
    DenseSet(
            InputIt first, InputIt last,
            const allocator_type& alloc)
        : DenseSet(first, last, C(), alloc) {}

    DenseSet(
            std::initializer_list<value_type>&& init,
            const C& comp = C(),
            const allocator_type& alloc = allocator_type())
        : Base(alloc)
        , comp_{comp}
    {
        insert(init.begin(), init.end());
    }

    DenseSet(
            std::initializer_list<value_type> init,
            const allocator_type& alloc)
        : DenseSet(std::forward<std::initializer_list<value_type>>(init), C(), alloc) {}

    DenseSet& operator=(const DenseSet& other) = default;
    DenseSet& operator=(DenseSet&& other) noexcept(NoexceptMoveAssignable()) = default;

    // DenseSet::get_allocator()
    //
    // Returns the allocator of the `DenseSet`.
    allocator_type get_allocator() const { return Base::get_allocator(); };

    // DenseSet::key_comp()
    //
    // Returns the function object that compares the keys, which is a copy of
    // this container's constructor argument C. It is the same as value_comp.
    C key_comp() const { return comp_; }

    // DenseSet::value_comp()
    //
    // Returns the function object that compares the keys, which is a copy of
    // this container's constructor argument C. It is the same as key_comp.
    C value_comp() const { return comp_; }

    // DenseSet::size()
    //
    // Returns the length of the `DenseSet`.
    size_type size() const noexcept { return Base::size(); }

    // DenseSet::max_size()
    //
    // Returns the largest possible value of `std::distance(begin(), end())` for a
    // `DenseSet<T, C, A>`. This is equivalent to the most possible addressable bytes
    // over the number of bytes taken by T.
    constexpr size_type max_size() const {
      return std::numeric_limits<difference_type>::max() / sizeof(value_type);
    }

    // DenseSet::swap()
    //
    // Swaps the contents of this `DenseSet` with the contents of `other`.
    void swap(DenseSet& other) noexcept {
        Base::swap(*this, other);
        std::swap(comp_, other.comp_);
    }

    // DenseSet::empty()
    //
    // Returns whether or not the `DenseSet` is empty.
    bool empty() const noexcept { return Base::empty(); }

    // DenseSet::clear()
    //
    // Removes all elements from the `DenseSet`.
    void clear() noexcept { Base::clear(); }

    // DenseSet::contains()
    //
    // Checks whether a value is in the `DenseSet`
    // Returns the true if element is in the `DenseSet` and false otherwise
    bool contains(const key_type& key) const noexcept {
        return std::binary_search(begin(), end(), key, comp_);
    }

    // DenseSet::count()
    //
    // Checks whether a value is in the `DenseSet`
    // Returns the 1 if element is in the `DenseSet` and 0 otherwise
    size_type count(const key_type& key) const noexcept {
        return contains(key) ? 1 : 0;
    }

    // DenseSet::emplace()
    //
    // Tries to insert an element into the `DenseSet`
    // NOTE: A true emplace is simply not possible, as we need the value for comparison.
    // Consequently this simply forwards the arguments to the constructor
    template<typename... Args, typename = absl::enable_if_t<std::is_constructible<T, Args...>::value>>
    std::pair<iterator,bool> emplace(Args&&... args) {
        return insert(value_type{args...});
    }

    // DenseSet::erase()
    //
    // Erases a single element from the `DenseSet`
    // Returns the iterator pointing to the new location of the element that followed
    // the element erased by the function call
    iterator erase(const_iterator pos) {
        return Base::erase(pos);
    }

    // DenseSet::erase()
    //
    // Erases a range from the `DenseSet`
    // Returns the iterator pointing to the new location of the element that followed
    // the last element erased by the function call
    iterator erase(const_iterator first, const_iterator last) {
        return Base::erase(first, last);
    }

    // DenseSet::erase()
    //
    // Tries to erase a value from the `DenseSet`
    // Returns 1 if the element was removed and 0 otherwise
    size_type erase(const key_type& key) {
        auto pos = lower_bound(key);
        if (pos == end() || comp_(key, *pos))
            return size_type{0};
        Base::erase(pos);
        return size_type{1};
    }

    // DenseSet::find()
    //
    // Searches for a value is in the `DenseSet`
    // Returns the the iterator to the element if it is in the `DenseSet`
    // and end() otherwise
    iterator find(const key_type& key) noexcept {
        auto pos = lower_bound(key);
        return (pos == end() || comp_(key, *pos)) ? end() : pos;
    }

    // DenseSet::find()
    //
    // Searches for a value is in the `DenseSet`
    // Returns the the iterator to the element if it is in the `DenseSet`
    // and end() otherwise
    const_iterator find(const key_type& key) const noexcept {
        auto pos = lower_bound(key);
        return (pos == end() || comp_(key, *pos)) ? end() : pos;
    }

    // DenseSet::insert()
    //
    // Tries to insert an element into the `DenseSet`
    std::pair<iterator,bool> insert(const value_type& value) {
        auto pos = lower_bound(value);
        if (pos == end() || comp_(value, *pos))
            return { Base::insert(pos, value), true };
        return { pos, false };
    }

    // DenseSet::insert()
    //
    // Tries to insert an element into the `DenseSet`
    std::pair<iterator,bool> insert(value_type&& value) {
        auto pos = lower_bound(value);
        if (pos == end() || comp_(value, *pos))
            return { Base::insert(pos, std::move(value)), true };
        return { pos, false };
    }

    // DenseSet::insert()
    //
    // Tries to insert an element into the `DenseSet` first checking a hint
    iterator insert(const_iterator pos, const value_type& value) {
        if (empty())
            return Base::insert(begin(), value);
        if (pos == end()) {
            if (comp_(Base::back(), value)) {
                return Base::insert(end(), value);
            }
            return insert(value).first;
        }
        if (comp_(value, *pos))
            pos = std::lower_bound(begin(), pos, value, comp_);
        else
            pos = std::lower_bound(pos, end(), value, comp_);
        if (pos == end() || comp_(value, *pos))
            return Base::insert(pos, value);
        return pos;
    }

    // DenseSet::insert()
    //
    // Tries to insert an element into the `DenseSet` first checking a hint
    iterator insert(const_iterator pos, value_type&& value) {
        if (empty())
            return Base::insert(begin(), value);
        if (pos == end()) {
            if (comp_(Base::back(), value)) {
                return Base::insert(end(), std::move(value));
            }
            return insert(std::move(value)).first;
        }
        if (comp_(value, *pos))
            pos = std::lower_bound(begin(), pos, value, comp_);
        else
            pos = std::lower_bound(pos, end(), value, comp_);
        if (pos == end() || comp_(value, *pos))
            return Base::insert(pos, std::move(value));
        return pos;
    }

    // DenseSet::insert()
    //
    // Tries to insert a range of elements into the `DenseSet`
    template<typename InputIt>
    void insert(InputIt first, InputIt last) {
        Base::reserve(size() + std::distance(first, last));
        while (first != last) {
            insert(*first);
            ++first;
        }
    }

    // DenseSet::insert()
    //
    // Tries to insert a range of elements into the `DenseSet`
    void insert(std::initializer_list<value_type> init) {
        insert(init.begin(), init.end());
    }

    // DenseSet::lower_bound()
    //
    // Returns an iterator pointing to the first element that is not less than key
    iterator lower_bound(const key_type& key) noexcept {
        return std::lower_bound(begin(), end(), key, comp_);
    }

    // DenseSet::lower_bound()
    //
    // Returns an iterator pointing to the first element that is not less than key
    const_iterator lower_bound(const key_type& key) const noexcept {
        return std::lower_bound(begin(), end(), key, comp_);
    }

    // DenseSet::upper_bound()
    //
    // Returns an iterator pointing to the first element that is greater than key.
    iterator upper_bound(const key_type& key) noexcept {
        return std::upper_bound(begin(), end(), key, comp_);
    }

    // DenseSet::upper_bound()
    //
    // Returns an iterator pointing to the first element that is greater than key.
    const_iterator upper_bound(const key_type& key) const noexcept {
        return std::upper_bound(begin(), end(), key, comp_);
    }

    // DenseSet::equal_range()
    //
    // Returns a range containing all elements with the given key in the container.
    // The range is defined by two iterators, one pointing to the first element that
    // is not less than key and another pointing to the first element greater than key.
    std::pair<iterator, iterator> equal_range(const key_type& key) noexcept {
        return { lower_bound(key), upper_bound(key) };
    }

    // DenseSet::equal_range()
    //
    // Returns a range containing all elements with the given key in the container.
    // The range is defined by two iterators, one pointing to the first element that
    // is not less than key and another pointing to the first element greater than key.
    std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const noexcept {
        return { lower_bound(key), upper_bound(key) };
    }

    // DenseSet::begin()
    //
    // Returns an iterator to the beginning of the `DenseSet`.
    iterator begin() noexcept { return Base::begin(); }

    // DenseSet::begin()
    //
    // Returns an const_iterator to the beginning of the `DenseSet`.
    const_iterator begin() const noexcept { return Base::begin(); }

    // DenseSet::cbegin()
    //
    // Returns an const_iterator to the beginning of the `DenseSet`.
    const_iterator cbegin() const noexcept { return Base::cbegin(); }

    // DenseSet::end()
    //
    // Returns an iterator to the end of the `DenseSet`.
    iterator end() noexcept { return Base::end(); }

    // DenseSet::begin()
    //
    // Returns an const_iterator to the end of the `DenseSet`.
    const_iterator end() const noexcept { return Base::end(); }

    // DenseSet::cbegin()
    //
    // Returns an const_iterator to the end of the `DenseSet`.
    const_iterator cend() const noexcept { return Base::cend(); }

    // DenseSet::rbegin()
    //
    // Returns an reverse_iterator to the end of the `DenseSet`.
    reverse_iterator rbegin() noexcept { return Base::rbegin(); }

    // DenseSet::begin()
    //
    // Returns an const_reverse_iterator to the end of the `DenseSet`.
    const_reverse_iterator rbegin() const noexcept { return Base::rbegin(); }

    // DenseSet::cbegin()
    //
    // Returns an const_reverse_iterator to the end of the `DenseSet`.
    const_reverse_iterator crbegin() const noexcept { return Base::crbegin(); }

    // DenseSet::rend()
    //
    // Returns an reverse_iterator to the beginning of the `DenseSet`.
    reverse_iterator rend() noexcept { return Base::rend(); }

    // DenseSet::begin()
    //
    // Returns an const_reverse_iterator to the beginning of the `DenseSet`.
    const_reverse_iterator rend() const noexcept { return Base::rend(); }

    // DenseSet::crbegin()
    //
    // Returns an const_reverse_iterator to the beginning of the `DenseSet`.
    const_reverse_iterator crend() const noexcept { return Base::crend(); }

    // Relational operators. Equality operators are elementwise using
    // `operator==`, while order operators order FixedArrays lexicographically.
    friend bool operator==(const DenseSet& lhs, const DenseSet& rhs) {
      return absl::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    friend bool operator!=(const DenseSet& lhs, const DenseSet& rhs) {
      return !(lhs == rhs);
    }

    friend bool operator<(const DenseSet& lhs, const DenseSet& rhs) {
      return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                          rhs.end());
    }

    friend bool operator>(const DenseSet& lhs, const DenseSet& rhs) {
      return rhs < lhs;
    }

    friend bool operator<=(const DenseSet& lhs, const DenseSet& rhs) {
      return !(rhs < lhs);
    }

    friend bool operator>=(const DenseSet& lhs, const DenseSet& rhs) {
      return !(lhs < rhs);
    }

    template< class Key, class Compare, class Alloc >
    friend void swap(DenseSet& lhs, DenseSet& rhs) noexcept(noexcept(Base::swap(lhs, rhs))) {
        Base::swap(lhs, rhs);
    }
private:
    // Comparison functor
    C comp_{C()};
};

}  // namespace absl
#endif  // ABSL_CONTAINER_DENSE_SET_H_
