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

const Edge *SimpleIndexer::FindEdge(VertexID from, VertexID to) const {
  const ConnectionInfo *conn = Find(from);
  if (conn == nullptr) {
    return nullptr;
  }
  for (const auto &edge : conn->outwards) {
    if (edge.get().to().id() == to) {
      return &edge.get();
    }
  }
  return nullptr;
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

void SimpleIndexer::RemoveVertex(VertexID vertex_id) {
  auto iter = connections_.find(vertex_id);
  if (iter == connections_.end()) {
    return;
  }

  // NOTE(breakds): If a vertex A has both inward and outward edges connecting
  // this vertex of interest, it will be found twice, which isn't ideal but it
  // is better to avoid such premature optimization.
  for (const auto &edge : iter->second->inwards) {
    VertexID from_id     = edge.get().from().id();
    ConnectionInfo *conn = FindMutable(from_id);

    if (conn == nullptr) {
      continue;
    }

    std::remove_if(conn->outwards.begin(), conn->outwards.end(),
                   [vertex_id](const std::reference_wrapper<const Edge> &e) -> bool {
                     return e.get().to().id() == vertex_id;
                   });
  }

  for (const auto &edge : iter->second->outwards) {
    VertexID to_id       = edge.get().from().id();
    ConnectionInfo *conn = FindMutable(to_id);

    if (conn == nullptr) {
      continue;
    }

    std::remove_if(conn->inwards.begin(), conn->inwards.end(),
                   [vertex_id](const std::reference_wrapper<const Edge> &e) -> bool {
                     return e.get().from().id() == vertex_id;
                   });
  }

  connections_.erase(iter);
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
