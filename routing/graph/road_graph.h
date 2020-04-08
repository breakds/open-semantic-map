#pragma once

#include <memory>
#include <string>
#include <vector>

namespace open_semap {
namespace graph {

class Edge;
class Vertex;

class RoadGraph {
 public:
  static RoadGraph LoadFromFile(const std::string &path);

 private:
  friend class Vertex;
  friend class Edge;

  std::vector<std::unique_ptr<Vertex>> vertices_;
  std::vector<std::unique_ptr<Edge>> edges_;
};

}  // namespace graph
}  // namespace open_semap
