#pragma once

#include <algorithm>
#include <utility>

#include "stx/allocator.h"
#include "stx/config.h"
#include "stx/result.h"
#include "stx/span.h"
#include "stx/struct.h"
#include "stx/try_ok.h"
#include "stx/void.h"

// TODO(lamarrr): these definitions should go into another file
#define STX_DISABLE_BOUNDS_CHECK 0

#if STX_DISABLE_BOUNDS_CHECK
#define STX_CHECK_IN_BOUND(cmp_first, cmp_last)
#define STX_CHECK_IN_RANGE(first, last, cmp_first, cmp_last)
#else
#define STX_CHECK_IN_BOUND(cmp_first, cmp_last)
#define STX_CHECK_IN_RANGE(first, last, cmp_first, cmp_last)
#endif

// TODO
#define STX_ENSURE(condition, error_message)

STX_BEGIN_NAMESPACE

// TODO(lamarrr): we should find another name other than Vec
//
// TODO(lamarrr): consider adding checks to the vec[]
//

// there is not enough memory for the insertion operation
enum class VecError : uint8_t { OutOfMemory };

template <typename T>
constexpr void destruct_range(T* start, size_t size) {
  if constexpr (std::is_trivially_destructible_v<T>) {
  } else {
    for (T& element : Span<T>{start, size}) {
      element.~T();
    }
  }
}

template <typename T>
constexpr void move_construct_range(T* start, size_t size, T* output) {
  for (T* iter = start; iter < (start + size); iter++, output++) {
    new (output) T{std::move(*iter)};
  }
}

constexpr size_t grow_vec_to_target(size_t present_capacity, size_t target) {
  return std::max(present_capacity << 1, target);
}

constexpr size_t grow_vec(size_t capacity, size_t new_target_size) {
  return capacity >= new_target_size
             ? capacity
             : grow_vec_to_target(capacity, new_target_size);
}

//
// ONLY NON-CONST METHODS INVALIDATE ITERATORS
//
//
//
// TODO(lamarrr): clear and all state-mutating ones should use std::move and a
// separate method.
//
//
//
template <typename T>
struct VecBase {
  static_assert(!std::is_reference_v<T>);

  static constexpr size_t alignment = alignof(T);
  static constexpr size_t element_size = sizeof(T);

  VecBase(Memory memory, size_t size, size_t capacity)
      : memory_{std::move(memory)}, size_{size}, capacity_{capacity} {}

  VecBase()
      : memory_{Memory{noop_allocator, nullptr}}, size_{0}, capacity_{0} {}

  explicit VecBase(Allocator allocator)
      : memory_{Memory{allocator, nullptr}}, size_{0}, capacity_{0} {}

  VecBase(VecBase&& other)
      : memory_{std::move(other.memory_)},
        size_{other.size_},
        capacity_{other.capacity_} {
    other.memory_.allocator = noop_allocator;
    other.memory_.handle = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
  }

  VecBase& operator=(VecBase&& other) {
    std::swap(memory_, other.memory_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);

    return *this;
  }

  STX_DISABLE_COPY(VecBase)

  ~VecBase() { destruct_range(begin(), size_); }

  Span<T> span() const { return Span<T>{begin(), size_}; }

  T& operator[](size_t index) const { return span()[index]; }

  Option<Ref<T>> at(size_t index) const { return span().at(index); }

  size_t size() const { return size_; }

  size_t capacity() const { return capacity_; }

  bool is_empty() const { return size_ == 0; }

  T* data() const { return static_cast<T*>(memory_.handle); }

  T* begin() const { return data(); }

  T* end() const { return data() + size_; }

  Memory memory_;
  size_t size_ = 0;
  size_t capacity_ = 0;
};

// Vec is an adapter to an allocator.
//
// Vec maintains a contiguous sequence of elements, and insertions or removal
// of elements causes the elements to be moved if necessary.
//
// the allocator must be alive for the lifetime of the Vec.
//
// Just like std::vector, Vec armortizes insertion to O(1) as allocation can be
// slow. but doesn't select its own allocator nor hide errors behind exceptions.
//
// Vecs are growable adapters abstractions for memory resources.
//
// Vec is also a memory allocation deferrer. i.e. it tries to minimize the
// costs of memory allocation and deallocation.
//
template <typename T>
struct Vec : public VecBase<T> {
  using base = VecBase<T>;

