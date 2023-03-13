#include "stx/stream.h"

#include "stx/allocator.h"
#include "stx/memory.h"
#include "stx/rc.h"
#include "gtest/gtest.h"

struct object_stats
{
  uint64_t n_construct = 0;
  uint64_t n_destroy   = 0;
};

struct ObjectMock
{
  ObjectMock()
  {
    id = make_id();
    stats().n_construct++;
  }

  ObjectMock(ObjectMock const &)
  {
    id = make_id();
    stats().n_construct++;
  }

  ObjectMock(ObjectMock &&other)
  {
    id = other.id;
    stats().n_construct++;
  }

  ObjectMock &operator=(ObjectMock const &)
  {
    id = make_id();
    return *this;
  }

  ObjectMock &operator=(ObjectMock &&other)
  {
    id = other.id;
    return *this;
  }

  ~ObjectMock()
  {
    stats().n_destroy++;
  }

  static object_stats &stats()
  {
    static object_stats stats_data;
    return stats_data;
  }

  static int64_t make_id()
  {
    static int64_t next_id = 0;
    int64_t        id      = next_id;
    next_id++;
    return id;
  }

  int64_t id;
};

TEST(StreamTest, Basic)
{
  using namespace stx;
  // heap allocated so we can also test for use-after-free and double-free, we
  // could instrument the code to check this but we use ASAN and UB-SAN.

  void *mem0, *mem1, *mem2, *mem3, *mem4, *mem5;
  ASSERT_EQ(os_allocator.handle->allocate(
                mem0, sizeof(RcOperation<DeallocateObject<StreamChunk<int>>>)),
            RawAllocError::None);
  ASSERT_EQ(os_allocator.handle->allocate(
                mem1, sizeof(RcOperation<DeallocateObject<StreamChunk<int>>>)),
            RawAllocError::None);
  ASSERT_EQ(os_allocator.handle->allocate(
                mem2, sizeof(RcOperation<DeallocateObject<StreamChunk<int>>>)),
            RawAllocError::None);
  ASSERT_EQ(os_allocator.handle->allocate(
                mem3, sizeof(RcOperation<DeallocateObject<StreamChunk<int>>>)),
            RawAllocError::None);
  ASSERT_EQ(os_allocator.handle->allocate(
                mem4, sizeof(RcOperation<DeallocateObject<StreamChunk<int>>>)),
            RawAllocError::None);
  ASSERT_EQ(os_allocator.handle->allocate(
                mem5, sizeof(RcOperation<DeallocateObject<StreamChunk<int>>>)),
            RawAllocError::None);

  using chunk_manager = RcOperation<DeallocateObject<StreamChunk<int>>>;

  auto *chunk0 = new (mem0) RcOperation<DeallocateObject<StreamChunk<int>>>{
      1, os_allocator, Manager{*static_cast<chunk_manager *>(mem0)}, 0};
  auto *chunk1 = new (mem1) RcOperation<DeallocateObject<StreamChunk<int>>>{
      1, os_allocator, Manager{*static_cast<chunk_manager *>(mem1)}, 1};
  auto *chunk2 = new (mem2) RcOperation<DeallocateObject<StreamChunk<int>>>{
      1, os_allocator, Manager{*static_cast<chunk_manager *>(mem2)}, 2};
  auto *chunk3 = new (mem3) RcOperation<DeallocateObject<StreamChunk<int>>>{
      1, os_allocator, Manager{*static_cast<chunk_manager *>(mem3)}, 3};
  auto *chunk4 = new (mem4) RcOperation<DeallocateObject<StreamChunk<int>>>{
      1, os_allocator, Manager{*static_cast<chunk_manager *>(mem4)}, 4};
  auto *chunk5 = new (mem5) RcOperation<DeallocateObject<StreamChunk<int>>>{
      1, os_allocator, Manager{*static_cast<chunk_manager *>(mem5)}, 5};

  {
    StreamState<int> state;

    // test that the first pop returns a pending error
    EXPECT_EQ(state.stream____pop(), Err(StreamError::Pending));

    // yield to the stream without closing it
    state.generator____yield(&chunk0->operation.object, false);

    // test that we can get the yielded value back from the stream
    EXPECT_EQ(state.stream____pop(), Ok(0));

    // test that we get a pending error when elements in the stream are
    // exhausted
    state.generator____yield(&chunk1->operation.object, false);
    state.generator____yield(&chunk2->operation.object, false);

    EXPECT_EQ(state.stream____pop(), Ok(1));
    EXPECT_EQ(state.stream____pop(), Ok(2));
    EXPECT_EQ(state.stream____pop(), Err(StreamError::Pending));
    EXPECT_EQ(state.stream____pop(), Err(StreamError::Pending));

    // test that closing immediately after yielding actually closes the stream
    state.generator____yield(&chunk3->operation.object, false);
    state.generator____yield(&chunk4->operation.object, true);
    state.generator____yield(&chunk5->operation.object, true);

    // test that the stream has been closed
    EXPECT_TRUE(state.stream____is_closed());

    // test that we can pop the remaining elements in the stream
    EXPECT_EQ(state.stream____pop(), Ok(3));
    EXPECT_EQ(state.stream____pop(), Ok(4));

    // check that we can't get any element added after the stream was closed
    EXPECT_EQ(state.stream____pop(), Err(StreamError::Closed));
    EXPECT_EQ(state.stream____pop(), Err(StreamError::Closed));
  }
}

