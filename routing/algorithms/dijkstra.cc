#include "algorithms/dijkstra.h"

#include <algorithm>
#include <cstdio>
#include <unordered_map>

#include "graph/simple_indexer.h"
#include "spdlog/spdlog.h"

namespace open_semap {

using graph::ConnectionInfo;
using graph::Edge;
using graph::SimpleIndexer;
using graph::Vertex;
using graph::VertexID;

const SearchNode *SearchTree::Find(VertexID vertex_id) const {
  auto iter = nodes_.find(vertex_id);
  if (iter == nodes_.end()) {
    return nullptr;
  }
  return &iter->second;
}

SearchTree RunDijkstra(const SimpleIndexer &indexer, VertexID start,
                       const std::unordered_set<VertexID> &goals) {
  SearchTree tree(start);

  // Before a vertex is finalized and inserted to the search tree, its
  // current best cost (score) is stored in the scoreboard. The
  // scoreboard is an unordered_map so that it can be queried against
  // the VertexID.
  std::unordered_map<VertexID, double> scoreboard;
  // NOTE(breakds): if later SearchNode becomes significantly larger, we can
  // consider make this vector<unique_ptr<SearchNode>> instead.
  std::vector<SearchNode> q;

  VertexID current    = start;
  double current_cost = 0.0;

  size_t num_goals = goals.size();
  size_t hit_goals = 0;

  do {
    const ConnectionInfo *conn = indexer.Find(current);

    if (conn == nullptr) {
      spdlog::critical("RunDijkstra(): Cannot find vertex with ID = {}", current);
      std::abort();
    }

    spdlog::debug("Evaluating Vertex {}", current);

    for (const auto &edge_ref : conn->outwards) {
      spdlog::debug("  Check edge {} -> {}", edge_ref.get().from().id(),
                    edge_ref.get().to().id());

      double edge_cost     = edge_ref.get().cost();
      VertexID neighbor_id = edge_ref.get().to().id();

      // Case I: The neighbor vertex is already finalized (i.e. it is already in
      // the search tree). In this case, we can skip relaxing it.
      if (tree.Has(neighbor_id)) {
        continue;
      }

      auto iter = scoreboard.find(neighbor_id);

      // Case II: If this is the first time it reaches this neighbor vertex, or
      // if this vertex can now be relaxed with a better cost, do it.
      if (iter == scoreboard.end() || current_cost + edge_cost < iter->second) {
        double updated_cost     = current_cost + edge_cost;
        scoreboard[neighbor_id] = updated_cost;
        spdlog::debug("    Update {} with cost {}", neighbor_id, updated_cost);
        // NOTE(breakds) that we lazily add the corresponding SearchNode to the
        // search tree. There may have already been SearchNode with the same ID,
        // but we can be sure that the new SearchNdoe has a better cost, so that
        // is will have a better position in the heap.
        //
        // Lazy update is mainly to avoid having to implement heap update.
        //
        // Although this may make the algorithm more expensive, but since the
        // RoadGraph is sparse, each vertex won't have more than O(1) edges, and
        // each vertex can only be relaxed by at most [number of edges] times,
        // this is fine.
        q.emplace_back(updated_cost, edge_ref.get());
        std::push_heap(q.begin(), q.end());
      }
    }

    bool can_continue = false;

    // Normally we just need to the get the top of the heap to go on. However,
    // since we are doing lazy update, the top may be a duplicate vertex that
    // has already been finalized. In this case, keep popping until we get a new
    // vertex or the heap is empty.
    while (!q.empty()) {
      std::pop_heap(q.begin(), q.end());
      SearchNode elected = q.back();
      q.pop_back();
      if (!tree.Has(elected.vertex().id())) {
        can_continue = true;
        current      = elected.vertex().id();
        current_cost = elected.cost();
        tree.Emplace(elected);
        spdlog::debug("  Elected {} with cost {}", current, current_cost);
        break;
      }
    }

    // This marks the end of the algorithm (i.e. exhaust of the graph).
    if (!can_continue) {
      break;
    }

    if (goals.count(current) > 0) {
      ++hit_goals;
      if (hit_goals == num_goals) {
        break;
      }
    }

  } while (true);

  return tree;
}

}  // namespace open_semap
