#pragma once

#include <memory>
#include <string>
#include <vector>

#include "osmium/osm/types.hpp"

#include "graph/defs.h"
#include "graph/edge.h"
#include "graph/vertex.h"

namespace open_semap {
namespace graph {

class RoadGraph {
 public:
  static RoadGraph LoadFromFile(const std::string &path);

  RoadGraph(std::vector<std::unique_ptr<Vertex>> &&vertices,
            std::vector<std::unique_ptr<Edge>> &&edges)
      : vertices_(std::move(vertices)), edges_(std::move(edges)) {}

  RoadGraph(RoadGraph &&) noexcept = default;
  RoadGraph &operator=(RoadGraph &&) noexcept = default;

  const std::vector<std::unique_ptr<Vertex>> &vertices() const;
  const std::vector<std::unique_ptr<Edge>> &edges() const;

 private:
  RoadGraph() = default;

  std::vector<std::unique_ptr<Vertex>> vertices_{};
  std::vector<std::unique_ptr<Edge>> edges_{};
};

}  // namespace graph
}  // namespace open_semap
