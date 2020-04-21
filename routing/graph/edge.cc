#include "graph/edge.h"

namespace open_semap {
namespace graph {

Edge::Edge(osmium::object_id_type id, const Vertex &from, const Vertex &to, double length)
    : id_(id), from_(from), to_(to), length_(length) {}

Edge::Edge(osmium::object_id_type id, const Vertex &from, const Vertex &to)
    : Edge(id, from, to, 0.0) {}

// Special constructor to create a shortcut edge.
Edge::Edge(osmium::object_id_type id, const Edge &a, const Edge &b)
    : id_(id),
      from_(a.from()),
      to_(b.to()),
      length_(a.cost() + b.cost()),
      car_(&a),
      con_(&b) {}

}  // namespace graph
}  // namespace open_semap
