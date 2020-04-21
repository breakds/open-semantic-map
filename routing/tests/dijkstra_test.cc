#include "algorithms/dijkstra.h"

#include <algorithm>
#include <memory>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "spdlog/spdlog.h"

#include "graph/road_graph.h"
#include "graph/simple_indexer.h"

using ::testing::AllOf;
using ::testing::DoubleEq;
using ::testing::PrintToString;
using ::testing::Property;

namespace open_semap {

using graph::Edge;
using graph::EdgeID;
using graph::RoadGraph;
using graph::SimpleIndexer;
using graph::Vertex;
using graph::VertexID;

MATCHER_P2(IsAnEdge, from_id, to_id,
           (negation ? std::string("isn't") : std::string("is")) + " an edge " +
               std::to_string(from_id) + " -> " + std::to_string(to_id)) {
  return arg.from().id() == from_id && arg.to().id() == to_id;
}

RoadGraph CreateSampleGraph() {
  std::vector<std::unique_ptr<Vertex>> vertices;
  std::vector<std::unique_ptr<Edge>> edges;

  auto TryAddVertex = [&](VertexID id) -> const Vertex & {
    auto iter = std::find_if(
        vertices.begin(), vertices.end(),
        [id](const std::unique_ptr<Vertex> &vertex) { return vertex->id() == id; });
    if (iter != vertices.end()) {
      return **iter;
    }
    vertices.emplace_back(std::make_unique<Vertex>(id, osmium::Location()));
    return *vertices.back();
  };

  auto AddEdge = [&](VertexID from, VertexID to, double length) {
    const Vertex &a = TryAddVertex(from);
    const Vertex &b = TryAddVertex(to);
    EdgeID edge_id  = static_cast<EdgeID>(edges.size());
    edges.emplace_back(std::make_unique<Edge>(edge_id, a, b, length));
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

TEST(DijkstraTest, SimpleGraphCase1) {
  RoadGraph graph       = CreateSampleGraph();
  SimpleIndexer indexer = SimpleIndexer::CreateFromRawGraph(graph);

  SearchTree search_tree = RunDijkstra(indexer, 1, {2, 3, 4});

  const SearchNode *n1 = search_tree.Find(1);
  EXPECT_EQ(nullptr, n1);

  const SearchNode *n2 = search_tree.Find(2);
  EXPECT_NE(nullptr, n2);
  EXPECT_THAT(*n2, AllOf(Property(&SearchNode::cost, DoubleEq(15.0)),
                         Property(&SearchNode::edge, IsAnEdge(1, 2))));

  const SearchNode *n3 = search_tree.Find(3);
  EXPECT_NE(nullptr, n3);
  EXPECT_THAT(*n3, AllOf(Property(&SearchNode::cost, DoubleEq(7.0)),
                         Property(&SearchNode::edge, IsAnEdge(4, 3))));

  const SearchNode *n4 = search_tree.Find(4);
  EXPECT_NE(nullptr, n4);
  EXPECT_THAT(*n4, AllOf(Property(&SearchNode::cost, DoubleEq(4.0)),
                         Property(&SearchNode::edge, IsAnEdge(1, 4))));
}

}  // namespace open_semap
