#include "gmock/gmock.h"
#include "graph/builder.h"
#include "graph/contraction_info.h"
#include "graph/road_graph.h"
#include "gtest/gtest.h"
#include "tests/testdata.h"

namespace open_semap {
namespace graph {

TEST(ContractionInfoIOTest, WriteAndLoad) {
  graph::RoadGraph graph = graph::RoadGraphBuilder()
                               .AddEdge(1, 2, 15.0)
                               .AddEdge(1, 3, 10.0)
                               .AddEdge(1, 4, 4.0)
                               .AddEdge(3, 1, 10.0)
                               .AddEdge(2, 3, 7.0)
                               .AddEdge(2, 4, 7.0)
                               .AddEdge(4, 3, 3.0)
                               .Build();

  ContractionInfo info;

  info.ordered_vertex_ids.emplace_back(3);
  info.shortcuts.emplace_back();
}

}  // namespace graph
}  // namespace open_semap
