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

#include "absl/container/dense_set.h"

#include <algorithm>
#include <set>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

template <class C>
class test_compare : private C {
    int data_;
public:
    explicit test_compare(int data = 0) : data_(data) {}

    typename C::result_type
    operator()(typename std::add_lvalue_reference<const typename C::first_argument_type>::type x,
               typename std::add_lvalue_reference<const typename C::second_argument_type>::type y) const
        {return C::operator()(x, y);}

    bool operator==(const test_compare& c) const
        {return data_ == c.data_;}

    bool operator!=(const test_compare& c) const
        {return !(*this == c); }
};
using Cmp = test_compare<std::less<int>>;
using CmpG = test_compare<std::greater<int>>;

template <typename T>
class test_alloc : public std::allocator<T> {
public:
    using Alloc = std::allocator<T>;
    using pointer = typename Alloc::pointer;
    using size_type = typename Alloc::size_type;

    test_alloc() = default;
    explicit test_alloc(int64_t b) : data_(b) {}

    template <typename U>
    test_alloc(const test_alloc<U>& x)
      : Alloc(x), data_(x.data_) {}

    pointer allocate(size_type n,
                   std::allocator<void>::const_pointer hint = nullptr) {
        return Alloc::allocate(n, hint);
    }

    void deallocate(pointer p, size_type n) {
        Alloc::deallocate(p, n);
    }

    template<typename U>
    class rebind {
    public:
        using other = test_alloc<U>;
    };

    friend bool operator==(const test_alloc& a,
                           const test_alloc& b) {
    return a.data_ == b.data_;
    }

    friend bool operator!=(const test_alloc& a,
                           const test_alloc& b) {
    return !(a == b);
    }
    int64_t data_{ 0 };
};
using Alloc = test_alloc<int>;

using denseSet = absl::DenseSet<int, test_compare<std::less<int>>, test_alloc<int>>;
using set = std::set<int, test_compare<std::less<int>>, test_alloc<int>>;

TEST(DenseSetConstructor, Default) {
  denseSet mySet;
  EXPECT_TRUE(mySet.empty());
  EXPECT_EQ(mySet.size(), 0);
  EXPECT_EQ(mySet.begin(), mySet.end());
  EXPECT_EQ(mySet.value_comp(), Cmp(0));
  EXPECT_EQ(mySet.key_comp(), Cmp(0));
  EXPECT_EQ(mySet.get_allocator(), Alloc(0));
}

TEST(DenseSetConstructor, DefaultComp) {
  denseSet mySet(Cmp(2));
  EXPECT_TRUE(mySet.empty());
  EXPECT_EQ(mySet.size(), 0);
  EXPECT_EQ(mySet.begin(), mySet.end());
  EXPECT_EQ(mySet.value_comp(), Cmp(2));
  EXPECT_EQ(mySet.key_comp(), Cmp(2));
  EXPECT_EQ(mySet.get_allocator(), Alloc(0));
}

TEST(DenseSetConstructor, DefaultAlloc) {
  denseSet mySet(Alloc(2));
  EXPECT_TRUE(mySet.empty());
  EXPECT_EQ(mySet.size(), 0);
  EXPECT_EQ(mySet.begin(), mySet.end());
  EXPECT_EQ(mySet.value_comp(), Cmp(0));
  EXPECT_EQ(mySet.key_comp(), Cmp(0));
  EXPECT_EQ(mySet.get_allocator(), Alloc(2));
}

TEST(DenseSetConstructor, DefaultCompAlloc) {
  denseSet mySet(Cmp(2), Alloc(2));
  EXPECT_TRUE(mySet.empty());
  EXPECT_EQ(mySet.size(), 0);
  EXPECT_EQ(mySet.begin(), mySet.end());
  EXPECT_EQ(mySet.value_comp(), Cmp(2));
  EXPECT_EQ(mySet.key_comp(), Cmp(2));
  EXPECT_EQ(mySet.get_allocator(), Alloc(2));
}

