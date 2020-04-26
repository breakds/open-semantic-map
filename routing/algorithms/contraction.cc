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

class ContractionProcessor {
 public:
  ContractionProcessor(SimpleIndexer *indexer) : indexer_(*indexer) {}

  size_t Contract(const graph::Vertex &center_vertex) {
    const ConnectionInfo *conn = indexer_.get().Find(center_vertex.id());
    if (conn == nullptr) {
      return 0;
    }

    std::unordered_set<graph::VertexID> goal_ids;
    for (const auto &outgoing : conn->outwards) {
      goal_ids.emplace(outgoing.get().to().id());
    }

    size_t num_existing_shortcuts = shortcuts_.size();

    // Going over all the vertices on the start side, and run Dijkstra to try to
    // reach the vertices on the other side. All the shortest path that uses
    // only the center vertex is going to be contracted.
    for (const auto &incoming : conn->inwards) {
      VertexID start = incoming.get().from().id();

      // NOTE(breakds): There is no chance that the outgoing vertices
      // can not be reached from start. This is because any of the
      // starts can at least reach all of the outgoing vertices via
      // center vertex (the input to this function).
      SearchTree tree          = RunDijkstra(indexer_.get(), start, goal_ids);
      const SearchNode *center = tree.Find(center_vertex.id());
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
        if (goal->edge().from().id() == center_vertex.id()) {
          // In this case, start -> center -> goal appear in the Dijkstra search
          // tree, suggesting a valid contraction.
          shortcuts_.emplace_back(std::make_unique<Edge>(center->edge(), goal->edge()));
          // NOTE(breakds): This may coexist with the origianl
          // non-shortcut edge.
          indexer_.get().AddEdge(*shortcuts_.back());
        }
      }
    }

    // Now, it is time to remove the center vertex from the indexer.
    indexer_.get().RemoveVertex(center_vertex.id());
    return shortcuts_.size() - num_existing_shortcuts;
  }

  std::vector<std::unique_ptr<Edge>> ReleaseShortcuts() {
    std::vector<std::unique_ptr<Edge>> result = std::move(shortcuts_);
    return result;
  }

 private:
  std::reference_wrapper<SimpleIndexer> indexer_;
  std::vector<std::unique_ptr<Edge>> shortcuts_;
};

std::vector<std::unique_ptr<Edge>> ContractGraph(
    const std::vector<std::reference_wrapper<const Vertex>> &ordered_vertices,
    SimpleIndexer *indexer) {
  ContractionProcessor processor(indexer);

  for (const auto &vertex : ordered_vertices) {
    processor.Contract(vertex.get());
  }

  return processor.ReleaseShortcuts();
}

std::vector<std::unique_ptr<Edge>> ContractGraph(
    const std::vector<graph::VertexID> &ordered_vertex_ids, SimpleIndexer *indexer) {
  std::vector<std::reference_wrapper<const Vertex>> ordered_vertices;
  for (VertexID id : ordered_vertex_ids) {
    const ConnectionInfo *conn = indexer->Find(id);
    if (conn == nullptr) {
      return {};
    }
    ordered_vertices.emplace_back(conn->vertex);
  }
  return ContractGraph(ordered_vertices, indexer);
}

}  // namespace open_semap
