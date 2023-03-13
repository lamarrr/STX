#pragma once

#include <type_traits>
#include <utility>

#include "stx/config.h"
#include "stx/err.h"
#include "stx/ok.h"

STX_BEGIN_NAMESPACE

template <typename T, typename E, bool trivial = false>
struct ResultStorage
{
  union
  {
    Ok<T>  ok_;
    Err<E> err_;
  };

  bool is_ok_;

  constexpr ResultStorage(ResultStorage &&other)
  {
    if (other.is_ok_)
    {
      finally_init(std::move(other.ok_));
    }
    else
    {
      finally_init(std::move(other.err_));
    }
  }

  constexpr ResultStorage &operator=(ResultStorage &&other)
  {
    if (other.is_ok_)
    {
      assign(std::move(other.ok_));
    }
    else
    {
      assign(std::move(other.err_));
    }

    return *this;
  }

  explicit constexpr ResultStorage(Ok<T> &&ok) :
      ok_{std::move(ok)}, is_ok_{true}
  {}

  explicit constexpr ResultStorage(Err<E> &&err) :
      err_{std::move(err)}, is_ok_{false}
  {}

  constexpr ResultStorage(ResultStorage const &)            = delete;
  constexpr ResultStorage &operator=(ResultStorage const &) = delete;

  constexpr ResultStorage() = delete;

  ~ResultStorage()
  {
    if (is_ok_)
    {
      ok_.ref().~T();
    }
    else
    {
      err_.ref().~E();
    }
  }

  void finally_init(Ok<T> &&ok)
  {
    new (&ok_) Ok{std::move(ok)};
    is_ok_ = true;
  }

  void finally_init(Err<E> &&err)
  {
    new (&err_) Err{std::move(err)};
    is_ok_ = false;
  }

  void assign(Ok<T> &&ok)
  {
    if (!is_ok_)
    {
      err_.ref().~E();
      finally_init(std::move(ok));
      is_ok_ = true;
    }
    else
    {
      ok_    = std::move(ok);
      is_ok_ = true;
    }
  }

  void assign(Err<E> &&err)
  {
    if (is_ok_)
    {
      ok_.ref().~T();
      finally_init(std::move(err));
      is_ok_ = false;
    }
    else
    {
      err_   = std::move(err);
      is_ok_ = false;
    }
  }
};

template <typename T, typename E>
struct ResultStorage<T, E, true>
{
  union
  {
    Ok<T>  ok_;
    Err<E> err_;
  };

  bool is_ok_;

  constexpr ResultStorage(ResultStorage &&)            = default;
  constexpr ResultStorage &operator=(ResultStorage &&) = default;

  explicit constexpr ResultStorage(Ok<T> &&ok) :
      ok_{std::move(ok)}, is_ok_{true}
  {}

  explicit constexpr ResultStorage(Err<E> &&err) :
      err_{std::move(err)}, is_ok_{false}
  {}

  constexpr ResultStorage(ResultStorage const &)            = delete;
  constexpr ResultStorage &operator=(ResultStorage const &) = delete;

  constexpr ResultStorage() = delete;

  constexpr void finally_init(Ok<T> &&ok)
  {
    ok_    = std::move(ok);
    is_ok_ = true;
  }

  constexpr void finally_init(Err<E> &&err)
  {
    err_   = std::move(err);
    is_ok_ = false;
  }

  constexpr void assign(Ok<T> &&ok)
  {
    finally_init(std::move(ok));
  }

  constexpr void assign(Err<T> &&err)
  {
    finally_init(std::move(err));
  }
};

STX_END_NAMESPACE
