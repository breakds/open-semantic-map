#include "graph/road_graph.h"

#include <unordered_map>
#include <utility>

#include "graph/edge.h"
#include "graph/vertex.h"

#include "osmium/osm/location.h"
#include "osmium/osm/node.h"
#include "osmium/osm/types.h"
#include "osmium/osm/way.h"

namespace open_semap {
namespace graph {

struct NodeInfo {
  osmium::object_id_type id;
  osmium::location loc;
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
      non_vertices_.emplace(node.id(), node.location());
    }
  }

  inline std::vector<NodeInfo> ReleaseVertices() {
    return std::move(vertices_);
  }

  inline std::unordered_map<osmium::object_id_type, osmium::location>
  ReleaseNonVertices() {
    return std::move(non_vertices_);
  }

 private:
  std::vector<std::pair<osmium::object_id_type, >> vertices_;
  std::unordered_map<osmium::object_id_type, osmium::location> non_vertices_;
};

RoadGraph RoadGraph::LoadFromFile(const std::string &path) {
}

}  // namespace graph
}  // namespace open_semap
