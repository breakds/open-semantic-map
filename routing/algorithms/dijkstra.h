#pragma once

#include <functional>
#include <unordered_set>

#include "graph/defs.h"
#include "graph/edge.h"
#include "graph/vertex.h"

namespace open_semap {

// Forward declaration
namespace graph {

class SimpleIndexer;

}  // namespace graph

class SearchNode {
 public:
  SearchNode(double cost, const graph::Edge &edge) : cost_(cost), edge_(edge) {}

  const graph::Vertex &vertex() const { return edge_.get().to(); }

  double cost() const { return cost_; }

  const graph::Edge &edge() const { return edge_.get(); }

  bool operator<(const SearchNode &other) const { return cost_ < other.cost_; }

 private:
  double cost_;
  std::reference_wrapper<const graph::Edge> edge_;
};  // namespace open_semap

class SearchTree {
 public:
  SearchTree(graph::VertexID start_id) : start_id_(start_id) {}

  bool Has(graph::VertexID vertex_id) {
    return vertex_id == start_id_ || nodes_.count(vertex_id) > 0;
  }

  void Emplace(const SearchNode &node) { nodes_.emplace(node.vertex().id(), node); }

 private:
  graph::VertexID start_id_;
  std::unordered_map<graph::VertexID, SearchNode> nodes_{};
};

// Run Dijkstra algorithm on the input graph represented by the indexer, with
// the specified vertex as the starting point, and a list of vertices as the
// goals. The algorithm stops after the all the goals are reached or when the
// search exhausts all the reachable vertices.
//
// Returns a search tree with one record for each of the reached vertices.
SearchTree RunDijkstra(const graph::SimpleIndexer &indexer, graph::VertexID start,
                       const std::unordered_set<graph::VertexID> &goals);

}  // namespace open_semap
