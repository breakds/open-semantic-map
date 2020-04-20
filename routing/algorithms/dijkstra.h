#pragma once

#include <functional>
#include <unordered_set>

namespace open_semap {

class SearchNode {
 public:
  SearchNode(double cost, const Edge &edge) : cost_(cost), edge_(edge) {}

  graph::VertexID &vertex() const { return edge_.get().to(); }

  double cost() const { return cost_; }

  const Edge &edge() const { return edge_.get(); }

 private:
  double cost_;
  std::reference_wrapper<const Edge> edge_;
};  // namespace open_semap

class SearchTree {
 public:
  SearchTree(graph::VertexID start_id) : start_id_(start_id) {}

  bool Has(graph::VertexID vertex_id) {
    return vertex_id == start_id_ || nodes_.find(vertex_id) > 0;
  }

  void Emplace(std::unique_ptr<SearchNode> &&node) {
    nodes_.emplace(node.vertex().id(), std::move(node));
  }

 private:
  std::unordered_map<graph::VertexID, std::unique_ptr<SearchNode>> nodes_;
};

// Run Dijkstra algorithm on the input graph represented by the indexer, with
// the specified vertex as the starting point, and a list of vertices as the
// goals. The algorithm stops after the all the goals are reached or when the
// search exhausts all the reachable vertices.
//
// Returns a search tree with one record for each of the reached vertices.
SearchTree RunDijkstra(const graph::SimpleIndexer &indexer, graph::VerticeID start,
                       const std::unordered_set<graph::VertexID> &goals);

}  // namespace open_semap
