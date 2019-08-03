// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef SIGNAL_HPP
#define SIGNAL_HPP

#include "inplace_function.h"
#include <boost/container/flat_map.hpp>

/**
 * Simple signal implementation based on an inplace_function.
 */
template <typename... Args> class signal {
public:
  /**
   * Connects a slot.
   */
  int connect(stdext::inplace_function<void(Args...)> const &slot) const {
    slots_.insert(std::make_pair(++id_, slot));
    return id_;
  }

  /**
   * Disconnect a slot.
   */
  void disconnect(const int id) const { slots_.erase(id); }

  /**
   *  Disconnects all slots.
   */
  void disconnect_all() const { slots_.clear(); }

  /**
   * Notify all observers.
   */
  void operator()(Args... args) {
    for (const auto &slot : slots_) {
      slot.second(args...);
    }
  }

private:
  mutable boost::container::flat_map<int,
                                     stdext::inplace_function<void(Args...)>>
      slots_;
  mutable int id_ = 0;
};

#endif /* SIGNAL_HPP */
