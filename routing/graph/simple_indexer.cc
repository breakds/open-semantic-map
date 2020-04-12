#include "graph/simple_indexer.h"

#include <memory>

#include "spdlog/spdlog.h"

namespace open_semap {
namespace graph {

const ConnectionInfo *SimpleIndexer::Find(VertexID vertex_id) const {
  auto iter = connections_.find(vertex_id);
  if (iter == connections_.end()) {
    return nullptr;
  }

  return iter->second.get();
}

ConnectionInfo *SimpleIndexer::FindMutable(VertexID vertex_id) {
  auto iter = connections_.find(vertex_id);
  if (iter == connections_.end()) {
    return nullptr;
  }

  return iter->second.get();
}

void SimpleIndexer::AddVertex(const Vertex &vertex) {
  connections_.emplace(vertex.id(), std::make_unique<ConnectionInfo>(vertex));
}

bool SimpleIndexer::AddEdge(const Edge &edge) {
  ConnectionInfo *from = FindMutable(edge.from().id());
  if (from == nullptr) {
    return false;
  }
  ConnectionInfo *to = FindMutable(edge.to().id());
  if (to == nullptr) {
    return false;
  }
  from->outwards.emplace_back(edge);
  to->inwards.emplace_back(edge);
  return true;
}

SimpleIndexer SimpleIndexer::CreateFromRawGraph(const RoadGraph &graph) {
  SimpleIndexer indexer;

  for (const std::unique_ptr<Vertex> &vertex : graph.vertices()) {
    indexer.AddVertex(*vertex);
  }

  for (const std::unique_ptr<Edge> &edge : graph.edges()) {
    if (!indexer.AddEdge(*edge)) {
      spdlog::info("SimpleIndexer cannot AddEdge ({} -> {}).", edge->from().id(),
                   edge->to().id());
    }
  }

  return indexer;
}

}  // namespace graph
}  // namespace open_semap
