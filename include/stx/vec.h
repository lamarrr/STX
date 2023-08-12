#pragma once

#include <algorithm>
#include <utility>

#include "stx/allocator.h"
#include "stx/config.h"
#include "stx/memory.h"
#include "stx/result.h"
#include "stx/span.h"
#include "stx/struct.h"
#include "stx/try_ok.h"
#include "stx/void.h"

STX_BEGIN_NAMESPACE

// there is not enough memory for the insertion operation
enum class VecError : uint8_t
{
  OutOfMemory
};

namespace impl
{
template <typename T>
constexpr void destruct_range(T *start, size_t size)
{
  if constexpr (!std::is_trivially_destructible_v<T>)
  {
    for (T &element : Span<T>{start, size})
    {
      element.~T();
    }
  }
}

template <typename T>
constexpr void move_construct_range(T *start, size_t size, T *output)
{
  for (T *iter = start; iter < (start + size); iter++, output++)
  {
    new (output) T{std::move(*iter)};
  }
}

constexpr size_t grow_vec_to_target(size_t present_capacity, size_t target)
{
  return std::max(present_capacity << 1, target);
}

constexpr size_t grow_vec(size_t capacity, size_t new_target_size)
{
  return capacity >= new_target_size ? capacity : grow_vec_to_target(capacity, new_target_size);
}

template <typename T>
constexpr void copy_construct_range(T const *source, size_t size,
                                    T *destination)
{
  for (size_t i = 0; i < size; i++)
  {
    new (destination + i) T{source[i]};
  }
}

}        // namespace impl

// ONLY NON-CONST METHODS INVALIDATE ITERATORS
//
template <typename T>
struct VecBase
{
  static_assert(!std::is_reference_v<T>);

  using Size     = size_t;
  using Index    = size_t;
  using Iterator = T *;
  using Pointer  = T *;

  VecBase(Memory memory, Size size, Size capacity) :
      memory_{std::move(memory)}, size_{size}, capacity_{capacity}
  {}

  VecBase() :
      memory_{Memory{os_allocator, nullptr}}, size_{0}, capacity_{0}
  {}

  explicit VecBase(Allocator allocator) :
      memory_{Memory{allocator, nullptr}}, size_{0}, capacity_{0}
  {}

  VecBase(VecBase &&other) :
      memory_{std::move(other.memory_)},
      size_{other.size_},
      capacity_{other.capacity_}
  {
    other.memory_.allocator = memory_.allocator;
    other.memory_.handle    = nullptr;
    other.size_             = 0;
    other.capacity_         = 0;
  }

  VecBase &operator=(VecBase &&other)
  {
    std::swap(memory_, other.memory_);
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);

    return *this;
  }

  STX_DISABLE_COPY(VecBase)

  ~VecBase()
  {
    impl::destruct_range(begin(), size_);
  }

  Span<T> span() const
  {
    return Span<T>{begin(), size_};
  }

  T &operator[](Index index) const
  {
    return span()[index];
  }

  Option<Ref<T>> at(Index index) const
  {
    return span().at(index);
  }

  Size size() const
  {
    return size_;
  }

  Size capacity() const
  {
    return capacity_;
  }

  bool is_empty() const
  {
    return size_ == 0;
  }

  Pointer data() const
  {
    return static_cast<T *>(memory_.handle);
  }

  Iterator begin() const
  {
    return data();
  }

  Iterator end() const
  {
    return data() + size_;
  }

  // reserve enough memory to contain at least n elements
  //
  // does not release excess memory.
  //
  // returns the error if memory allocation fails
  //
  // invalidates references
  //
  Result<Void, AllocError> reserve(size_t cap)
  {
    size_t new_capacity       = capacity_ > cap ? capacity_ : cap;
    size_t new_capacity_bytes = new_capacity * sizeof(T);

    if (new_capacity != capacity_)
    {
      if constexpr (std::is_trivially_move_constructible_v<T> &&
                    std::is_trivially_destructible_v<T>)
      {
        TRY_OK(ok, mem::reallocate(memory_, new_capacity_bytes));

        (void) ok;

        capacity_ = new_capacity;
      }
      else
      {
        TRY_OK(new_memory,
               mem::allocate(memory_.allocator, new_capacity_bytes));

        T *new_location = static_cast<T *>(new_memory.handle);

        T *iter = new_location;

        for (T &element : span())
        {
          new (iter) T{std::move(element)};
          iter++;
        }

        impl::destruct_range(begin(), size_);

        memory_   = std::move(new_memory);
        capacity_ = new_capacity;
      }

      return Ok(Void{});
    }
    else
    {
      return Ok(Void{});
    }
  }

  // capacity is unchanged
  void clear()
  {
    impl::destruct_range(begin(), size());
    size_ = 0;
  }

  // `capacity` is unchanged
  //
  // `first` must be a valid pointer to an element in the range or the `end` of
  // the vec.
  //
  // `last` must be greater than `end`.
  //
  void erase(Span<T> range)
  {
    STX_SPAN_ENSURE(begin() <= range.begin() && end() >= range.end(),
                    "erase operation out of Vec range");

    size_t destruct_size = range.size();

    T *erase_start = range.begin();
    T *erase_end   = range.end();

    impl::destruct_range(erase_start, destruct_size);

    size_t num_trailing = end() - erase_end;

    // move trailing elements to the front
    impl::move_construct_range(erase_end, num_trailing, erase_start);

    size_ -= destruct_size;
  }

  Memory memory_;
  Size   size_     = 0;
  Size   capacity_ = 0;
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
struct Vec : public VecBase<T>
{
  using base = VecBase<T>;

