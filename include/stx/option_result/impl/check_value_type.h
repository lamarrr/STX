#pragma once

#include "stx/common.h"
#include "stx/config.h"

STX_BEGIN_NAMESPACE

namespace impl {

template <typename T>
struct check_value_type {
  static_assert(movable<T>,
                "Value type `T` for `Option`, `Result`, `Some`, `Ok`, and "
                "`Err` must be movable");
  static_assert(
      !is_reference<T>,
      "Cannot use a reference for value type `T` for `Option`, `Result`, "
      "`Some`, `Ok`, and `Err` ' , To prevent subtleties use type wrappers "
      "like std::reference_wrapper (stx::Ref) or any of the "
      "`stx::ConstRef` or `stx::MutRef` specialized aliases instead");
};

template <typename E>
struct check_err_type {
  static_assert(movable<E>,
                "Value type `E` for `Result`, `Some`, `Ok`, and "
                "`Err` must be movable");
  static_assert(
      !is_reference<E>,
      "Cannot use a reference for value type `T` for `Option`, `Result`, "
      "`Some`, `Ok`, and `Err` ' , To prevent subtleties use type wrappers "
      "like std::reference_wrapper (stx::Ref) or any of the "
      "`stx::ConstRef` or `stx::MutRef` specialized aliases instead");
};

}  // namespace impl

STX_END_NAMESPACE
