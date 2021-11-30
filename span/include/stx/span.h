/**
 * @file span.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-08-04
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2021 Basit Ayantunde
 *
 */

#pragma once
#include <algorithm>
#include <array>
#include <cinttypes>
#include <cstddef>
#include <iterator>
#include <limits>
#include <type_traits>
#include <utility>

#include "stx/config.h"
#include "stx/option.h"
#include "stx/utils/enable_if.h"

// TODO(lamarrr): use do{} while(0)
#define STX_RAISE_BUG(evaluation_identifier, condition_violated, \
                      violating_expression)                      \
  do {                                                           \
  } while (0)

#define STX_RAISE_BUF_IF(...) \
  do {                        \
  } while (0)
#define STX_SPAN_CHECK_BOUNDS(valid_span_size, span_index) \
  do {                                                     \
  } while (0)
#define STX_SPAN_CHECK_RANGE() \
  do {                         \
  } while (0)

#define STX_SPAN_CHECK_SIZE_EQ(...) \
  do {                              \
  } while (0)

STX_BEGIN_NAMESPACE

namespace impl {
template <typename T, typename U>
struct match_cv_impl {
  using type = U;
};

template <typename T, typename U>
struct match_cv_impl<T const, U> {
  using type = U const;
};

template <typename T, typename U>
struct match_cv_impl<T volatile, U> {
  using type = U volatile;
};

template <typename T, typename U>
struct match_cv_impl<T const volatile, U> {
  using type = U const volatile;
};

template <typename T, typename U>
using match_cv = typename match_cv_impl<T, U>::type;

template <typename Source, typename Target>
constexpr bool is_span_convertible =
    std::is_convertible_v<Source (*)[], Target (*)[]>;

template <typename T>
void type_ptr_and_size(T*, size_t) {}

template <typename T, typename = void>
struct is_container_impl : std::false_type {};

template <typename T>
struct is_container_impl<T, decltype((type_ptr_and_size(
                                         std::data(std::declval<T>()),
                                         std::size(std::declval<T>()))),
                                     (void)0)> : std::true_type {};

template <typename T>
constexpr bool is_container = is_container_impl<T>::value;

template <typename T, typename Element, typename = void>
struct is_compatible_container_impl : std::false_type {};

template <typename T, typename Element>
struct is_compatible_container_impl<
    T, Element, std::void_t<decltype(std::data(std::declval<T>()))>>
    : std::bool_constant<is_span_convertible<
          Element,
          std::remove_pointer_t<decltype(std::data(std::declval<T>()))>>> {};

template <typename T, typename Element>
constexpr bool is_compatible_container =
    is_compatible_container_impl<T, Element>::value;

}  // namespace impl

//!
//! # Span
//!
//! `Span` is a referencing/non-owning type-erased view over a contiguous
//! sequence of objects (sequential container) and typically help to eliminate
//! the use of raw pointers. A `Span` can either have a static extent, in which
//! case the number of elements in the sequence is known at compile-time and
//! encoded in the type, or a dynamic extent, in which case the number of
//! elements in the sequence is known at runtime.
//!
//! `Span` is conceptually a pointer and a length into an already existing
//! contiguous memory. Passing a properly-constructed `Span` instead of raw
//! pointers avoids many issues related to index out of bounds errors.
//!
//! A static-extent span is a span whose length is known at compile-time.
//! A dynamic-extent span is a span whose length varies and is only known at
//! runtime.
//!
//!
//!
//! # Usage
//!
//! ```cpp
//!
//! std::vector<int> vec = {1, 2, 3, 4, 5};
//!
//! // Construct a static-extent span (size known at compile time)
//! Span<int, 5> a = vec;
//!
//! // Construct a static-extent span pointing to the first 2 elements of the
//! // vector
//! Span<int, 2> b = vec;
//!
//! // Construct a dynamic-extent span (size known at runtime)
//! Span<int> c = vec;
//!
//!
//! // Construct a static-extent span pointing to the first 2 elements of the
//! // vector
//! auto d = Span<int>(vec.data(), 2);
//!
//!
//! ```
//!
//!
//!
// TODO(lamarrr): pointers given to span must be valid
// pointers returned from span are always valid
//
//
// iterators returned are always valid, except iterator returned from the
// default-constructed span.
//
//
template <typename T>
struct Span {
  using Type = T;
  using Reference = T&;
  using Iterator = T*;
  using ConstIterator = T const*;
  using Size = size_t;
  using Index = size_t;

