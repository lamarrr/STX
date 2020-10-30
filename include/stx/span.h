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

STX_BEGIN_NAMESPACE

namespace internal {
template <typename T, typename U>
struct match_cv_impl {
  using type = U;
};

template <typename T, typename U>
struct match_cv_impl<T const, U> {
  using type = U const;
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

}  // namespace internal

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
template <typename Element, size_t Extent = dynamic_extent>
struct Span {
  using element_type = Element;
  using value_type = std::remove_cv_t<Element>;
  using reference = element_type&;
  using pointer = element_type*;
  using const_pointer = element_type const*;
  using iterator = element_type*;
  using const_iterator = element_type const*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using size_type = size_t;
  using index_type = size_t;
  using difference_type = ptrdiff_t;

  static constexpr size_t extent = Extent;
  static constexpr bool is_dynamic_span = false;

 private:
  // i.e. if `Element` is const or const-volatile, make `T` same
  template <typename T>
  using cv_match_ = internal::match_cv<element_type, T>;

  static constexpr size_type byte_extent_ =
      Extent * (sizeof(element_type) / sizeof(std::byte));
  static constexpr size_type u8_extent_ =
      Extent * (sizeof(element_type) / sizeof(uint8_t));

 public:
  Span() = delete;

  /// copy-construct static-extent span from another static-extent span
  /// (compile-time bounds-checked).
  template <typename SrcElement, size_type SrcExtent>
  constexpr Span(Span<SrcElement, SrcExtent> const& src) noexcept
      : data_{static_cast<pointer>(src.data())} {
    static_assert(Extent <= SrcExtent,
                  "performing a subspan from a static-extent span source of "
                  "smaller extent");
  }

  /// factory function for copy-constructing a static-extent span from another
  /// static-extent span (bounds-checked).
  template <typename SrcElement, size_type SrcExtent>
  static STX_OPTION_CONSTEXPR Option<Span> try_init(
      Span<SrcElement, SrcExtent> const& src) noexcept {
    if constexpr (Extent > SrcExtent) return None;
    return Some(Span(src));
  }

  /// copy-construct static-extent span from a dynamic-extent span (not
  /// bounds-checked).
  ///
  /// Also, see: bounds-checked `Span::try_init`.
  template <typename SrcElement>
  explicit constexpr Span(Span<SrcElement, dynamic_extent> const& src) noexcept
      : data_{static_cast<pointer>(src.data())} {}

  /// factory function for copy-constructing a static-extent span from a
  /// dynamic-extent span (bounds-checked).
  template <typename SrcElement>
  static STX_OPTION_CONSTEXPR Option<Span> try_init(
      Span<SrcElement, dynamic_extent> const& src) noexcept {
    if (Extent > src.size()) return None;
    return Some(Span(src));
  }

  /// construct static-extent span from iterator/raw-pointer (not
  /// bounds-checked).
  constexpr Span(iterator begin) noexcept : data_{begin} {};

  /// construct static-extent span from array (compile-time bounds-checked).
  template <typename SrcElement, size_type Length>
  constexpr Span(SrcElement (&array)[Length]) noexcept
      : data_{static_cast<pointer>(array)} {
    static_assert(Extent <= Length,
                  "Span extent is more than static array length");
  }

  /// construct static-extent span from array (bounds-checked).
  template <typename SrcElement, size_type Length>
  static STX_OPTION_CONSTEXPR Option<Span> try_init(
      SrcElement (&array)[Length]) noexcept {
    if constexpr (Extent > Length) return None;
    return Some(Span(static_cast<pointer>(array)));
  }

  /// construct static-extent span from std::array (compile-time
  /// bounds-checked).
  template <typename SrcElement, size_type Length>
  constexpr Span(std::array<SrcElement, Length>& array) noexcept
      : data_{static_cast<pointer>(array.data())} {
    static_assert(Extent <= Length,
                  "Span extent is more than std::array's length");
  }

  /// construct static-extent span from std::array (bounds-checked).
  template <typename SrcElement, size_type Length>
  static STX_OPTION_CONSTEXPR Option<Span> try_init(
      std::array<SrcElement, Length>& array) noexcept {
    if constexpr (Extent > Length) return None;
    return Some(Span(static_cast<pointer>(array.data())));
  }