TEST(DenseSetConstructor, InitializerList) {
  denseSet mySet({1, 7, 5});
  set reference{1, 7, 5};
  EXPECT_TRUE(std::equal(mySet.begin(), mySet.end(), reference.begin()));
  EXPECT_EQ(mySet.value_comp(), Cmp(0));
  EXPECT_EQ(mySet.key_comp(), Cmp(0));
  EXPECT_EQ(mySet.get_allocator(), Alloc(0));
}

TEST(DenseSetConstructor, InitializerListComp) {
  absl::DenseSet<int, CmpG> mySet({1, -2, 5}, CmpG(4));
  std::set<int, CmpG> reference({1, -2, 5}, CmpG(4));
  EXPECT_TRUE(std::equal(mySet.begin(), mySet.end(), reference.begin()));
  EXPECT_EQ(mySet.value_comp(), CmpG(4));
  EXPECT_EQ(mySet.key_comp(), CmpG(4));
  EXPECT_EQ(mySet.get_allocator(), Alloc(0));
}

TEST(DenseSetConstructor, InitializerListAlloc) {
  absl::DenseSet<int, CmpG, Alloc> mySet({1, -2, 5}, Alloc(3));
  std::set<int, CmpG> reference({1, -2, 5}, Alloc(3));
  EXPECT_TRUE(std::equal(mySet.begin(), mySet.end(), reference.begin()));
  EXPECT_EQ(mySet.value_comp(), CmpG(0));
  EXPECT_EQ(mySet.key_comp(), CmpG(0));
  EXPECT_EQ(mySet.get_allocator(), Alloc(3));
}

TEST(DenseSetConstructor, InitializerListCompAlloc) {
  absl::DenseSet<int, CmpG> mySet({1, -2, 5}, CmpG(4), Alloc(3));
  std::set<int, CmpG> reference({1, -2, 5}, CmpG(4), Alloc(3));
  EXPECT_TRUE(std::equal(mySet.begin(), mySet.end(), reference.begin()));
  EXPECT_EQ(mySet.value_comp(), CmpG(4));
  EXPECT_EQ(mySet.key_comp(), CmpG(4));
  EXPECT_EQ(mySet.get_allocator(), Alloc(3));
}

TEST(DenseSetConstructor, InitializerListDuplicate) {
  absl::DenseSet<int, CmpG> mySet({1, -2, 5, 1}, CmpG(4));
  std::set<int, CmpG> reference({1, -2, 5, 1}, CmpG(4));
  EXPECT_FALSE(mySet.empty());
  EXPECT_TRUE(std::equal(mySet.begin(), mySet.end(), reference.begin()));
  EXPECT_EQ(mySet.value_comp(), CmpG(4));
  EXPECT_EQ(mySet.key_comp(), CmpG(4));
  EXPECT_EQ(mySet.get_allocator(), Alloc(0));
}

TEST(DenseSetConstructor, Iterators) {
  auto input = {4, 13, 5, 9};
  denseSet mySet(input.begin(), input.end());
  set reference(input.begin(), input.end());
  EXPECT_TRUE(std::equal(mySet.begin(), mySet.end(), reference.begin()));
  EXPECT_EQ(mySet.size(), reference.size());
  EXPECT_EQ(mySet.value_comp(), Cmp(0));
  EXPECT_EQ(mySet.key_comp(), Cmp(0));
  EXPECT_EQ(mySet.get_allocator(), Alloc(0));
}

TEST(DenseSetConstructor, IteratorsComp) {
  auto input = {25, 13, 7, 1};
  absl::DenseSet<int, CmpG> mySet(input.begin(), input.end(), CmpG(3));
  std::set<int, CmpG> reference(input.begin(), input.end(),  CmpG(3));
  EXPECT_TRUE(std::equal(mySet.begin(), mySet.end(), reference.begin()));
  EXPECT_EQ(mySet.value_comp(), CmpG(3));
  EXPECT_EQ(mySet.key_comp(), CmpG(3));
  EXPECT_EQ(mySet.get_allocator(), Alloc(0));
}

