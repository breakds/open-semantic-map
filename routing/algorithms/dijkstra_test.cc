#include "algorithms/dijkstra.h"

#include <algorithm>
#include <memory>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using graph::Edge;
using graph::EdgeID;
using graph::RoadGraph;
using graph::Vertex;
using graph::VertexID;

namespace open_semap {

RoadGraph CreateSampleGraph() {
  std::vector<std::unique_ptr<Vertex>> vertices;
  std::vector<std::unique_ptr<Edge>> edges;

  auto TryAddVertex =
      [&](VertexID id) {
        auto iter = std::find_if(
            vertices.begin(), vertices.end(),
            [id](const std::unique_ptr<Vertex> &vertex) { return vertex.id() == id; });
        if (iter != vertex.end()) {
          return **iter;
        }
        vertices.emplace_back(std::make_unique<Vertex>(id, osmium::Location()));
        return *vertices.back();
      }

  auto AddEdge = [&](VertexID from, VertexID to, double length) {
    TryAddVertex(from);
    TryAddVertex(to);
    EdgeID edge_id = static_cast<EdgeID>(edges.size());
    edges.emplace_back(std::make_unique<Edge>(edge_id, from, to, length));
  };

  AddEdge(1, 2, 15.0);
  AddEdge(1, 3, 10.0);
  AddEdge(1, 4, 4.0);
  AddEdge(3, 1, 10.0);
  AddEdge(2, 3, 7.0);
  AddEdge(2, 4, 7.0);
  AddEdge(4, 3, 3.0);

  return RoadGraph(std::move(vertices), std::move(edges));
}

TEST(DijkstraTest, SimpleGraph) {
  RoadGraph graph = CreateSampleGraph();
}

}  // namespace open_semap
