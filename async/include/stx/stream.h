
#include <atomic>
#include <cinttypes>
#include <utility>

#include "stx/allocator.h"
#include "stx/async.h"
#include "stx/manager.h"
#include "stx/mem.h"
#include "stx/option.h"
#include "stx/rc.h"
#include "stx/spinlock.h"

namespace stx {

enum class [[nodiscard]] YieldAllocError : uint8_t{MemoryFull};
enum class [[nodiscard]] StreamError : uint8_t{Pending, Closed};

//
// # Design Problems
//
// - The stream's memory is never released or re-used when done with. we
// need a notion of unique streams. such that copying onto other streams
// will be explicit and once a stream chunk is processed it is released.
// - This also means we need async managing of the list, preferrably O(1)
// locked or lock-free.
//
// - We want to be able to maintain the indices of the generated data, we'll
// thus need some methods or data member book-keeping to ensure ordering of
// the streams.
//
//
template <typename T>
struct [[nodiscard]] StreamChunk;

// how do we get memory for the stream and its containing data,
// whilst having decent perf?

// # Sharing
//
//
// ## Lifetime Management
//
// how is lifetime managed?
// the stream manages its lifetime via a ref-counted state. the chunks
// individually have different lifetimes and are also ref-counted as they
// will all be shared across executors, filtered, mapped, etc. the stream
// shares the chunks with the executors and observers.
//
//
// ## Cacheline Packing
//
// The streams are unlikely to be processed on the same thread they were
// generated from so cache locality here is probably not a high priority and
// we often allocate the chunks individually over time. though we could
// allocate them at once if the bound is known but that would give little to
// no benefit for non-sequentially processed streams.
//
// Also: the data contained in streams are typically quite heavy, i.e.
// vectors, buffers, arrays, and they will often fit a cacheline.
//
//
// # Locking
//
// The stream is lock-free but its chunks' data are locked via a spinlock
// since we intend to distribute processing across threads and we thus need
// sharing. we use a cheap and fast spinlock since the operations performed
// on the shared data are usually very short-lived compared to the rest of
// the pipeline, ideally nanoseconds. i.e. copy, move, map.
//
template <typename T>
struct [[nodiscard]] StreamChunk {
  // pinned since this is just for placing the object on the address of this
  // chunk.
  STX_MAKE_PINNED(StreamChunk)

  template <typename... Args>
  explicit StreamChunk(Manager imanager, Args &&...iargs)
      : manager{imanager}, data{std::forward<Args>(iargs)...} {}

  // used for sorting ordered and sequential streams.
  // used for getting data from the streams using indices.
  // uint64_t index;

  Manager manager;

  // points to the next added element in the stream it belongs to (if any).
  // must always be nullptr until added into the stream.
  StreamChunk<T> *next{nullptr};