  /// construct static-extent span from std::array (compile-time
  /// bounds-checked).
  template <typename SrcElement, size_type Length>
  constexpr Span(std::array<SrcElement, Length> const& array) noexcept
      : data_{static_cast<pointer>(array.data())} {
    static_assert(Extent <= Length,
                  "Span extent is more than std::array's length");
  }

  /// construct static-extent span from std::array (bounds-checked).
  template <typename SrcElement, size_type Length>
  STX_OPTION_CONSTEXPR Option<Span> try_init(
      std::array<SrcElement, Length> const& array) noexcept {
    if constexpr (Extent > Length) return None;
    return Some(Span(static_cast<pointer>(array.data())));
  }

  template <typename SrcElement, size_type Length>
  constexpr Span(std::array<SrcElement, Length>&& array) noexcept = delete;

  template <typename SrcElement, size_type Length>
  STX_OPTION_CONSTEXPR Option<Span> try_init(
      std::array<SrcElement, Length>&& array) noexcept = delete;

  /// construct static-extent span from any container (not bounds-checked).
  ///
  /// Also, see: bounds-checked `Span::try_init`.
  ///
  /// # NOTE
  ///
  /// use only for containers storing a contiguous sequence of elements.
  template <typename Container,
            std::enable_if_t<internal::is_container<Container>, int> = 0>
  explicit constexpr Span(Container& container) noexcept
      : data_{static_cast<pointer>(std::data(container))} {}

  /// factory function for constructing a static-extent span from any container
  /// (bounds-checked).
  template <typename Container,
            std::enable_if_t<internal::is_container<Container>, int> = 0>
  static STX_OPTION_CONSTEXPR Option<Span> try_init(
      Container& container) noexcept {
    if (Extent > std::size(container)) return None;
    return Some(Span(std::data(container)));
  }

  constexpr Span(Span const&) noexcept = default;
  constexpr Span(Span&&) noexcept = default;
  constexpr Span& operator=(Span const&) noexcept = default;
  constexpr Span& operator=(Span&&) noexcept = default;
  ~Span() noexcept = default;

  /// returns a pointer to the beginning of the sequence of elements.
  constexpr pointer data() const noexcept { return data_; };

  /// returns the number of elements in the sequence.
  constexpr size_type size() const noexcept { return size_; };

  /// returns the size of the sequence in bytes.
  constexpr size_type size_bytes() const noexcept { return byte_extent_; }

  /// checks if the sequence is empty.
  constexpr bool empty() const noexcept { return size() == 0; };

  /// returns an iterator to the beginning.
  constexpr iterator begin() const noexcept { return data_; };

  /// returns an iterator to the end.
  constexpr iterator end() const noexcept { return begin() + size(); };

  /// returns a constant iterator to the beginning.
  constexpr const_iterator cbegin() const noexcept { return begin(); };

  /// returns a constant iterator to the end.
  constexpr const_iterator cend() const noexcept { return end(); };

  /// returns a reverse iterator to the beginning.
  constexpr reverse_iterator rbegin() const noexcept {
    return reverse_iterator(end());
  };

  /// returns a reverse iterator to the end.
  constexpr reverse_iterator rend() const noexcept {
    return reverse_iterator(begin());
  };

  /// returns a constant reverse iterator to the beginning.
  constexpr const_reverse_iterator crbegin() const noexcept {
    return rbegin();
  };

  /// returns a constant reverse iterator to the end.
  constexpr const_reverse_iterator crend() const noexcept { return rend(); };

  /// accesses an element of the sequence (not bounds-checked).
  constexpr reference operator[](index_type index) const noexcept {
    return data()[index];
  };

  /// accesses an element of the sequence (bounds-checked).
  STX_OPTION_CONSTEXPR auto at(index_type index) const noexcept
      -> Option<Ref<element_type>> {
    if (index < size()) {
      return Some<Ref<element_type>>(data()[index]);
    } else {
      return None;
    }
  };

  /// accesses an element of the sequence (bounds-checked).
  template <index_type Pos>
  STX_OPTION_CONSTEXPR auto at() const noexcept -> Option<Ref<element_type>> {
    if constexpr (Pos < Extent) {
      return Some<Ref<element_type>>(data()[Pos]);
    } else {
      return None;
    }
  }

  /// obtains a subspan starting at an offset (not bounds-checked).
  constexpr Span<element_type> subspan(index_type offset) const noexcept {
    return Span<element_type>(begin() + offset, end());
  };

