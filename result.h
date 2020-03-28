
#include <charconv>
#include <cinttypes>
#include <cstddef>
#include <iostream>
#include <new>
#include <type_traits>
#include <utility>

template <typename T, typename C>
concept same_as = std::is_same_v<T, C>;

template <typename T, typename C>
concept convertible_to = std::is_convertible_v<T, C>;

template <typename Fn, typename Arg>
concept Invocable = std::is_invocable_v<Fn, Arg>;

template <typename HandlerType, typename Result, typename Error>
concept Handler = requires(HandlerType handler, Error&& error,
                           Result&& result) {
  {handler.no_error(result)};
  {handler.no_value(error)};
};

template <typename T>
struct ValueWrapper {
  T value;
  explicit ValueWrapper(T v) : value{v} {};

  ValueWrapper(ValueWrapper const&) = default;
  ValueWrapper(ValueWrapper&&) = default;
  ValueWrapper& operator=(ValueWrapper const&) = default;
  ValueWrapper& operator=(ValueWrapper&&) = default;
  ~ValueWrapper() = default;
};

template <typename T>
struct OkType : public ValueWrapper<T> {
  using base = ValueWrapper<T>;
  using base::base;
};
template <typename T>
struct ErrType : public ValueWrapper<T> {
  using base = ValueWrapper<T>;
  using base::base;
};

template <typename T>
inline auto Ok(T&& value) {
  return OkType<T>(std::forward<T&&>(value));
}

template <typename T>
inline auto Err(T&& error) {
  return ErrType<T>(std::forward<T&&>(error));
}

template <typename T, typename E, Handler<T, E> HandlerType>
class Result : private HandlerType {
 public:
  using error_type = E;
  using value_type = T;
  using handler_type = HandlerType;

  bool is_ok() const { return is_ok_; }
  bool is_err() const { return !is_ok(); }

  Result() = delete;

  Result(OkType<value_type>&& result) : handler_type{} {
    value_() = std::move(result.value);
    is_ok_ = true;
  }
  Result(ErrType<error_type>&& err) : handler_type{} {
    err_() = std::move(err.value);
    is_ok_ = false;
  }

  auto unwrap() -> T {
    if (is_ok()) {
      is_ok_ = false;
      return std::move(value_());
    } else {
      handler_type::no_error(std::move(err_()));
      // expected to terminate or throw exception here
      return value_();
    }
  };

  template <Invocable<T> Fn>
  auto map(Fn op) -> Result<std::invoke_result_t<Fn, T>, E, handler_type> {
    if (is_ok()) {
      is_ok_ = false;
      return Ok(op(std::move(value_())));
    } else {
      return Err(std::move(err_()));
    }
  }

  template <typename CmpResult>
  auto logical_and(CmpResult&& res) -> CmpResult {
    static_assert(same_as<typename CmpResult::error_type, E>);

    if (is_ok()) {
      is_ok_ = false;
      return res;
    } else {
      return Err(std::move(err_()));
    }
  }

  template <Invocable<T> Fn>
  auto and_then(Fn&& op)
      -> Result<std::invoke_result_t<Fn, T>, E, handler_type> {
    if (is_ok()) {
      is_ok_ = false;
      return Ok(op(value_()));
    } else {
      return Err(std::move(err_()));
    }
  }

  template <Invocable<T> Fn, typename TargetType>
  auto map_or(Fn op, TargetType&& alt) -> TargetType {
    static_assert(std::is_same_v<std::invoke_result_t<Fn, T>, TargetType>);

    if (is_ok()) {
      is_ok_ = false;
      return Ok(op(std::move(value_())));
    } else {
      return Ok(alt);
    }
  }

  template <Invocable<T> ValueFn, Invocable<E> ErrFn>
  auto map_or_else(ValueFn value_op, ErrFn err_op)
      -> Result<std::invoke_result_t<ValueFn, T>,
                std::invoke_result_t<ErrFn, E>, handler_type> {
    if (is_ok()) {
      is_ok_ = false;
      return Ok(value_op(std::move(value_())));
    } else {
      return Err(err_op(std::move(err_())));
    }
  }

  auto unwrap_or(value_type&& alt) {
    if (is_ok()) {
      is_ok_ = false;
      return std::move(value_());
    } else {
      return alt;
    }
  }
  auto unwrap_err() -> E {
    if (!is_ok()) {
      // to be moved or not to be moved
      return std::move(err_());
    } else {
      handler_type::no_err(value_());
      return err_();
    }
  };
  auto map_err();

 private:
  bool is_ok_ = false;
  alignas(error_type) std::byte error_buffer_[sizeof(error_type)];
  alignas(value_type) std::byte value_buffer_[sizeof(value_type)];

  error_type& err_() {
    return *std::launder(reinterpret_cast<error_type*>(error_buffer_));
  }
  value_type& value_() {
    return *std::launder(reinterpret_cast<value_type*>(value_buffer_));
  }
};

template <typename T>
concept Subscriptable = requires(T t) {
  { t.slice(0, 0) }
  ->same_as<void>;
  { t[0] }
  ->same_as<typename T::element_type>;
};

class Fr {
  using element_type = int;

 public:
  void slice(int, int) {}
  int operator[](int) const { return 0; }
};

// Type your code here, or load an example.
int square(Subscriptable auto const& num) { return num[0] * num[0]; }
struct ThrowingHandler {
  void no_error(int num) {
    throw std::out_of_range{"Handled num: " + std::to_string(num)};
    std::abort();
  }

  void no_value(int num) {
    throw std::out_of_range{"Handled num: " + std::to_string(num)};
    std::abort();
  }
};

struct AbortingHandler {
  void no_error(int num) {
    std::cout << "Handled num: " + std::to_string(num) << "\nAborting..."
              << std::endl;
    std::abort();
  }

  void no_value(int num) {
    std::cout << "Handled num: " + std::to_string(num) << "\nAborting..."
              << std::endl;
    std::abort();
  }
};

Result<int, int, AbortingHandler> calculate(int x, int y) {
  if (x == y) return Err(2);
  return Ok(10);
}

volatile int x = 0;
volatile int y = 1;
int main() {
  Fr fr;
  square(fr);
  auto i = calculate(x, y).map([](int y) { return y * 8; });
  std::cout << "(main): " << i.unwrap() << std::endl;
}