 private:
  // i.e. if `T` is const, volatile, or const-volatile, make `U` same
  template <typename U>
  using ConstVolatileMatched = impl::match_cv<T, U>;

 public:
  constexpr Span() = default;
  constexpr Span(Iterator data, Size size)
      : ____iterator{data}, ____size{size} {}
  constexpr Span(Span const&) = default;
  constexpr Span(Span&&) = default;
  constexpr Span& operator=(Span const&) = default;
  constexpr Span& operator=(Span&&) = default;

  template <typename U, STX_ENABLE_IF(impl::is_span_convertible<U, T>)>
  constexpr Span(Span<U> src)
      : ____iterator{static_cast<Iterator>(src.____iterator)},
        ____size{src.____size} {}

  template <Size Length>
  constexpr Span(T (&array)[Length])
      : ____iterator{static_cast<Iterator>(array)}, ____size{Length} {}

  template <Size Length>
  constexpr Span(std::array<T, Length>& array)
      : ____iterator{static_cast<Iterator>(array.data())}, ____size{Length} {}

  template <Size Length>
  constexpr Span(std::array<T, Length> const& array)
      : ____iterator{static_cast<Iterator>(array.data())}, ____size{Length} {}

  template <typename U, Size Length>
  constexpr Span(std::array<U, Length>&& array) = delete;

  template <typename Container,
            std::enable_if_t<impl::is_container<Container&> &&
                                 impl::is_compatible_container<Container&, T>,
                             int> = 0>
  constexpr Span(Container& container) noexcept
      : ____iterator{static_cast<Iterator>(std::data(container))},
        ____size{std::size(container)} {}

  constexpr Iterator data() const { return ____iterator; }

  constexpr Size size() const { return ____size; }

  constexpr Size size_bytes() const { return ____size * sizeof(T); }

  constexpr bool is_empty() const { return ____size == 0; }

  constexpr Iterator begin() const { return ____iterator; }

  constexpr Iterator end() const { return begin() + ____size; }

  constexpr ConstIterator cbegin() const { return begin(); }

  constexpr ConstIterator cend() const { return end(); }

  constexpr Reference operator[](Index index) const {
    STX_SPAN_CHECK_BOUNDS(____size, index);
    return ____iterator[index];
  }

  auto at(Index index) const -> Option<Ref<T>> {
    if (index < ____size) {
      return Some<Ref<T>>(____iterator[index]);
    } else {
      return None;
    }
  }

  //
  //
  // TODO(lamarrr): No naked moves
  //
  //
  // Choices and implications -> Rust still doesn't solve this
  //
  //
  //
  //
  // nested use-after-move that the compiler can't reach.
  //
  // it is very rare for a container or type to try to use a moved-from object.
  //
  //  overload doom
  //
  // application hierarchy and pointers/memory as the lowest level
  //
  //
  // Inconsequantial Behaviour and requirements
  //
  // - I want std::vector, means I want a vectir that uses operator new,
  // operator delete, and I want a const one.
  //
  // Multi-type template parameters
  //
  //
  //
  // Isolating static behaviour by naming
  //
  //
  // Isolating undefined behavior
  //
  // Lifetimes
  //
  // You must not overload std::move
  // move is a potentially destructive operation and moved-from objects must
  // never be touched.
  // containers must never define move operators.
  //
  // Objects must not have invalid state that invalidates
  // their exposed operations by default Operations that invalidate returned
  // data must use a move construct
  //
  //  constexpr void move_to(Span<T> output) const;

