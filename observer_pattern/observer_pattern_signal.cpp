// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "geometry.h"
#include <any>
#include <boost/signals2.hpp>
#include <iostream>
#include <variant>

using namespace boost;
using namespace signals2;

namespace {

template <typename R, typename Args...> signal<R(Args..)> geo_data_signal;

auto path_renderer = [](auto geo_data) {
  std::cout << "Path renderer: "
            << "\n";
};

auto field_renderer = [](ring field) {
  std::cout << "Field renderer: "
            << "\n";
};

void variant_detail_renderer(std::variant<point, line, ring> detail) {
  std::cout << "Variant Detail renderer: "
            << "\n";
}

void any_detail_renderer(std::any detail) {
  std::cout << "Any Detail renderer: "
            << "\n";
}

} // namespace

int main(int argc, char *argv[]) {
  point a{0.0, 0.0};
  point b{0.0, 0.5};
  point c{5.0, 5.0};
  point d{5.00, 0};
  line path{a, b};
  ring field{a, b, c, d, a};

  std::variant<point, line, ring> variant_geo_data = field;
  std::any any_geo_data = path;

  //  render.connect(path_renderer);
  //  render.connect(field_renderer);
  geo_data_signal<void(std::any)>.connect(any_detail_renderer);
  geo_data_signal<void(std::any)>(any_geo_data);

  // provider.render.disconnect(field_renderer);
  // provider.render();

  return 0;
}
