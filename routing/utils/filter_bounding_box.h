#pragma once

#include <functional>
#include <string>
#include <unordered_set>

#include "osmium/handler.hpp"
#include "osmium/osm.hpp"

namespace open_semap {

// Main API of this library. Filter the list of junction nodes, and
// keep only the ones that are within the bounding box.
std::unordered_set<osmium::object_id_type> FilterBoundingBox(
    const std::string& path, const std::unordered_set<osmium::object_id_type>& junctions,
    const osmium::Box& bounding_box);

class FilterBoundingBoxHandler : public osmium::handler::Handler {
 public:
  using IdSet = std::unordered_set<osmium::object_id_type>;

  FilterBoundingBoxHandler(const IdSet& junctions_, const osmium::Box& bounding_box_)
      : junctions(junctions_), bounding_box(bounding_box_) {
  }

  void node(const osmium::Node& node);

  IdSet ReleaseJunctions() {
    return std::move(filtered_junctions);
  }

 private:
  std::reference_wrapper<const IdSet> junctions;
  osmium::Box bounding_box;
  IdSet filtered_junctions{};
};

}  // namespace open_semap
