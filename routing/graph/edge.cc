#include "graph/edge.h"

namespace open_semap {
namespace graph {

Edge::Edge(osmium::object_id_type id, const Vertex &from, const Vertex &to)
    : id_(id), from_(from), to_(to) {
}

}  // namespace graph
}  // namespace open_semap
