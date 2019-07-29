// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace {

void log(std::string message) { std::cout << "Logging: " + message << "\n"; }

void save(std::string message) { std::cout << "Saving: " + message << "\n"; }

void send(std::string message) { std::cout << "Sending: " + message << "\n"; }

} // namespace

/**
 * ! Command pattern can be expressed by lambdas
 */
void CommandTest() {
  std::vector<std::function<void()>> tasks;
  tasks.push_back([] { log("Hi"); });
  tasks.push_back([] { save("Cheers"); });
  tasks.push_back([] { send("Bye"); });

  for (const auto &task : tasks) {
    task();
  }
}

int main() {
  CommandTest();
  return 0;
}
