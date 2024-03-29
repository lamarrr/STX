#pragma once

#include <cinttypes>
#include <cstddef>
#include <cstdlib>

#include "stx/config.h"
#include "stx/enum.h"

STX_BEGIN_NAMESPACE

// any memory handle (contextual, i.e. read-only or writable)
using memory_handle = void *;

// readonly memory handle
using readonly_memory_handle = void const *;

// writable memory handle
using writable_memory_handle = void *;

using static_str_handle = char const *;

enum class [[nodiscard]] RawAllocError : uint8_t
{
  None,
  NoMemory
};

enum class [[nodiscard]] AllocError : uint8_t
{
  NoMemory = enum_uv(RawAllocError::NoMemory)
};

// a static allocator is always available for the lifetime of the program.
//
// a static allocator *should* be thread-safe (preferably lock-free).
// (single-threaded programs don't need them to be thread-safe).
//
// allocators MUST never throw
//
// memory returned from the allocator can be read-only
//
struct AllocatorHandle
{
  // returns `AllocError::NoMemory` if allocation fails
  //
  // returns `nullptr` output if `size` is 0.
  //
  // alignment must be greater than 0.
  //
  virtual RawAllocError allocate(memory_handle &out_mem, size_t size) = 0;

  // if there is not enough memory, the old memory block is not freed and
  // `AllocError::NoMemory` is returned without modifying the output pointer.
  //
  // if `ptr` is nullptr, it should behave as if `allocate(size, alignment)` was
  // called.
  //
  // if `new_size` is 0, the implementation must behave as if
  // `deallocate(nullptr)` is called.
  //
  // `new_size` must not be equal to the present size of the pointer memory.
  //
  // if `ptr` is not `nullptr`, it must have been previously returned by
  // `allocate`.
  //
  // if successful, the bytes in the previous pointer must be copied into the
  // new pointer.
  //
  virtual RawAllocError reallocate(memory_handle &out_mem, size_t new_size) = 0;

  // if `ptr` is `nullptr`, nothing is done.
  // if `ptr` is not `nullptr`, it must have been previously allocated by
  // calling `allocate`, or `reallocate`
  //
  virtual void deallocate(memory_handle mem) = 0;
};

struct NoopAllocatorHandle final : public AllocatorHandle
{
  virtual RawAllocError allocate(memory_handle &, size_t) override
  {
    return RawAllocError::NoMemory;
  }

  virtual RawAllocError reallocate(memory_handle &, size_t) override
  {
    return RawAllocError::NoMemory;
  }

  virtual void deallocate(memory_handle) override
  {
    // no-op
  }
};

struct AllocatorStubHandle final : public AllocatorHandle
{
  virtual RawAllocError allocate(memory_handle &, size_t) override
  {
    return RawAllocError::NoMemory;
  }

  virtual RawAllocError reallocate(memory_handle &, size_t) override
  {
    return RawAllocError::NoMemory;
  }

  virtual void deallocate(memory_handle) override
  {
    // no-op
  }
};

// it has no memory once program is initialized
struct StaticStorageAllocatorHandle final : public AllocatorHandle
{
  virtual RawAllocError allocate(memory_handle &, size_t) override
  {
    return RawAllocError::NoMemory;
  }

  virtual RawAllocError reallocate(memory_handle &, size_t) override
  {
    return RawAllocError::NoMemory;
  }

  virtual void deallocate(memory_handle) override
  {
    // no-op
  }
};

struct OsAllocatorHandle final : public AllocatorHandle
{
  virtual RawAllocError allocate(memory_handle &out_mem, size_t size) override
  {
    if (size == 0)
    {
      out_mem = nullptr;
      return RawAllocError::None;
    }

    memory_handle mem = std::malloc(size);
    if (mem == nullptr)
    {
      return RawAllocError::NoMemory;
    }
    else
    {
      out_mem = mem;
      return RawAllocError::None;
    }
  }

  virtual RawAllocError reallocate(memory_handle &out_mem,
                                   size_t         new_size) override
  {
    if (out_mem == nullptr)
    {
      return allocate(out_mem, new_size);
    }

    if (new_size == 0)
    {
      deallocate(out_mem);
      out_mem = nullptr;
      return RawAllocError::None;
    }

    memory_handle mem = std::realloc(out_mem, new_size);

    if (mem == nullptr)
    {
      return RawAllocError::NoMemory;
    }
    else
    {
      out_mem = mem;
      return RawAllocError::None;
    }
  }

  virtual void deallocate(memory_handle mem) override
  {
    free(mem);
  }
};

constexpr const inline NoopAllocatorHandle noop_allocator_handle;
constexpr const inline AllocatorStubHandle allocator_stub_handle;
constexpr const inline StaticStorageAllocatorHandle
                                         static_storage_allocator_handle;
constexpr const inline OsAllocatorHandle os_allocator_handle;

struct Allocator
{
  explicit constexpr Allocator(AllocatorHandle &allocator_handle) :
      handle{&allocator_handle}
  {}

  constexpr Allocator(Allocator &&other) :
      handle{other.handle}
  {
    other.handle = const_cast<AllocatorStubHandle *>(&allocator_stub_handle);
  }

  constexpr Allocator &operator=(Allocator &&other)
  {
    AllocatorHandle *tmp = other.handle;
    other.handle         = handle;
    handle               = tmp;
    return *this;
  }

  constexpr Allocator(Allocator const &)                 = default;
  constexpr Allocator &operator=(Allocator const &other) = default;

  AllocatorHandle *handle;
};

// const-cast necessary to prove to the compiler that the contents of the
// addresses will not be changed even if the non-const functions are called.
// otherwise, when the compiler sees the allocators, it'd assume the function
// pointer's addresses changed whilst we in fact know they don't change.
//
inline constexpr const Allocator noop_allocator{
    const_cast<NoopAllocatorHandle &>(noop_allocator_handle)};

inline constexpr const Allocator allocator_stub{
    const_cast<AllocatorStubHandle &>(allocator_stub_handle)};

inline constexpr const Allocator static_storage_allocator{
    const_cast<StaticStorageAllocatorHandle &>(
        static_storage_allocator_handle)};

inline constexpr const Allocator os_allocator{
    const_cast<OsAllocatorHandle &>(os_allocator_handle)};

STX_END_NAMESPACE