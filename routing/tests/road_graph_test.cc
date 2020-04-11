#include "graph/road_graph.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "tests/testdata.h"

TEST(RoadGraphTest, LoadGraph) {
  RoadGraph graph = RoadGraph::LoadFromFile(std::string(TEST_DATA_PATH) + "/adobe_wells_routes.osm");
}

