// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <string>

static void publish_text(std::string text,
                         std::function<bool(std::string)> filter,
                         std::function<std::string(std::string)> format) {
  if (filter(text)) {
    std::cout << format(text) << "\n";
  }
}

// TODO: use concepts to define correct return types for UnaryPredicate and
// UnaryOperator
template <typename UnaryPredicate, typename UnaryOperator>
static void publish_text2(std::string text, UnaryPredicate &&filter,
                          UnaryOperator &&format) {
  if (filter(text)) {
    std::cout << format(text) << "\n";
  }
}

int main() {
  publish_text2(
      "ERROR - something bad happened", [](std::string text) { return true; },
      [](std::string text) {
        std::transform(text.begin(), text.end(), text.begin(), ::toupper);
        return text;
      });
  publish_text("DEBUG - I'm here",
               [](std::string text) {
                 if (text.length() < 20) {
                   return true;
                 } else {
                   return false;
                 }
               },
               [](std::string text) {
                 std::transform(text.begin(), text.end(), text.begin(),
                                ::tolower);
                 return text;
               });
}
