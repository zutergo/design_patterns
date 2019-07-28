// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "geometry.h"
#include <iostream>
#include <list>

template <typename T> class map_renderer {
public:
  using GeoData = T;
  virtual void render(GeoData data) = 0;
  virtual ~map_renderer() = default;
};

template <typename T> class path_map_renderer : public map_renderer<T> {
public:
  using GeoData = map_renderer<T>::GeoData;
  void render(geo_data data) {
    std::cout << "Render path"
              << "\n";
  }
};

template <typename T> class field_map_renderer : public map_renderer<T> {
public:
  using GeoData = map_renderer<T>::GeoData;
  void render(geo_data data) {
    std::cout << "Render field"
              << "\n";
  }
};

template <typename T> class geo_data_provider {
public:
  using GeoData = T;
  virtual void register_map_renderer(map_renderer *renderer) = 0;
  virtual void unregister_map_renderer(map_renderer *renderer) = 0;
  virtual void send_geo_data(GeoData data) = 0;
  virtual ~geo_data_provider() = default;
};

template <typename T>
class ecu_geo_data_provider : public geo_data_provider<T> {
public:
  using GeoData = geo_data_provider<T>::GeoData;

  void register_map_renderer(map_renderer *renderer) override {
    map_renderers_.push_back(renderer);
  }

  void unregister_map_renderer(map_renderer *renderer) override {
    map_renderers_.remove(renderer);
  }

  void send_geo_data(GeoData data) override {
    for (const auto renderer : map_renderers_) {
      renderer->render(data);
    }
  }

private:
  // vector
  using RendererContainer = std::list<map_renderer *>;
  RendererContainer map_renderers_;
};

int main(int argc, char *argv[]) {
  ecu_geo_data_provider provider;

  // use unique_ptr
  map_renderer *path_renderer = new path_map_renderer();
  map_renderer *field_renderer = new field_map_renderer();

  provider.register_map_renderer(path_renderer);
  provider.register_map_renderer(field_renderer);

  point p;
  provider.send_geo_data(p);

  provider.unregister_map_renderer(path_renderer);
  provider.send_geo_data(point);
  provider.unregister_map_renderer(field_renderer);

  return 0;
}
