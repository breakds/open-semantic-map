#pragma once

namespace open_semap {
namespace graph {

struct ContractionInfo {
  std::vector<VertexID> ordered_vertex_ids;
  std::vector<std::unique_ptr<graph::Edge>> shortcuts;

  // For persistent the constractoin information onto and from disk.
  void Save(const std::string &path) const;
  static ContractionInfo Load(const std::string &path);
};

}  // namespace graph
}  // namespace open_semap