  explicit Vec(Memory memory, size_t size, size_t capacity) :
      base{std::move(memory), size, capacity}
  {}

  Vec() :
      base{}
  {}

  explicit Vec(Allocator allocator) :
      base{allocator}
  {}

  STX_DEFAULT_MOVE(Vec)
  STX_DISABLE_COPY(Vec)
  STX_DEFAULT_DESTRUCTOR(Vec)

  // invalidates references
  //
  //
  // typically needed for non-movable types
  template <typename... Args>
  Result<Void, AllocError> push_inplace(Args &&...args)
  {
    static_assert(std::is_constructible_v<T, Args &&...>);

    size_t const target_size  = base::size_ + 1;
    size_t const new_capacity = impl::grow_vec(base::capacity_, target_size);

    TRY_OK(ok, base::reserve(new_capacity));

    (void) ok;

    T *inplace_construct_pos = base::begin() + base::size_;

    new (inplace_construct_pos) T{std::forward<Args>(args)...};

    base::size_ = target_size;

    return Ok(Void{});
  }

  // invalidates references
  //
  // value is not moved if an allocation error occurs
  Result<Void, AllocError> push(T &&value)
  {
    return push_inplace(std::move(value));
  }

  Result<Void, AllocError> resize(size_t target_size, T const &to_copy = {})
  {
    size_t const previous_size = base::size();

    if (target_size > previous_size)
    {
      size_t const new_capacity = impl::grow_vec(base::capacity(), target_size);

      TRY_OK(ok, base::reserve(new_capacity));

      (void) ok;

      T *copy_construct_begin = base::begin() + previous_size;
      T *copy_construct_end   = base::begin() + target_size;

      for (T *iter = copy_construct_begin; iter < copy_construct_end; iter++)
      {
        new (iter) T{to_copy};
      }

      base::size_ = target_size;

      return Ok(Void{});
    }
    else
    {
      // target_size <= previous_size
      T *destruct_begin = base::begin() + target_size;
      impl::destruct_range(destruct_begin, previous_size - target_size);

      base::size_ = target_size;

      return Ok(Void{});
    }
  }

  // resizes the vector to the given size without initializing new elements
  //
  // returns the span of the additional uninitialized elements on success, or an AllocError on failure
  //
  // WARNING: ensure you initialize the types before calling any other methods of Vec to ensure the elements
  // contain valid objects, which would otherwise be catastrophic
  Result<Span<T>, AllocError> unsafe_resize_uninitialized(size_t target_size)
  {
    size_t previous_size = base::size();

    if (target_size > previous_size)
    {
      size_t const new_capacity = impl::grow_vec(base::capacity(), target_size);

      TRY_OK(ok, base::reserve(new_capacity));

      (void) ok;

      const size_t num_uninitialized = target_size - previous_size;
      base::size_                    = target_size;

      return Ok(Span<T>{base::begin() + previous_size, num_uninitialized});
    }
    else
    {
      // target_size <= previous_size
      T *destruct_begin = base::begin() + target_size;
      impl::destruct_range(destruct_begin, previous_size - target_size);

      base::size_ = target_size;

      return Ok(Span<T>{});
    }
  }

  Result<Vec<T>, AllocError> copy(Allocator allocator) const
  {
    TRY_OK(memory,
           mem::allocate(allocator, base::capacity() * sizeof(T)));

    impl::copy_construct_range(base::begin(), base::size(),
                               static_cast<T *>(memory.handle));

    return Ok(Vec<T>{std::move(memory), base::size(), base::capacity()});
  }

  Result<Void, AllocError> extend(Span<T const> other)
  {
    TRY_OK(ok, base::reserve(base::size() + other.size()));

    (void) ok;

    impl::copy_construct_range(other.begin(), other.size(), base::end());

    base::size_ += other.size();

    return Ok(Void{});
  }

  Result<Void, AllocError> extend_move(Span<T> other)
  {
    TRY_OK(ok, base::reserve(base::size() + other.size()));

    (void) ok;

    impl::move_construct_range(other.begin(), other.size(), base::end());

    base::size_ += other.size();

    return Ok(Void{});
  }

