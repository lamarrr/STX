#pragma once

#include <cinttypes>
#include <type_traits>
#include <utility>
#include <variant>

#include "stx/async.h"
#include "stx/config.h"
#include "stx/fn.h"
#include "stx/void.h"

STX_BEGIN_NAMESPACE

template <typename T, typename... Ts>
struct filter_duplicates {
  using type = T;
};

template <template <typename...> class C, typename... Ts, typename U,
          typename... Us>
struct filter_duplicates<C<Ts...>, U, Us...>
    : std::conditional_t<(std::is_same_v<U, Ts> || ...),
                         filter_duplicates<C<Ts...>, Us...>,
                         filter_duplicates<C<Ts..., U>, Us...>> {};

template <typename T>
struct unique_variant_impl;

template <typename... Ts>
struct unique_variant_impl<std::variant<Ts...>>
    : filter_duplicates<std::variant<>, Ts...> {};

template <typename T>
using unique_variant = typename unique_variant_impl<T>::type;

template <typename Type, typename... PlaceHolders>
struct append_type_impl {};

template <typename Type, template <typename...> class Variant, typename... Args>
struct append_type_impl<Type, Variant<Args...>> {
  using type = Variant<Type, Args...>;
};

template <typename Type, typename Variant>
using append_type = typename append_type_impl<Type, Variant>::type;

template <typename Arg, typename Fn>
struct check_chain_phase_valid {
  static_assert(!std::is_void_v<Arg>,
                "void chain arguments and results are not supported, consider "
                "using stx::Void");

  static_assert(std::is_invocable_v<Fn &, Arg &&>,
                "the invocables must be chainable i.e. with Chain{f1, f2, f3}, "
                "expression f3( f2( f1( arg ) ) ) must be valid");

  static_assert(!std::is_void_v<std::invoke_result_t<Fn &, Arg &&>>,
                "void chain arguments and results are not supported, consider "
                "using a regular void (stx::Void)");
};

template <typename Arg, typename Fn, typename... OtherFns>
struct check_chain_valid
    : check_chain_phase_valid<Arg, Fn>,
      check_chain_valid<std::invoke_result_t<Fn &, Arg &&>, OtherFns...> {};

template <typename Arg, typename Fn>
struct check_chain_valid<Arg, Fn> : check_chain_phase_valid<Arg, Fn> {};

template <typename Arg, typename Fn, typename... OtherFns>
struct chain_stack_variant_impl {
  using variant = append_type<
      Arg, typename chain_stack_variant_impl<std::invoke_result_t<Fn &, Arg &&>,
                                             OtherFns...>::variant>;
};

template <typename Arg, typename Fn>
struct chain_stack_variant_impl<Arg, Fn> {
  using variant = std::variant<Arg, std::invoke_result_t<Fn &, Arg &&>>;
};

template <typename Arg, typename Fn, typename... OtherFns>
using chain_stack_variant = unique_variant<
    typename chain_stack_variant_impl<Arg, Fn, OtherFns...>::variant>;

struct ChainState {
  // only valid if the function finishes and next_phase_index !=
  // num_phases
  ServiceToken service_token{};
  // once next_phase_index == num_chains then the chain has reached completion,
  // otherwise, it is assumed to be in a suspended state
  uint8_t next_phase_index = 0;
};

template <uint8_t PhaseIndex, typename Arg, typename Fn, typename... OtherFns>
struct ChainPhase {
  using arg_type = Arg;
  using function_type = raw_function_decay<Fn>;
  using result_type = std::invoke_result_t<Fn &, Arg &&>;
  using next_phase_type = ChainPhase<PhaseIndex + 1, result_type, OtherFns...>;

  using last_phase_result_type =
      typename next_phase_type::last_phase_result_type;

  explicit constexpr ChainPhase(Fn &&ifn, OtherFns &&...iothers)
      : fn{std::move(ifn)}, next_phase{std::move(iothers)...} {}

  template <typename Variant>
  void resume(Variant &stack, ChainState &state, RequestProxy &proxy) {
    // we need to resume at a precise specified phase.
    //
    //
    // is this phase the intended resumption point? then start execution from
    // here and proceed to the next phase, otherwise skip this phase and pass on
    // to the desired phase
    if (PhaseIndex == state.next_phase_index) {
      arg_type arg = std::move(std::get<arg_type>(stack));
      stack = fn(std::forward<arg_type>(arg));

      state.next_phase_index++;

      RequestedCancelState const cancel_request = proxy.fetch_cancel_request();
      RequestedSuspendState const suspend_request =
          proxy.fetch_suspend_request();

      if (cancel_request == RequestedCancelState::Canceled) {
        state.service_token = ServiceToken{cancel_request};
        return;
      }

      if (suspend_request == RequestedSuspendState::Suspended) {
        state.service_token = ServiceToken{suspend_request};
        return;
      }

      next_phase.resume(stack, state, proxy);
    } else {
      next_phase.resume(stack, state, proxy);
    }
  }

  function_type fn;
  next_phase_type next_phase;
};

template <uint8_t PhaseIndex, typename Arg, typename Fn>
struct ChainPhase<PhaseIndex, Arg, Fn> {
  using arg_type = Arg;
  using function_type = raw_function_decay<Fn>;
  using result_type = std::invoke_result_t<function_type, arg_type &&>;
  using last_phase_result_type = result_type;

  explicit constexpr ChainPhase(function_type &&ifn) : fn{std::move(ifn)} {}

  template <typename Variant>
  void resume(Variant &stack, ChainState &state, RequestProxy &) {
    if (PhaseIndex == state.next_phase_index) {
      arg_type arg = std::move(std::get<arg_type>(stack));
      stack = fn(std::forward<arg_type>(arg));

      state.next_phase_index++;
      return;
    }
  }

  function_type fn;
};

template <typename Fn, typename... OtherFns>
struct Chain : check_chain_valid<Void, Fn, OtherFns...> {
  static constexpr uint8_t num_phases = (1 + sizeof...(OtherFns));

  static_assert(num_phases <= (stx::u8_max - 2),
                "maximum depth of chain is 253");

  using phases_type = ChainPhase<0, stx::Void, Fn, OtherFns...>;
  using stack_type = chain_stack_variant<stx::Void, Fn, OtherFns...>;
  using last_phase_result_type = typename phases_type::last_phase_result_type;

  explicit constexpr Chain(Fn &&fn, OtherFns &&...others)
      : phases{std::move(fn), std::move(others)...} {}

  Chain(Chain const &) = delete;
  Chain &operator=(Chain const &) = delete;
  Chain(Chain &&) = default;
  Chain &operator=(Chain &&) = default;

  template <typename Variant>
  void resume(Variant &stack, ChainState &state, RequestProxy &proxy) {
    phases.resume(stack, state, proxy);
  }

  phases_type phases;
};

STX_END_NAMESPACE
