#include "algorithms/dijkstra.h"

#include <algorithm>

using graph::ConnectionInfo;
using graph::SimpleIndexer;
using graph::Vertex;
using graph::VertexID;

namespace open_semap {

SearchTree RunDijkstra(const SimpleIndexer &indexer, VerticeID start,
                       const std::unordered_set<VertexID> &goals) {
  SearchTree tree(start);

  // Before a vertex is finalized and inserted to the search tree, its
  // current best cost (score) is stored in the scoreboard. The
  // scoreboard is an unordered_map so that it can be queried against
  // the VertexID.
  std::unordered_set<VertexID, double> scoreboard;
  std::vector<std::unique_ptr<SearchNode>> q;

  VertexID current    = start;
  double current_cost = 0.0;

  size_t num_goals = goals.size();
  size_t hit_goals = 0;

  do {
    const ConnectionInfo &conn = indexer.Find(current);

    for (const auto &edge_ref : conn.outwards) {
      const Edge &edge = edge_ref.get();
      if (tree.Has(edge.to())) {
        continue;
      }

      auto iter = scoreboard.find(edge.to());

      if (iter == scoreboard.end() || current_cost + edge.cost() < iter.second) {
        double updated_cost   = current_cost + edge.cost();
        scoreboard[edge.to()] = updated_cost;
        q.emplace_back(std::make_unique<SearchNode>(updated_cost, edge));
        std::push_heap(q.begin(), q.end());
      }
    }

    if (q.empty()) {
      break;
    }

    std::pop_heap(q.begin(), q.end());
    current      = q.back()->vertex().id();
    current_cost = q.back()->cost();
    tree.Emplace(std::move(q.back()));
    q.pop_back();

    if (goals.count(current) > 0) {
      ++hit_goals;
      if (hit_goals == num_goals) {
        break;
      }
    }

  } while (!q.empty());

  return tree;
}

}  // namespace open_semap
