#include "utils/extract_junction.h"

#include "osmium/io/pbf_input.hpp"
#include "osmium/io/reader_with_progress_bar.hpp"
#include "osmium/io/xml_input.hpp"
#include "osmium/visitor.hpp"

#include "utils/predicates.h"

namespace open_semap {

std::unordered_set<osmium::object_id_type> ExtractJunction(const std::string& path) {
  osmium::io::File input_file(path);
  // Only process the ways.
  osmium::io::ReaderWithProgressBar reader(true, input_file,
                                           osmium::osm_entity_bits::way);
  ExtractJunctionHandler extract_junction;
  osmium::apply(reader, extract_junction);
  reader.close();
  return extract_junction.ReleaseJunctions();
}

void ExtractJunctionHandler::way(const osmium::Way& way) {
  if (!predicate::IsValidRoad(way)) {
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

}  // namespace open_semap
