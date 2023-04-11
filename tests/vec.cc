

#include "stx/vec.h"

#include "gtest/gtest.h"

using stx::Vec;

struct Life
{
  Life()
  {
    inc();
  }

  Life(Life const &)
  {
    inc();
  }

  Life &operator=(Life const &)
  {
    clobber();
    return *this;
  }

  Life(Life &&)
  {
    inc();
  }

  Life &operator=(Life &&)
  {
    clobber();
    return *this;
  }

  ~Life()
  {
    dec();
  }

  static int64_t add(int64_t inc)
  {
    static int64_t count = 0;
    count += inc;
    return count;
  }

  static void inc()
  {
    EXPECT_NE(add(1), 0);
  }

  static void dec()
  {
    EXPECT_GE(add(-1), 0);
  }

  static void clobber()
  {
    int f                  = 9;
    *(volatile int *) (&f) = 82;
  }
};

#define EXPECT_VALID_VEC(vec)        \
  EXPECT_GE(vec.end(), vec.begin()); \
  EXPECT_GE(vec.capacity(), vec.size())

TEST(VecTest, MoveAssignmentAndConstruction)
{
  Vec<int> a{stx::os_allocator};
  a.push(0).unwrap();
  a.push(1).unwrap();
  a.push(2).unwrap();

  EXPECT_EQ(a.size(), 3);

  Vec<int> b = std::move(a);

  EXPECT_EQ(b.size(), 3);
  EXPECT_EQ(a.size(), 0);

  a = std::move(b);

  EXPECT_EQ(a.size(), 3);
  EXPECT_EQ(b.size(), 0);

  EXPECT_EQ(a[0], 0);
  EXPECT_EQ(a[1], 1);
  EXPECT_EQ(a[2], 2);
}

TEST(VecTest, Destructor)
{
  {
    Vec<int> vec{stx::Memory{stx::os_allocator, nullptr}, 0, 0};

    for (size_t i = 0; i < 10000; i++)
      vec.push_inplace(8).unwrap();

    EXPECT_EQ(vec.size(), 10000);
    EXPECT_VALID_VEC(vec);
  }

  {
    Vec<int> vec{stx::os_allocator};

    EXPECT_VALID_VEC(vec);
  }
}

TEST(VecTest, Resize)
{
  {
    Vec<int> vec{stx::os_allocator};

    vec.resize(10, 69).unwrap();

    EXPECT_VALID_VEC(vec);

    for (auto &el : vec.span())
    {
      EXPECT_EQ(el, 69);
    }

    vec.resize(20, 42).unwrap();

    EXPECT_VALID_VEC(vec);

    EXPECT_EQ(vec.size(), 20);

    for (auto &el : vec.span().slice(0, 10))
    {
      EXPECT_EQ(el, 69);
    }

    for (auto &el : vec.span().slice(10, 10))
    {
      EXPECT_EQ(el, 42);
    }
  }
}

TEST(VecTest, ResizeLifetime)
{
  {
    Vec<Life> vec{stx::os_allocator};
    vec.resize(1).unwrap();
    vec.resize(5).unwrap();

    EXPECT_VALID_VEC(vec);
  }
}

TEST(VecTest, Noop)
{
  stx::Vec<int> vec{stx::os_allocator};

  vec.push(3).unwrap();

  vec.push_inplace(3).unwrap();
  vec.reserve(444).unwrap();
  vec.span();
  vec.span().at(1).unwrap().get() = 0;

  vec.clear();

  EXPECT_TRUE(vec.is_empty());

  stx::FixedVec<int> g{stx::os_allocator};

  EXPECT_DEATH_IF_SUPPORTED(g.push_inplace(4783).expect("unable to push"),
                            ".*");

  stx::Vec<int> no_vec{stx::noop_allocator};

  EXPECT_DEATH_IF_SUPPORTED(no_vec.push_inplace(4783).expect("unable to push"),
                            ".*");
}