  constexpr Span<T> copy(Span<T const> input) {
    static_assert(std::is_copy_assignable_v<T>);
    for (Index position = 0; position < std::min(size(), input.size());
         position++) {
      ____iterator[position] = input[position];
    }

    return *this;
  }

  template <typename Predicate>
  constexpr bool is_any(Predicate&& predicate) const {
    static_assert(std::is_invocable_v<Predicate, T&>);
    static_assert(std::is_same_v<std::invoke_result_t<Predicate, T&>, bool>);

    for (T& element : *this) {
      bool condition = std::forward<Predicate>(predicate)(element);
      if (condition) return true;
    }

    return false;
  }

  template <typename Predicate>
  constexpr bool is_all(Predicate&& predicate) const {
    static_assert(std::is_invocable_v<Predicate, T&>);
    static_assert(std::is_same_v<std::invoke_result_t<Predicate, T&>, bool>);

    for (T& element : *this) {
      bool condition = std::forward<Predicate>(predicate)(element);
      if (!condition) return false;
    }

    return !is_empty();
  }

  template <typename Predicate>
  constexpr bool is_none(Predicate&& predicate) const {
    static_assert(std::is_invocable_v<Predicate, T&>);
    static_assert(std::is_same_v<std::invoke_result_t<Predicate, T&>, bool>);

    for (T& element : *this) {
      bool condition = std::forward<Predicate>(predicate)(element);
      if (condition) return false;
    }

    return true;
  }

  constexpr bool all_equals(T const& cmp) const {
    return is_all([&cmp](T const& a) { return a == cmp; });
  }

  constexpr bool any_equals(T const& cmp) const {
    return is_any([&cmp](T const& a) { return a == cmp; });
  }

  constexpr bool none_equals(T const& cmp) const {
    return is_none([&cmp](T const& a) { return a == cmp; });
  }

  template <typename Func>
  constexpr Span<T> apply(Func&& func) const {
    static_assert(std::is_invocable_v<Func, T&>);
    for (T& element : *this) {
      std::forward<Func>(func)(element);
    }

    return *this;
  }

  template <typename Generator>
  constexpr Span<T> generate(Generator&& generator) const {
    static_assert(std::is_invocable_v<Generator, T&>);
    static_assert(std::is_assignable_v<T, std::invoke_result_t<Generator, T&>>);

    for (T& element : *this) {
      element = std::forward<Generator>(generator)(element);
    }

    return *this;
  }

  constexpr Span<T> set(T const& value) const {
    static_assert(std::is_copy_assignable_v<T>);

    for (T& element : *this) {
      element = value;
    }

    return *this;
  }

  // span of 1 element if found, otherwise span of zero elements
  constexpr Span<T> find(T const& object) const {
    // TODO(lamarrr): add equality comparable
    for (T* iter = ____iterator; iter < (____iterator + ____size); iter++) {
      if (*iter == object) {
        return Span<T>{iter, 1};
      }
    }

    return Span<T>{____iterator + ____size, 0};
  }

  template <typename Predicate>
  constexpr Span<T> which(Predicate&& predicate) const {
    static_assert(std::is_invocable_v<Predicate, T const&>);
    static_assert(
        std::is_same_v<std::invoke_result_t<Predicate, T const&>, bool>);

    for (T* iter = ____iterator; iter < (____iterator + ____size); iter++) {
      if (std::forward<Predicate>(predicate)(*iter)) {
        return Span<T>{iter, 1};
      }
    }

    return Span<T>{____iterator + ____size, 0};
  }

  constexpr Span<T> slice(Index offset) const {
    STX_SPAN_CHECK_BOUNDS(____size, offset);
    return Span<T>{____iterator + offset, ____size - offset};
  }

  constexpr Span<T> slice(Index offset, Size length_to_slice) const {
    STX_SPAN_CHECK_BOUNDS(____size, offset);

    if (length_to_slice > 0)
      STX_SPAN_CHECK_BOUNDS(____size, offset + (length_to_slice - 1));

    return Span<T>{____iterator + offset, length_to_slice};
  }