  /// obtains a subspan starting at an offset and with a length
  /// (not bounds-checked).
  constexpr Span<element_type> subspan(index_type offset,
                                       size_type length) const noexcept {
    return Span<element_type>(begin() + offset, length);
  };

  /// obtains a subspan starting at an offset (bounds-checked).
  STX_OPTION_CONSTEXPR Option<Span<element_type>> try_subspan(
      index_type offset) const noexcept {
    if (offset >= size()) return None;
    return Some(subspan(offset));
  };

  /// obtains a subspan starting at an offset and with a length
  /// (bounds-checked).
  STX_OPTION_CONSTEXPR Option<Span<element_type>> try_subspan(
      index_type offset, size_type length) const noexcept {
    if (offset >= size()) return None;
    if (begin() + offset + length > end()) return None;
    return Some(subspan(offset, length));
  }

  /// obtains a subspan with offset provided via a template parameter
  /// (compile-time bounds-checked).
  template <index_type Offset>
  constexpr Span<element_type, (Extent - Offset)> subspan() const noexcept {
    static_assert(Offset < Extent,
                  "Offset can not be greater than static-extent span's size");
    return Span<element_type, (Extent - Offset)>(begin() + Offset);
  }

  /// obtains a subspan with offset and length provided via a template
  /// parameter (compile-time bounds-checked).
  template <index_type Offset, size_type Length>
  constexpr Span<element_type, Length> subspan() const noexcept {
    static_assert(Offset < Extent,
                  "Offset can not be greater than static-extent span's size");
    static_assert(Offset + Length <= Extent,
                  "subspan length exceeds span's range");
    return Span<element_type, Length>(begin() + Offset);
  }

  /// obtains a subspan with offset provided via a template parameter
  /// (bounds-checked).
  template <index_type Offset>
  STX_OPTION_CONSTEXPR Option<Span<element_type, (Extent - Offset)>>
  try_subspan() const noexcept {
    if constexpr (Offset >= Extent) return None;
    return Some(Span<element_type, (Extent - Offset)>(begin() + Offset));
  }

  /// obtains a subspan with offset and length provided via a template parameter
  /// (bounds-checked).
  template <index_type Offset, size_type Length>
  STX_OPTION_CONSTEXPR Option<Span<element_type, Length>> try_subspan()
      const noexcept {
    if constexpr (Offset >= Extent) return None;
    if (begin() + Offset + Length > end()) return None;
    return Some(Span<element_type, Length>(begin() + Offset));
  }

  /// converts the span into a view of its underlying bytes (represented with
  /// `std::byte`).
  constexpr Span<cv_match_<std::byte>, byte_extent_> as_bytes() const noexcept {
    return Span<cv_match_<std::byte>, byte_extent_>(
        reinterpret_cast<cv_match_<std::byte>*>(data()));
  }

  /// converts the span into a view of its underlying bytes (represented with
  /// `uint8_t`).
  constexpr Span<cv_match_<uint8_t>, u8_extent_> as_u8() const noexcept {
    return Span<cv_match_<uint8_t>, u8_extent_>(
        reinterpret_cast<cv_match_<uint8_t>*>(data()));
  }

  /// converts the span into an immutable span.
  constexpr Span<element_type const, Extent> as_const() const noexcept {
    return *this;
  }

  /// converts the span into another span in which reads
  /// and writes to the contiguous sequence are performed as volatile
  /// operations.
  constexpr Span<element_type volatile, Extent> as_volatile() const noexcept {
    return *this;
  }

 private:
  pointer data_;
  static constexpr size_t size_ = Extent;
};

template <typename Element>
struct Span<Element, dynamic_extent> {
  using element_type = Element;
  using value_type = std::remove_cv_t<Element>;
  using reference = element_type&;
  using pointer = element_type*;
  using const_pointer = element_type const*;
  using iterator = element_type*;
  using const_iterator = element_type const*;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using size_type = size_t;
  using index_type = size_t;
  using difference_type = ptrdiff_t;

  static constexpr size_t extent = dynamic_extent;
  static constexpr bool is_dynamic_span = true;

 private:
  // i.e. if `Element` is const or const-volatile, make `T` same
  template <typename T>
  using cv_match_ = internal::match_cv<element_type, T>;

 public:
  constexpr Span() noexcept : data_{nullptr}, size_{0} {}