TEST(StreamRingMemoryTest, Basic)
{
  using namespace stx;

  {
    auto generator = make_memory_backed_generator<int>(os_allocator, 200);
  }
  {
    auto generator_state =
        stx::rc::make_inplace<stx::StreamState<int>>(os_allocator).unwrap();

    Generator<int> generator{std::move(generator_state)};
  }

  auto smp_buffer_rc =
      make_managed_smp_ring_buffer(
          os_allocator,
          make_fixed_buffer_memory<StreamChunk<int>>(os_allocator, 3).unwrap())
          .unwrap();

  auto buffer = &smp_buffer_rc.handle->buffer;

  Manager manager{*smp_buffer_rc.handle};
  // auto buffer =  generator.ring_buffer_manager.handle->buffer.memory[0];

  // buffer[0]

  EXPECT_TRUE(buffer->manager____push_inplace(manager, 0).is_ok());
  EXPECT_TRUE(buffer->manager____push_inplace(manager, 1).is_ok());
  EXPECT_TRUE(buffer->manager____push_inplace(manager, 2).is_ok());

  EXPECT_EQ(buffer->memory[0]->data, 0);
  EXPECT_EQ(buffer->memory[1]->data, 1);
  EXPECT_EQ(buffer->memory[2]->data, 2);
  EXPECT_EQ(buffer->available_start, 0);
  EXPECT_EQ(buffer->num_available, 0);
  EXPECT_EQ(buffer->next_destruct_index, 0);

  EXPECT_TRUE(buffer->manager____push_inplace(manager, 3).is_err());

  buffer->manager____pop();

  EXPECT_TRUE(buffer->manager____push_inplace(manager, 3).is_ok());
  EXPECT_FALSE(buffer->manager____push_inplace(manager, 3).is_ok());

  EXPECT_EQ(buffer->memory[0]->data, 3);

  buffer->manager____pop();
  buffer->manager____pop();
  buffer->manager____pop();
}

TEST(StreamRingMemoryTest, ObjectMockTest)
{
  using namespace stx;

  auto smp_ring_buffer =
      make_managed_smp_ring_buffer<ObjectMock>(
          os_allocator,
          make_fixed_buffer_memory<ObjectMock>(os_allocator, 69).unwrap())
          .unwrap();

  auto &buffer = smp_ring_buffer.handle->buffer;

  // add more than the actual number of elements so we can tell if the C++
  // object model is respected
  for (size_t i = 0; i < 69; i++)
  {
    EXPECT_TRUE(buffer.manager____push_inplace().is_ok());
  }

  for (size_t i = 0; i < 200; i++)
  {
    EXPECT_FALSE(buffer.manager____push_inplace().is_ok());
  }

  // pop all the actually added elements
  for (size_t i = 0; i < 69; i++)
  {
    buffer.manager____pop();
  }

  EXPECT_EQ(ObjectMock::stats().n_construct, ObjectMock::stats().n_destroy);
}

TEST(STream, x)
{
  using namespace stx;

  Stream<int> stream{rc::make_inplace<StreamState<int>>(os_allocator).unwrap()};

  EXPECT_FALSE(stream.is_closed());
  EXPECT_EQ(stream.pop(), Err(StreamError::Pending));
  stream.close();
  Stream child = stream.fork();
  EXPECT_TRUE(stream.is_closed());
  EXPECT_TRUE(child.is_closed());
  EXPECT_EQ(stream.pop(), Err(StreamError::Closed));
}

TEST(Stream, MemoryBackedGenerator)
{
  using namespace stx;

  auto   generator = make_memory_backed_generator<int>(os_allocator, 4).unwrap();
  Stream stream{generator.generator.state.share()};

  EXPECT_FALSE(generator.is_closed());
  EXPECT_TRUE(generator.yield(0, false).is_ok());
  EXPECT_TRUE(generator.yield(1, false).is_ok());
  EXPECT_TRUE(generator.yield(2, true).is_ok());

  EXPECT_EQ(stream.pop(), Ok(0));
  EXPECT_EQ(stream.pop(), Ok(1));
  EXPECT_EQ(stream.pop(), Ok(2));
  EXPECT_EQ(stream.pop(), Err(StreamError::Closed));
}

/*
TEST(MemBacked, HSJjjs) {
  using namespace stx;

  Rc state = rc::make_inplace<StreamState<int>>(stx::os_allocator).unwrap();

  Rc generator = rc::make_inplace<MemoryBackedGeneratorState<int, 2>>(
                     stx::os_allocator, Generator{std::move(state)})
                     .unwrap();

  EXPECT_TRUE(generator.handle->generator____yield(0, false).is_ok());
  EXPECT_TRUE(generator.handle->generator____yield(0, false).is_ok());
  EXPECT_TRUE(generator.handle->generator____yield(0, false).is_err());
}

TEST(NonMemBacked, Yhd) {
  using namespace stx;

  // TODO(lamarrr): i.e. optional internal state.

  Generator generator{
      rc::make_inplace<StreamState<int>>(os_allocator).unwrap()};
  Stream stream{generator.state.share()};

  for (int i = 0; i < 20; i++) {
    EXPECT_TRUE(
        generator.yield(os_allocator, static_cast<int>(i), false).is_ok());
  }

  for (int i = 0; i < 20; i++) {
    EXPECT_EQ(stream.pop(), Ok(static_cast<int>(i)));
  }

  EXPECT_EQ(stream.pop(), Err(StreamError::Pending));
}
*/
