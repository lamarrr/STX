
#pragma once
#include <atomic>
#include <cinttypes>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "stx/allocator.h"
#include "stx/config.h"
#include "stx/manager.h"
#include "stx/memory.h"
#include "stx/rc.h"
#include "stx/result.h"
#include "stx/struct.h"
#include "stx/try_ok.h"

STX_BEGIN_NAMESPACE

/// Handles refer to a representation of a resource that needs to be managed.
/// this resource can be memory, C-API resource, etc.
///
///
/// Handle types are just values to be passed and moved around and whose
/// validity is guaranteed by the manager, i.e. the manager determines if a
/// nullptr is a valid memory resource handle and handles it appropriately.
///
template <typename T>
constexpr bool is_resource_handle_type =
    std::is_move_constructible_v<T> && std::is_move_assignable_v<T>;

/// Rc - reference-counted resource
///
/// primarily intended for dynamic lifetime management dispatch
///
/// NOTE: our `Rc<T*>` does not accept nullptr and can't be
/// a nullptr. if you need a nullable, consider`Option<Rc<T*>>` or use
/// `std::shared_ptr`
///
///
/// undefined behaviour to copy/move from/to a moved-from `Rc`
///
/// NOTE: `Rc` is neither a pointer nor a function. It just does one thing:
/// manages lifetime
///
///
/// the only point of vulnerability is a use after std::move.
/// shared_ptr leaves the pointer as a nullptr and the compiler can totally
/// ignore it being dereferenced too. so both use-after-free and nullptr
/// dereference are undefined behaviour.
///
/// probably more useful for debugging since the state of all the Rcs will be
/// captured and you'd get to see which have been released and which haven't
///
///
///
/// Rc sort of guarantees that a use-after-free will only happen due to a
/// use-after-move which is arguably more easily detectable than chasing raw
/// pointers.
///
///
///
template <typename HandleType>
struct Rc
{
  using handle_type = HandleType;

  static_assert(is_resource_handle_type<handle_type>);

  constexpr Rc(handle_type ihandle, Manager imanager) :
      handle{std::move(ihandle)}, manager{std::move(imanager)}
  {}

  constexpr Rc(Rc &&other) :
      handle{std::move(other.handle)}, manager{std::move(other.manager)}
  {
    other.manager = manager_stub;
  }

  constexpr Rc &operator=(Rc &&other)
  {
    std::swap(handle, other.handle);
    std::swap(manager, other.manager);

    return *this;
  }

  Rc(Rc const &other) = delete;

  Rc &operator=(Rc const &other) = delete;

  ~Rc()
  {
    manager.unref();
  }

  Rc share() const
  {
    manager.ref();

    return Rc{handle_type{handle}, Manager{manager}};
  }

  handle_type handle;
  Manager     manager;
};

template <typename Object>
struct Rc<Object *>
{
  using handle_type = Object *;
  using object_type = Object;

  constexpr Rc(handle_type ihandle, Manager imanager) :
      handle{std::move(ihandle)}, manager{std::move(imanager)}
  {}

  constexpr Rc(Rc &&other) :
      handle{std::move(other.handle)}, manager{std::move(other.manager)}
  {
    other.manager = manager_stub;
  }

  constexpr Rc &operator=(Rc &&other)
  {
    std::swap(handle, other.handle);
    std::swap(manager, other.manager);

    return *this;
  }

  Rc(Rc const &other) = delete;

  Rc &operator=(Rc const &other) = delete;

  ~Rc()
  {
    manager.unref();
  }

  Rc share() const
  {
    manager.ref();

    return Rc{handle_type{handle}, Manager{manager}};
  }

  constexpr handle_type operator->() const
  {
    return handle;
  }

  constexpr object_type &operator*() const
  {
    return *handle;
  }

  handle_type handle;
  Manager     manager;
};

// A uniquely owned resource.
//
// can not be shared.
//
// only one instance of the associated resource is meant to be valid/active
// throughout the lifetime of the program.
//
// only ever calls unref once.
//
template <typename HandleType>
struct Unique
{
  using handle_type = HandleType;

  static_assert(is_resource_handle_type<handle_type>);

  constexpr Unique(handle_type ihandle, Manager imanager) :
      handle{std::move(ihandle)}, manager{std::move(imanager)}
  {}

  constexpr Unique(Unique &&other) :
      handle{std::move(other.handle)}, manager{std::move(other.manager)}
  {
    other.manager = manager_stub;
  }

  constexpr Unique &operator=(Unique &&other)
  {
    std::swap(handle, other.handle);
    std::swap(manager, other.manager);

    return *this;
  }

  Unique(Unique const &other)            = delete;
  Unique &operator=(Unique const &other) = delete;

  ~Unique()
  {
    manager.unref();
  }

  handle_type handle;
  Manager     manager;
};

template <typename Object>
struct Unique<Object *>
{
  using handle_type = Object *;
  using object_type = Object;

  static_assert(is_resource_handle_type<handle_type>);

