#include "algorithms/contraction.h"

using graph::RoadGraph;
using graph::SimpleIndexer;

namespace open_semap {

class ContractionProcessor {
 public:
  ContractionProcessor(SimpleIndexer *indexer) : indexer_(*indexer) {}

  size_t Contract(const graph::Vertex &vertex) {
    const ConnectionInfo &conn = indexer_->Find(vertex.id());
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
