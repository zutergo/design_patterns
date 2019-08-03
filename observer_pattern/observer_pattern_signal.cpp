// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "geometry.h"
#include "observers.h"
#include "signal.h"
#include <any>
#include <variant>

template <typename... Args> using geo_data_signal = signal<Args...>;

int main(int argc, char *argv[]) {
  point a{0.0, 0.0};
  point b{0.0, 5.0};
  point c{5.0, 5.0};
  point d{5.0, 0.0};
  line path{a, b};
  ring field{a, b, c, d, a};

  std::variant<point, line, ring> variant_geo_data = field;
  std::any any_geo_data = path;

  signal<line> line_signal;
  signal<ring> ring_signal;

  line_signal.connect(path_renderer);
  auto ring_connection = ring_signal.connect(path_renderer);
  auto field_connection = ring_signal.connect(field_renderer);
  line_signal(path);

  for (int i = 0; i < 100000000; ++i) {
    ring_signal(field);
  }

  ring_signal.disconnect(field_connection);
  ring_signal(field);

  return 0;
}