TEST(VecTest, Copy)
{
  {
    stx::Vec<int> vec{stx::os_allocator};

    vec.push(3).unwrap();
    vec.push(4).unwrap();

    auto x = vec.copy(stx::os_allocator).unwrap();

    EXPECT_EQ(x.size(), 2);
    EXPECT_EQ(x.size(), vec.size());
    EXPECT_EQ(x[0], vec[0]);
    EXPECT_EQ(x[1], vec[1]);
  }

  {
    stx::FixedVec<int> vec =
        stx::vec::make_fixed<int>(stx::os_allocator, 20).unwrap();

    vec.push(3).unwrap();
    vec.push(4).unwrap();

    auto x = vec.copy(stx::os_allocator).unwrap();

    EXPECT_EQ(x.size(), 2);
    EXPECT_EQ(x.size(), vec.size());
    EXPECT_EQ(x[0], vec[0]);
    EXPECT_EQ(x[1], vec[1]);
  }
}

TEST(VecTest, Extend)
{
  stx::Vec<int> a{stx::os_allocator};
  a.extend({}).unwrap();
  a.extend_move({}).unwrap();

  stx::FixedVec<int> b{stx::os_allocator};
  b.extend({}).unwrap();
  b.extend_move({}).unwrap();
}

TEST(VecTest, Pop)
{
  {
    int           data[] = {1, 2, 3, 4, 5};
    stx::Vec<int> a{stx::os_allocator};
    a.extend(data).unwrap();

    EXPECT_EQ(a.pop(), stx::Some(5));
    EXPECT_EQ(a.pop(), stx::Some(4));
    EXPECT_EQ(a.pop(), stx::Some(3));
    EXPECT_EQ(a.pop(), stx::Some(2));
    EXPECT_EQ(a.pop(), stx::Some(1));
    EXPECT_EQ(a.pop(), stx::None);

    stx::Vec<int> b{stx::os_allocator};

    EXPECT_EQ(b.pop(), stx::None);
  }

  {
    int                data[] = {1, 2, 3, 4, 5};
    stx::FixedVec<int> a =
        stx::vec::make_fixed<int>(stx::os_allocator, 5).unwrap();
    a.extend(data).unwrap();

    EXPECT_EQ(a.pop(), stx::Some(5));
    EXPECT_EQ(a.pop(), stx::Some(4));
    EXPECT_EQ(a.pop(), stx::Some(3));
    EXPECT_EQ(a.pop(), stx::Some(2));
    EXPECT_EQ(a.pop(), stx::Some(1));
    EXPECT_EQ(a.pop(), stx::None);

    stx::FixedVec<int> b{stx::os_allocator};

    EXPECT_EQ(b.pop(), stx::None);
  }
}

TEST(VecTest, UnsafeResizeUninitialized)
{
  {
    Vec<int> vec{stx::os_allocator};

    // Initialize the vector with some values
    vec.resize(3, 0).unwrap();

    auto uninitialized_span = vec.unsafe_resize_uninitialized(5).unwrap();
    EXPECT_VALID_VEC(vec);

  EXPECT_EQ(uninitialized_span.size(), 2);

    // Check that the uninitialized memory is not null
    EXPECT_NE(uninitialized_span.data(), nullptr);    
  
    // Initialize the uninitialized memory
    for (auto &el : uninitialized_span)
      {
        el = 0;
      }
    // Verify that all values are correct
    for (auto &el : vec.span())
      {
       EXPECT_EQ(el, 0);
      }
  }
}

TEST(VecTest, UnsafeResizeUninitializedLifetime)
{
  {
Vec<Life> vec{stx::os_allocator};

auto uninitialized_span = vec.unsafe_resize_uninitialized(5).unwrap();

for (auto& el : uninitialized_span) {
  new (&el) Life{}; // Manually initialize 
}

EXPECT_VALID_VEC(vec);

  }
}