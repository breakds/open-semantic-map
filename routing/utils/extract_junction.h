#pragma once

#include <string>
#include <unordered_set>

#include "osmium/handler.hpp"
#include "osmium/osm.hpp"

namespace open_semap {

// Main API of this library. Extract all the junction nodes and returns a set of
// their IDs. A node is called a junction node if it appears in more than one
// regular roads.
std::unordered_set<osmium::object_id_type> ExtractJunction(const std::string& path);

class ExtractJunctionHandler : public osmium::handler::Handler {
 public:
  using IdSet = std::unordered_set<osmium::object_id_type>;

  ExtractJunctionHandler() = default;

  void way(const osmium::Way& way);

  IdSet ReleaseJunctions() {
    return std::move(junctions);
  }

 private:
  IdSet visited;
  IdSet junctions;
};

}  // namespace open_semap