TEST(DenseSetConstructor, IteratorsAlloc) {
  auto input = {25, 13, 7, 1};
  absl::DenseSet<int, CmpG, Alloc> mySet(input.begin(), input.end(), Alloc(3));
  std::set<int, CmpG> reference(input.begin(), input.end(),  Alloc(3));
  EXPECT_TRUE(std::equal(mySet.begin(), mySet.end(), reference.begin()));
  EXPECT_EQ(mySet.value_comp(), CmpG(0));
  EXPECT_EQ(mySet.key_comp(), CmpG(0));
  EXPECT_EQ(mySet.get_allocator(), Alloc(3));
}

TEST(DenseSetConstructor, IteratorsCompAlloc) {
  auto input = {25, 13, 7, 1};
  absl::DenseSet<int, CmpG, Alloc> mySet(input.begin(), input.end(), CmpG(3), Alloc(4));
  std::set<int, CmpG> reference(input.begin(), input.end(),  CmpG(3), Alloc(4));
  EXPECT_TRUE(std::equal(mySet.begin(), mySet.end(), reference.begin()));
  EXPECT_EQ(mySet.value_comp(), CmpG(3));
  EXPECT_EQ(mySet.key_comp(), CmpG(3));
  EXPECT_EQ(mySet.get_allocator(), Alloc(4));
}

TEST(DenseSetConstructor, IteratorsDuplicate) {
  auto input = {4, 13, 5, 5, 9};
  denseSet mySet(input.begin(), input.end());
  set reference(input.begin(), input.end());
  EXPECT_TRUE(std::equal(mySet.begin(), mySet.end(), reference.begin()));
  EXPECT_EQ(mySet.value_comp(), Cmp(0));
  EXPECT_EQ(mySet.key_comp(), Cmp(0));
  EXPECT_EQ(mySet.get_allocator(), Alloc(0));
}

TEST(DenseSetConstructor, CopyConstructor) {
    denseSet mySet({1, 7, 5});
    denseSet otherSet(mySet);
    EXPECT_EQ(mySet.size(), otherSet.size());
    EXPECT_TRUE(std::equal(otherSet.begin(), otherSet.end(), mySet.begin()));
    EXPECT_EQ(mySet.value_comp(), otherSet.value_comp());
    EXPECT_EQ(mySet.key_comp(), otherSet.key_comp());
    EXPECT_EQ(mySet.get_allocator(), otherSet.get_allocator());
}

TEST(DenseSetConstructor, CopyConstructorComp) {
    denseSet mySet({1, 7, 5}, Cmp(1));
    denseSet otherSet(mySet);
    EXPECT_EQ(mySet.size(), otherSet.size());
    EXPECT_TRUE(std::equal(otherSet.begin(), otherSet.end(), mySet.begin()));
    EXPECT_EQ(mySet.value_comp(), otherSet.value_comp());
    EXPECT_EQ(mySet.key_comp(), otherSet.key_comp());
    EXPECT_EQ(mySet.get_allocator(), otherSet.get_allocator());
}

TEST(DenseSetConstructor, CopyConstructorAlloc) {
    denseSet mySet({1, 7, 5}, Alloc(1));
    denseSet otherSet(mySet, Alloc(2));
    EXPECT_EQ(mySet.size(), otherSet.size());
    EXPECT_TRUE(std::equal(otherSet.begin(), otherSet.end(), mySet.begin()));
    EXPECT_EQ(mySet.value_comp(), otherSet.value_comp());
    EXPECT_EQ(mySet.key_comp(), otherSet.key_comp());
    EXPECT_NE(mySet.get_allocator(), otherSet.get_allocator());
}

TEST(DenseSetConstructor, CopyConstructorCompAlloc) {
    denseSet mySet({1, 7, 5}, Cmp(4), Alloc(1));
    denseSet otherSet(mySet, Alloc(2));
    EXPECT_EQ(mySet.size(), otherSet.size());
    EXPECT_TRUE(std::equal(otherSet.begin(), otherSet.end(), mySet.begin()));
    EXPECT_EQ(mySet.value_comp(), otherSet.value_comp());
    EXPECT_EQ(mySet.key_comp(), otherSet.key_comp());
    EXPECT_NE(mySet.get_allocator(), otherSet.get_allocator());
}

