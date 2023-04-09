/**
 * @file span.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-08-04
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020-2022 Basit Ayantunde
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
#include "stx/enable_if.h"
#include "stx/option.h"
#include "stx/panic.h"

#define STX_SPAN_ENSURE(condition, message)             \
  do                                                    \
  {                                                     \
    if (!(condition))                                   \
    {                                                   \
      ::stx::panic("condition: '" #condition            \
                   "' failed. explanation: " #message); \
    }                                                   \
  } while (0)

STX_BEGIN_NAMESPACE

namespace impl
{
template <typename T, typename U>
struct match_cv_impl
{
  using type = U;
};

template <typename T, typename U>
struct match_cv_impl<T const, U>
{
  using type = U const;
};

template <typename T, typename U>
struct match_cv_impl<T volatile, U>
{
  using type = U volatile;
};

template <typename T, typename U>
struct match_cv_impl<T const volatile, U>
{
  using type = U const volatile;
};

template <typename T, typename U>
using match_cv = typename match_cv_impl<T, U>::type;

template <typename Source, typename Target>
constexpr bool is_span_convertible = std::is_convertible_v<Source (*)[], Target (*)[]>;

template <typename T>
void type_ptr_and_size(T *, size_t)
{}

template <typename T, typename = void>
struct is_container_impl : std::false_type
{};

template <typename T>
struct is_container_impl<T, decltype((type_ptr_and_size(
                                         std::data(std::declval<T>()),
                                         std::size(std::declval<T>()))),
                                     (void) 0)> : std::true_type
{};

template <typename T>
constexpr bool is_container = is_container_impl<T>::value;

template <typename T, typename Element, typename = void>
struct is_compatible_container_impl : std::false_type
{};

template <typename T, typename Element>
struct is_compatible_container_impl<
    T, Element, std::void_t<decltype(std::data(std::declval<T>()))>>
    : std::bool_constant<is_span_convertible<
          std::remove_pointer_t<decltype(std::data(std::declval<T>()))>,
          Element>>
{};

template <typename T, typename Element>
constexpr bool is_compatible_container = is_compatible_container_impl<T, Element>::value;

}        // namespace impl

///
/// # Span
///
/// `Span` is a referencing/non-owning type-erased view over a contiguous
/// sequence of objects (sequential container) and typically help to eliminate
/// the use of raw pointers.
///
/// `Span` is conceptually a pointer and a length into an already existing
/// contiguous memory. Passing a properly-constructed `Span` instead of raw
/// pointers avoids many issues related to index out of bounds errors.
///
///
/// # Usage
///
/// ```cpp
///
/// std::vector<int> vec = {1, 2, 3, 4, 5};
///
/// // Construct a span
/// Span<int> c = vec;
///
///
/// ```
///
///
template <typename T>
struct Span
{
  static_assert(!std::is_reference_v<T>);

  using Type          = T;
  using Reference     = T &;
  using Iterator      = T *;
  using ConstIterator = T const *;
  using Pointer       = T *;
  using Size          = size_t;
  using Index         = size_t;

private:
  // i.e. if `T` is const, volatile, or const-volatile, make `U` same
  template <typename U>
  using ConstVolatileMatched = impl::match_cv<T, U>;

public:
  constexpr Span() = default;
  constexpr Span(Iterator data, Size size) :
      iterator_{data}, size_{size}
  {}
  constexpr Span(Span const &)            = default;
  constexpr Span(Span &&)                 = default;
  constexpr Span &operator=(Span const &) = default;
  constexpr Span &operator=(Span &&)      = default;

  template <typename U, STX_ENABLE_IF(impl::is_span_convertible<U, T>)>
  constexpr Span(Span<U> src) :
      iterator_{static_cast<Iterator>(src.iterator_)}, size_{src.size_}
  {}

  template <Size Length>
  constexpr Span(T (&array)[Length]) :
      iterator_{static_cast<Iterator>(array)}, size_{Length}
  {}

  template <Size Length>
  constexpr Span(std::array<T, Length> &array) :
      iterator_{static_cast<Iterator>(array.data())}, size_{Length}
  {}

  template <Size Length>
  constexpr Span(std::array<T, Length> const &array) :
      iterator_{static_cast<Iterator>(array.data())}, size_{Length}
  {}

  template <typename U, Size Length>
  constexpr Span(std::array<U, Length> &&array) = delete;

  template <typename Container, STX_ENABLE_IF(impl::is_container<Container &> &&impl::is_compatible_container<Container &, T>)>
  constexpr Span(Container &container) noexcept :
      iterator_{static_cast<Iterator>(std::data(container))}, size_{std::size(container)}
  {}

  constexpr Pointer data() const
  {
    return iterator_;
  }

  constexpr Size size() const
  {
    return size_;
  }

  constexpr Size size_bytes() const
  {
    return size_ * sizeof(T);
  }

  constexpr bool is_empty() const
  {
    return size_ == 0;
  }

  constexpr Iterator begin() const
  {
    return iterator_;
  }

  constexpr Iterator end() const
  {
    return begin() + size_;
  }

  constexpr ConstIterator cbegin() const
  {
    return begin();
  }

  constexpr ConstIterator cend() const
  {
    return end();
  }

  constexpr Reference operator[](Index index) const
  {
    STX_SPAN_ENSURE(index < size_, "index out of bounds");
    return iterator_[index];
  }

  auto at(Index index) const -> Option<Ref<T>>
  {
    if (index < size_)
    {
      return Some<Ref<T>>(iterator_[index]);
    }
    else
    {
      return None;
    }
  }

  constexpr Span<T> slice(Index offset) const
  {
    STX_SPAN_ENSURE(offset <= size_, "index out of bounds");
    return Span<T>{iterator_ + offset, size_ - offset};
  }

  constexpr Span<T> slice(Index offset, Size length_to_slice) const
  {
    STX_SPAN_ENSURE(offset <= size_, "index out of bounds");

    if (length_to_slice > 0)
    {
      STX_SPAN_ENSURE(offset + (length_to_slice - 1) < size_, "index out of bounds");
    }

    return Span<T>{iterator_ + offset, length_to_slice};
  }

  Option<T *> last() const
  {
    if (size_ == 0)
    {
      return None;
    }
    return Some(iterator_ + size_ - 1);
  }

  template <typename U>
  constexpr bool equals(stx::Span<U const> other) const
  {
    if (size_ != other.size_)
    {
      return false;
    }

    for (Size i = 0; i < size_; i++)
    {
      if (iterator_[i] != other.iterator_[i])
      {
        return false;
      }
    }

    return true;
  }

  template <typename Predicate>
  constexpr bool is_any(Predicate &&predicate) const
  {
    static_assert(std::is_invocable_v<Predicate, T &>);
    static_assert(std::is_same_v<std::invoke_result_t<Predicate, T &>, bool>);

    for (T &element : *this)
    {
      bool condition = std::forward<Predicate>(predicate)(element);
      if (condition)
      {
        return true;
      }
    }

    return false;
  }

  template <typename Predicate>
  constexpr bool is_all(Predicate &&predicate) const
  {
    static_assert(std::is_invocable_v<Predicate, T &>);
    static_assert(std::is_same_v<std::invoke_result_t<Predicate, T &>, bool>);

    for (T &element : *this)
    {
      bool condition = std::forward<Predicate>(predicate)(element);
      if (!condition)
      {
        return false;
      }
    }

    return !is_empty();
  }

  template <typename Predicate>
  constexpr bool is_none(Predicate &&predicate) const
  {
    static_assert(std::is_invocable_v<Predicate, T &>);
    static_assert(std::is_same_v<std::invoke_result_t<Predicate, T &>, bool>);

    for (T &element : *this)
    {
      bool condition = std::forward<Predicate>(predicate)(element);
      if (condition)
      {
        return false;
      }
    }

    return true;
  }

  constexpr bool all_equals(T const &cmp) const
  {
    return is_all([&cmp](T const &a) { return a == cmp; });
  }

  constexpr bool any_equals(T const &cmp) const
  {
    return is_any([&cmp](T const &a) { return a == cmp; });
  }

  constexpr bool none_equals(T const &cmp) const
  {
    return is_none([&cmp](T const &a) { return a == cmp; });
  }

  constexpr Span<T> copy(Span<T const> input)
  {
    static_assert(std::is_copy_assignable_v<T>);

    for (Index position = 0; position < std::min(size(), input.size()); position++)
    {
      iterator_[position] = input[position];
    }

    return *this;
  }

  template <typename Func>
  constexpr Span<T> for_each(Func &&func) const
  {
    static_assert(std::is_invocable_v<Func, T &>);

    for (T &element : *this)
    {
      std::forward<Func>(func)(element);
    }

    return *this;
  }

  template <typename Generator>
  constexpr Span<T> generate(Generator &&generator) const
  {
    static_assert(std::is_invocable_v<Generator, T &>);
    static_assert(std::is_assignable_v<T, std::invoke_result_t<Generator, T &>>);

    for (T &element : *this)
    {
      element = std::forward<Generator>(generator)(element);
    }

    return *this;
  }

  constexpr Span<T> fill(T const &value) const
  {
    static_assert(std::is_copy_assignable_v<T>);

    for (T &element : *this)
    {
      element = value;
    }

    return *this;
  }

  /// span of 1 element if found, otherwise span of zero elements
  constexpr Span<T> find(T const &object) const
  {
    // TODO(lamarrr): consider adding equality comparable

    for (Iterator iter = iterator_; iter < (iterator_ + size_); iter++)
    {
      if (*iter == object)
      {
        return Span<T>{iter, 1};
      }
    }

    return Span<T>{iterator_ + size_, 0};
  }

  constexpr bool contains(T const &object) const
  {
    return !find(object).is_empty();
  }

  template <typename Predicate>
  constexpr Span<T> which(Predicate &&predicate) const
  {
    static_assert(std::is_invocable_v<Predicate, T const &>);
    static_assert(std::is_same_v<std::invoke_result_t<Predicate, T const &>, bool>);

    for (Iterator iter = iterator_; iter < (iterator_ + size_); iter++)
    {
      if (std::forward<Predicate>(predicate)(*iter))
      {
        return Span<T>{iter, 1};
      }
    }

    return Span<T>{iterator_ + size_, 0};
  }

  template <typename Func, typename Output>
  constexpr Span<Output> map(Func &&transformer, Span<Output> output) const
  {
    static_assert(!std::is_const_v<Output>);
    static_assert(std::is_move_assignable_v<Output>);
    static_assert(std::is_invocable_v<Func, T &>);
    static_assert(std::is_assignable_v<Output &, std::invoke_result_t<Func, T &>>);

    STX_SPAN_ENSURE(size() == output.size(), "source and destination span size mismatch");

    for (Index position = 0; position < size(); position++)
    {
      output[position] = std::forward<Func>(transformer)(iterator_[position]);
    }

    return output;
  }

  template <typename Cmp>
  constexpr Span<T> sort(Cmp &&cmp) const
  {
    static_assert(std::is_invocable_v<Cmp, T &, T &>);
    static_assert(std::is_convertible_v<std::invoke_result_t<Cmp, T &, T &>, bool>);

    std::sort(begin(), end(), std::forward<Cmp>(cmp));

    return *this;
  }

  constexpr bool is_sorted() const
  {
    return std::is_sorted(begin(), end());
  }

  template <typename Cmp>
  constexpr bool is_sorted(Cmp &&cmp) const
  {
    // TODO(lamarrr): add type checks

    return std::is_sorted(begin(), end(), std::forward<Cmp>(cmp));
  }

  template <typename Predicate>
  constexpr std::pair<Span<T>, Span<T>> partition(Predicate &&predicate) const
  {
    // TODO(lamarrr): add type checks

    auto first_partition_end = std::stable_partition(
        iterator_, iterator_ + size_, std::forward<Predicate>(predicate));

    T *second_partition_end = iterator_ + size_;

    return std::make_pair(
        Span<T>{iterator_, static_cast<Size>(first_partition_end - iterator_)},
        Span<T>{first_partition_end, static_cast<Size>(second_partition_end - first_partition_end)});
  }

  template <typename Predicate>
  constexpr std::pair<Span<T>, Span<T>> unstable_partition(Predicate &&predicate) const
  {
    // TODO(lamarrr): add type checks

    auto first_partition_end = std::partition(iterator_, iterator_ + size_, std::forward<Predicate>(predicate));

    Iterator second_partition_end = iterator_ + size_;

    return std::make_pair(Span<T>{iterator_, first_partition_end}, Span<T>{first_partition_end, second_partition_end - first_partition_end});
  }

  // TODO(lamarrr): also check for rust lang's name for these
  // is_partitioned
  // accumulate, reduce
  // inner_product
  // count
  // count_if
  // move
  // max_element
  // min_element
  // minmax_element
  // equal
  // reverse

  /// converts the span into a view of its underlying bytes (represented with `uint8_t`).
  constexpr Span<ConstVolatileMatched<uint8_t>> as_u8() const
  {
    return Span<ConstVolatileMatched<uint8_t>>{reinterpret_cast<ConstVolatileMatched<uint8_t> *>(iterator_), size_bytes()};
  }

  /// converts the span into a view of its underlying bytes (represented with `char`).
  constexpr Span<ConstVolatileMatched<char>> as_char() const
  {
    return Span<ConstVolatileMatched<char>>{reinterpret_cast<ConstVolatileMatched<char> *>(iterator_), size_bytes()};
  }

  /// converts the span into an immutable span.
  constexpr Span<T const> as_const() const
  {
    return *this;
  }

  /// converts the span into another span in which reads
  /// and writes to the contiguous sequence are performed as volatile
  /// operations.
  constexpr Span<T volatile> as_volatile() const
  {
    return *this;
  }

  template <typename U>
  constexpr Span<U> transmute() const
  {
    return stx::Span<U>{reinterpret_cast<U *>(iterator_), size_bytes() / sizeof(U)};
  }

  Iterator iterator_ = nullptr;
  Size     size_     = 0;
};

template <typename SrcElement, size_t Length>
Span(SrcElement (&)[Length]) -> Span<SrcElement>;

template <typename SrcElement, size_t Length>
Span(std::array<SrcElement, Length> &) -> Span<SrcElement>;

template <typename SrcElement, size_t Length>
Span(std::array<SrcElement, Length> const &) -> Span<SrcElement const>;

template <typename Container>
Span(Container &cont) -> Span<std::remove_pointer_t<decltype(std::data(cont))>>;

STX_END_NAMESPACE
