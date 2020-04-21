#pragma once

#include <functional>
#include <vector>

#include "osmium/osm/location.hpp"
#include "osmium/osm/types.hpp"

namespace open_semap {
namespace graph {

class Vertex;

class Edge {
 public:
  Edge(osmium::object_id_type id, const Vertex &from, const Vertex &to);

  Edge(osmium::object_id_type id, const Vertex &from, const Vertex &to, double length);

  // Special constructor to create a shortcut edge.
  Edge(osmium::object_id_type id, const Edge &a, const Edge &b);

  inline osmium::object_id_type id() const { return id_; }

  inline double cost() const {
    // FIXME: for routing, we should store the travel time instead of
    // travel distance.
    return length_;
  }

  inline const Vertex &from() const { return from_.get(); }

  inline const Vertex &to() const { return to_.get(); }

 public:
  osmium::object_id_type id_;
  std::reference_wrapper<const Vertex> from_;
  std::reference_wrapper<const Vertex> to_;
  // Store the intermediate points for visualization uses. They do not
  // participate in the routing algorithm.
  std::vector<osmium::Location> points_{};
  double length_ = 0.0;
  // points, it stores a pair of edges that makes this shortcut. Use
  // car and con as a convention from Lisp.
  const Edge *car = nullptr;
  const Edge *con = nullptr;
};

}  // namespace graph
}  // namespace open_semap
