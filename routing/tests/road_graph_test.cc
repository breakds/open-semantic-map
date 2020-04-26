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

}  // namespace testing
}  // namespace open_semap
