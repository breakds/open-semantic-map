#pragma once

#include <functional>
#include <string>
#include <unordered_set>

#include "osmium/handler.hpp"
#include "osmium/memory/buffer.hpp"
#include "osmium/osm.hpp"

namespace open_semap {

// Main API of this library. Given the junction nodes and the roads,
// split each of the road into several by the junctions on it, and
// write the end result into the output path.
//
// 1. Only the road in the road in `roads` are considered.
//
// 2. The resulting graph will have junctions as the vertex and the
//    split roads as the edges.
//
// 3. The length of the split roads will be written.
void SplitRoad(const std::string& path, const std::string& output_path,
               const std::unordered_set<osmium::object_id_type>& junctions,
               const std::unordered_set<osmium::object_id_type>& roads);

class SplitRoadHandler : public osmium::handler::Handler {
 public:
  using IdSet = std::unordered_set<osmium::object_id_type>;

  SplitRoadHandler(const IdSet& junctions_, const IdSet& approved_roads_,
                   osmium::memory::Buffer& output_buffer_)
      : junctions(junctions_),
        approved_roads(approved_roads_),
        output_buffer(output_buffer_) {
  }

  void node(const osmium::Node& node);

  void way(const osmium::Way& way);

 private:
  std::reference_wrapper<const IdSet> junctions;
  std::reference_wrapper<const IdSet> approved_roads;
  std::reference_wrapper<osmium::memory::Buffer> output_buffer;
};

}  // namespace open_semap