  /// copy-construct dynamic-extent span from a static-extent span.
  template <typename SrcElement, size_type SrcExtent>
  constexpr Span(Span<SrcElement, SrcExtent> const& src) noexcept
      : data_{static_cast<pointer>(src.data())}, size_{SrcExtent} {}

  /// copy-construct dynamic-extent span from another dynamic-extent span.
  template <typename SrcElement>
  constexpr Span(Span<SrcElement, dynamic_extent> const& src) noexcept
      : data_{static_cast<pointer>(src.data())}, size_{src.size()} {}

  /// construct dynamic-extent span with an iterator/raw-pointer and a size
  template <
      typename Iterator,
      std::enable_if_t<std::is_convertible_v<Iterator&, iterator>, int> = 0>
  constexpr Span(Iterator begin, size_type size) noexcept
      : data_{static_cast<iterator>(begin)}, size_{size} {}

  /// construct dynamic-extent span from two iterators/raw-pointers.
  /// `end` must be greater than `begin`. (unchecked)
  ///
  /// Also, see checked `Span::try_init`.
  template <
      typename Iterator,
      std::enable_if_t<std::is_convertible_v<Iterator&, iterator>, int> = 0>
  constexpr Span(Iterator begin, Iterator end) noexcept
      : data_{begin},
        size_{static_cast<size_type>(static_cast<iterator>(end) -
                                     static_cast<iterator>(begin))} {}

  /// factory function for constructing a span from two iterators/raw-pointers
  /// (checked).
  //
  /// constexpr since C++20.
  template <
      typename Iterator,
      std::enable_if_t<std::is_convertible_v<Iterator&, iterator>, int> = 0>
  static STX_OPTION_CONSTEXPR Option<Span> try_init(Iterator begin,
                                                    Iterator end) noexcept {
    if (end < begin) return None;
    return Some(Span(static_cast<iterator>(begin), static_cast<iterator>(end)));
  }

  /// construct dynamic-extent span from array
  template <typename SrcElement, size_type Length>
  constexpr Span(SrcElement (&array)[Length])
      : data_{static_cast<pointer>(array)}, size_{Length} {}

  /// construct span from any container.
  ///
  /// # NOTE
  ///
  /// use only for containers storing a contiguous sequence of elements.
  template <typename Container,
            std::enable_if_t<internal::is_container<Container>, int> = 0>
  constexpr Span(Container& container)
      : data_{static_cast<pointer>(std::data(container))},
        size_{std::size(container)} {}

  constexpr Span(Span const&) noexcept = default;
  constexpr Span(Span&&) noexcept = default;
  constexpr Span& operator=(Span const&) noexcept = default;
  constexpr Span& operator=(Span&&) noexcept = default;
  ~Span() noexcept = default;

  /// returns a pointer to the beginning of the sequence of elements.
  constexpr pointer data() const noexcept { return data_; };

  /// returns the number of elements in the sequence.
  constexpr size_type size() const noexcept { return size_; };

  /// returns the size of the sequence in bytes.
  constexpr size_type size_bytes() const noexcept {
    return size() * sizeof(element_type);
  }

  /// checks if the sequence is empty.
  constexpr bool empty() const noexcept { return size() == 0; };

  /// returns an iterator to the beginning.
  constexpr iterator begin() const noexcept { return data_; };

  /// returns an iterator to the end.
  constexpr iterator end() const noexcept { return begin() + size(); };

  /// returns a constant iterator to the beginning.
  constexpr const_iterator cbegin() const noexcept { return begin(); };

  /// returns a constant iterator to the end.
  constexpr const_iterator cend() const noexcept { return end(); };

  /// returns a reverse iterator to the beginning.
  constexpr reverse_iterator rbegin() const noexcept {
    return reverse_iterator(end());
  };

  /// returns a reverse iterator to the end.
  constexpr reverse_iterator rend() const noexcept {
    return reverse_iterator(begin());
  };

  /// returns a constant reverse iterator to the beginning.
  constexpr const_reverse_iterator crbegin() const noexcept {
    return rbegin();
  };

  /// returns a constant reverse iterator to the end.
  constexpr const_reverse_iterator crend() const noexcept { return rend(); };

  /// accesses an element of the sequence (not bounds-checked).
  constexpr reference operator[](index_type index) const noexcept {
    return data()[index];
  };

