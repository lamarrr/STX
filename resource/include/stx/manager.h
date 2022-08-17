#pragma once

#include <utility>

#include "stx/config.h"

STX_BEGIN_NAMESPACE

/// A handle/abstract interface to a polymorphic resource manager.
///
/// the operations are specified atomically so they can be used for reftable
/// implementations that choose to use atomic (multi-threaded) or non-atomic
/// operations (single-threaded) for synchronization.
///
///
/// thread-safety depends on implementation.
///
struct ManagerHandle {
  /// increase the strong ref count of the associated resource.
  /// ensure resource is valid before this is called.
  /// a resource with a refcount of 1 or more must always be valid.
  ///
  virtual void ref() = 0;
  /// decrement the ref count of the associated resource.
  ///
  /// a resource with a refcount of 0 needs not be valid.
  ///
  /// the manager handle is not required to be valid once the resource ref count
  /// becomes 0.
  ///
  virtual void unref() = 0;
};

/// a static storage manager handle represents a no-op. i.e. no operation is
/// required for managing lifetimes of the associated static resource.
///
/// thread-safe
///
struct StaticStorageManagerHandle final : public ManagerHandle {
  virtual void ref() override {}
  virtual void unref() override {}
};

/// this handle type has no effect on the state of the program.
/// we used this to help prevent cases where we have to perform branches to
/// check validity of the manager handle.
///
/// thread-safe
///
struct NoopManagerHandle final : public ManagerHandle {
  virtual void ref() override {}
  virtual void unref() override {}
};

/// once a resource is released, this manager is put in its place
///
/// thread-safe
///
struct ManagerStub final : public ManagerHandle {
  virtual void ref() override {}
  virtual void unref() override {}
};

inline constexpr const StaticStorageManagerHandle static_storage_manager_handle;
inline constexpr const NoopManagerHandle noop_manager_handle;

// inlined to mean it is same across all translation units. marking them as
// static would mean they are different across translation units.
inline constexpr const ManagerStub manager_stub_handle;

/// this is a polymorphic resource manager.
/// the resource can be a part of the manager
/// (intrusive/self-managed) or even be externally located (non-intrusive, or
/// separate control block). management of the resource can be intrusive or
/// non-intrusive which makes it flexible.
///
/// the manager is free to delete itself once the resource ref count reaches
/// 0. the manager can also delegate the destruction of itself
/// and its associated resource. i.e. delegating resource management to a memory
/// pool segment or bulk-allocated memory segment.
///
/// the resource management is decoupled from the resource or control block.
///
/// this enables a couple of use-cases:
///
/// - use in embedded systems (via static storage and static memory pools)
/// - use in single-threaded environments where ref-counting might not be
/// needed.
/// - use in scenarios where the user is certain the resource will always
/// outlive the Rc
/// - usage with custom memory management solutions (i.e. pool/bulk-based
/// solutions)
///
/// this is a scalable abstraction over resource management, ***I think***.
///
/// coincidentally, this should be able to support  reference counting
/// by swapping out the `ManagerType` arguments of the Rc, how that'd be useful,
/// I'm still figuring out.
///
///
/// resource handles can be of any type. not just pointers which is only what
/// `shared_ptr` supports.
///
///
struct Manager {
  explicit constexpr Manager(ManagerHandle& ihandle) : handle{&ihandle} {}

  /// on-copy, the handles must refer to the same manager
  constexpr Manager(Manager const& other) = default;
  constexpr Manager& operator=(Manager const& other) = default;

  /// on-move, the manager must copy and then invalidate the other
  /// manager's handle, the moved-from manager is required to be valid but
  /// unable to affect the associated state of the resource, i.e. (no-op). why
  /// not nullptr? nullptr means we'd have to perform a check everytime we want
  /// to send calls to the manager. but with a no-op we don't have any
  /// branches, though we'd have an extra copy on move (noop manager
  /// handle assignment).
  ///
  constexpr Manager(Manager&& other) : handle{other.handle} {
    /// unarm other and prevent it from affecting the state of any object
    other.handle = const_cast<ManagerStub*>(&manager_stub_handle);
  }

  constexpr Manager& operator=(Manager&& other) {
    ManagerHandle* tmp = other.handle;
    other.handle = handle;
    handle = tmp;
    return *this;
  }

  void ref() const { handle->ref(); }

  void unref() const { handle->unref(); }

  ManagerHandle* handle;
};

inline constexpr const Manager static_storage_manager{
    const_cast<StaticStorageManagerHandle&>(static_storage_manager_handle)};

inline constexpr const Manager noop_manager{
    const_cast<NoopManagerHandle&>(noop_manager_handle)};

// inlined to mean it is same across all translation units. marking them as
// static would mean they are different across translation units.
inline constexpr const Manager manager_stub{
    const_cast<ManagerStub&>(manager_stub_handle)};

STX_END_NAMESPACE