  explicit Vec(Memory memory, size_t size, size_t capacity)
      : base{std::move(memory), size, capacity} {}

  Vec() : base{} {}

  explicit Vec(Allocator allocator) : base{allocator} {}
  // TODO(lamarrr): test move, test move-assign

  STX_DEFAULT_MOVE(Vec)
  STX_DISABLE_COPY(Vec)
  STX_DEFAULT_DESTRUCTOR(Vec)
};

// a fixed capacity vec
template <typename T>
struct FixedVec : public VecBase<T> {
  using base = VecBase<T>;

  explicit FixedVec(Memory memory, size_t size, size_t capacity)
      : base{std::move(memory), size, capacity} {}

  FixedVec() : base{} {}

  explicit FixedVec(Allocator allocator) : base{allocator} {}

  STX_DEFAULT_MOVE(FixedVec)
  STX_DISABLE_COPY(FixedVec)
  STX_DEFAULT_DESTRUCTOR(FixedVec)
};

namespace vec {

template <typename T>
Result<Vec<T>, AllocError> make(Allocator allocator, size_t capacity = 0) {
  TRY_OK(memory, mem::allocate(allocator, capacity * sizeof(T)));

  return Ok(Vec<T>{std::move(memory), 0, capacity});
}

template <typename T>
Result<FixedVec<T>, AllocError> make_fixed(Allocator allocator,
                                           size_t capacity = 0) {
  TRY_OK(memory, mem::allocate(allocator, capacity * sizeof(T)));

  return Ok(FixedVec<T>{std::move(memory), 0, capacity});
}

// reserve enough memory to contain at least n elements
//
// does not release excess memory.
//
// returns the error if memory allocation fails
//
// invalidates references
//
template <typename T>
Result<Void, AllocError> vec____reserve(VecBase<T>& base, size_t cap) {
  size_t new_capacity = base.capacity_ > cap ? base.capacity_ : cap;
  size_t new_capacity_bytes = new_capacity * base.element_size;

  if (new_capacity != base.capacity_) {
    if constexpr (std::is_trivially_move_constructible_v<T> &&
                  std::is_trivially_destructible_v<T>) {
      TRY_OK(ok, mem::reallocate(base.memory_, new_capacity_bytes));

      (void)ok;

      base.capacity_ = new_capacity;
    } else {
      TRY_OK(new_memory,
             mem::allocate(base.memory_.allocator, new_capacity_bytes));

      T* new_location = static_cast<T*>(new_memory.handle);

      T* iter = new_location;

      for (T& element : base.span()) {
        new (iter) T{std::move(element)};
        iter++;
      }

      destruct_range(base.begin(), base.size_);

      base.memory_ = std::move(new_memory);
      base.capacity_ = new_capacity;
    }

    return Ok(Void{});

  } else {
    return Ok(Void{});
  }
}

template <typename T>
Result<Vec<T>, AllocError> reserve(Vec<T>&& vec, size_t capacity) {
  TRY_OK(ok, vec____reserve(vec, capacity));

  (void)ok;

  return Ok(std::move(vec));
}

template <typename T>
Result<FixedVec<T>, AllocError> reserve(FixedVec<T>&& vec, size_t capacity) {
  TRY_OK(ok, vec____reserve(vec, capacity));

  return Ok(std::move(vec));
}

// invalidates references
//
//
// typically needed for non-movable types
template <typename T, typename... Args>
Result<Vec<T>, AllocError> push_inplace(Vec<T>&& vec, Args&&... args) {
  static_assert(std::is_constructible_v<T, Args&&...>);

  size_t const target_size = vec.size_ + 1;
  size_t const new_capacity = grow_vec(vec.capacity_, target_size);

  TRY_OK(new_vec, reserve(std::move(vec), new_capacity));

  T* inplace_construct_pos = new_vec.begin() + new_vec.size_;

  new (inplace_construct_pos) T{std::forward<Args>(args)...};

  new_vec.size_ = target_size;

  return Ok(std::move(new_vec));
}

// invalidates references
//
// value is not moved if an allocation error occurs
template <typename T>
Result<Vec<T>, AllocError> push(Vec<T>&& vec, T&& value) {
  return push_inplace(std::move(vec), std::move(value));
}

template <typename T>
Result<Vec<T>, AllocError> push(Vec<T>&& vec, T& value) = delete;

template <typename T, typename... Args>
Result<FixedVec<T>, VecError> push_inplace(FixedVec<T>&& vec, Args&&... args) {
  static_assert(std::is_constructible_v<T, Args&&...>);
  size_t const target_size = vec.size_ + 1;

  if (vec.capacity_ < target_size) {
    return Err(VecError::OutOfMemory);
  } else {
    new (vec.begin() + vec.size_) T{std::forward<Args>(args)...};

    vec.size_ = target_size;

    return Ok(std::move(vec));
  }
}

template <typename T>
Result<FixedVec<T>, VecError> push(FixedVec<T>&& vec, T&& value) {
  return push_inplace(std::move(vec), std::move(value));
}

template <typename T>
Result<FixedVec<T>, VecError> push(FixedVec<T>&& vec, T& value) = delete;

template <typename T>
Result<Vec<T>, AllocError> copy(Allocator, Vec<T> const&);

// smaller size or zero?
//
///
//
//
//
//
// TODO(lamarrr): resize and move should use move semantics
//
//
//
//
//
//
//

template <typename T>
Result<Vec<T>, AllocError> resize(Vec<T>&& vec, size_t target_size,
                                  T const& to_copy = {}) {
  size_t const previous_size = vec.size();

  if (target_size > previous_size) {
    size_t const new_capacity = grow_vec(vec.capacity(), target_size);

    TRY_OK(new_vec, reserve(std::move(vec), new_capacity));

    T* copy_construct_begin = new_vec.begin() + previous_size;
    T* copy_construct_end = new_vec.begin() + target_size;

    for (T* iter = copy_construct_begin; iter < copy_construct_end; iter++) {
      new (iter) T{to_copy};
    }

    new_vec.size_ = target_size;

    return Ok(std::move(new_vec));

  } else {
    // target_size <= previous_size
    T* destruct_begin = vec.begin() + target_size;
    destruct_range(destruct_begin, previous_size - target_size);

    vec.size_ = target_size;

    return Ok(std::move(vec));
  }
}

// smaller size or zero?
template <typename T>
Result<Void, VecError> resize(FixedVec<T>&& vec, size_t target_size,
                              T const& to_copy = {}) {
  size_t const previous_size = vec.size();

  if (target_size > previous_size) {
    if (target_size > vec.capacity()) {
      return Err(VecError::OutOfMemory);
    }

    T* copy_construct_begin = vec.begin() + previous_size;
    T* copy_construct_end = vec.begin() + target_size;
    for (T* iter = copy_construct_begin; iter < copy_construct_end; iter++) {
      new (iter) T{to_copy};
    }
  } else if (target_size < previous_size) {
    T* destruct_begin = vec.begin() + target_size;
    destruct_range(destruct_begin, previous_size - target_size);
  } else {
    // equal
  }

  vec.size_ = target_size;

  return Ok(std::move(vec));
}

// TODO(lamarrr): verify validity of these methods
// capacity is unchanged
template <typename T>
void vec____clear(VecBase<T>& base) {
  destruct_range(base.begin(), base.end());
  base.size_ = 0;
}

template <typename T>
void clear(Vec<T>&& vec) {
  vec____clear(vec);
}

template <typename T>
void clear(FixedVec<T>&& vec) {
  vec____clear(vec);
}

// `capacity` is unchanged
//
// `first` must be a valid pointer to an element in the range or the `end` of
// the vec.
//
// `last` must be greater than `end`.
//
// TODO(lamarrr): this should take a span
//
//
template <typename T>
void vec____erase(VecBase<T>& base, Span<T> range) {
  STX_ENSURE(base.begin() <= range.begin() && base.end() >= range.end(),
             "erase operation out of Vec range");

  size_t destruct_size = range.size();

  T* erase_start = range.begin();
  T* erase_end = range.end();

  destruct_range(erase_start, destruct_size);

  size_t num_trailing = base.end() - erase_end;

  // move trailing elements to the front
  move_construct_range(erase_end, num_trailing, erase_start);

  base.size_ -= destruct_size;
}

template <typename T>
Vec<T> erase(Vec<T>&& vec, Span<T> range) {
  vec____erase(vec, range);
  return std::move(vec);
}

template <typename T>
Vec<T> erase(FixedVec<T>&& vec, Span<T> range) {
  vec____erase(vec, range);
  return std::move(vec);
}

}  // namespace vec

STX_END_NAMESPACE