  template <typename Func, typename Output>
  constexpr Span<Output> map(Func&& transformer, Span<Output> output) const {
    static_assert(!std::is_const_v<Output>);
    static_assert(std::is_move_assignable_v<Output>);
    static_assert(std::is_invocable_v<Func, T&>);
    static_assert(
        std::is_assignable_v<Output&, std::invoke_result_t<Func, T&>>);

    STX_SPAN_CHECK_SIZE_EQ(this->size(), output.size());

    for (Index position = 0; position < size(); position++) {
      output[position] =
          std::forward<Func>(transformer)(____iterator[position]);
    }

    return output;
  }

  template <typename Predicate>
  constexpr std::pair<Span<T>, Span<T>> partition(Predicate&& predicate) const {
    auto first_partition_end =
        std::stable_partition(____iterator, ____iterator + ____size,
                              std::forward<Predicate>(predicate));

    T* second_partition_end = ____iterator + ____size;

    return std::make_pair(
        Span<T>{____iterator,
                static_cast<Size>(first_partition_end - ____iterator)},
        Span<T>{first_partition_end,
                static_cast<Size>(second_partition_end - first_partition_end)});
  }

  template <typename Predicate>
  constexpr std::pair<Span<T>, Span<T>> unstable_partition(
      Predicate&& predicate) const {
    auto first_partition_end =
        std::partition(____iterator, ____iterator + ____size,
                       std::forward<Predicate>(predicate));

    T* second_partition_end = ____iterator + ____size;

    return std::make_pair(Span<T>{____iterator, first_partition_end},
                          Span<T>{first_partition_end,
                                  second_partition_end - first_partition_end});
  }

  // Span<T>, Span<T> get_partitions()
  // might not be partitioned

  // TODO(lamarrr): overlaps address, join two related spans, etc. must check
  // order

  constexpr Span<ConstVolatileMatched<std::byte> const> as_bytes() const {
    return Span<ConstVolatileMatched<std::byte> const>(
        reinterpret_cast<ConstVolatileMatched<std::byte> const*>(____iterator),
        size_bytes());
  }

  /// converts the span into a view of its underlying bytes (represented with
  /// `uint8_t`).
  constexpr Span<ConstVolatileMatched<uint8_t> const> as_u8() const {
    return Span<ConstVolatileMatched<uint8_t> const>(
        reinterpret_cast<ConstVolatileMatched<uint8_t> const*>(____iterator),
        size_bytes());
  }

  /// converts the span into an immutable span.
  constexpr Span<T const> as_const() const { return *this; }

  /// converts the span into another span in which reads
  /// and writes to the contiguous sequence are performed as volatile
  /// operations.
  constexpr Span<T volatile> as_volatile() const { return *this; }

  Iterator ____iterator = nullptr;
  Size ____size = 0;

 // static constexpr uint64_t ABI_TAG = STX_ABI_VERSION;
  //
  // we just need to be selective about what types we want to support across
  // ABIs
  //
  // complex objects usually don't cross ABIs.
  //
  //
  // Allocators and complex objects shouldn't decide the behavior of objects.
  //
  // enum class AbiTag
  //
  //
  // template<typename Output, AbiTag tag>
  // abi_cast( stx::Span<Output>  ) {
  // }
  //
  // define own abi cast by accessing internal members. must be vetted.
  //
  //
};

template <typename SrcElement, size_t Length>
Span(SrcElement (&)[Length])->Span<SrcElement>;

template <typename SrcElement, size_t Length>
Span(std::array<SrcElement, Length>&)->Span<SrcElement>;

template <typename SrcElement, size_t Length>
Span(std::array<SrcElement, Length> const&)->Span<SrcElement const>;

template <typename Container>
Span(Container& cont)->Span<std::remove_pointer_t<decltype(std::data(cont))>>;

STX_END_NAMESPACE