TEST(DenseSetConstructor, CopyAssignment) {
    denseSet mySet({-12, 42, 7, 5});
    denseSet otherSet = mySet;
    EXPECT_EQ(mySet.size(), otherSet.size());
    EXPECT_TRUE(std::equal(otherSet.begin(), otherSet.end(), mySet.begin()));
    EXPECT_EQ(mySet.value_comp(), otherSet.value_comp());
    EXPECT_EQ(mySet.key_comp(), otherSet.key_comp());
    EXPECT_EQ(mySet.get_allocator(), otherSet.get_allocator());
}

TEST(DenseSetConstructor, CopyAssignmentComp) {
    denseSet mySet({-12, 42, 7, 5}, Cmp(7));
    denseSet otherSet = mySet;
    EXPECT_EQ(mySet.size(), otherSet.size());
    EXPECT_TRUE(std::equal(otherSet.begin(), otherSet.end(), mySet.begin()));
    EXPECT_EQ(mySet.value_comp(), otherSet.value_comp());
    EXPECT_EQ(mySet.key_comp(), otherSet.key_comp());
    EXPECT_EQ(mySet.get_allocator(), otherSet.get_allocator());
}

TEST(DenseSetConstructor, CopyAssignmentAlloc) {
    denseSet mySet({-12, 42, 7, 5}, Alloc(7));
    denseSet otherSet = mySet;
    EXPECT_EQ(mySet.size(), otherSet.size());
    EXPECT_TRUE(std::equal(otherSet.begin(), otherSet.end(), mySet.begin()));
    EXPECT_EQ(mySet.value_comp(), otherSet.value_comp());
    EXPECT_EQ(mySet.key_comp(), otherSet.key_comp());
    EXPECT_EQ(mySet.get_allocator(), otherSet.get_allocator());
}

TEST(DenseSetConstructor, CopyAssignmentAllocComp) {
    denseSet mySet({-12, 42, 7, 5}, Cmp(7), Alloc(5));
    denseSet otherSet = mySet;
    EXPECT_EQ(mySet.size(), otherSet.size());
    EXPECT_TRUE(std::equal(otherSet.begin(), otherSet.end(), mySet.begin()));
    EXPECT_EQ(mySet.value_comp(), otherSet.value_comp());
    EXPECT_EQ(mySet.key_comp(), otherSet.key_comp());
    EXPECT_EQ(mySet.get_allocator(), otherSet.get_allocator());
}

TEST(DenseSetConstructor, MoveConstructor) {
    denseSet mySet({2, 11, -5, 6}, Cmp(7), Alloc(4));
    denseSet otherSet(std::move(mySet));
    EXPECT_FALSE(otherSet.empty());
    EXPECT_EQ(otherSet.size(), 4);
    EXPECT_TRUE(mySet.empty());
    auto res = { -5, 2, 6, 11 };
    EXPECT_TRUE(std::equal(otherSet.begin(), otherSet.end(), res.begin()));
    EXPECT_NE(mySet.value_comp(), otherSet.value_comp());
    EXPECT_NE(mySet.key_comp(), otherSet.key_comp());
    EXPECT_NE(mySet.get_allocator(), otherSet.get_allocator());
}

TEST(DenseSetConstructor, MoveConstructorAlloc) {
    denseSet mySet({2, 11, -5, 6}, Cmp(7), Alloc(4));
    denseSet otherSet(std::move(mySet), Alloc(4));
    EXPECT_FALSE(otherSet.empty());
    EXPECT_EQ(otherSet.size(), 4);
    EXPECT_TRUE(mySet.empty());
    auto res = { -5, 2, 6, 11 };
    EXPECT_TRUE(std::equal(otherSet.begin(), otherSet.end(), res.begin()));
    EXPECT_NE(mySet.value_comp(), otherSet.value_comp());
    EXPECT_NE(mySet.key_comp(), otherSet.key_comp());
    EXPECT_EQ(mySet.get_allocator(), otherSet.get_allocator());
}

