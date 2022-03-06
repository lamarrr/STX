#pragma once

#include <cinttypes>

// needed since the app has a lot of allocations.
// we don't care about the types, we just want fine cacheline patterns
//
//
// must respect alignment
//
// pretends to be static
//
//
//
// can not be shutdown until all handled out memory references are released
//
//
//
// memory is a linear resource. categorizing by objects is never ok. you pack
// into usage groups.
//
//
struct MemoryPoolAllocator {
  // trivially destructible blocks
  // non-trivially destructible blocks
  explicit MemoryPoolAllocator(uint64_t);
  MemoryPoolAllocator(MemoryPoolAllocator const&) = delete;
  MemoryPoolAllocator& operator=(MemoryPoolAllocator const&) = delete;

  uint64_t chunk_size_;
};
