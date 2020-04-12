#include "utils/split_road.h"

#include <vector>

#include "osmium/builder/osm_object_builder.hpp"
#include "osmium/io/pbf_input.hpp"
#include "osmium/io/pbf_output.hpp"
#include "osmium/io/reader_with_progress_bar.hpp"
#include "osmium/io/xml_input.hpp"
#include "osmium/io/xml_output.hpp"
#include "osmium/visitor.hpp"
#include "spdlog/spdlog.h"

namespace open_semap {

void SplitRoad(const std::string& path, const std::string& output_path,
               const std::unordered_set<osmium::object_id_type>& junctions,
               const std::unordered_set<osmium::object_id_type>& roads,
               const std::unordered_set<osmium::object_id_type>& useful_nodes) {
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
    SplitRoadHandler handler(junctions, roads, useful_nodes, output_buffer);
    osmium::apply(input_buffer, handler);
    writer(std::move(output_buffer));
  }

  writer.close();
  reader.close();
}

void SplitRoadHandler::node(const osmium::Node& node) {
  if (junctions.get().count(node.id()) > 0) {
    osmium::builder::NodeBuilder builder(output_buffer.get());
    builder.set_id(node.id()).set_uid(node.uid()).set_location(node.location());
    osmium::builder::TagListBuilder tag_builder(builder);
    tag_builder.add_tag("junction", "yes");
    tag_builder.add_tag("vertex", "yes");
  } else if (useful_nodes.get().count(node.id()) > 0) {
    osmium::builder::NodeBuilder builder(output_buffer.get());
    builder.set_id(node.id()).set_uid(node.uid()).set_location(node.location());
    osmium::builder::TagListBuilder tag_builder(builder);
    tag_builder.add_tag("junction", "no");
    tag_builder.add_tag("vertex", "no");
  }
  output_buffer.get().commit();
}

void SplitRoadHandler::way(const osmium::Way& way) {
  if (approved_roads.get().count(way.id()) == 0) {
    return;
  }

  const char* highway = way.tags()["highway"];
  const char* name    = way.tags()["name"];

  std::vector<size_t> vertex_indices;
  vertex_indices.reserve(way.nodes().size());

  for (size_t j = 0; j < way.nodes().size(); ++j) {
    if (junctions.get().count(way.nodes()[j].ref()) > 0) {
      vertex_indices.emplace_back(j);
    }
  }

  for (size_t i = 0; i + 1 < vertex_indices.size(); ++i) {
    osmium::builder::WayBuilder builder(output_buffer.get());
    // NOTE: new_way_id is atomic.
    osmium::object_id_type way_id = new_way_id++;
    builder.set_id(way_id).set_uid(way_id);

    // Add nodes
    {
      osmium::builder::WayNodeListBuilder node_list_builder(builder);
      for (size_t j = vertex_indices[i]; j <= vertex_indices[i + 1]; ++j) {
        node_list_builder.add_node_ref(way.nodes()[j]);
      }
    }

    // Add highway tag and from/to tag
    {
      osmium::builder::TagListBuilder tag_builder(builder);
      tag_builder.add_tag("highway", highway == nullptr ? "road" : highway);
      tag_builder.add_tag("name", name == nullptr ? "NONAME Rd" : name);
    }
  }

  output_buffer.get().commit();
}

}  // namespace open_semap
