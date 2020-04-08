#pragma once

#include <functional>
#include <vector>

#include "osmium/osm/types.h"

namespace open_semap {
namespace graph {

class Vertex;

class Edge {
 public:
  inline osmium::object_id_type id() const {
    return id_;
  }

  inline double cost() const {
    // FIXME: for routing, we should store the travel time instead of
    // travel distance.
    return length_;
  }

  inline const Vertex &from() const {
    return from_.get();
  }

  inline const Vertex &to() const {
    return to_.get();
  }

 private:
  osmium::object_id_type id_;
  std::reference_wrapper<const Vertex> from_;
  std::reference_wrapper<const Vertex> to_;
  // Store the intermediate points for visualization uses. They do not
  // participate in the routing algorithm.
  std::vector<osmium::location> points_;
  double length_;
};

}  // namespace graph
}  // namespace open_semap
