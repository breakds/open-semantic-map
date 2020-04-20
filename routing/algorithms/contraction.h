#pragma once

#include <memory>

namespace open_semap {
namespace graph {

// Forward declaration
class SimpleIndexer;
class Edge;
class RoadGraph;

}  // namespace graph

void ContractEdgePair(SimpleIndexer *indexer, const Edge &incoming, const Edge &outgoing);

std::vector<std::unique_ptr<Edge>> ContractGraph(const RoadGraph &graph,
                                                 SimpleIndexer *indexer);

}  // namespace open_semap
