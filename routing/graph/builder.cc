#include "graph/builder.h"

namespace open_semap {
namespace graph {

static auto AddVertexIfNotExist(VertexID id,
                                std::vector<std::unique_ptr<Vertex>> *vertices)
    -> const Vertex & {
  auto iter = std::find_if(
      vertices->begin(), vertices->end(),
      [id](const std::unique_ptr<Vertex> &vertex) { return vertex->id() == id; });
  if (iter != vertices->end()) {
    return **iter;
  }
  vertices->emplace_back(std::make_unique<Vertex>(id, osmium::Location()));
  return *vertices->back();
};

RoadGraphBuilder &RoadGraphBuilder::AddEdge(VertexID from, VertexID to, double length) {
  const Vertex &a = AddVertexIfNotExist(from, &vertices_);
  const Vertex &b = AddVertexIfNotExist(to, &vertices_);
  EdgeID edge_id  = static_cast<EdgeID>(edges_.size());
  edges_.emplace_back(std::make_unique<Edge>(edge_id, a, b, length));
  return *this;
}

RoadGraph RoadGraphBuilder::Build() {
  return RoadGraph(std::move(vertices_), std::move(edges_));
}

}  // namespace graph
}  // namespace open_semap