  Option<T> pop()
  {
    if (base::size_ == 0)
      return None;

    T last = std::move(*(base::begin() + base::size_ - 1));

    resize(base::size_ - 1).unwrap();

    return Some(std::move(last));
  }
};

// a fixed capacity vec
template <typename T>
struct FixedVec : public VecBase<T>
{
  using base = VecBase<T>;

  explicit FixedVec(Memory memory, size_t size, size_t capacity) :
      base{std::move(memory), size, capacity}
  {}

  FixedVec() :
      base{}
  {}

  explicit FixedVec(Allocator allocator) :
      base{allocator}
  {}

  STX_DEFAULT_MOVE(FixedVec)
  STX_DISABLE_COPY(FixedVec)
  STX_DEFAULT_DESTRUCTOR(FixedVec)

  template <typename... Args>
  Result<Void, VecError> push_inplace(Args &&...args)
  {
    static_assert(std::is_constructible_v<T, Args &&...>);
    size_t const target_size = base::size_ + 1;

    if (base::capacity_ < target_size)
    {
      return Err(VecError::OutOfMemory);
    }
    else
    {
      new (base::begin() + base::size_) T{std::forward<Args>(args)...};

      base::size_ = target_size;

      return Ok(Void{});
    }
  }

  Result<Void, VecError> push(T &&value)
  {
    return push_inplace(std::move(value));
  }

  Result<Void, VecError> resize(size_t target_size, T const &to_copy = {})
  {
    size_t const previous_size = base::size();

    if (target_size > previous_size)
    {
      if (target_size > base::capacity())
      {
        return Err(VecError::OutOfMemory);
      }

      T *copy_construct_begin = base::begin() + previous_size;
      T *copy_construct_end   = base::begin() + target_size;
      for (T *iter = copy_construct_begin; iter < copy_construct_end; iter++)
      {
        new (iter) T{to_copy};
      }
    }
    else if (target_size < previous_size)
    {
      T *destruct_begin = base::begin() + target_size;
      impl::destruct_range(destruct_begin, previous_size - target_size);
    }
    else
    {
      // equal
    }

    base::size_ = target_size;

    return Ok(Void{});
  }

  Result<FixedVec<T>, AllocError> copy(Allocator allocator) const
  {
    TRY_OK(memory,
           mem::allocate(allocator, base::capacity() * sizeof(T)));

    impl::copy_construct_range(base::begin(), base::size(),
                               static_cast<T *>(memory.handle));

    return Ok(FixedVec<T>{std::move(memory), base::size(), base::capacity()});
  }

  Result<Void, AllocError> extend(Span<T const> other)
  {
    TRY_OK(ok, base::reserve(base::size() + other.size()));

    (void) ok;

    impl::copy_construct_range(other.begin(), other.size(), base::end());

    base::size_ += other.size();

    return Ok(Void{});
  }

  Result<Void, AllocError> extend_move(Span<T> other)
  {
    TRY_OK(ok, base::reserve(base::size() + other.size()));

    (void) ok;

    impl::move_construct_range(other.begin(), other.size(), base::end());

    base::size_ += other.size();

    return Ok(Void{});
  }

  Option<T> pop()
  {
    if (base::size_ == 0)
      return None;

    T last = std::move(*(base::begin() + base::size_ - 1));

    resize(base::size_ - 1).unwrap();

    return Some(std::move(last));
  }
};

namespace vec
{
template <typename T>
Result<Vec<T>, AllocError> make(Allocator allocator, size_t capacity = 0)
{
  TRY_OK(memory, mem::allocate(allocator, capacity * sizeof(T)));
  return Ok(Vec<T>{std::move(memory), 0, capacity});
}

template <typename T>
Result<Vec<std::remove_const_t<T>>, AllocError> make_copy(Allocator allocator, stx::Span<T const> src)
{
  TRY_OK(v, vec::make<std::remove_const_t<T>>(allocator, src.size()));
  TRY_OK(_, v.extend(src));
  return stx::Ok(std::move(v));
}

template <typename T>
Result<Vec<T>, AllocError> make_move(Allocator allocator, stx::Span<T> src)
{
  static_assert(!std::is_const_v<T>);
  TRY_OK(v, vec::make<T>(allocator, src.size()));
  TRY_OK(_, v.extend_move(src));
  return stx::Ok(std::move(v));
}

template <typename T>
Result<FixedVec<T>, AllocError> make_fixed(Allocator allocator, size_t capacity = 0)
{
  TRY_OK(memory, mem::allocate(allocator, capacity * sizeof(T)));
  return Ok(FixedVec<T>{std::move(memory), 0, capacity});
}

}        // namespace vec

STX_END_NAMESPACE
