#include "utils/filter_bounding_box.h"

#include "osmium/io/pbf_input.hpp"
#include "osmium/io/reader_with_progress_bar.hpp"
#include "osmium/io/xml_input.hpp"
#include "osmium/visitor.hpp"
#include "spdlog/spdlog.h"

namespace open_semap {

std::unordered_set<osmium::object_id_type> FilterBoundingBox(
    const std::string& path, const std::unordered_set<osmium::object_id_type>& junctions,
    const osmium::Box& bounding_box) {
  spdlog::info("Filtering based on bounding box.");
  osmium::io::File input_file(path);
  // Only process the ways.
  osmium::io::ReaderWithProgressBar reader(true, input_file,
                                           osmium::osm_entity_bits::node);
  FilterBoundingBoxHandler handler(junctions, bounding_box);
  osmium::apply(reader, handler);
  reader.close();
  return handler.ReleaseJunctions();
}

void FilterBoundingBoxHandler::node(const osmium::Node& node) {
  if (bounding_box.contains(node.location())) {
    if (junctions.get().count(node.id()) > 0) {
      filtered_junctions.emplace(node.id());
    }
  }
}

}  // namespace open_semap
