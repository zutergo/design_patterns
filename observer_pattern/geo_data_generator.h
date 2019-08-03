// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "geometry.h"
#include <cstdlib>
#include <ctime>

#ifndef GEO_DATA_GENERATOR_H
#define GEO_DATA_GENERATOR_H

double create_random() { return (double)rand() / 100; }

point create_point() { return point{create_random(), create_random()}; }

template <typename GeoData> GeoData create_geo_data() { return create_point(); }

#endif /* end of include guard: GEO_DATA_GENERATOR_H*/
