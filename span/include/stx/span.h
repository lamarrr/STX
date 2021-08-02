/**
 * @file span.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @date 2020-08-04
 *
 * @copyright MIT License
 *
 * Copyright (c) 2020 Basit Ayantunde
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#pragma once
#include <cinttypes>
#include <limits>
#include <type_traits>

#include "stx/config.h"
#include "stx/option.h"
#include "stx/utils/enable_if.h"

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

template <typename Element, typename OtherElement>
constexpr bool is_compatible =
    std::is_convertible_v<OtherElement (*)[], Element (*)[]>;

template <typename T, typename Element, typename = void>
struct is_compatible_container_impl : std::false_type {};

template <typename T, typename Element>
struct is_compatible_container_impl<
    T, Element, std::void_t<decltype(std::data(std::declval<T>()))>>
    : std::bool_constant<is_compatible<
          Element,
          std::remove_pointer_t<decltype(std::data(std::declval<T>()))>>> {};

template <typename T, typename Element>
constexpr bool is_compatible_container =
    is_compatible_container_impl<T, Element>::value;

}  // namespace impl

constexpr size_t dynamic_extent = std::numeric_limits<size_t>::max();

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

template <typename T, size_t E = dynamic_extent>
struct Span {
  using element_type = T;
  using value_type = std::remove_cv_t<T>;
  using reference = T&;
  using pointer = T*;
  using const_pointer = T const*;
  using iterator = T*;
  using const_iterator = T const*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using size_type = size_t;
  using index_type = size_t;
  using difference_type = ptrdiff_t;

  static constexpr size_t extent = E;
  static constexpr size_t byte_extent =
      (extent == dynamic_extent ? extent : (sizeof(T) * extent));
  static constexpr bool is_dynamic_span = E == dynamic_extent;

 private:
  // i.e. if `T` is const or const-volatile, make `Y` same
  template <typename Y>
  using cv_match = impl::match_cv<T, Y>;

 public:
  constexpr Span() : data_{nullptr}, size_{0} {
    static_assert(
        extent == 0 || extent == dynamic_extent,
        "specified extent for Span but attempted to default-initialize it");
  }

  template <typename SrcT, size_type SrcE,
            STX_ENABLE_IF(impl::is_span_convertible<SrcT, T>)>
  constexpr Span(Span<SrcT, SrcE> src)
      : data_{static_cast<pointer>(src.data())},
        size_{(extent != dynamic_extent ? extent
                                        : static_cast<size_type>(src.size()))} {
    static_assert(
        (extent == dynamic_extent ? true
                                  : (extent <= static_cast<size_type>(SrcE))),
        "performing a subspan from a span source of "
        "smaller extent");
  }

  template <
      typename Iterator,
      STX_ENABLE_IF(
          std::is_convertible_v<Iterator&, iterator>&&
              impl::is_span_convertible<
                  typename std::iterator_traits<Iterator>::value_type, T> &&
          !std::is_convertible_v<Iterator&, size_type>)>
  explicit constexpr Span(Iterator begin, Iterator end)
      : data_{static_cast<iterator>(begin)},
        size_{static_cast<size_type>(static_cast<iterator>(end) -
                                     static_cast<iterator>(begin))} {}

  template <
      typename Iterator,
      STX_ENABLE_IF(
          std::is_convertible_v<Iterator&, iterator>&&
              impl::is_span_convertible<
                  typename std::iterator_traits<Iterator>::value_type, T>)>
  explicit constexpr Span(Iterator begin, size_type size)
      : data_{static_cast<iterator>(begin)},
        size_{(extent != dynamic_extent ? extent : size)} {}

  template <typename SrcT, size_type Length,
            STX_ENABLE_IF(impl::is_span_convertible<SrcT, T>)>
  constexpr Span(SrcT (&array)[Length])
      : data_{static_cast<pointer>(array)},
        size_{(extent != dynamic_extent ? extent : Length)} {
    static_assert((extent == dynamic_extent ? true : (extent <= Length)),
                  "Span extent is more than static array length");
  }

  template <typename SrcT, size_type Length,
            STX_ENABLE_IF(impl::is_span_convertible<SrcT, T>)>
  constexpr Span(std::array<SrcT, Length>& array)
      : data_{static_cast<pointer>(array.data())},
        size_{(extent != dynamic_extent ? extent : Length)} {
    static_assert((extent == dynamic_extent ? true : (extent <= Length)),
                  "Span extent is more than std::array's length");
  }

  template <typename SrcT, size_type Length,
            STX_ENABLE_IF(impl::is_span_convertible<SrcT const, T>)>
  constexpr Span(std::array<SrcT, Length> const& array)
      : data_{static_cast<pointer>(array.data())},
        size_{(extent != dynamic_extent ? extent : Length)} {
    static_assert((extent == dynamic_extent ? true : (extent <= Length)),
                  "Span extent is more than std::array's length");
  }

  template <typename SrcT, size_type Length>
  constexpr Span(std::array<SrcT, Length>&& array) = delete;

  template <typename Container,
            STX_ENABLE_IF(impl::is_span_container<Container&>&& impl::
                              is_span_compatible_container<T, Container&>)>
  constexpr Span(Container& container)
      : data_{static_cast<pointer>(std::data(container))},
        size_{(extent != dynamic_extent
                   ? extent
                   : static_cast<size_type>(std::size(container)))} {}

  constexpr Span(Span const&) = default;
  constexpr Span(Span&&) = default;
  constexpr Span& operator=(Span const&) = default;
  constexpr Span& operator=(Span&&) = default;

  constexpr pointer data() const { return data_; }

  constexpr size_type size() const {
    if constexpr (extent == dynamic_extent) {
      return size_;
    } else {
      return extent;
    }
  }

  constexpr size_type size_bytes() const { return size() * sizeof(T); }

  constexpr bool empty() const { return size() == 0; }

  constexpr iterator begin() const { return data_; }

  constexpr iterator end() const { return begin() + size(); }

  constexpr const_iterator cbegin() const { return begin(); }

  constexpr const_iterator cend() const { return end(); }

  constexpr reverse_iterator rbegin() const { return reverse_iterator(end()); }

  constexpr reverse_iterator rend() const { return reverse_iterator(begin()); }

  constexpr const_reverse_iterator crbegin() const { return rbegin(); }

  constexpr const_reverse_iterator crend() const { return rend(); }

  constexpr reference operator[](index_type index) const {
    return data_[index];
  }

  auto at(index_type index) const -> Option<Ref<T>> {
    if (index < size_) {
      return Some<Ref<T>>(data_[index]);
    } else {
      return None;
    }
  }

  constexpr Span<T> subspan(index_type offset) const {
    return Span<T>(begin() + offset, end());
  }

  constexpr Span<T> subspan(index_type offset, size_type length) const {
    return Span<T>(begin() + offset, length);
  }

  template <size_type Offset>
  constexpr Span<T> subspan() {
    static_assert((extent == dynamic_extent ? true : (Offset < extent)),
                  "Offset is greater than extent");
    return Span < T,
           extent == dynamic_extent
               ? extent
               : (extent - Offset) > (begin() + Offset, size() - Offset);
  }

  template <size_type Offset, size_type Length>
  constexpr Span<T> subspan() {
    static_assert((extent == dynamic_extent ? true : (Offset < extent)),
                  "Offset is greater than extent");
    static_assert(
        (extent == dynamic_extent ? true : ((Offset + Length) <= extent)),
        "Length exceeds span extent");
    return Span < T, extent == dynamic_extent
                         ? extent
                         : Length > (begin() + Offset, Length);
  }

  constexpr Span<cv_match<std::byte>, byte_extent> as_bytes() const {
    return Span<cv_match<std::byte>, byte_extent>(
        reinterpret_cast<cv_match<std::byte>*>(data_), size_bytes());
  }

  /// converts the span into a view of its underlying bytes (represented with
  /// `uint8_t`).
  constexpr Span<cv_match<uint8_t>, byte_extent> as_u8() const {
    return Span<cv_match<uint8_t>, byte_extent>(
        reinterpret_cast<cv_match<uint8_t>*>(data_), size_bytes());
  }

  /// converts the span into an immutable span.
  constexpr Span<T const, extent> as_const() const { return *this; }

  /// converts the span into another span in which reads
  /// and writes to the contiguous sequence are performed as volatile
  /// operations.
  constexpr Span<T volatile, extent> as_volatile() const { return *this; }

 private:
  pointer data_;
  size_type size_;
};

template <typename T, size_t N, typename Container>
inline stx::Option<stx::Span<T, N>> make_checked_span(Container& container) {
  if (N > static_cast<size_t>(std::size(container))) {
    return stx::None;
  } else {
    return stx::Some(stx::Span<T, N>(container));
  }
}

template <typename T, size_t N>
inline stx::Option<stx::Span<T, N>> make_checked_span(stx::Span<T> span) {
  if (N > static_cast<size_t>(span.size())) {
    return stx::None;
  } else {
    return stx::Some(stx::Span<T, N>(span));
  }
}

template <typename T>
inline stx::Option<stx::Span<T>> checked_subspan(stx::Span<T> source,
                                                 size_t offset, size_t length) {
  if (offset >= source.size() || (offset + length) > source.size()) {
    return stx::None;
  } else {
    return source.subspan(offset, length);
  }
}

template <typename T, size_t E>
inline stx::Option<stx::Span<T>> checked_subspan(stx::Span<T, E> source,
                                                 size_t offset) {
  if (offset >= source.size()) return stx::None;
  return checked_subspan(source, offset, source.size() - offset);
}

template <typename T, size_t E>
Span(T (&)[E])->Span<T, E>;

template <typename T, size_t E>
Span(std::array<T, E>&)->Span<T, E>;

template <typename T, size_t E>
Span(std::array<T, E> const&)->Span<T const, E>;

template <typename Container>
Span(Container& cont)->Span<std::remove_pointer_t<decltype(std::data(cont))>>;

STX_END_NAMESPACE
