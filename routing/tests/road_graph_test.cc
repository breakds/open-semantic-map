#include "graph/road_graph.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "tests/testdata.h"

namespace open_semap {
namespace testing {

TEST(RoadGraphTest, LoadGraph) {
  graph::RoadGraph graph = graph::RoadGraph::LoadFromFile(std::string(TEST_DATA_PATH) +
                                                          "/adobe_wells_routes.osm");
}

}  // namespace testing

}  // namespace open_semap
