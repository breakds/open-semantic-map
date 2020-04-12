#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

#include "graph/defs.h"
#include "graph/edge.h"
#include "graph/road_graph.h"
#include "graph/vertex.h"

namespace open_semap {
namespace graph {

struct ConnectionInfo {
  ConnectionInfo(const Vertex &v) : vertex(v) {}

  std::reference_wrapper<const Vertex> vertex;
  std::vector<std::reference_wrapper<const Edge>> inwards{};
  std::vector<std::reference_wrapper<const Edge>> outwards{};
};

class SimpleIndexer {
 public:
  static SimpleIndexer CreateFromRawGraph(const RoadGraph &graph);

  SimpleIndexer() = default;

  SimpleIndexer(SimpleIndexer &&) noexcept = default;
  SimpleIndexer &operator=(SimpleIndexer &&) noexcept = default;

  inline const std::unordered_map<VertexID, std::unique_ptr<ConnectionInfo>>
      &connections() const {
    return connections_;
  }

  // Returns nullptr if the corresponding connection info of the
  // vertex of the input ID cannot be found.
  const ConnectionInfo *Find(VertexID vertex_id) const;

  // ==================== Mutable APIs ====================

  ConnectionInfo *FindMutable(VertexID vertex_id);

  void AddVertex(const Vertex &vertex);

  // Returns false if it cannot find the from and to vertices'
  // corresponding connections already in the indexer.
  bool AddEdge(const Edge &edge);

 private:
  std::unordered_map<VertexID, std::unique_ptr<ConnectionInfo>> connections_{};
};

}  // namespace graph
}  // namespace open_semap