  constexpr Unique(handle_type ihandle, Manager imanager) :
      handle{std::move(ihandle)}, manager{std::move(imanager)}
  {}

  constexpr Unique(Unique &&other) :
      handle{std::move(other.handle)}, manager{std::move(other.manager)}
  {
    other.manager = manager_stub;
  }

  constexpr Unique &operator=(Unique &&other)
  {
    std::swap(handle, other.handle);
    std::swap(manager, other.manager);

    return *this;
  }

  Unique(Unique const &other) = delete;

  Unique &operator=(Unique const &other) = delete;

  ~Unique()
  {
    manager.unref();
  }

  constexpr handle_type operator->() const
  {
    return handle;
  }

  constexpr object_type &operator*() const
  {
    return *handle;
  }

  handle_type handle;
  Manager     manager;
};

/// Transmute a resource that uses a polymorphic manager.
/// transmutation involves pretending that a target resource constructed from
/// another source resource is valid provided the other source resource is
/// valid.
///
/// This is more of an alias or possibly unsafe alias as we can't guarantee its
/// validity.
///
/// i.e. `Rc<string_view, Manager>` can transmute `Rc<std::string *,
/// Manager>`. this means the contained `string_view` is valid as long as
/// the string pointer is valid.
///
///
/// NOTE: transmuting a Rc handle means the manager knows how to handle the
/// resource without using the resource handle. which is the case for resources
/// that use a polymorphic manager but not so for resources with non-polymorphic
/// managers.
/// this functions similar similar to `std::shared_ptr's` aliasing constructors.
///
///
template <typename Target, typename Source>
constexpr Rc<Target> transmute(Target target, Rc<Source> source)
{
  return Rc<Target>{std::move(target), std::move(source.manager)};
}

template <typename Target, typename Source>
constexpr Rc<Target> cast(Rc<Source> source)
{
  Target target = static_cast<Target>(std::move(source.handle));
  return transmute(std::move(target), std::move(source));
}

template <typename Target, typename Source>
constexpr Unique<Target> transmute(Target target, Unique<Source> source)
{
  return Unique<Target>{std::move(target), std::move(source.manager)};
}

template <typename Target, typename Source>
constexpr Unique<Target> cast(Unique<Source> source)
{
  Target target = static_cast<Target>(std::move(source.handle));
  return transmute(std::move(target), std::move(source));
}

/// thread-safe
///
/// RefCnt objects should be created in batches to avoid false
/// sharing issues
///
/// we assume the user is sharing data/instructions and their side effects via a
/// shared object shared across threads, so we perform acquire when performing
/// unref.
///
// if the application is multi-threaded, the compiler can make more informed
// decisions about reference-counting.
struct RefCount
{
  STX_MAKE_PINNED(RefCount)

  std::atomic<uint64_t> ref_count;

  explicit RefCount(uint64_t initial_ref_count) :
      ref_count{initial_ref_count}
  {}

  uint64_t ref()
  {
    return ref_count.fetch_add(1, std::memory_order_relaxed);
  }

  // required to be acquire memory order since it could have been modified and
  // we need to ensure proper instruction ordering
  [[nodiscard]] uint64_t unref()
  {
    return ref_count.fetch_sub(1, std::memory_order_acquire);
  }
};

// if for a single type, we can use a pool allocator depending on whether it is
// trivially destructible or not. and not have to store destructors. but that
// would rarely happen in our use case.
//
//
//
//===========================
// we need to store manager too if non-trivial.
//
//
//
// needs lock to hold pool when allocating memory
//
// non-trivially destructible?
// allocate
// ObjectManager{ Object; RefCount;   void unref() {  delete Object;   } }
// allocate storage for pool
//
// trivially destructible blocks with trivial chunks, needs no destructor
// non-trivially destructible blocks with trivial chunks, needs destructor
//
//
// Memory Chunk - uninitialized memory, needs to be destroyed once the last
// object is destroyed, i.e. refcount reaches zero
//
// Memory Chunk  Rc{refcount, memory}. will need destructor function pointer to
// run once unref is called?
//
// create object = Rc<Memory>::share() => new(memory) {object} => Rc<Object*>
// destructor????
//
//
template <typename Object>
struct DeallocateObject
{
  STX_MAKE_PINNED(DeallocateObject)

  using object_type = Object;

  // placed in union to ensure the object's destructor is not called
  union
  {
    Object object;
  };

  Allocator allocator;

  template <typename... Args>
  explicit DeallocateObject(Allocator iallocator, Args &&...args) :
      object{std::forward<Args>(args)...}, allocator{std::move(iallocator)}
  {}

  constexpr void operator()(void *memory)
  {
    object.~Object();
    allocator.handle->deallocate(memory);
  }

  ~DeallocateObject()
  {}
};

/// thread-safe in ref-count and deallocation only
///
/// an independently managed object/memory.
///
/// can be used for bulk object sharing.
///
template <typename Functor>
struct RcOperation final : public ManagerHandle
{
  static_assert(std::is_invocable_v<Functor, void *>);

