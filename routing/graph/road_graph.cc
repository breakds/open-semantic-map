#include "graph/road_graph.h"

#include <utility>

#include "graph/edge.h"
#include "graph/vertex.h"

#include "osmium/handler.hpp"
#include "osmium/io/pbf_input.hpp"
#include "osmium/io/reader_with_progress_bar.hpp"
#include "osmium/io/xml_input.hpp"
#include "osmium/osm/location.hpp"
#include "osmium/osm/node.hpp"
#include "osmium/osm/types.hpp"
#include "osmium/osm/way.hpp"
#include "osmium/visitor.hpp"

#include "spdlog/spdlog.h"

namespace open_semap {
namespace graph {

struct NodeInfo {
  NodeInfo(osmium::object_id_type id_, osmium::Location loc_) : id(id_), loc(loc_) {
  }

  osmium::object_id_type id;
  osmium::Location loc;
};

class NodeLoaderHandler : public osmium::handler::Handler {
 public:
  NodeLoaderHandler() = default;

  void node(const osmium::Node &node) {
    // Take a look at the "vertex" tag of the node and decide where to
    // put it.
    const char *vertex_tag = node.tags()["vertex"];
    bool is_vertex = !(vertex_tag == nullptr || std::strcmp(vertex_tag, "no") == 0);

    if (is_vertex) {
      vertices_.emplace_back(node.id(), node.location());
    } else {
      node_map_.emplace(node.id(), node.location());
    }
  }

  inline std::vector<std::unique_ptr<Vertex>> ReleaseVertices() {
    return std::move(vertices_);
  }

  inline std::unordered_map<osmium::object_id_type, osmium::Location> ReleaseNodeMap() {
    return std::move(node_map_);
  }

 private:
  std::vector<std::unique_ptr<Vertex>> vertices_;
  std::unordered_map<osmium::object_id_type, osmium::Location> node_map_;
};

std::unordered_map<RoadGraph::VertexID, std::reference_wrapper<Vertex>>
GenerateIdToVertexMap(const std::vector<std::unique_ptr<Vertex>> &vertices) {
  std::unordered_map<RoadGraph::VertexID, std::reference_wrapper<Vertex>> result;
  for (const std::unique_ptr<Vertex> &vertex : vertices) {
    result.emplace(vertex->id(), *vertex);
  }
  return result;
}

RoadGraph RoadGraph::LoadFromFile(const std::string &path) {
  spdlog::info("Loading the road graph.");

  RoadGraph graph;
  std::unordered_map<osmium::object_id_type, osmium::Location> node_map;

  {
    osmium::io::File input_file(path);
    osmium::io::ReaderWithProgressBar reader(true, input_file,
                                             osmium::osm_entity_bits::node);
    NodeLoaderHandler handler;
    osmium::apply(handler, reader);
    graph.vertices_     = handler.ReleaseVertices();
    graph.id_to_vertex_ = GenerateIdToVertexMap(graph.vertices_);
    node_map            = handler.ReleaseNodeMap();
    reader.close();
  }

  return graph;
}

}  // namespace graph
}  // namespace open_semap
