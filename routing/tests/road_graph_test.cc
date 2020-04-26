#include "graph/road_graph.h"

#include "gmock/gmock.h"
#include "graph/builder.h"
#include "graph/simple_indexer.h"
#include "gtest/gtest.h"
#include "tests/testdata.h"

using ::testing::AllOf;
using ::testing::DoubleEq;
using ::testing::DoubleNear;
using ::testing::ElementsAre;
using ::testing::Property;

namespace open_semap {
namespace testing {

MATCHER_P2(IsAnEdgeRef, from_id, to_id,
           (negation ? std::string("isn't") : std::string("is")) + " an edge ref " +
               std::to_string(from_id) + " -> " + std::to_string(to_id)) {
  return arg.get().from().id() == from_id && arg.get().to().id() == to_id;
}

TEST(RoadGraphTest, LoadGraphAndIndexing) {
  graph::RoadGraph graph = graph::RoadGraph::LoadFromFile(std::string(TEST_DATA_PATH) +
                                                          "/adobe_wells_routes.osm");
  EXPECT_EQ(44, graph.vertices().size());
  EXPECT_EQ(66, graph.edges().size());

  graph::SimpleIndexer indexer = graph::SimpleIndexer::CreateFromRawGraph(graph);

  const graph::ConnectionInfo *conn = indexer.Find(421266660);
  EXPECT_NE(nullptr, conn);

  EXPECT_THAT(
      conn->vertex.get(),
      AllOf(Property(&graph::Vertex::id, 421266660),
            Property(&graph::Vertex::loc,
                     AllOf(Property(&osmium::Location::lon, DoubleEq(-121.9911)),
                           Property(&osmium::Location::lat, DoubleEq(37.4029826))))));

  EXPECT_THAT(conn->outwards,
              ElementsAre(AllOf(
                  Property(&graph::Edge::from, Property(&graph::Vertex::id, 421266660)),
                  Property(&graph::Edge::to, Property(&graph::Vertex::id, 421266661)),
                  Property(&graph::Edge::cost, DoubleNear(49.5252, 1e-3)))));

  EXPECT_THAT(
      conn->inwards,
      ElementsAre(
          AllOf(Property(&graph::Edge::from, Property(&graph::Vertex::id, 421266658)),
                Property(&graph::Edge::to, Property(&graph::Vertex::id, 421266660)),
                Property(&graph::Edge::cost, DoubleNear(48.8853, 1e-3))),
          AllOf(Property(&graph::Edge::from, Property(&graph::Vertex::id, 421266698)),
                Property(&graph::Edge::to, Property(&graph::Vertex::id, 421266660)),
                Property(&graph::Edge::cost, DoubleNear(140.172, 1e-3)))));
}

TEST(SimpleIndexerTest, FindEdge) {
  graph::RoadGraph graph = graph::RoadGraphBuilder()
                               .AddEdge(1, 2, 15.0)
                               .AddEdge(1, 3, 10.0)
                               .AddEdge(1, 4, 4.0)
                               .AddEdge(3, 1, 10.0)
                               .AddEdge(2, 3, 7.0)
                               .AddEdge(2, 4, 7.0)
                               .AddEdge(4, 3, 3.0)
                               .Build();
  graph::SimpleIndexer indexer = graph::SimpleIndexer::CreateFromRawGraph(graph);
  EXPECT_NE(nullptr, indexer.FindEdge(1, 2));
}

TEST(SimpleIndexerTest, RemoveVertex) {
  graph::RoadGraph graph = graph::RoadGraphBuilder()
                               .AddEdge(1, 2, 15.0)
                               .AddEdge(2, 3, 10.0)
                               .AddEdge(2, 4, 7.0)
                               .Build();
  graph::SimpleIndexer indexer = graph::SimpleIndexer::CreateFromRawGraph(graph);

  // Before Remove node 2
  {
    const auto *conn1 = indexer.Find(1);
    EXPECT_THAT(conn1->inwards, ElementsAre());
    EXPECT_THAT(conn1->outwards, ElementsAre(IsAnEdgeRef(1, 2)));

    const auto *conn2 = indexer.Find(2);
    EXPECT_THAT(conn2->inwards, ElementsAre(IsAnEdgeRef(1, 2)));
    EXPECT_THAT(conn2->outwards, ElementsAre(IsAnEdgeRef(2, 3), IsAnEdgeRef(2, 4)));

    const auto *conn3 = indexer.Find(3);
    EXPECT_THAT(conn3->inwards, ElementsAre(IsAnEdgeRef(2, 3)));
    EXPECT_THAT(conn3->outwards, ElementsAre());

    const auto *conn4 = indexer.Find(4);
    EXPECT_THAT(conn4->inwards, ElementsAre(IsAnEdgeRef(2, 4)));
    EXPECT_THAT(conn4->outwards, ElementsAre());
  }

  indexer.RemoveVertex(2);

  // After Remove node 2
  {
    const auto *conn1 = indexer.Find(1);
    EXPECT_THAT(conn1->inwards, ElementsAre());
    EXPECT_THAT(conn1->outwards, ElementsAre());

    const auto *conn2 = indexer.Find(2);
    EXPECT_EQ(nullptr, conn2);

    const auto *conn3 = indexer.Find(3);
    EXPECT_THAT(conn3->inwards, ElementsAre());
    EXPECT_THAT(conn3->outwards, ElementsAre());

    const auto *conn4 = indexer.Find(4);
    EXPECT_THAT(conn4->inwards, ElementsAre());
    EXPECT_THAT(conn4->outwards, ElementsAre());
  }
}

}  // namespace testing
}  // namespace open_semap
