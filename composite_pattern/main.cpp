#include <cassert>
#include <functional>
#include <utility>

/**
 * @brief Makes a function object from a value of type T.
 * @param id Some value of type T.
 */
template <class T> auto identity(T id) {
  return [id]() { return id; };
};

/**
 * @brief Creates a function from a partially applied function.
 * @param f Function to be partially applied.
 * @param args Arguments of the partial application.
 */
template <typename F, typename... Args> auto partialapply(F f, Args... args) {
  return [=](auto... rargs) { return f(args..., rargs...); };
};

/**
 * @brief Compose functions to the form first(second(arg1,...,argn)).
 *
 * @param first First function to be composed.
 * @param second Second function to be composed.
 */
template <typename First, typename Second>
auto compose(First first, Second second) {
  return [=](auto &&... rargs) { return first(second(rargs...)); };
}

/**
 * @brief Compose functions to the form first(second(rest(...))).
 *
 * @param first First function to be composed.
 * @param second Second function to be composed.
 * @param rest All other functions to be composed.
 */
template <typename First, typename Second, typename... Rest>
auto compose(First first, Second second, Rest... rest) {
  return compose(std::move(first),
                 compose(std::move(second), std::move(rest)...));
}

static int increment(int value) { return value + 1; }
static int powerOf2(int value) { return value * value; }
static int multiply(int value1, int value2) { return value1 * value2; }

int main(int argc, char *argv[]) {
  auto incrementPowOf2Mult =
      compose([](int value) { return value + 1; }, powerOf2, multiply);
  assert(10 == incrementPowOf2Mult(3, 1));
  auto incrementPowOf2Mult2 = compose(increment, compose(powerOf2, multiply));
  assert(10 == incrementPowOf2Mult(3, 1));
  auto incrementPowOf2 = compose(increment, powerOf2);
  auto powOf2Increment = compose(powerOf2, increment);
  auto multIncrement = compose(
      increment, identity(multiply(incrementPowOf2(2), powOf2Increment(2))));
  assert(46 == multIncrement());

  return 0;
}