TEST(DenseSetConstructor, MoveConstructorCompAlloc) {
    denseSet mySet({2, 11, -5, 6}, Cmp(3), Alloc(4));
    denseSet otherSet(std::move(mySet));
    EXPECT_FALSE(otherSet.empty());
    EXPECT_EQ(otherSet.size(), 4);
    EXPECT_TRUE(mySet.empty());
    auto res = { -5, 2, 6, 11 };
    EXPECT_TRUE(std::equal(otherSet.begin(), otherSet.end(), res.begin()));
    EXPECT_NE(mySet.value_comp(), otherSet.value_comp());
    EXPECT_NE(mySet.key_comp(), otherSet.key_comp());
    EXPECT_NE(mySet.get_allocator(), otherSet.get_allocator());
}

TEST(DenseSetConstructor, MoveAssignment) {
    denseSet mySet({-13, 12, -5, 5}, Cmp(5), Alloc(5));
    denseSet otherSet = std::move(mySet);
    EXPECT_FALSE(otherSet.empty());
    EXPECT_EQ(otherSet.size(), 4);
    EXPECT_TRUE(mySet.empty());
    auto res = { -13, -5, 5, 12 };
    EXPECT_TRUE(std::equal(otherSet.begin(), otherSet.end(), res.begin()));
    EXPECT_NE(mySet.value_comp(), otherSet.value_comp());
    EXPECT_NE(mySet.key_comp(), otherSet.key_comp());
    EXPECT_NE(mySet.get_allocator(), otherSet.get_allocator());
}

TEST(DenseSetUtility, Contains) {
    denseSet mySet({1, 7, 5});
    EXPECT_TRUE(mySet.contains(7));
    EXPECT_FALSE(mySet.contains(3));
}

TEST(DenseSetUtility, Find) {
    denseSet mySet({-12, 42, 7, 5});
    set reference({-12, 42, 7, 5});
    EXPECT_EQ(std::distance(mySet.find(7), mySet.end()),
              std::distance(reference.find(7), reference.end()));
    EXPECT_EQ(std::distance(mySet.find(43), mySet.end()),
              std::distance(reference.find(43), reference.end()));
}

TEST(DenseSetUtility, Size) {
    denseSet mySet({2, -5 , 4});
    EXPECT_EQ(mySet.size(), 3);
    mySet.insert(-11);
    EXPECT_EQ(mySet.size(), 4);
}

TEST(DenseSetUtility, Empty) {
    denseSet mySet;
    EXPECT_TRUE(mySet.empty());
    mySet.insert(-11);
    EXPECT_FALSE(mySet.empty());
}

TEST(DenseSetUtility, Begin) {
    auto input = { 2, 3, 4, 5 };
    denseSet mySet(input.begin(), input.end());
    set reference(input.begin(), input.end());
    EXPECT_TRUE(std::equal(mySet.begin(), mySet.end(), reference.begin()));
}

TEST(DenseSetUtility, ConstBegin) {
    auto input = { 2, 3, 4, 5 };
    const denseSet mySet(input.begin(), input.end());
    const set reference(input.begin(), input.end());
    EXPECT_TRUE(std::equal(mySet.begin(), mySet.end(), reference.begin()));
}

TEST(DenseSetUtility, CBegin) {
    auto input = { -13, -5, 5, 12 };
    denseSet mySet(input.begin(), input.end());
    set reference(input.begin(), input.end());
    EXPECT_TRUE(std::equal(mySet.cbegin(), mySet.cend(), reference.cbegin()));
}

TEST(DenseSetUtility, RBegin) {
    auto input = {-56, 3, 12, 6, 3};
    denseSet mySet(input.begin(), input.end());
    set reference(input.begin(), input.end());
    EXPECT_TRUE(std::equal(mySet.rbegin(), mySet.rend(), reference.rbegin()));
}

