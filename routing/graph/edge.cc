#include "graph/edge.h"

namespace open_semap {
namespace graph {

Edge::Edge(const Vertex &from, const Vertex &to, double length)
    : from_(from), to_(to), length_(length) {}

Edge::Edge(const Vertex &from, const Vertex &to) : Edge(from, to, 0.0) {}

// Special constructor to create a shortcut edge.
Edge::Edge(const Edge &a, const Edge &b)
    : from_(a.from()), to_(b.to()), length_(a.cost() + b.cost()), car_(&a), cdr_(&b) {}

}  // namespace graph
}  // namespace open_semap
