#include "algorithms/contraction.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "algorithms/dijkstra.h"
#include "graph/edge.h"
#include "graph/road_graph.h"
#include "graph/simple_indexer.h"
#include "graph/vertex.h"
#include "spdlog/spdlog.h"

namespace open_semap {

using graph::ConnectionInfo;
using graph::Edge;
using graph::RoadGraph;
using graph::SimpleIndexer;
using graph::Vertex;
using graph::VertexID;

struct SingleContractionPlan {
  // The connection info about the vertex of interest (called the center). The
  // plan itself is about removing the center vertex and adding a bunch of
  // shortcuts so that all shortest paths are preserved for the other vertices.
  const ConnectionInfo *center = nullptr;
  std::vector<
      std::pair<std::reference_wrapper<const Edge>, std::reference_wrapper<const Edge>>>
      planned;

  // Returns the number of shortcuts generated.
  size_t CarryOut(SimpleIndexer *indexer, std::vector<std::unique_ptr<Edge>> *shortcuts) {
    if (center == nullptr) {
      // Do not carry a plan twice. When a plan is carried out, center will be
      // set to nullptr.
      return 0;
    }

    for (const auto &item : planned) {
      shortcuts->emplace_back(
          std::make_unique<Edge>(item.first.get(), item.second.get()));
      // NOTE(breakds): This may coexist with the origianl non-shortcut edge.
      indexer->AddEdge(*shortcuts->back());
    }
    indexer->RemoveVertex(center->vertex.get().id());
    size_t num_generated = planned.size();

    // Clear the plan.
    center = nullptr;
    planned.clear();
    return num_generated;
  }

  // Returns the net increase of number of edges if the plan is carried out.
  //
  // This equals to # added shortcuts - # original edges. Note that the number
  // can be negative.
  double ComputeEdgeDifference() const {
    if (center == nullptr) {
      return 0;
    }
    return static_cast<double>(planned.size()) -
           static_cast<double>(center->inwards.size() + center->outwards.size());
  }
};

static SingleContractionPlan DryRunContraction(const SimpleIndexer &indexer,
                                               const ConnectionInfo &conn) {
  VertexID center_vertex_id = conn.vertex.get().id();

  SingleContractionPlan plan;
  plan.center = &conn;

  std::unordered_set<graph::VertexID> goal_ids;
  for (const auto &outgoing : conn.outwards) {
    goal_ids.emplace(outgoing.get().to().id());
  }

  // Going over all the vertices on the start side, and run Dijkstra to try to
  // reach the vertices on the other side. All the shortest path that uses
  // only the center vertex is going to be contracted.
  for (const auto &incoming : conn.inwards) {
    VertexID start = incoming.get().from().id();

    // NOTE(breakds): There is no chance that the outgoing vertices
    // can not be reached from start. This is because any of the
    // starts can at least reach all of the outgoing vertices via
    // center vertex (the input to this function).
    SearchTree tree          = RunDijkstra(indexer, start, goal_ids);
    const SearchNode *center = tree.Find(center_vertex_id);
    // If the center vertex does not even appear in the search tree, no
    // shortcut can be added for this start vertex. Or, if the center vertex
    // isn't directly reached by the start vertex, no shortcut as well.
    if (center == nullptr || center->edge().from().id() != start) {
      continue;
    }

    // Add shortcut for via-center reached goals.
    for (VertexID goal_id : goal_ids) {
      const SearchNode *goal = tree.Find(goal_id);
      if (goal == nullptr) {
        continue;
      }
      if (goal->edge().from().id() == center_vertex_id) {
        // In this case, start -> center -> goal appear in the Dijkstra search
        // tree, suggesting a valid shortcut.
        plan.planned.emplace_back(center->edge(), goal->edge());
      }
    }
  }

  return plan;
}

std::vector<std::unique_ptr<Edge>> ContractVertices(
    const std::vector<graph::VertexID> &ordered_vertex_ids, SimpleIndexer *indexer) {
  std::vector<std::unique_ptr<Edge>> shortcuts;

  for (VertexID id : ordered_vertex_ids) {
    const ConnectionInfo *conn = indexer->Find(id);
    if (conn != nullptr) {
      SingleContractionPlan plan = DryRunContraction(*indexer, *conn);
      plan.CarryOut(indexer, &shortcuts);
    }
  }

  return shortcuts;
}

static constexpr double SIGNIFICANT_SCORE_DIFF  = 0.5;
static constexpr double ANTI_CLUSTERING_PENALTY = 0.4;

struct RankingInfo {
  RankingInfo(const ConnectionInfo *conn_, double edge_diff_, double extra_)
      : conn(conn_), edge_diff(edge_diff_), extra(extra_) {}

  RankingInfo(const ConnectionInfo *conn_, double edge_diff_)
      : conn(conn_), edge_diff(edge_diff_) {}

  RankingInfo(const RankingInfo &) = default;

  inline double score() const { return edge_diff + extra; }

