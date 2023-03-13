#pragma once

#include <utility>

#include "stx/allocator.h"
#include "stx/result.h"
#include "stx/void.h"

STX_BEGIN_NAMESPACE

// an always-valid memory
struct Memory
{
  constexpr Memory(Allocator iallocator, writable_memory_handle imemory) :
      allocator{iallocator}, handle{imemory}
  {}

  Memory(Memory const &)            = delete;
  Memory &operator=(Memory const &) = delete;

  Memory(Memory &&other) :
      allocator{other.allocator}, handle{other.handle}
  {
    other.allocator = allocator_stub;
    other.handle    = nullptr;
  }

  Memory &operator=(Memory &&other)
  {
    std::swap(allocator, other.allocator);
    std::swap(handle, other.handle);

    return *this;
  }

  ~Memory()
  {
    allocator.handle->deallocate(handle);
  }

  Allocator              allocator;
  writable_memory_handle handle;
};

// could possibly be from static-storage. i.e. c-strings
struct ReadOnlyMemory
{
  constexpr ReadOnlyMemory(Allocator iallocator, readonly_memory_handle imemory) :
      allocator{iallocator}, handle{imemory}
  {}

  explicit constexpr ReadOnlyMemory(Memory &&other) :
      allocator{other.allocator}, handle{other.handle}
  {
    other.allocator = noop_allocator;
    other.handle    = nullptr;
  }

  ReadOnlyMemory(ReadOnlyMemory const &)            = delete;
  ReadOnlyMemory &operator=(ReadOnlyMemory const &) = delete;

  ReadOnlyMemory(ReadOnlyMemory &&other) :
      allocator{other.allocator}, handle{other.handle}
  {
    other.allocator = allocator_stub;
    other.handle    = nullptr;
  }

  ReadOnlyMemory &operator=(ReadOnlyMemory &&other)
  {
    std::swap(allocator, other.allocator);
    std::swap(handle, other.handle);
    return *this;
  }

  ~ReadOnlyMemory()
  {
    allocator.handle->deallocate(const_cast<memory_handle>(handle));
  }

  Allocator              allocator;
  readonly_memory_handle handle;
};

namespace mem
{

inline Result<Memory, AllocError> allocate(Allocator allocator, size_t size)
{
  memory_handle memory = nullptr;

  RawAllocError error = allocator.handle->allocate(memory, size);

  if (error != RawAllocError::None)
  {
    return Err(AllocError{enum_uv(error)});
  }
  else
  {
    return Ok(Memory{allocator, memory});
  }
}

inline Result<Void, AllocError> reallocate(Memory &memory, size_t new_size)
{
  memory_handle new_memory_handle = memory.handle;

  RawAllocError error =
      memory.allocator.handle->reallocate(new_memory_handle, new_size);

  if (error != RawAllocError::None)
  {
    return Err(AllocError{enum_uv(error)});
  }
  else
  {
    memory.handle = new_memory_handle;
    return Ok(Void{});
  }
}

}        // namespace mem

STX_END_NAMESPACE