  /// accesses an element of the sequence (bounds-checked).
  STX_OPTION_CONSTEXPR auto at(index_type index) const noexcept
      -> Option<Ref<element_type>> {
    if (index < size()) {
      return Some<Ref<element_type>>(data()[index]);
    } else {
      return None;
    }
  };

  /// accesses an element of the sequence (bounds-checked).
  template <index_type Pos>
  STX_OPTION_CONSTEXPR auto at() const noexcept -> Option<Ref<element_type>> {
    if (Pos < size()) {
      return Some<Ref<element_type>>(data()[Pos]);
    } else {
      return None;
    }
  }

  /// obtains a subspan starting at an offset (not bounds-checked).
  constexpr Span<element_type> subspan(index_type offset) const noexcept {
    return Span<element_type>(begin() + offset, end());
  };

  /// obtains a subspan starting at an offset and with a length
  /// (not bounds-checked).
  constexpr Span<element_type> subspan(index_type offset,
                                       size_type length) const noexcept {
    return Span<element_type>(begin() + offset, length);
  };

  /// obtains a subspan starting at an offset (bounds-checked).
  STX_OPTION_CONSTEXPR Option<Span<element_type>> try_subspan(
      index_type offset) const noexcept {
    if (offset >= size()) return None;
    return Some(subspan(offset));
  };

  /// obtains a subspan starting at an offset and with a length
  /// (bounds-checked).
  STX_OPTION_CONSTEXPR Option<Span<element_type>> try_subspan(
      index_type offset, size_type length) const noexcept {
    if (offset >= size()) return None;
    if (begin() + offset + length > end()) return None;
    return Some(subspan(offset, length));
  };

  /// obtains a subspan with offset provided via a template parameter
  /// (compile-time bounds-checked).
  template <index_type Offset>
  constexpr Span<element_type> subspan() const noexcept {
    return Span<element_type>(begin() + Offset, size() - Offset);
  }

  /// obtains a subspan with offset provided via a template parameter
  /// (compile-time bounds-checked).
  template <index_type Offset, size_type Length>
  constexpr Span<element_type, Length> subspan() const noexcept {
    return Span<element_type, Length>(begin() + Offset);
  }

  /// obtains a subspan with offset provided via a template parameter
  /// (bounds-checked).
  template <index_type Offset>
  STX_OPTION_CONSTEXPR Option<Span<element_type>> try_subspan() const noexcept {
    if (Offset >= size()) return None;
    return Some(Span<element_type>(begin() + Offset));
  }

  /// obtains a subspan with offset and length provided via a template parameter
  /// (bounds-checked).
  template <index_type Offset, size_type Length>
  STX_OPTION_CONSTEXPR Option<Span<element_type, Length>> try_subspan()
      const noexcept {
    if (Offset >= size()) return None;
    if (begin() + Offset + Length > end()) return None;
    return Some(Span<element_type, Length>(begin() + Offset));
  }

  /// converts the span into a view of its underlying bytes (represented with
  /// `std::byte`).
  constexpr Span<cv_match_<std::byte>> as_bytes() const noexcept {
    return Span<cv_match_<std::byte>>(
        reinterpret_cast<cv_match_<std::byte>*>(data()), size_bytes());
  }

  /// converts the span into a view of its underlying bytes (represented with
  /// `uint8_t`).
  constexpr Span<cv_match_<uint8_t>> as_u8() const noexcept {
    return Span<cv_match_<uint8_t>>(
        reinterpret_cast<cv_match_<uint8_t>*>(data()), size_bytes());
  }

  /// converts the span into an immutable span.
  constexpr Span<element_type const> as_const() const noexcept { return *this; }

  /// converts the span into another span in which reads
  /// and writes to the contiguous sequence are performed as volatile
  /// operations.
  constexpr Span<element_type volatile> as_volatile() const noexcept {
    return *this;
  }

 private:
  pointer data_;
  size_type size_;
};

template <typename SrcElement, size_t Length>
Span(SrcElement (&)[Length]) -> Span<SrcElement, Length>;

template <typename SrcElement, size_t Length>
Span(std::array<SrcElement, Length>&) -> Span<SrcElement, Length>;

template <typename SrcElement, size_t Length>
Span(std::array<SrcElement, Length> const&) -> Span<SrcElement const, Length>;

template<typename Container>
Span(Container& cont) -> Span<std::remove_pointer_t<decltype(std::data(cont))>>;

STX_END_NAMESPACE
