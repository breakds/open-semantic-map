#include "algorithms/contraction.h"

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
  int64_t EdgeDifference() const {
    if (center == nullptr) {
      return 0;
    }
    return static_cast<int64_t>(planned.size()) -
           static_cast<int64_t>(center->inwards.size() + center->outwards.size());
  }
};

static SingleContractionPlan DryRunContraction(const SimpleIndexer &indexer,
                                               VertexID center_vertex_id) {
  const ConnectionInfo *conn = indexer.Find(center_vertex_id);
  if (conn == nullptr) {
    return SingleContractionPlan();
  }

  SingleContractionPlan plan;
  plan.center = conn;

  std::unordered_set<graph::VertexID> goal_ids;
  for (const auto &outgoing : conn->outwards) {
    goal_ids.emplace(outgoing.get().to().id());
  }

  // Going over all the vertices on the start side, and run Dijkstra to try to
  // reach the vertices on the other side. All the shortest path that uses
  // only the center vertex is going to be contracted.
  for (const auto &incoming : conn->inwards) {
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
    SingleContractionPlan plan = DryRunContraction(*indexer, id);
    plan.CarryOut(indexer, &shortcuts);
  }

  return shortcuts;
}

}  // namespace open_semap
