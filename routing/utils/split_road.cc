#include "utils/split_road.h"

#include "osmium/builder/osm_object_builder.hpp"
#include "osmium/io/pbf_input.hpp"
#include "osmium/io/pbf_output.hpp"
#include "osmium/io/reader_with_progress_bar.hpp"
#include "osmium/io/xml_input.hpp"
#include "osmium/io/xml_output.hpp"
#include "osmium/visitor.hpp"
#include "spdlog/spdlog.h"

#include "utils/predicates.h"

namespace open_semap {

void SplitRoad(const std::string& path, const std::string& output_path,
               const std::unordered_set<osmium::object_id_type>& junctions,
               const std::unordered_set<osmium::object_id_type>& roads) {
  spdlog::info("Splitting roads and generating routing graph.");
  osmium::io::File input_file(path);
  // Process both ways and nodes.
  osmium::io::ReaderWithProgressBar reader(
      true, input_file, osmium::osm_entity_bits::way | osmium::osm_entity_bits::node);

  // Construct the writer.
  osmium::io::Header header = reader.header();
  header.set("generator", "routing_graph");
  osmium::io::Writer writer(output_path, header, osmium::io::overwrite::allow);

  while (osmium::memory::Buffer input_buffer = reader.read()) {
    // Allow output buffer to grow if needed.
    osmium::memory::Buffer output_buffer(input_buffer.committed(),
                                         osmium::memory::Buffer::auto_grow::yes);
    SplitRoadHandler handler(junctions, roads, output_buffer);
    osmium::apply(input_buffer, handler);
    writer(std::move(output_buffer));
  }

  writer.close();
  reader.close();
}

void SplitRoadHandler::node(const osmium::Node& node) {
  if (junctions.get().count(node.id()) > 0) {
    {
      osmium::builder::NodeBuilder builder(output_buffer.get());
      builder.set_id(node.id()).set_uid(node.uid()).set_location(node.location());
      osmium::builder::TagListBuilder tag_builder(builder);
      tag_builder.add_tag("junction", "yes");
    }
    output_buffer.get().commit();
  }
}

void SplitRoadHandler::way(const osmium::Way& way) {
  if (approved_roads.get().count(way.id()) > 0) {
    {
      osmium::builder::WayBuilder builder(output_buffer.get());
      builder.set_id(way.id()).set_uid(way.uid());
      builder.add_item(way.nodes());
    }
    output_buffer.get().commit();
  }
}

}  // namespace open_semap
