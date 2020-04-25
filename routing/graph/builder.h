#pragma once

#include <memory>

#include "graph/edge.h"
#include "graph/road_graph.h"
#include "graph/vertex.h"

namespace open_semap {
namespace graph {

// Provides an utility to build a Road Graph. This is especially
// useful for writing unit tests.
class RoadGraphBuilder {
 public:
  RoadGraphBuilder() = default;

  // Add an edge that connects the vertices `from` and `to`, with the
  // specified length.
  RoadGraphBuilder &AddEdge(VertexID from, VertexID to, double length);

  RoadGraph Build();

 private:
  std::vector<std::unique_ptr<Vertex>> vertices_{};
  std::vector<std::unique_ptr<Edge>> edges_{};
};

}  // namespace graph
}  // namespace open_semap