TEST(DenseSetUtility, ConstRBegin) {
    auto input = { 2, 3, 4, 5 };
    const denseSet mySet(input.begin(), input.end());
    const set reference(input.begin(), input.end());
    EXPECT_TRUE(std::equal(mySet.rbegin(), mySet.rend(), reference.rbegin()));
}

TEST(DenseSetUtility, CRBegin) {
    auto input = {-12, 42, 7, 5};
    denseSet mySet(input.begin(), input.end());
    set reference(input.begin(), input.end());
    EXPECT_TRUE(std::equal(mySet.crbegin(), mySet.crend(), reference.crbegin()));
}

TEST(DenseSetUtility, LowerBound) {
    auto input = {-56, 3, 12, 6, 3};
    denseSet mySet(input.begin(), input.end());
    set reference(input.begin(), input.end());
    EXPECT_EQ(std::distance(mySet.begin(), mySet.lower_bound(6)),
              std::distance(reference.begin(), reference.lower_bound(6)));
}

TEST(DenseSetUtility, LowerBoundNotFound) {
    auto input = {2, 7, 23, -12, -7};
    denseSet mySet(input.begin(), input.end());
    set reference(input.begin(), input.end());
    EXPECT_EQ(std::distance(mySet.begin(), mySet.lower_bound(79)),
              std::distance(reference.begin(), reference.lower_bound(79)));
}

TEST(DenseSetUtility, UpperBound) {
    auto input = {-56, 3, 12, 6, 3};
    denseSet mySet(input.begin(), input.end());
    set reference(input.begin(), input.end());
    EXPECT_EQ(std::distance(mySet.begin(), mySet.lower_bound(2)),
              std::distance(reference.begin(), reference.lower_bound(2)));
}

TEST(DenseSetUtility, UpperBoundNotFound) {
    auto input = {2, 7, 23, -12, -7};
    denseSet mySet(input.begin(), input.end());
    set reference(input.begin(), input.end());
    EXPECT_EQ(std::distance(mySet.begin(), mySet.lower_bound(-13)),
              std::distance(reference.begin(), reference.lower_bound(-31)));
}

TEST(DenseSetUtility, EqualRangeFound) {
    auto input = {1, 3, 5, 7, 9};
    denseSet mySet(input.begin(), input.end());
    set reference(input.begin(), input.end());
    auto res1 = mySet.equal_range(5);
    auto res2 = reference.equal_range(5);
    EXPECT_NE(res1.first, res1.second);
    EXPECT_EQ(std::distance(mySet.begin(), res1.first),
              std::distance(reference.begin(), res2.first));
    EXPECT_EQ(std::distance(mySet.begin(), res1.second),
              std::distance(reference.begin(), res2.second));
}

TEST(DenseSetUtility, EqualRangeNotFound) {
    auto input = {2, 4, 6, 7, -9};
    denseSet mySet(input.begin(), input.end());
    set reference(input.begin(), input.end());
    auto res1 = mySet.equal_range(11);
    auto res2 = reference.equal_range(11);
    EXPECT_EQ(res1.first, res1.second);
    EXPECT_EQ(std::distance(mySet.begin(), res1.first),
              std::distance(reference.begin(), res2.first));
    EXPECT_EQ(std::distance(mySet.begin(), res1.second),
              std::distance(reference.begin(), res2.second));
}

TEST(DenseSetUtility, EqualRangeEnd) {
    auto input = {2, 4, 6, 7, -9};
    denseSet mySet(input.begin(), input.end());
    set reference(input.begin(), input.end());
    auto res1 = mySet.equal_range(3);
    auto res2 = reference.equal_range(3);
    EXPECT_EQ(res1.first, res1.second);
    EXPECT_EQ(std::distance(mySet.begin(), res1.first),
              std::distance(reference.begin(), res2.first));
    EXPECT_EQ(std::distance(mySet.begin(), res1.second),
              std::distance(reference.begin(), res2.second));
}

