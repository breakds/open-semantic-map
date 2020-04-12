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

 private:
  osmium::object_id_type id_;
  osmium::Location loc_;
};

}  // namespace graph
}  // namespace open_semap
