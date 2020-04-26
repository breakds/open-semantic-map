#include "algorithms/contraction.h"

#include <algorithm>
#include <memory>

#include "gmock/gmock.h"
#include "graph/builder.h"
#include "graph/road_graph.h"
#include "graph/simple_indexer.h"
#include "gtest/gtest.h"
#include "spdlog/spdlog.h"

using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::Pointee;
using ::testing::Property;

namespace open_semap {

using graph::Edge;
using graph::RoadGraph;
using graph::RoadGraphBuilder;
using graph::SimpleIndexer;
using graph::Vertex;

TEST(ContractionTest, SingleVertexContraction1) {
  // Trivial graph: v1 ---> v2 ---> v3
  //                    5       7
  //
  // Let's contract vertex 2.
  RoadGraph graph = RoadGraphBuilder().AddEdge(1, 2, 5.0).AddEdge(2, 3, 7.0).Build();
  SimpleIndexer indexer = SimpleIndexer::CreateFromRawGraph(graph);

  const Edge *edge12 = indexer.FindEdge(1, 2);
  const Edge *edge23 = indexer.FindEdge(2, 3);

  std::vector<std::unique_ptr<Edge>> shortcuts = ContractGraph({2}, &indexer);

  // The edges should have been removed already by contraction.
  EXPECT_EQ(nullptr, indexer.FindEdge(1, 2));
  EXPECT_EQ(nullptr, indexer.FindEdge(2, 3));

  EXPECT_THAT(shortcuts,
              ElementsAre(Pointee(AllOf(Property(&Edge::car, edge12),
                                        Property(&Edge::cdr, edge23)))));
}

}  // namespace open_semap