  STX_MAKE_PINNED(RcOperation)

  RefCount ref_count;

  // operation to be performed once. i.e.
  // once the ref count reaches zero, synchronized across threads.
  Functor operation;

  template <typename... Args>
  explicit RcOperation(uint64_t initial_ref_count, Args &&...args) :
      ref_count{initial_ref_count}, operation{std::forward<Args>(args)...}
  {}

  virtual void ref() override final
  {
    ref_count.ref();
  }

  virtual void unref() override final
  {
    // NOTE: the last user of the object might have made modifications to the
    // object's address just before unref is called, this means we need to
    // ensure correct ordering of the operations/instructions relative to the
    // unref call (instruction re-ordering).
    //
    // we don't need to ensure correct ordering of instructions after
    // decreasing the ref-count atomically and the object being marked for
    // destruction since the object by contract is not meant to be used
    // afterwards.
    //
    // the destructor's instructions only needs to be ordered relative to the
    // last-owning thread's instructions.
    //
    if (ref_count.unref() == 1)
    {
      operation(reinterpret_cast<void *>(this));
    }
  }
};

template <typename Functor>
struct UniqueRcOperation final : public ManagerHandle
{
  static_assert(std::is_invocable_v<Functor, void *>);

  STX_MAKE_PINNED(UniqueRcOperation)

  // operation to be performed once.
  Functor operation;

  template <typename... Args>
  explicit UniqueRcOperation(Args &&...args) :
      operation{std::forward<Args>(args)...}
  {}

  virtual void ref() override final
  {}

  virtual void unref() override final
  {
    operation(reinterpret_cast<void *>(this));
  }
};

namespace rc
{

template <typename T, typename... Args>
Result<Rc<T *>, AllocError> make_inplace(Allocator allocator, Args &&...args)
{
  TRY_OK(memory,
         mem::allocate(allocator, sizeof(RcOperation<DeallocateObject<T>>)));

  void *mem = memory.handle;

  // release ownership of memory
  memory.allocator = allocator_stub;

  using destroy_operation_type = RcOperation<DeallocateObject<T>>;

  destroy_operation_type *destroy_operation_handle =
      new (mem) RcOperation<DeallocateObject<T>>{0, std::move(allocator),
                                                 std::forward<Args>(args)...};

  // this polymorphic manager manages itself.
  // unref can be called on a polymorphic manager with a different pointer since
  // it doesn't need the handle, it can delete itself independently
  Manager manager{*destroy_operation_handle};

  // we perform a reference count increment for debugging/runtime hooking
  // purpose. I know we can start with a 1 ref-count but no.
  manager.ref();

  Rc<destroy_operation_type *> destroy_operation_rc{
      static_cast<destroy_operation_type *>(destroy_operation_handle), manager};

  return Ok(transmute(&destroy_operation_handle->operation.object,
                      std::move(destroy_operation_rc)));
}

template <typename T>
auto make(Allocator allocator, T &&value)
{
  return make_inplace<T>(allocator, std::forward<T>(value));
}

/// adopt an object memory handle that is guaranteed to be valid for the
/// lifetime of this mem::Rc struct and any mem::Rc structs constructed or
/// assigned from it. typically used for static storage lifetimes.
///
/// it is advised that this should not be used for scope-local storage as it
/// would be difficult to guarantee that a called function does not retain a
/// copy or move an mem::Rc constructed using this method. However, static
/// storage objects live for the whole duration of the program so this is safe.
///
template <typename T>
Rc<T *> make_static(T &object)
{
  Manager manager = static_storage_manager;
  manager.ref();
  return Rc<T *>{&object, std::move(manager)};
}

template <typename T, typename... Args>
Result<Unique<T *>, AllocError> make_unique_inplace(Allocator allocator,
                                                    Args &&...args)
{
  TRY_OK(memory, mem::allocate(allocator,
                               sizeof(UniqueRcOperation<DeallocateObject<T>>)));

  void *mem = memory.handle;

  memory.allocator = allocator_stub;

  using destroy_operation_type = UniqueRcOperation<DeallocateObject<T>>;

  destroy_operation_type *destroy_operation_handle =
      new (mem) UniqueRcOperation<DeallocateObject<T>>{
          allocator, std::forward<Args>(args)...};

  Manager manager{*destroy_operation_handle};

  manager.ref();

  Unique<destroy_operation_type *> destroy_operation_rc{
      static_cast<destroy_operation_type *>(destroy_operation_handle), manager};

  return Ok(transmute(&destroy_operation_handle->operation.object,
                      std::move(destroy_operation_rc)));
}

template <typename T>
auto make_unique(Allocator allocator, T &&value)
{
  return make_unique_inplace<T>(allocator, std::forward<T>(value));
}

template <typename T>
Unique<T *> make_unique_static(T &object)
{
  Manager manager = static_storage_manager;
  manager.ref();
  return Unique<T *>{&object, std::move(manager)};
}

}        // namespace rc

STX_END_NAMESPACE