  T data;
};

// a sink that schedules tasks once data from a stream is available
//
// how will the future be awaited? Stream<Map<T>>
//
//
// guaranteeing cacheline packing of streamed data will be in chunks.
// which means if many allocations happen to occur in-between the chunks,
// there will be a lot of cacheline misses when moving from chunk to chunk.
// but that's not important nor a concern since the Stream will be observed
// by the sink in non-deterministic patterns anyway (depending on the number
// of tasks on the executor and their priorities).
//
//
// # Sources and Sinks
// - Streams can get data from multiple sources and yielded-to or streamed
// across multiple threads. (multi-source multi-sink)
// - chunks enter the stream in the order they were inserted.
//
//
// # Responsibilities Delegation
//
// ## Error Handling and Interruption
//
// The generator is left to determine how to handle and report errors to the
// stream and future. i.e. if we run out of memory whilst processing a video
// stream, do we close the stream and return an error via the future or do
// we swallow the error and try again?.
// Also, some streams have non-fatal errors that don't terminate the whole
// stream but only the individual chunks, i.e. packet processing and
// streaming, if a data packet is sent and it timed-out, it is non-fatal and
// okay to try again or ignore, report error and continue.
// Some might even have heuristics. i.e. after 20s of packet transmission
// failure, close the stream and complete the future with an error.
//
// ## Stream Ordering across streams.
//
// i.e. if we need a stream of data and want to process them and then
// perform actions on them in the order they appeared from the root stream.
// i.e. read file in stream sequentially with the indices but spread the
// processing of the streams in any order, process each chunk and then
// re-organize them by indices into an output stream that needs to write
// them out in the order they were received. i.e. read file in stream,
// spread processing across cores, and
// then....................................................
//
//
// We use the indices of the streams. and each operation carries over the
// previous operation's indices if they are linear.
//
//
// TODO(lamarrr): reduce will try to use indices, how do we do this and
// remove the indices, do we store a tag to notify that the stream is
// unordered from the root???
//
//
//==============
// can be a single-source or multi-source stream. for a multi-source stream
// events are gotten into the stream in no specific order between different
// executors.
//================
//
// and the source streams must agree on the indexes of the streams, the
// streams indices should be unique to function with sequential processing
// or ordered streams.
//======================
//
//
// guarantees consistency from point of close
//
//
// Supporting the most parallel and distrubitive of workloads
//
//
// cancelation? doesn't need to be attended to at all or even attended to on
// time. once you request for cancelation, you don't need to wait. proceed
// with what you are doing.
//
//
//
//
// The generator is expected to coordinate itself. i.e. completing the
// future after closing the stream across threads.
//
// The generator is also expected to report errors and decide to handle,
// retry, or continue the stream.
//
//
//
// Consistency Guarantees:
// - closing of the stream is guaranteed to be consistent across streams.
// this means if one stream successfully closes the stream, more data will
// not enter the stream, therefore ensuring consistency of the chunks. i.e.
// chunk inserted whilst closing the stream will always be the last observed
// chunk.
//
//
// IMPORTANT:
// - we can't panic on the executor thread.
// - we need it to be lock-free so we can't ask for a vector as it requires
// locking and mutual exclusion, and even though insertion is armortized we
// can't afford the scenario where it is as expensive as O(n).
//
//
// Essentially a linked list
//
//
template <typename T>
struct [[nodiscard]] StreamState {
  static_assert(!std::is_void_v<T>);
  static_assert(std::is_move_constructible_v<T>);

  STX_MAKE_PINNED(StreamState)

  StreamState() = default;

  SpinLock lock;
  bool closed = false;
  StreamChunk<T> *pop_it = nullptr;
  StreamChunk<T> *yield_last = nullptr;

  // yield is O(1)
  // contention is O(1) and not proportional to the contained object nor
  // management of the chunks.
  //
  // yielding never fails here.
  //
  // REQUIREMENTS:
  //
  // - chunk_handle should ideally be initialized with a ref count of 1 or
  // higher, i.e. unique or multi-owned.
  //
  // if any executor yields before the close request is serviced, they will
  // still be able to yield to the stream.
  //
  void generator____yield(StreamChunk<T> *chunk_handle, bool should_close) {
    bool was_added = false;

    WITH_LOCK(lock, {
      if (closed) {
        was_added = false;
        break;
      }

      // yield_last == nullptr ?: we haven't yielded anything yet
      // pop_it == nullptr?: popping has caught up to yielding and released
      // all the previous handles. so, no handle is valid now.
      //
      if (yield_last == nullptr || pop_it == nullptr) {
        yield_last = chunk_handle;
      } else {
        yield_last->next = chunk_handle;
        yield_last = chunk_handle;
      }

      // popping has previously caught up with yielding, we need to update
      // the popping iterator (to notify that new data has been added)
      if (pop_it == nullptr) {
        pop_it = yield_last;
      }

      closed = should_close;
      was_added = true;
    });

    if (!was_added) {
      chunk_handle->manager.unref();
    }
  }

  void generator____close() {
    WITH_LOCK(lock, {
      //
      closed = true;
    });
  }

  // NOTE that it might still have items in the stream
  bool stream____is_closed() {
    WITH_LOCK(lock, {
      //
      return closed;
    });
  }

