#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "osmium/osm/types.hpp"

#include "graph/edge.h"
#include "graph/vertex.h"

namespace open_semap {
namespace graph {

class RoadGraph {
 public:
  using VertexID = osmium::object_id_type;
  using EdgeID   = osmium::object_id_type;

  static RoadGraph LoadFromFile(const std::string &path);

  RoadGraph(RoadGraph &&) noexcept = default;
  RoadGraph &operator=(RoadGraph &&) noexcept = default;

  // Simple Accessors
  const Vertex &GetVertex(VertexID id) const;
  const std::vector<std::unique_ptr<Vertex>> &vertices() const;
  const std::vector<std::unique_ptr<Edge>> &edges() const;

 private:
  RoadGraph() = default;

  std::vector<std::unique_ptr<Vertex>> vertices_{};
  std::vector<std::unique_ptr<Edge>> edges_{};
  std::unordered_map<VertexID, std::reference_wrapper<Vertex>> id_to_vertex_{};
};

}  // namespace graph
}  // namespace open_semap
