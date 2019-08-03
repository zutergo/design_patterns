// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "geometry.h"
#include <any>
#include <iostream>
#include <variant>

#ifndef OBSERVERS_H
#define OBSERVERS_H

/**
 * Calculates the sum of distances between consecutive points.
 * @param geo_data Geometry object that defines a set of points (e.g. line,
 * ring)
 */
auto path_renderer = [](const auto &geo_data) {
  //  std::cout << "Path length: " << boost::geometry::length(geo_data) << "\n";
};

/**
 * Calculates the area of a field.
 * @param field A field defined by a closed line which should not be
 * selfintersecting.
 */
auto field_renderer = [](const ring &field) {
  //  std::cout << "Field area: " << boost::geometry::area(field) << "\n";
};

/**
 * Render details in a geographical map.
 * @param detail A point, line or ring that will be inserted in the map.
 */
void variant_detail_renderer(std::variant<point, line, ring> detail) {
  std::cout << "Variant Detail renderer: "
            << "\n";
}

/**
 * Render details in a geographical map.
 * @param detail Any object that will be inserted in the map.
 */
void any_detail_renderer(std::any detail) {
  std::cout << "Any Detail renderer: "
            << "\n";
}

#endif /* end of include guard: OBSERVERS_H*/