  inline bool operator<(const RankingInfo &other) const {
    return score() > other.score();
  }

  const ConnectionInfo *conn = nullptr;
  double edge_diff           = 0.0;
  double extra               = 0.0;
};  // namespace open_semap

void FetchNeighborVertexIDs(const ConnectionInfo &conn,
                            std::vector<VertexID> *neighbors) {
  neighbors->clear();
  for (const auto &item : conn.inwards) {
    neighbors->emplace_back(item.get().from().id());
  }
  for (const auto &item : conn.outwards) {
    neighbors->emplace_back(item.get().to().id());
  }
}

std::vector<std::unique_ptr<graph::Edge>> ContractGraph(graph::SimpleIndexer *indexer,
                                                        bool print_debug_info) {
  if (print_debug_info && indexer->connections().size() > 20) {
    spdlog::warn(
        "ContractGraph: You asked to print debug info but the graph is too big to do "
        "so. Will ignore 'print_debug_info'.");
  }

  // The owner of the created shortcuts. Will be returned and
  // transferred to the caller.
  std::vector<std::unique_ptr<Edge>> shortcuts;

  // A priority queue that ranks the remaining vertices by their
  // score. It gives the next vertex to contract.
  //
  // FIXME: In the future we might want to do partial ranking to avoid
  // rank every vertex together and save the memory.
  std::vector<RankingInfo> rankings;
  rankings.reserve(indexer->connections().size());

  // Unlike `rankings`, scoreboard is not a priority queue. However,
  // it has better up-to-date information than in the priority queue.
  // When a vertex is contracted, its neighbors will be updated (with
  // a worse score). The updated score will only be in the scoreboard,
  // but not in the `rankings` (because updating the heap is costly).
  // However, when a vertex is popped from the heap, it will have to
  // be compared with the scoreboard.
  std::unordered_map<VertexID, RankingInfo> scoreboard;

  // Initialize the rankings with the edege difference of each vertex.
  for (const auto &item : indexer->connections()) {
    const ConnectionInfo *conn = item.second.get();
    SingleContractionPlan plan = DryRunContraction(*indexer, *conn);
    double edge_diff           = plan.ComputeEdgeDifference();
    rankings.emplace_back(conn, edge_diff);
    scoreboard.emplace(std::piecewise_construct,
                       std::forward_as_tuple(conn->vertex.get().id()),
                       std::forward_as_tuple(conn, edge_diff));
  }
  std::make_heap(rankings.begin(), rankings.end());

  std::vector<VertexID> neighbors;

  while (!rankings.empty()) {
    std::pop_heap(rankings.begin(), rankings.end());
    VertexID center_vertex_id = rankings.back().conn->vertex.get().id();
    double current_score      = rankings.back().score();
    rankings.pop_back();

    if (print_debug_info) {
      spdlog::info("Contracting Vertex {}", center_vertex_id);
      for (const auto &entry : scoreboard) {
        spdlog::info("  - {}: {}", entry.first, entry.second.score());
      }
    }

    auto center_iter = scoreboard.find(center_vertex_id);
    if (center_iter == scoreboard.end()) {
      spdlog::critical(
          "ContractGraph: Vertex {} is not contracted but cannot be found in scoreboard.",
          center_vertex_id);
    }

    if (current_score + SIGNIFICANT_SCORE_DIFF < center_iter->second.score() &&
        // If there are only 2 or less vertices left (ranking's size
        // would be 1), do not do the update trick as the localization
        // heuristic won't work anyway.
        rankings.size() > 1) {
      if (print_debug_info) {
        spdlog::info("  Re-insert vertex {} with worse score {}.", center_vertex_id,
                     center_iter->second.score());
      }
      rankings.emplace_back(center_iter->second);
      std::push_heap(rankings.begin(), rankings.end());
      continue;
    }

    FetchNeighborVertexIDs(*center_iter->second.conn, &neighbors);

    SingleContractionPlan plan = DryRunContraction(*indexer, *center_iter->second.conn);
    plan.CarryOut(indexer, &shortcuts);
    if (print_debug_info) {
      spdlog::info("  Carry out plan on vertex {}. There are {} shortcuts now.",
                   center_vertex_id, shortcuts.size());
    }
    scoreboard.erase(center_vertex_id);

    // Now, update all the neighbors as center is now gone since the
    // plan is carried out.
    for (VertexID neighbor_id : neighbors) {
      auto item = scoreboard.find(neighbor_id);
      if (item == scoreboard.end()) {
        spdlog::warn("ContractGraph: The neighbor {} of {} is already gone.", neighbor_id,
                     center_vertex_id);
        continue;
      }
      item->second.extra += ANTI_CLUSTERING_PENALTY;
      SingleContractionPlan new_plan = DryRunContraction(*indexer, *item->second.conn);
      item->second.edge_diff         = new_plan.ComputeEdgeDifference();
    }
  }

  return shortcuts;
}

}  // namespace open_semap
