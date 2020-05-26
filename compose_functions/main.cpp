#include <cassert>

template <typename F, typename G> auto compose(F f, G g) {
  return [=](auto value) { return f(g(value)); };
}

static int increment(const int value) { return value + 1; }
static int powerOf2(const int value) { return value * value; }

int main(int argc, char *argv[]) {
  auto incrementWithPowOf2 = compose(increment, powerOf2);
  assert(10 == incrementWithPowOf2(3));
  auto incrementIncrementWithPowOf2 = compose(increment, incrementWithPowOf2);
  assert(11 == incrementIncrementWithPowOf2(3));
  return 0;
}
