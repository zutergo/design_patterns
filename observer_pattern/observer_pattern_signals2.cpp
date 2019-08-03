// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "geometry.h"
//#include "inplace_function.h"
#include "observers.h"
#include <any>
#include <boost/signals2.hpp>
#include <variant>

/**
 *! Implements the observer pattern for map rendering.
 */
namespace bs2 = boost::signals2;
template <typename... Args>
typename bs2::signal_type<void(Args...),
                          bs2::keywords::mutex_type<bs2::dummy_mutex>>::type
    geo_data_signal;

// template <typename T>
// bs2::signal<void(T), bs2::optional_last_value<void>, int, std::less<int>,
//            stdext::inplace_function<void(T)>,
//            stdext::inplace_function<void(const bs2::connection &)>,
//            bs2::dummy_mutex>
//    geo_data_signal;

int main(int argc, char *argv[]) {
  point a{0.0, 0.0};
  point b{0.0, 5.0};
  point c{5.0, 5.0};
  point d{5.0, 0.0};
  line path{a, b};
  ring field{a, b, c, d, a};

  std::variant<point, line, ring> variant_geo_data = field;
  std::any any_geo_data = path;

  geo_data_signal<line>.connect(path_renderer);
  auto ring_connection = geo_data_signal<ring>.connect(path_renderer);
  geo_data_signal<ring>.connect(field_renderer);
  geo_data_signal<line>(path);

  for (int i = 0; i < 100000000; ++i) {
    geo_data_signal<ring>(field);
  }

  geo_data_signal<ring>.disconnect(field_renderer);
  ring_connection.disconnect();

  geo_data_signal<ring>(field);

  return 0;
}
