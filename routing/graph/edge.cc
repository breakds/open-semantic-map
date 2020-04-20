#include "graph/edge.h"

namespace open_semap {
namespace graph {

Edge::Edge(osmium::object_id_type id, const Vertex &from, const Vertex &to, double length)
    : id_(id), from_(from), to_(to), length_(length) {}

Edge::Edge(osmium::object_id_type id, const Vertex &from, const Vertex &to)
    : Edge(id, from, to, 0.0) {}

}  // namespace graph
}  // namespace open_semap
