#include "algorithms/contraction.h"

#include "algorithms/dijkstra.h"

using graph::RoadGraph;
using graph::SimpleIndexer;
using graph::Vertex;
using graph::VertexID;

namespace open_semap {

class ContractionProcessor {
 public:
  ContractionProcessor(SimpleIndexer *indexer) : indexer_(*indexer) {}

  size_t Contract(const graph::Vertex &center_vertex) {
    const ConnectionInfo &conn = indexer_->Find(center_vertex.id());
    std::vector<VertexID> incoming_vertices;
    std::unordered_set<VertexID> outgoing_vertices;

    // Going over all the vertices on the start side, and run Dijkstra to try to
    // reach the vertices on the other side. All the shortest path that uses
    // only the center vertex is going to be contracted.
    for (VertexID start : incoming_vertices) {
      // NOTE(breakds): There is no chance that the outgoing vertices
      // can not be reached from start. This is because any of the
      // starts can at least reach all of the outgoing vertices via
      // center vertex (the input to this function).
      SearchTree tree = RunDijkstra(*indexer_, start, outgoing_vertices);

      const SearchNode *center = tree.Find(center_vertex.id());
      // If the center vertex does not even appear in the search tree, no
      // shortcut can be added for this start vertex. Or, if the center vertex
      // isn't directly reached by the start vertex, no shortcut as well.
      if (center == nullptr || center->edge().from().id() != start) {
        continue;
      }

      // Add shortcut for via-center reached goals.
      for (VertexID goal_id : outgoing_vertices) {
        const SearchNode *goal = tree.Find(goal_id);
        if (goal == nullptr) {
          continue;
        }
        if (goal->edge().from.id() == center_vertex.id()) {
          // In this case, start -> center -> goal appear in the Dijkstra search
          // tree, suggesting a valid contraction.
          shortcuts_.emplace_back(
              std::make_unique<Edge>(some_id, center->edge(), goal->edge()));
          indexer_->AddEdge(*shortcuts_.back());
        }
      }
    }

    // Now, it is time to remove the center vertex from the indexer.
    indexer_.RemoveVertex(center_vertex.id());
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

}  // namespace open_semap
