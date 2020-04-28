/**
 * @file storage.h
 * @author Basit Ayantunde <rlamarrr@gmail.com>
 * @brief
 * @version  0.1
 * @date 2020-04-18
 *
 * @copyright Copyright (c) 2020
 *
 */

#ifndef STX_STORAGE_H_
#define STX_STORAGE_H_

#include <cstddef>

namespace stx {

// provides aligned storage for a single value of type ```T```, of which the
// object can be present or not present in the storage buffer
//
//
//
// NOTE:
// Ideally, this storage is not to be swapped, copied, nor moved. It should be
// static to the owning class, the object it represents defines the move, copy,
// construction, and destruction semantics, not this storage buffer.
template <typename T>
class option_variant_storage {
 public:
  using pointer_type = std::byte*;
  using const_pointer_type = std::byte const*;

  constexpr option_variant_storage() = default;

  constexpr option_variant_storage(option_variant_storage const&) = delete;
  constexpr option_variant_storage(option_variant_storage&& rv) = delete;
  constexpr option_variant_storage& operator=(option_variant_storage const&) =
      delete;
  constexpr option_variant_storage& operator=(option_variant_storage&& rv) =
      delete;

  constexpr pointer_type get() noexcept { return content; }
  constexpr const_pointer_type get() const noexcept { return content; }
  constexpr auto& get_ref() noexcept { return content; }

 private:
  alignas(T) std::byte content[sizeof(T)];
};

template <typename A, typename B, bool same_alignment>
class pvs_helper {
 public:
  using pointer_type = std::byte*;
  using const_pointer_type = std::byte const*;

  constexpr pvs_helper() = default;

  constexpr pvs_helper(pvs_helper const&) = delete;
  constexpr pvs_helper(pvs_helper&& rv) = delete;
  constexpr pvs_helper& operator=(pvs_helper const&) = delete;
  constexpr pvs_helper& operator=(pvs_helper&& rv) = delete;

  constexpr pointer_type first() noexcept { return storage_; }
  constexpr pointer_type second() noexcept { return storage_; }

  constexpr auto& first_ref() noexcept { return storage_; }
  constexpr auto& second_ref() noexcept { return storage_; }

  constexpr const_pointer_type first() const noexcept { return storage_; }
  constexpr const_pointer_type second() const noexcept { return storage_; }

 private:
  static constexpr size_t size_ = sizeof(A) > sizeof(B) ? sizeof(A) : sizeof(B);
  alignas(A) std::byte storage_[size_];
};

template <typename A, typename B>
class pvs_helper<A, B, false> {
 public:
  using pointer_type = std::byte*;
  using const_pointer_type = std::byte const*;

  constexpr pvs_helper() = default;

  constexpr pvs_helper(pvs_helper const&) = delete;
  constexpr pvs_helper(pvs_helper&& rv) = delete;
  constexpr pvs_helper& operator=(pvs_helper const&) = delete;
  constexpr pvs_helper& operator=(pvs_helper&& rv) = delete;

  constexpr pointer_type first() noexcept { return storage_a_; }
  constexpr pointer_type second() noexcept { return storage_b_; }

  constexpr auto& first_ref() noexcept { return storage_a_; }
  constexpr auto& second_ref() noexcept { return storage_b_; }

  constexpr const_pointer_type first() const noexcept { return storage_a_; }
  constexpr const_pointer_type second() const noexcept { return storage_b_; }

 private:
  alignas(A) std::byte storage_a_[sizeof(A)];
  alignas(B) std::byte storage_b_[sizeof(B)];
};

// provides aligned storage for a pair variant of type ```A``` and ```B```
// this storage buffer is also optimized for the scenario where both have the
// same alignment in which they use the same buffer. As a variant, only one
// instance of ```A``` or ```B``` can and should be stored at once
//
//
//
//
// NOTE:
// Ideally, this storage is not to be swapped, copied, nor moved. It should be
// static to the owning class, the object it represents defines the move, copy,
// construction, and destruction semantics, not this storage buffer.
template <typename A, typename B>
struct pair_variant_storage
    : public pvs_helper<A, B, alignof(A) == alignof(B)> {
  using base = pvs_helper<A, B, alignof(A) == alignof(B)>;
  using base::base;
};

};  // namespace stx

#endif  // STX_STORAGE_H_
