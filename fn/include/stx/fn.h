#pragma once

#include <cstdlib>
#include <type_traits>
#include <utility>

#include "stx/config.h"
#include "stx/mem.h"

STX_BEGIN_NAMESPACE

template <typename Signature>
struct Fn {};

// NOTE that this is just an handle and doesn't manage any lifetime.
//
//
// it is essentially a trivial struct. it is just contains pointers (resource
// handle).
//
// func must never be initialized with a nullptr or invalid handle.
//
//
// Fn is to function pointers and functors as std::span is to std::vector and
// std::array (a view).
//
// we'd ideally lock this whilst it is being called but the user could
// potentially already have internally thread-safe data structures.
//
//
// for high-perf scenarios, std::function is just terrible, especially if you
// have millions of it being created.
//        - it performs allocation on it's own without being able to provide an
//        allocator. meaning the memory it uses could be disjoint from the data
//        it needs to operate on, which is totally terrible performance-wise as
//        you'd be jumping along the cacheline
//        - Surprisingly, its copy constructor copies the containing data and
//        the function ptr/definition whenever it is copied. WTF? Copy
//        constructors are implicit and it is extremely easy to have accidental
//        copies with them. in fact, the notion of `copy` is implicit and
//        ambigous and should be explicit and disambiguated for non-trivial
//        types
//
template <typename ReturnType, typename... Args>
struct Fn<ReturnType(Args...)> {
  using func_type = ReturnType (*)(memory_handle, Args...);

  explicit constexpr Fn(func_type idispatcher, memory_handle idata_addr)
      : dispatcher{idispatcher}, data_addr{idata_addr} {}

  constexpr ReturnType operator()(Args... args) const {
    return dispatcher(data_addr, std::forward<Args>(args)...);
  }

  func_type dispatcher = nullptr;
  memory_handle data_addr = nullptr;
};

template <typename Signature>
using RcFn = Rc<Fn<Signature>>;

template <typename T>
struct raw_function_decay_impl {
  using type = T;
};

template <typename ReturnType, typename... Args>
struct raw_function_decay_impl<ReturnType(Args...)> {
  using type = ReturnType (*)(Args...);
};

template <typename T>
struct is_function_pointer_impl : public std::false_type {};

template <typename ReturnType, typename... Args>
struct is_function_pointer_impl<ReturnType(Args...)> : public std::true_type {};

template <typename ReturnType, typename... Args>
struct is_function_pointer_impl<ReturnType (*)(Args...)>
    : public std::true_type {};

template <typename T>
using raw_function_decay = typename raw_function_decay_impl<T>::type;

template <typename T>
constexpr bool is_function_pointer = is_function_pointer_impl<T>::value;

template <typename Signature>
struct raw_fn_impl {};

template <typename ReturnType, typename... Args>
struct raw_fn_impl<ReturnType(Args...)> {
  using type = ReturnType (*)(Args...);
};

template <typename ReturnType, typename... Args>
struct raw_fn_impl<ReturnType (*)(Args...)> {
  using type = ReturnType (*)(Args...);
};

template <typename Type>
using RawFn = typename raw_fn_impl<Type>::type;

template <typename RawFunctionType>
struct RawFnTraits {};

template <typename ReturnType, typename... Args>
struct RawFunctionDispatcher {
  static constexpr ReturnType dispatch(memory_handle data_addr, Args... args) {
    using ptr = ReturnType (*)(Args...);

    ptr function_ptr = reinterpret_cast<ptr>(data_addr);

    return function_ptr(std::forward<Args>(args)...);
  }
};

template <typename ReturnType, typename... Args>
struct RawFnTraits<ReturnType(Args...)> {
  using ptr = ReturnType (*)(Args...);
  using signature = ReturnType(Args...);
  using fn = Fn<signature>;
  using dispatcher = RawFunctionDispatcher<ReturnType, Args...>;
  using return_type = ReturnType;
};

template <typename ReturnType, typename... Args>
struct RawFnTraits<ReturnType (*)(Args...)>
    : public RawFnTraits<ReturnType(Args...)> {};

template <typename Type, typename ReturnType, typename... Args>
struct FunctorDispatcher {
  static constexpr ReturnType dispatch(memory_handle data_addr, Args... args) {
    return (*(reinterpret_cast<Type*>(data_addr)))(std::forward<Args>(args)...);
  }
};

template <class MemberFunctionSignature>
struct MemberFnTraits {};

// non-const member functions
template <class Type, typename ReturnType, typename... Args>
struct MemberFnTraits<ReturnType (Type::*)(Args...)> {
  using ptr = ReturnType (*)(Args...);
  using signature = ReturnType(Args...);
  using fn = Fn<signature>;
  using type = Type;
  using dispatcher = FunctorDispatcher<type, ReturnType, Args...>;
  using return_type = ReturnType;
};