TEST(DenseSetUtility, Equality) {
    auto input = {2, 4, 6, 7, -9};
    denseSet mySet1(input.begin(), input.end());
    denseSet mySet2(input.begin(), input.end());
    EXPECT_TRUE(mySet1 == mySet2);
    EXPECT_FALSE(mySet1 != mySet2);
}

TEST(DenseSetUtility, EqualityDifferentSize) {
    auto input = {2, 4, 6, 7, -9};
    denseSet mySet1(input.begin(), input.end());
    denseSet mySet2(input.begin(), input.end());
    mySet1.erase(4);
    EXPECT_FALSE(mySet1 == mySet2);
    EXPECT_TRUE(mySet1 != mySet2);
}

TEST(DenseSetUtility, Inequality) {
    denseSet mySet1({2, 4, 6, 7, -9});
    denseSet mySet2({-2, -5, 6, 7, -9});
    mySet1.erase(4);
    EXPECT_FALSE(mySet1 == mySet2);
    EXPECT_TRUE(mySet1 != mySet2);
}

TEST(DenseSetUtility, Less) {
    denseSet mySet1({2, 4, 6, 7, -9});
    denseSet mySet2({-2, -5, 3, 6, -11});
    EXPECT_FALSE(mySet1 < mySet2);
    EXPECT_TRUE(mySet1 >= mySet2);
    EXPECT_TRUE(mySet1 > mySet2);
    EXPECT_FALSE(mySet1 <= mySet2);
}

TEST(DenseSetUtility, Greater) {
    denseSet mySet1({2, 4, 6, 7, -9});
    denseSet mySet2({-2, -5, 3, 6, -11});
    EXPECT_FALSE(mySet1 < mySet2);
    EXPECT_TRUE(mySet1 >= mySet2);
    EXPECT_TRUE(mySet1 > mySet2);
    EXPECT_FALSE(mySet1 <= mySet2);
}

TEST(DenseSetUtility, Swap) {
    denseSet mySet1({2, 4, 6, 7, -9});
    denseSet mySet2({-2, -5, 3, 6, -11});
    denseSet mySet3(mySet2);
    EXPECT_TRUE(mySet2 == mySet3);
    EXPECT_FALSE(mySet1 == mySet3);
    swap(mySet1, mySet3);
    EXPECT_TRUE(mySet1 == mySet2);
    EXPECT_FALSE(mySet2 == mySet3);
}

TEST(DenseSetManipulation, Clear) {
    denseSet mySet({2, -4, 11});
    mySet.clear();
    EXPECT_TRUE(mySet.empty());
}

TEST(DenseSetManipulation, EraseIterator) {
    denseSet mySet({-27, -6, -4, 7});
    set reference({-27, -6, -4, 7});
    EXPECT_TRUE(mySet.contains(-4));
    EXPECT_EQ(mySet.size(), reference.size());
    EXPECT_EQ(std::distance(mySet.begin(), mySet.erase(mySet.find(-4))),
              std::distance(reference.begin(), reference.erase(reference.find(-4))));
    EXPECT_FALSE(mySet.contains(-4));
    EXPECT_EQ(mySet.size(), reference.size());
}

TEST(DenseSetManipulation, EraseKey) {
    denseSet mySet({13, -9, 1, 5});
    set reference({13, -9, 1, 5});
    EXPECT_TRUE(mySet.contains(1));
    EXPECT_EQ(mySet.size(), reference.size());
    EXPECT_EQ(mySet.erase(1), reference.erase(1));
    EXPECT_FALSE(mySet.contains(1));
    EXPECT_EQ(mySet.size(), reference.size());
}

TEST(DenseSetManipulation, EraseNonExistingKey) {
    denseSet mySet({3, 6, -9, -12});
    set reference({3, 6, -9, -12});
    EXPECT_FALSE(mySet.contains(17));
    EXPECT_EQ(mySet.size(), reference.size());
    EXPECT_EQ(mySet.erase(1), reference.erase(1));
    EXPECT_EQ(mySet.size(), reference.size());
}

