
#pragma once
#include <type_traits>
#include <utility>

#include "stx/manager.h"

namespace stx {
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
    std::is_move_constructible_v<T>&& std::is_move_assignable_v<T>;

/// Rc - reference-counted resource
///
/// primarily intended for dynamic lifetime management dispatch
///
/// NOTE: our `RcPtr` does not accept nullptr and can't be
/// a nullptr. if you need a nullable RcPtr, consider
/// `Option<RcPtr>` or use `std::shared_ptr`
///
///
/// undefined behaviour to copy/move from/to a moved-from Rc
///
/// NOTE: Rc is neither a pointer nor a function. It just does one thing:
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
struct Rc {
  static_assert(is_resource_handle_type<HandleType>);

  using handle_type = HandleType;

  constexpr Rc(HandleType&& ihandle, Manager imanager)
      : handle{std::move(ihandle)}, manager{std::move(imanager)} {}

  constexpr Rc(Rc&& other)
      : handle{std::move(other.handle)}, manager{std::move(other.manager)} {
    other.manager = manager_stub;
  }

  constexpr Rc& operator=(Rc&& other) {
    std::swap(handle, other.handle);
    std::swap(manager, other.manager);

    return *this;
  }

  Rc(Rc const& other) = delete;
  Rc& operator=(Rc const& other) = delete;

  Rc share() const {
    manager.ref();

    return Rc{HandleType{handle}, Manager{manager}};
  }

  ~Rc() { manager.unref(); }

  HandleType handle;
  Manager manager;
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
struct Unique {
  static_assert(is_resource_handle_type<HandleType>);

  constexpr Unique(HandleType&& ihandle, Manager imanager)
      : handle{std::move(ihandle)}, manager{std::move(imanager)} {}

  constexpr Unique(Unique&& other)
      : handle{std::move(other.handle)}, manager{std::move(other.manager)} {
    other.manager = manager_stub;
  }

  constexpr Unique& operator=(Unique&& other) {
    std::swap(handle, other.handle);
    std::swap(manager, other.manager);

    return *this;
  }

  Unique(Unique const& other) = delete;
  Unique& operator=(Unique const& other) = delete;

  ~Unique() { manager.unref(); }

  HandleType handle;
  Manager manager;
};

// make_unique will thus not need a ref-count

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
constexpr Rc<Target> transmute(Target target, Rc<Source>&& source) {
  return Rc<Target>{std::move(target), std::move(source.manager)};
}

template <typename Target, typename Source>
constexpr Rc<Target> cast(Rc<Source>&& source) {
  Target target = static_cast<Target>(std::move(source.handle));
  return transmute(std::move(target), std::move(source));
}

template <typename Target, typename Source>
constexpr Unique<Target> transmute(Target target, Unique<Source>&& source) {
  return Unique<Target>{std::move(target), std::move(source.manager)};
}

template <typename Target, typename Source>
constexpr Unique<Target> cast(Unique<Source>&& source) {
  Target target = static_cast<Target>(std::move(source.handle));
  return transmute(std::move(target), std::move(source));
}

}  // namespace stx
