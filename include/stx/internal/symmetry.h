/**
 * @file option_result.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @version  0.0.1
 * @date 2020-06-05
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

// DO NOT INCLUDE!!!

/// @cond

#pragma once

namespace stx {

/// @file
///
/// Symmetric Comparisons, Normally automatic on C++20
///
///

#define STX_SOME_SYMMETRY(cmp_type)                         \
  template <typename T>                                     \
  [[nodiscard]] STX_FORCE_INLINE constexpr bool operator==( \
      cmp_type const& cmp, Some<T> const& some) {           \
    return some == cmp;                                     \
  }                                                         \
                                                            \
  template <typename T>                                     \
  [[nodiscard]] STX_FORCE_INLINE constexpr bool operator!=( \
      cmp_type const& cmp, Some<T> const& some) {           \
    return some != cmp;                                     \
  }

STX_SOME_SYMMETRY(Some<MutRef<T>>)
STX_SOME_SYMMETRY(Some<ConstRef<T>>)
STX_SOME_SYMMETRY(Some<T*>)
STX_SOME_SYMMETRY(Some<T const*>)
STX_SOME_SYMMETRY(NoneType)

#undef STX_SOME_SYMMETRY

#define STX_OPTION_SYMMETRY(cmp_type)                       \
  template <typename T>                                     \
  [[nodiscard]] STX_FORCE_INLINE constexpr bool operator==( \
      cmp_type const& cmp, Option<T> const& option) {       \
    return option == cmp;                                   \
  }                                                         \
  template <typename T>                                     \
  [[nodiscard]] STX_FORCE_INLINE constexpr bool operator!=( \
      cmp_type const& cmp, Option<T> const& option) {       \
    return option != cmp;                                   \
  }

STX_OPTION_SYMMETRY(Some<T>)
STX_OPTION_SYMMETRY(Some<ConstRef<T>>)
STX_OPTION_SYMMETRY(Some<MutRef<T>>)
STX_OPTION_SYMMETRY(Some<T const*>)
STX_OPTION_SYMMETRY(Some<T*>)
STX_OPTION_SYMMETRY(NoneType)

#undef STX_OPTION_SYMMETRY

// Symmetric Equality, Normally automatic on C++20

#define STX_OK_SYMMETRY(cmp_type)                           \
  template <typename T>                                     \
  [[nodiscard]] STX_FORCE_INLINE constexpr bool operator==( \
      cmp_type const& cmp, Ok<T> const& ok) {               \
    return ok == cmp;                                       \
  }                                                         \
                                                            \
  template <typename T>                                     \
  [[nodiscard]] STX_FORCE_INLINE constexpr bool operator!=( \
      cmp_type const& cmp, Ok<T> const& ok) {               \
    return ok != cmp;                                       \
  }

STX_OK_SYMMETRY(Ok<ConstRef<T>>)
STX_OK_SYMMETRY(Ok<MutRef<T>>)
STX_OK_SYMMETRY(Ok<T*>)
STX_OK_SYMMETRY(Ok<T const*>)

template <typename U, typename T>
[[nodiscard]] STX_FORCE_INLINE constexpr bool operator==(
    Err<U> const&, Ok<T> const&) noexcept {
  return false;
}

template <typename U, typename T>
[[nodiscard]] STX_FORCE_INLINE constexpr bool operator!=(
    Err<U> const&, Ok<T> const&) noexcept {
  return true;
}

#undef STX_OK_SYMMETRY

// Symmetric Equality, Normally automatic on C++20

#define STX_ERR_SYMMETRY(cmp_type)                          \
                                                            \
  template <typename E, typename T>                         \
  [[nodiscard]] STX_FORCE_INLINE constexpr bool operator==( \
      cmp_type const& cmp, Err<T> const& err) {             \
    return err == cmp;                                      \
  }                                                         \
                                                            \
  template <typename E, typename T>                         \
  [[nodiscard]] STX_FORCE_INLINE constexpr bool operator!=( \
      cmp_type const& cmp, Err<T> const& err) {             \
    return err != cmp;                                      \
  }

STX_ERR_SYMMETRY(Err<ConstRef<E>>)
STX_ERR_SYMMETRY(Err<MutRef<E>>)
STX_ERR_SYMMETRY(Err<E*>)
STX_ERR_SYMMETRY(Err<E const*>)

template <typename U, typename T>
[[nodiscard]] STX_FORCE_INLINE constexpr bool operator==(
    Ok<U> const&, Err<T> const&) noexcept {
  return false;
}

template <typename U, typename T>
[[nodiscard]] STX_FORCE_INLINE constexpr bool operator!=(
    Ok<U> const&, Err<T> const&) noexcept {
  return true;
}

#undef STX_ERR_SYMMETRY

}  // namespace stx

/// @endcond
