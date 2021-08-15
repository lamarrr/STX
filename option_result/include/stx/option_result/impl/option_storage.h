#pragma once

#include <type_traits>
#include <utility>

#include "stx/config.h"
#include "stx/deferred_init.h"
#include "stx/none.h"
#include "stx/some.h"

STX_BEGIN_NAMESPACE

template <typename T, bool trivial = false>
struct OptionStorage {
  union {
    Some<T> some_;
  };
  static constexpr bool is_trivial = false;

  bool is_none_;

  explicit constexpr OptionStorage(deferred_init_tag) {}

  explicit constexpr OptionStorage(NoneType) : is_none_{true} {}

  explicit constexpr OptionStorage(Some<T>&& some)
      : some_{std::move(some)}, is_none_{false} {}

  void finally_init(Some<T>&& some) {
    new (&some_) Some{std::move(some)};
    is_none_ = false;
  }

  void finally_init(NoneType) { is_none_ = true; }

  void assign(NoneType) {
    if (!is_none_) {
      some_.ref().~T();
      is_none_ = true;
    } else {
      is_none_ = true;
    }
  }

  void assign(Some<T>&& some) {
    if (is_none_) {
      finally_init(std::move(some));
      is_none_ = false;
    } else {
      some_ = std::move(some);
      is_none_ = false;
    }
  }

  ~OptionStorage() {
    if (!is_none_) {
      some_.ref().~T();
    }
  }
};

template <typename T>
struct OptionStorage<T, true> {
  union {
    Some<T> some_;
  };

  bool is_none_;

  static constexpr bool is_trivial = true;

  explicit constexpr OptionStorage(deferred_init_tag) {}

  explicit constexpr OptionStorage(NoneType) : is_none_{true} {}

  explicit constexpr OptionStorage(Some<T>&& some)
      : some_{std::move(some)}, is_none_{false} {}

  constexpr void finally_init(Some<T>&& some) {
    some_ = std::move(some);
    is_none_ = false;
  }

  constexpr void finally_init(NoneType) { is_none_ = true; }

  constexpr void assign(NoneType) { finally_init(None); }

  constexpr void assign(Some<T>&& some) { finally_init(std::move(some)); }
};

STX_END_NAMESPACE
