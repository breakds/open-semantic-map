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
using ::testing::UnorderedElementsAre;

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

  EXPECT_THAT(shortcuts, ElementsAre(Pointee(AllOf(Property(&Edge::car, edge12),
                                                   Property(&Edge::cdr, edge23)))));
}

RoadGraph MakePaperExampleGraph() {
  // Example comes from: https://algo2.iti.kit.edu/download/presentation.pdf
  //
  //    2     3     2     1     2
  // 2 --- 6 --- 1 --- 4 --- 3 --- 5
  return RoadGraphBuilder()
      .AddEdge(2, 6, 2.0)
      .AddEdge(6, 2, 2.0)
      .AddEdge(6, 1, 3.0)
      .AddEdge(1, 6, 3.0)
      .AddEdge(1, 4, 2.0)
      .AddEdge(4, 1, 2.0)
      .AddEdge(5, 3, 2.0)
      .AddEdge(3, 5, 2.0)
      .AddEdge(3, 4, 1.0)
      .AddEdge(4, 3, 1.0)
      .Build();
}

TEST(ContractionTest, PaperExampleMultiStep) {
  RoadGraph graph       = MakePaperExampleGraph();
  SimpleIndexer indexer = SimpleIndexer::CreateFromRawGraph(graph);

  std::vector<std::unique_ptr<Edge>> shortcuts;

  {
    // Contract Node 1: Add 6 --> 4 and 4 --> 6

    const Edge *edge61 = indexer.FindEdge(6, 1);
    const Edge *edge16 = indexer.FindEdge(1, 6);
    const Edge *edge41 = indexer.FindEdge(4, 1);
    const Edge *edge14 = indexer.FindEdge(1, 4);

    std::vector<std::unique_ptr<Edge>> added = ContractGraph({1}, &indexer);

    EXPECT_EQ(nullptr, indexer.FindEdge(6, 1));
    EXPECT_EQ(nullptr, indexer.FindEdge(1, 6));
    EXPECT_EQ(nullptr, indexer.FindEdge(4, 1));
    EXPECT_EQ(nullptr, indexer.FindEdge(1, 4));
    EXPECT_EQ(nullptr, indexer.Find(1));

    EXPECT_THAT(
        added,
        UnorderedElementsAre(
            Pointee(AllOf(Property(&Edge::car, edge61), Property(&Edge::cdr, edge14))),
            Pointee(AllOf(Property(&Edge::car, edge41), Property(&Edge::cdr, edge16)))));

    for (std::unique_ptr<Edge> &edge : added) {
      shortcuts.emplace_back(std::move(edge));
    }
  }

  {
    // Node 2 does not contract.
    std::vector<std::unique_ptr<Edge>> added = ContractGraph({2}, &indexer);
    EXPECT_EQ(nullptr, indexer.Find(2));
    ASSERT_EQ(0, added.size());
  }

  {
    // Contract node 3: Add 4 --> 5 and 5 --> 4
    const Edge *edge43 = indexer.FindEdge(4, 3);
    const Edge *edge34 = indexer.FindEdge(3, 4);
    const Edge *edge35 = indexer.FindEdge(3, 5);
    const Edge *edge53 = indexer.FindEdge(5, 3);

    std::vector<std::unique_ptr<Edge>> added = ContractGraph({3}, &indexer);

    EXPECT_EQ(nullptr, indexer.FindEdge(4, 3));
    EXPECT_EQ(nullptr, indexer.FindEdge(3, 4));
    EXPECT_EQ(nullptr, indexer.FindEdge(3, 5));
    EXPECT_EQ(nullptr, indexer.FindEdge(5, 3));
    EXPECT_EQ(nullptr, indexer.Find(3));

    EXPECT_THAT(
        added,
        UnorderedElementsAre(
            Pointee(AllOf(Property(&Edge::car, edge43), Property(&Edge::cdr, edge35))),
            Pointee(AllOf(Property(&Edge::car, edge53), Property(&Edge::cdr, edge34)))));

    for (std::unique_ptr<Edge> &edge : added) {
      shortcuts.emplace_back(std::move(edge));
    }
  }

  {
    // Contract node 3: Add 4 --> 5 and 5 --> 4
    const Edge *edge64 = indexer.FindEdge(6, 4);
    const Edge *edge45 = indexer.FindEdge(4, 5);
    const Edge *edge54 = indexer.FindEdge(5, 4);
    const Edge *edge46 = indexer.FindEdge(4, 6);

    std::vector<std::unique_ptr<Edge>> added = ContractGraph({4}, &indexer);

    EXPECT_EQ(nullptr, indexer.FindEdge(6, 4));
    EXPECT_EQ(nullptr, indexer.FindEdge(4, 5));
    EXPECT_EQ(nullptr, indexer.FindEdge(5, 4));
    EXPECT_EQ(nullptr, indexer.FindEdge(4, 6));
    EXPECT_EQ(nullptr, indexer.Find(4));

    EXPECT_THAT(
        added,
        UnorderedElementsAre(
            Pointee(AllOf(Property(&Edge::car, edge64), Property(&Edge::cdr, edge45))),
            Pointee(AllOf(Property(&Edge::car, edge54), Property(&Edge::cdr, edge46)))));

    for (std::unique_ptr<Edge> &edge : added) {
      shortcuts.emplace_back(std::move(edge));
    }
  }

  {
    // Node 5 does not contract.
    std::vector<std::unique_ptr<Edge>> added = ContractGraph({5}, &indexer);
    EXPECT_EQ(nullptr, indexer.Find(5));
    ASSERT_EQ(0, added.size());
  }
}

TEST(ContractionTest, PaperExampleOneBatch) {
  RoadGraph graph       = MakePaperExampleGraph();
  SimpleIndexer indexer = SimpleIndexer::CreateFromRawGraph(graph);

  std::vector<std::unique_ptr<Edge>> shortcuts = ContractGraph({1, 2, 3, 4, 5}, &indexer);

  EXPECT_EQ(6, shortcuts.size());
}

}  // namespace open_semap
