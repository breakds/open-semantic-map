#include "graph/road_graph.h"

#include <unordered_map>

#include "osmium/geom/haversine.hpp"
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

using VertexMap = std::unordered_map<VertexID, std::reference_wrapper<const Vertex>>;

class NodeLoaderHandler : public osmium::handler::Handler {
 public:
  NodeLoaderHandler() = default;

  void node(const osmium::Node &node) {
    // Take a look at the "vertex" tag of the node and decide where to
    // put it.
    const char *vertex_tag = node.tags()["vertex"];
    bool is_vertex = !(vertex_tag == nullptr || std::strcmp(vertex_tag, "no") == 0);

    if (is_vertex) {
      vertices_.emplace_back(std::make_unique<Vertex>(node.id(), node.location()));
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

VertexMap GenerateIdToVertexMap(const std::vector<std::unique_ptr<Vertex>> &vertices) {
  VertexMap result;
  for (const std::unique_ptr<Vertex> &vertex : vertices) {
    result.emplace(vertex->id(), *vertex);
  }
  return result;
}

struct EdgeInfo {
  EdgeInfo(osmium::object_id_type id_) : id(id_) {}

  osmium::object_id_type id;
  std::vector<osmium::object_id_type> point_ids{};
};

class WayLoaderHandler : public osmium::handler::Handler {
 public:
  WayLoaderHandler() = default;

  void way(const osmium::Way &way) {
    edges_.emplace_back(way.id());
    for (const auto &node : way.nodes()) {
      edges_.back().point_ids.emplace_back(node.ref());
    }
  }

  void Reset() { edges_.clear(); }

  const std::vector<EdgeInfo> &Edges() const { return edges_; }

 private:
  std::vector<EdgeInfo> edges_{};
};

const Vertex &QueryVertex(
    VertexID id, const std::unordered_map<VertexID, std::reference_wrapper<const Vertex>>
                     &id_to_vertex) {
  static Vertex INVALID_VERTEX(-1, osmium::Location(0.0, 0.0));
  auto iter = id_to_vertex.find(id);
  if (iter != id_to_vertex.end()) {
    return iter->second.get();
  }
  return INVALID_VERTEX;
}

RoadGraph RoadGraph::LoadFromFile(const std::string &path) {
  spdlog::info("Loading the road graph.");

  RoadGraph graph;
  std::unordered_map<osmium::object_id_type, osmium::Location> node_map;
  VertexMap vertex_map;

  // Load nodes (vertices + imtermediate points)
  {
    osmium::io::File input_file(path);
    osmium::io::ReaderWithProgressBar reader(true, input_file,
                                             osmium::osm_entity_bits::node);
    NodeLoaderHandler handler;
    osmium::apply(reader, handler);
    graph.vertices_ = handler.ReleaseVertices();
    vertex_map      = GenerateIdToVertexMap(graph.vertices_);
    node_map        = handler.ReleaseNodeMap();
    reader.close();
  }

  // Load ways (edges)
  {
    osmium::io::File input_file(path);
    osmium::io::ReaderWithProgressBar reader(true, input_file,
                                             osmium::osm_entity_bits::way);
    WayLoaderHandler handler;

    while (osmium::memory::Buffer buffer = reader.read()) {
      handler.Reset();
      osmium::apply(buffer, handler);
      for (const auto &edge_info : handler.Edges()) {
        if (edge_info.point_ids.size() < 2) {
          spdlog::warn("Skipping as edge {} has {} points, which is less than 2.",
                       edge_info.id, edge_info.point_ids.size());
          continue;
        }
        // Construct edges and fill edge information in vertices. This
        // exploits the friendship between RoadGraph and Edge/Vertex.
        const Vertex &from = QueryVertex(edge_info.point_ids.front(), vertex_map);
        const Vertex &to   = QueryVertex(edge_info.point_ids.back(), vertex_map);
        if (from.id() == -1 || to.id() == -1) {
          spdlog::critical("Cannot find vertex {} or {}.", edge_info.point_ids.front(),
                           edge_info.point_ids.back());
        }
        graph.edges_.emplace_back(std::make_unique<Edge>(from, to));
        graph.edges_.back()->points_.emplace_back(from.loc());

        osmium::Location prev_loc = from.loc();
        for (size_t i = 1; i + 1 < edge_info.point_ids.size(); ++i) {
          auto iter = node_map.find(edge_info.point_ids[i]);
          if (iter == node_map.end()) {
            spdlog::critical("Cannot find point with node id = {}",
                             edge_info.point_ids[i]);
          }
          graph.edges_.back()->points_.emplace_back(iter->second);
          graph.edges_.back()->length_ +=
              osmium::geom::haversine::distance(prev_loc, iter->second);
          prev_loc = iter->second;
        }
        graph.edges_.back()->points_.emplace_back(to.loc());
        graph.edges_.back()->length_ +=
            osmium::geom::haversine::distance(prev_loc, to.loc());
      }
    }

    reader.close();
  }

  return graph;
}

const std::vector<std::unique_ptr<Vertex>> &RoadGraph::vertices() const {
  return vertices_;
}

const std::vector<std::unique_ptr<Edge>> &RoadGraph::edges() const { return edges_; }

}  // namespace graph
}  // namespace open_semap
