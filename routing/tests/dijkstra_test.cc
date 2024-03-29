#include "algorithms/dijkstra.h"

#include <algorithm>
#include <memory>

#include "gmock/gmock.h"
#include "graph/builder.h"
#include "graph/road_graph.h"
#include "graph/simple_indexer.h"
#include "gtest/gtest.h"
#include "spdlog/spdlog.h"

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

RoadGraph CreateTrivialGraph() {
  return graph::RoadGraphBuilder().AddEdge(1, 2, 5.0).AddEdge(2, 3, 7.0).Build();
}

TEST(DijkstraTest, TrivialGraph1) {
  RoadGraph graph       = CreateTrivialGraph();
  SimpleIndexer indexer = SimpleIndexer::CreateFromRawGraph(graph);

  SearchTree search_tree = RunDijkstra(indexer, 1, {3});

  const SearchNode *n1 = search_tree.Find(1);
  EXPECT_EQ(nullptr, n1);

  const SearchNode *n2 = search_tree.Find(2);
  EXPECT_THAT(*n2, AllOf(Property(&SearchNode::cost, DoubleEq(5.0)),
                         Property(&SearchNode::edge, IsAnEdge(1, 2))));

  const SearchNode *n3 = search_tree.Find(3);
  EXPECT_THAT(*n3, AllOf(Property(&SearchNode::cost, DoubleEq(12.0)),
                         Property(&SearchNode::edge, IsAnEdge(2, 3))));
}

RoadGraph CreateSampleGraph() {
  return graph::RoadGraphBuilder()
      .AddEdge(1, 2, 15.0)
      .AddEdge(1, 3, 10.0)
      .AddEdge(1, 4, 4.0)
      .AddEdge(3, 1, 10.0)
      .AddEdge(2, 3, 7.0)
      .AddEdge(2, 4, 7.0)
      .AddEdge(4, 3, 3.0)
      .Build();
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

TEST(DijkstraTest, SimpleGraphCase2Partial) {
  RoadGraph graph       = CreateSampleGraph();
  SimpleIndexer indexer = SimpleIndexer::CreateFromRawGraph(graph);

  // Even if we only ask for 2, the algorithm goes far to get 3 and 4
  // too because they are closer.
  SearchTree search_tree = RunDijkstra(indexer, 1, {2});

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

TEST(DijkstraTest, SimpleGraphCase2EarlyStop) {
  RoadGraph graph       = CreateSampleGraph();
  SimpleIndexer indexer = SimpleIndexer::CreateFromRawGraph(graph);

  // Vertex 4 is going to be finalized first, so that the algorithm
  // early stops without going on to get 2 and 3.
  SearchTree search_tree = RunDijkstra(indexer, 1, {4});

  const SearchNode *n1 = search_tree.Find(1);
  EXPECT_EQ(nullptr, n1);

  const SearchNode *n2 = search_tree.Find(2);
  EXPECT_EQ(nullptr, n2);

  const SearchNode *n3 = search_tree.Find(3);
  EXPECT_EQ(nullptr, n3);

  const SearchNode *n4 = search_tree.Find(4);
  EXPECT_NE(nullptr, n4);
  EXPECT_THAT(*n4, AllOf(Property(&SearchNode::cost, DoubleEq(4.0)),
                         Property(&SearchNode::edge, IsAnEdge(1, 4))));
}

}  // namespace open_semap