  // yield is O(1)
  // contention is O(1) and not proportional to the contained object nor
  // management of the chunks.
  //
  Result<T, StreamError> stream____pop() {
    StreamChunk<T> *chunk = nullptr;

    WITH_LOCK(lock, {
      if (pop_it == nullptr) break;

      chunk = pop_it;

      pop_it = pop_it->next;
    });

    if (chunk == nullptr) {
      if (closed) {
        return Err(StreamError::Closed);
      } else {
        return Err(StreamError::Pending);
      }
    } else {
      T item{std::move(chunk->data)};

      // release the chunk
      chunk->manager.unref();

      return Ok(std::move(item));
    }
  }

 private:
  // we unref the entries in a bottom-up order (inwards-outwards).
  // NOTE: we don't begin unref-ing the entries until we reach the end of
  // the chunks. since the the top-most element refers to the next one,
  // otherwise we'd risk a use-after-unref (use-after-free).
  //
  void unref_pass(StreamChunk<T> *chunk_handle) const {
    if (chunk_handle != nullptr) {
      unref_pass(chunk_handle->next);

      // release the chunk
      chunk_handle->manager.unref();
    }
  }

  void unref_items() const { unref_pass(pop_it); }

 public:
  // guaranteed to not happen along or before the operations possible on the
  // streams.
  ~StreamState() { unref_items(); }
};

template <typename T>
struct [[nodiscard]] BufferMemory {
  STX_DISABLE_COPY(BufferMemory)

  BufferMemory(Memory imemory, uint64_t icapacity)
      : memory{std::move(imemory)}, capacity{icapacity} {}

  BufferMemory() : memory{Memory{noop_allocator, nullptr}}, capacity{0} {}

  BufferMemory(BufferMemory &&other)
      : memory{std::move(other.memory)}, capacity{other.capacity} {
    other.memory.handle = nullptr;
    other.memory.allocator = noop_allocator;
    other.capacity = 0;
  }

  BufferMemory &operator=(BufferMemory &&other) {
    std::swap(memory, other.memory);
    std::swap(capacity, other.capacity);

    return *this;
  }

  T *operator[](uint64_t index) const {
    return static_cast<T *>(memory.handle) + index;
  }

  Memory memory;
  uint64_t capacity;
};

enum class [[nodiscard]] RingBufferError : uint8_t{None = 0, NoMemory};

// A ring buffer for objects that accounts for the C++ object model
template <typename T>
struct [[nodiscard]] SmpRingBuffer {
  STX_MAKE_PINNED(SmpRingBuffer)

  explicit SmpRingBuffer(BufferMemory<T> imemory)
      : memory{std::move(imemory)}, num_available{memory.capacity} {}

  SpinLock lock;
  BufferMemory<T> memory;
  //
  // meta data for tracking allocations and destruction is carefully selected
  // for our use-case. this also ensures there is no double-destroy,
  // double-delete, nor construction at an in-use memory.
  //

  // index to next memory chunk in the ring.
  uint64_t available_start = 0;

  // index to the next available memory chunk.
  uint64_t num_available;

  // index to the next element that needs destructing.
  uint64_t next_destruct_index = 0;

  template <typename... Args>
  Result<T *, RingBufferError> manager____push_inplace(Args &&...args) {
    uint64_t selected = u64_max;

    WITH_LOCK(lock, {
      if (num_available == 0) break;

      selected = available_start;

      available_start = (available_start + 1) % memory.capacity;
      num_available--;
    });

    if (selected == u64_max) return Err(RingBufferError::NoMemory);

    // construct at the allocated memory
    T *placement = new (memory[selected]) T{std::forward<Args>(args)...};

    return Ok(static_cast<T *>(placement));
  }

  Result<T *, RingBufferError> manager____push(T &&value) {
    return manager____push_inplace(std::move(value));
  }

