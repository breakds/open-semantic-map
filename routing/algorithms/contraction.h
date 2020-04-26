#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "graph/defs.h"

namespace open_semap {
namespace graph {

// Forward declaration
class SimpleIndexer;
class Edge;
class RoadGraph;
class Vertex;

}  // namespace graph

// Contract the graph based on the Contraction Hierarchies algorithm.
// The order of contraction is given. The algorithm will generate
// shortcuts when each vertex is contracted, and the generated
// shortcuts will be returned.
std::vector<std::unique_ptr<graph::Edge>> ContractGraph(
    const std::vector<std::reference_wrapper<const graph::Vertex>> &ordered_vertices,
    graph::SimpleIndexer *indexer);

// This does the same thing as the above API, except for that it
// accepts a list of vertex IDs instead of vertices.
std::vector<std::unique_ptr<graph::Edge>> ContractGraph(
    const std::vector<graph::VertexID> &ordered_vertex_ids,
    graph::SimpleIndexer *indexer);

}  // namespace open_semap
