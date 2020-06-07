#include <iostream>
#include <variant>

template <class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> overloaded(Ts...)->overloaded<Ts...>;

struct Connection {
  struct Disconnected {
    Disconnected() {
      std::cout << "Disconnected"
                << "\n";
    }
  };
  struct Connecting {
    Connecting() {
      std::cout << "Connecting"
                << "\n";
    }
  };
  struct Connected {
    Connected() {
      std::cout << "Connected"
                << "\n";
    }
  };

  using State = std::variant<Disconnected, Connecting, Connected>;

  Connection() {
    std::visit(
        overloaded{
            [](auto &&) { return Connected(); },
        },
        m_connection);
  }

  void interrupt() {
    std::visit(
        overloaded{
            [](auto &&) { return Disconnected(); },
        },
        m_connection);
  }

  State m_connection = Connecting();
};

int main(int argc, char *argv[]) {
  Connection con;
  con.interrupt();

  return 0;
}