  void manager____pop() {
    uint64_t to_destroy = 0;

    WITH_LOCK(lock, {
      // get the next object that needs destruction and memory release
      to_destroy = next_destruct_index;

      next_destruct_index = (next_destruct_index + 1) % memory.capacity;
    });

    // NOTE: how we've released the lock before destroying the element.
    // this is based on the guarantee that the oldest element in the stream
    // is always destroyed first before the others.
    // and only elements are only ever destroyed once.
    //
    // `next_destruct_index` is always unique.
    //
    // if another request for deletion comes in whilst we are processing one
    // deletion request, it is not suspended and both can happen at the same
    // time.
    //
    // NOTE: we can't allocate the memory the chunk belongs to whilst it is
    // being deleted.
    //
    memory[to_destroy]->~T();

    WITH_LOCK(lock, {
      // only declared as re-usable memory once the object is destroyed.
      num_available = (num_available + 1) % memory.capacity;
    });
  }
};

// essentially a ring-buffer-memory for the stream.
//
// deallocation needs to happen on another thread
//
// belongs to a single generator.
//
// NOTE: streams can use fixed-size ring buffers because they are popped in
// the order they were added (FIFO). This is the primary contract that
// allows this memory type and optimization for streams.
//
//
//
// GeneratorRingBuffer
//
template <typename T>
struct [[nodiscard]] SmpRingBufferManagerHandle final : public ManagerHandle {
  STX_MAKE_PINNED(SmpRingBufferManagerHandle)

  explicit SmpRingBufferManagerHandle(BufferMemory<T> memory)
      : buffer{std::move(memory)} {}

  SmpRingBuffer<T> buffer;

  virtual void ref() override {}
  virtual void unref() override { buffer.manager____pop(); }
};

template <typename T>
Result<BufferMemory<T>, AllocError> make_fixed_buffer_memory(
    Allocator allocator, uint64_t capacity) {
  TRY_OK(memory, mem::allocate(allocator, capacity * sizeof(T)));

  return Ok(BufferMemory<T>{std::move(memory), capacity});
}

template <typename T>
Result<Unique<SmpRingBufferManagerHandle<T> *>, AllocError>
make_managed_smp_ring_buffer(Allocator allocator, BufferMemory<T> memory) {
  return rc::make_unique_inplace<SmpRingBufferManagerHandle<T>>(
      allocator, std::move(memory));
}

template <typename T>
struct [[nodiscard]] Generator {
  STX_DEFAULT_MOVE(Generator)
  STX_DISABLE_COPY(Generator)

  explicit Generator(Rc<StreamState<T> *> &&istate)
      : state{std::move(istate)} {}

  Result<Void, AllocError> yield(Allocator allocator, T &&value,
                                 bool should_close = false) const {
    TRY_OK(memory,
           mem::allocate(
               allocator,
               sizeof(UniqueRcOperation<DeallocateObject<StreamChunk<T>>>)));

    // release memory
    memory.allocator = allocator_stub;

    using chunk_manager = UniqueRcOperation<DeallocateObject<StreamChunk<T>>>;

    auto *chunk = new (memory.handle) chunk_manager{
        os_allocator, Manager{*static_cast<chunk_manager *>(memory.handle)},
        std::move(value)};

    state.handle->generator____yield(&chunk->operation.object, should_close);

    return Ok(Void{});
  }

  void close() const { state.handle->generator____close(); }

  Generator fork() const { return Generator{state.share()}; }

  [[nodiscard]] bool is_closed() const {
    return state.handle->stream____is_closed();
  }

  Rc<StreamState<T> *> state;
};

// NOTE: the Memory Backed generator can use the base generator if it runs out
// of memory
//
//
// The memory backed generator is most efficient for a variety of reasons:
//
// - high data and instruction cache re-use
// - data spatial and temporal locality
// - zero allocation
//
template <typename T>
struct [[nodiscard]] MemoryBackedGenerator {
  STX_DEFAULT_MOVE(MemoryBackedGenerator)
  STX_DISABLE_COPY(MemoryBackedGenerator)

  explicit MemoryBackedGenerator(
      Generator<T> igenerator,
      Unique<SmpRingBufferManagerHandle<StreamChunk<T>> *> iring_buffer_manager)
      : generator{std::move(igenerator)},
        ring_buffer_manager{std::move(iring_buffer_manager)} {}

  Result<Void, RingBufferError> yield(T &&value,
                                      bool should_close = false) const {
    Manager manager{*ring_buffer_manager.handle};

    TRY_OK(placement,
           ring_buffer_manager.handle->buffer.manager____push_inplace(
               manager, std::move(value)));

    generator.state.handle->generator____yield(placement, should_close);

    return Ok(Void{});
  }

