#pragma once

#include <functional>
#include <vector>

#include "osmium/osm/location.hpp"
#include "osmium/osm/types.hpp"

namespace open_semap {
namespace graph {

class Edge;

class Vertex {
 public:
  Vertex(osmium::object_id_type id, osmium::Location location) : id_(id), loc_(location) {
  }

  inline osmium::object_id_type id() const {
    return id_;
  }

  inline const osmium::Location &loc() const {
    return loc_;
  }

  inline const std::vector<std::reference_wrapper<const Edge>> &inwards() const {
    return inwards_;
  }

  inline const std::vector<std::reference_wrapper<const Edge>> &outwards() const {
    return outwards_;
  }

 private:
  osmium::object_id_type id_;
  osmium::Location loc_;
  // NOTE(breakds): Since this is a graph abstracting 2D map of roads,
  // each intersection (vertex) will have just a few in and out edges.
  std::vector<std::reference_wrapper<const Edge>> inwards_;
  std::vector<std::reference_wrapper<const Edge>> outwards_;
};

}  // namespace graph
}  // namespace open_semap

