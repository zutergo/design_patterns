#include <functional>
#include <iostream>

struct U {};

template <class T> class IO {
public:
  IO(std::function<T()> f) : _act(f) {}
  T run() { return _act(); }
  std::function<T()> _act;

  template <class F> auto mbind(F f) -> decltype(f(_act())) {
    auto act = _act;
    return IO<decltype(f(_act()).run())>([act, f]() {
      T x = act();
      return f(x).run();
    });
  }
};

IO<U> putStr(std::string s) {
  return IO<U>([s]() {
    std::cout << s;
    return U();
  });
}

IO<std::string> getLine(U) {
  return IO<std::string>([]() {
    std::string s;
    std::getline(std::cin, s);
    return s;
  });
}

IO<U> test() {
  return putStr("Tell me your name!\n")
      .mbind(getLine)
      .mbind([](std::string str) { return putStr("Hi " + str + "\n"); });
}

int main() {
  IO<U> io = test();
  io.run();
}