// const member functions
template <class Type, typename ReturnType, typename... Args>
struct MemberFnTraits<ReturnType (Type::*)(Args...) const> {
  using ptr = ReturnType (*)(Args...);
  using signature = ReturnType(Args...);
  using fn = Fn<signature>;
  using type = Type const;
  using dispatcher = FunctorDispatcher<type, ReturnType, Args...>;
  using return_type = ReturnType;
};

template <class Type>
struct FunctorFnTraits : public MemberFnTraits<decltype(&Type::operator())> {};

template <typename T, typename Stub = void>
struct is_functor_impl : public std::false_type {};

template <typename T>
struct is_functor_impl<T, decltype(&T::operator(), (void)0)>
    : public std::true_type {};

template <typename T>
constexpr bool is_functor = is_functor_impl<T>::value;

namespace fn {
// make a function view from a functor reference. Functor should outlive the Fn
template <typename Functor>
auto make_functor(Functor& functor) {
  static_assert(is_functor<Functor>);

  using traits = FunctorFnTraits<Functor>;
  using fn = typename traits::fn;
  using dispatcher = typename traits::dispatcher;

  return fn{&dispatcher::dispatch, reinterpret_cast<void*>(&functor)};
}

// make a function view from a raw function pointer.
template <typename RawFunctionType,
          std::enable_if_t<is_function_pointer<RawFunctionType>, int> = 0>
auto make_static(RawFunctionType* function_pointer) {
  using traits = RawFnTraits<RawFunctionType>;
  using fn = typename traits::fn;
  using dispatcher = typename traits::dispatcher;

  return fn{&dispatcher::dispatch, reinterpret_cast<void*>(function_pointer)};
}

// make a function view from a data-less functor (i.e. lambda's without data)
template <typename StaticFunctor,
          std::enable_if_t<is_functor<StaticFunctor>, int> = 0>
auto make_static(StaticFunctor functor) {
  using traits = FunctorFnTraits<StaticFunctor>;
  using ptr = typename traits::ptr;

  static_assert(std::is_convertible_v<StaticFunctor, ptr>);

  ptr function_pointer = static_cast<ptr>(functor);

  return make_static(function_pointer);
}

// TODO(lamarrr): documentation
namespace rc {

template <typename Functor>
Result<Rc<typename FunctorFnTraits<Functor>::fn>, AllocError> make_functor(
    Allocator allocator, Functor&& functor) {
  TRY_OK(fn_rc, stx::rc::make(allocator, std::move(functor)));

  Fn fn = stx::fn::make_functor(*fn_rc.handle);

  return Ok(stx::transmute(fn, std::move(fn_rc)));
}

template <typename Functor>
Result<Rc<typename FunctorFnTraits<Functor>::fn>, AllocError> make_functor(
    Allocator allocator, Functor& functor) = delete;

template <typename RawFunctionType,
          std::enable_if_t<is_function_pointer<RawFunctionType>, int> = 0>
auto make_static(RawFunctionType* function_pointer) {
  using traits = RawFnTraits<RawFunctionType>;
  using fn = typename traits::fn;
  using dispatcher = typename traits::dispatcher;

  Manager manager = static_storage_manager;
  manager.ref();

  return Rc{
      fn{&dispatcher::dispatch, reinterpret_cast<void*>(function_pointer)},
      std::move(manager)};
}

template <typename StaticFunctor,
          std::enable_if_t<is_functor<StaticFunctor>, int> = 0>
auto make_static(StaticFunctor functor) {
  using traits = FunctorFnTraits<StaticFunctor>;
  using ptr = typename traits::ptr;

  static_assert(std::is_convertible_v<StaticFunctor, ptr>,
                "functor is not convertible to function pointer");

  ptr function_pointer = static_cast<ptr>(functor);

  return make_static(function_pointer);
}

}  // namespace rc
}  // namespace fn

STX_END_NAMESPACE

// (1) fn::make(Functor<R(A...)>) -> Fn<R(A...)>; functors
//
// (2) fn::make_static(R(*)(A...)) -> Fn<R(A...)>; function pointers and static
// lambdas (i.e. lambdas without data/functor static functions)
//
// fn::rc::make(Allocator, Functor<R(A...)>) -> Rc<Fn<R(A...)>>; same as (1) but
// takes ownership of its arguments and returns a reference count to them,
// AllocResult
//
// fn::rc::make_static(Allocator, R(*)(A...)) -> Rc<Fn<R(A...)>>; same as (2)
// but takes ownership of its arguments and returns a reference count to them,
// AllocResult
//
