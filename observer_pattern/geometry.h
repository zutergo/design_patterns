// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef GEOMETRY_H
#define GEOMETRY_H

struct point {
  double x;
  double y;
};

struct line {
  point a;
  point b;
};

// use polygon or linear ring from boost
struct area {
  point min_corner;
  point max_corner;
};

// include <boost/geometry.hpp>
// using point = boost::geometry::model::point<double, 2,
// boost::geometry::cs::cartesian>; using line =
// boost::geometry::model::linestring<point>; using area =
// boost::geometry::model::box<point>;
// using ring = boost::geometry::model::ring<point>;

#endif /* end of include guard: GEOMETRY_H */
