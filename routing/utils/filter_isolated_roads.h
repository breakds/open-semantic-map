#pragma once

#include <functional>
#include <string>
#include <unordered_set>

#include "osmium/handler.hpp"
#include "osmium/osm.hpp"

namespace open_semap {

// Main API of this library. Go through all the ways in the OSM file
// and returns a set of roads that are inter-connected with each
// other, i.e. isolated roads are filtered out. Isolated roads are
// regular roads that do not contain any junction nodes.
std::unordered_set<osmium::object_id_type> FilterIsolatedRoads(
    const std::string& path, const std::unordered_set<osmium::object_id_type>& junctions);

class FilterIsolatedRoadsHandler : public osmium::handler::Handler {
 public:
  using IdSet = std::unordered_set<osmium::object_id_type>;

  FilterIsolatedRoadsHandler(const IdSet& junctions_) : junctions(junctions_) {
  }

  void way(const osmium::Way& way);

  IdSet ReleaseResult() {
    return std::move(inter_connected_roads);
  }

 private:
  std::reference_wrapper<const IdSet> junctions;
  IdSet inter_connected_roads{};
};

}  // namespace open_semap