TEST(DenseSetManipulation, EraseRange) {
    denseSet mySet({2, -4, -6, -8});
    set reference({2, -4, -6, -8});
    EXPECT_EQ(mySet.size(), reference.size());
    EXPECT_EQ(std::distance(mySet.begin(), mySet.erase(mySet.begin(), mySet.find(-4))),
              std::distance(reference.begin(), reference.erase(reference.begin(), reference.find(-4))));
    EXPECT_EQ(mySet.size(), reference.size());
}

TEST(DenseSetManipulation, InsertKey) {
    denseSet mySet({-1, -3});
    set reference({-1, -3});
    EXPECT_FALSE(mySet.contains(6));
    EXPECT_EQ(mySet.size(), reference.size());
    EXPECT_EQ(std::distance(mySet.begin(), mySet.insert(6).first),
              std::distance(reference.begin(), reference.insert(6).first));
    EXPECT_EQ(mySet.size(), reference.size());
}

TEST(DenseSetManipulation, InsertExistingKey) {
    denseSet mySet({5, 3});
    set reference({5, 3});
    EXPECT_EQ(mySet.size(), reference.size());
    EXPECT_EQ(std::distance(mySet.begin(), mySet.insert(3).first),
              std::distance(reference.begin(), reference.insert(3).first));
    EXPECT_EQ(mySet.size(), reference.size());
}

TEST(DenseSetManipulation, InsertRange) {
    denseSet mySet({-27, -6, -4, 7});
    set reference({-27, -6, -4, 7});
    auto data = {3, 7, 5};
    EXPECT_EQ(mySet.size(), reference.size());
    mySet.insert(data.begin(), data.end());
    reference.insert(data.begin(), data.end());
    EXPECT_TRUE(absl::equal(mySet.begin(), mySet.end(), reference.begin(), reference.end()));
}

TEST(DenseSetManipulation, InsertInitializerList) {
    denseSet mySet({17, 2, 6, -7, 5});
    set reference({17, 2, 6, -7, 5});
    mySet.insert({3, 7, 5});
    reference.insert({3, 7, 5});
    EXPECT_TRUE(absl::equal(mySet.begin(), mySet.end(), reference.begin(), reference.end()));
}

TEST(DenseSetManipulation, InsertHintKeyEnd) {
    denseSet mySet({-1, -3});
    set reference({-1, -3});
    EXPECT_EQ(mySet.size(), reference.size());
    EXPECT_EQ(std::distance(mySet.begin(), mySet.insert(mySet.end(), 3)),
              std::distance(reference.begin(), reference.insert(reference.end(), 3)));
    EXPECT_EQ(mySet.size(), reference.size());
}

TEST(DenseSetManipulation, InsertHintKeyEndExisting) {
    denseSet mySet({-1, -3});
    set reference({-1, -3});
    EXPECT_EQ(mySet.size(), reference.size());
    EXPECT_EQ(std::distance(mySet.begin(), mySet.insert(mySet.end(), -1)),
              std::distance(reference.begin(), reference.insert(reference.end(), -1)));
    EXPECT_EQ(mySet.size(), reference.size());
}

TEST(DenseSetManipulation, InsertHintKeyBegin) {
    denseSet mySet({4, 2, -1, 5});
    set reference({4, 2, -1, 5});
    EXPECT_EQ(mySet.size(), reference.size());
    EXPECT_EQ(std::distance(mySet.begin(), mySet.insert(mySet.begin(), -3)),
              std::distance(reference.begin(), reference.insert(reference.begin(), -3)));
    EXPECT_EQ(mySet.size(), reference.size());
}

TEST(DenseSetManipulation, InsertHintKeyBeginExisting) {
    denseSet mySet({4, 2, -1, 5});
    set reference({4, 2, -1, 5});
    EXPECT_EQ(mySet.size(), reference.size());
    EXPECT_EQ(std::distance(mySet.begin(), mySet.insert(mySet.begin(), 4)),
              std::distance(reference.begin(), reference.insert(reference.begin(), 4)));
    EXPECT_EQ(mySet.size(), reference.size());
}

}  // anonymous namespace
