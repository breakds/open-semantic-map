#include "utils/extract_junction.h"

#include "spdlog/spdlog.h"

#include "osmium/io/pbf_input.hpp"
#include "osmium/io/reader_with_progress_bar.hpp"
#include "osmium/io/xml_input.hpp"
#include "osmium/visitor.hpp"

namespace open_semap {

static bool IsValidRoad(const osmium::Way& way);

std::unordered_set<osmium::object_id_type> ExtractJunction(const std::string& path) {
  osmium::io::File input_file(path);
  osmium::io::ReaderWithProgressBar reader(true, input_file);
  ExtractJunctionHandler extract_junction;
  osmium::apply(reader, extract_junction);
  reader.close();
  return extract_junction.ReleaseJunctions();
}

void ExtractJunctionHandler::way(const osmium::Way& way) {
  if (!IsValidRoad(way)) {
    return;
  }

  for (const auto& node : way.nodes()) {
    if (visited.count(node.ref()) > 0) {
      junctions.emplace(node.ref());
    } else {
      visited.emplace(node.ref());
    }
  }
}

bool IsValidRoad(const osmium::Way& way) {
  const char* highway = way.tags()["highway"];

  // If there is no highway tag, this Way object does not represent a
  // road and we do not care about it.
  if (highway == nullptr) {
    return false;
  }

  // Crosswalks and footways are not valid roads.
  if (strcmp(highway, "footway") == 0) {
    return false;
  }

  // See https://wiki.openstreetmap.org/wiki/Tag:highway%3Dservice
  if (strcmp(highway, "service") == 0) {
    return false;
  }

  return true;
}

}  // namespace open_semap