  [[nodiscard]] bool is_closed() const { return generator.is_closed(); }

  void close() const { generator.close(); }

  Generator<T> fork() const { return generator.fork(); }

  Generator<T> generator;

  // IMPRORTANT:
  // the struct is packed so that the buffer's memory is not released before the
  // generator is destroyed. the buffer is also pinned to the address since we
  // need to access the memory throughout the lifetime of the generator.
  //
  // memory can not be shared.
  //
  Unique<SmpRingBufferManagerHandle<StreamChunk<T>> *> ring_buffer_manager;
};

template <typename T>
Result<MemoryBackedGenerator<T>, AllocError> make_memory_backed_generator(
    Allocator allocator, BufferMemory<StreamChunk<T>> &&buffer_memory) {
  TRY_OK(generator_state, rc::make_inplace<StreamState<T>>(allocator));

  TRY_OK(smp_ring_buffer_manager, make_managed_smp_ring_buffer<StreamChunk<T>>(
                                      allocator, std::move(buffer_memory)));

  return Ok(MemoryBackedGenerator{Generator<T>{std::move(generator_state)},
                                  std::move(smp_ring_buffer_manager)});
}

template <typename T>
Result<MemoryBackedGenerator<T>, AllocError> make_memory_backed_generator(
    Allocator allocator, uint64_t capacity) {
  TRY_OK(buffer_memory,
         make_fixed_buffer_memory<StreamChunk<T>>(allocator, capacity));

  return make_memory_backed_generator<T>(allocator, std::move(buffer_memory));
}

// TODO(lamarrr): only have generator.get_stream as the way to make a stream
template <typename T>
struct [[nodiscard]] Stream {
  STX_DEFAULT_MOVE(Stream)
  STX_DISABLE_COPY(Stream)

  explicit Stream(Rc<StreamState<T> *> &&istate) : state{std::move(istate)} {}

  Result<T, StreamError> pop() const { return state.handle->stream____pop(); }

  Stream fork() const { return Stream{state.share()}; }

  [[nodiscard]] bool is_closed() const {
    return state.handle->stream____is_closed();
  }

  void close() const { return state.handle->generator____close(); }

  Rc<StreamState<T> *> state;
};

template <typename T>
Result<Generator<T>, AllocError> make_generator(Allocator allocator) {
  TRY_OK(state, rc::make_inplace<StreamState<T>>(allocator));
  return Ok{Generator<T>{std::move(state)}};
}

template <typename T>
Stream<T> make_stream(Generator<T> const &generator) {
  return Stream<T>{generator.state.share()};
}

// scheduler handles operations on the stream.
// it pops attached streams until nothing is left and perfroms the required
// operations on them. this will make the scheduler act as a pipeline for
// streams and generators

// map (fast)
// filter
// enumerate
// seq?
//
// map_seq (slow, needs to be processed one by one to ensure sequential
// execution across threads)
//
// problem now is that how do we know the stream is ordered or not.
//
// i.e. after a filter, it is still sequential but has omitted elements.
//
//
//
// await
//
//

//
//
// we shouldn't support filtering or reducing, the user should handle those
// manually. filtering could be potentially expensive
//
// filter (needs to return index along with data?) -> gapped (for sequential
// processing preceding this we need to interleave their processing)
//

//
// if marked as ordered-source, ordering requirements don't need to wait and
// thus process immediately
//
// if marked as unordered, stream sinks need to wait
// for all of the stream to complete????
//
//
//
// ordered and sequentially processed
// unordered and ....
//
//
//
//
// gapped tag. i.e. filter in which it has to be waited to complete in some
// cases
//
//
//
// combinations of these will consume too much memory
//

/*
enum class StreamTag : uint8_t {
  None = 0,
  Ordered = 0b001,
  Unordered = 0b010,
  Gapped = 0b100
};
*/

/* STX_DEFINE_ENUM_BIT_OPS(StreamTag) */

// or
/* struct StreamAttributes {
  enum class Ordering {} ordering;
  enum class Gapping {} gapping;
};
*/
// limitations: entries are retained even when not needed
// Stream<Stream<int>>???
// this is because of the deferred guarantee
//
//
//

}  // namespace stx
