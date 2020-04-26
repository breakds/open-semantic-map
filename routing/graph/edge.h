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
  Edge(const Vertex &from, const Vertex &to);

  Edge(const Vertex &from, const Vertex &to, double length);

  // Special constructor to create a shortcut edge.
  Edge(const Edge &a, const Edge &b);

  inline double cost() const {
    // FIXME: for routing, we should store the travel time instead of
    // travel distance.
    return length_;
  }

  inline const Vertex &from() const { return from_.get(); }

  inline const Vertex &to() const { return to_.get(); }

  inline const Edge *car() const { return car_; }
  inline const Edge *cdr() const { return cdr_; }

 public:
  std::reference_wrapper<const Vertex> from_;
  std::reference_wrapper<const Vertex> to_;
  // Store the intermediate points for visualization uses. They do not
  // participate in the routing algorithm.
  std::vector<osmium::Location> points_{};
  double length_ = 0.0;
  // points, it stores a pair of edges that makes this shortcut. Use
  // car and con as a convention from Lisp.
  const Edge *car_ = nullptr;
  const Edge *cdr_ = nullptr;
};

}  // namespace graph
}  // namespace open_semap
