#include "utils/filter_isolated_roads.h"

#include "osmium/io/pbf_input.hpp"
#include "osmium/io/reader_with_progress_bar.hpp"
#include "osmium/io/xml_input.hpp"
#include "osmium/visitor.hpp"
#include "spdlog/spdlog.h"

#include "utils/predicates.h"

namespace open_semap {

std::unordered_set<osmium::object_id_type> FilterIsolatedRoads(
    const std::string& path,
    const std::unordered_set<osmium::object_id_type>& junctions) {
  spdlog::info("Prcoessing ways to filter out isolated roads.");
  osmium::io::File input_file(path);
  // Only process the ways.
  osmium::io::ReaderWithProgressBar reader(true, input_file,
                                           osmium::osm_entity_bits::way);
  FilterIsolatedRoadsHandler filter(junctions);
  osmium::apply(reader, filter);
  reader.close();
  return filter.ReleaseResult();
}

void FilterIsolatedRoadsHandler::way(const osmium::Way& way) {
  if (!predicate::IsValidRoad(way)) {
    return;
  }

  for (const auto& node : way.nodes()) {
    if (junctions.get().count(node.ref()) > 0) {
      inter_connected_roads.emplace(way.id());
      break;
    }
  }
}

}  // namespace open_semap
