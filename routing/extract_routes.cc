#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "gflags/gflags.h"
#include "osmium/osm/box.hpp"
#include "osmium/osm/types.hpp"
#include "spdlog/spdlog.h"

#include "utils/extract_junction.h"
#include "utils/filter_bounding_box.h"
#include "utils/filter_isolated_roads.h"
#include "utils/split_road.h"

DEFINE_string(input, "/home/breakds/dataset/osm/kirkwood.osm",
              "The input OSM file in xml or pbf format.");

DEFINE_string(output, "/home/breakds/dataset/osm/kirkwood_routes.osm",
              "The output OSM file in xml or pbf format.");

DEFINE_string(
    bounding_box, "-180.0,-90.0,180.0,90.0",
    "The bounding box of interest, in format \"lon_min,lat_min,lon_max,lat_max\".");

osmium::Box ParseBoundingBox(const std::string text) {
  std::stringstream stream(text);
  std::string token;
  std::vector<double> values;
  while (std::getline(stream, token, ',')) {
    values.emplace_back(std::stod(token));
  }
  if (values.size() != 4) {
    spdlog::critical("Failed to parse {} as bounding box.", text);
  }
  return osmium::Box(values[0], values[1], values[2], values[3]);
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  osmium::Box box = ParseBoundingBox(FLAGS_bounding_box);

  std::unordered_set<osmium::object_id_type> junctions =
      open_semap::ExtractJunction(FLAGS_input);

  junctions = open_semap::FilterBoundingBox(FLAGS_input, junctions, box);

  std::unordered_set<osmium::object_id_type> roads;
  std::unordered_set<osmium::object_id_type> useful_nodes;
  // Thank god we have copy ellision.
  std::tie(roads, useful_nodes) = open_semap::FilterIsolatedRoads(FLAGS_input, junctions);

  spdlog::info("Stats: {} nodes", useful_nodes.size());
  spdlog::info("Stats: {} junctions", junctions.size());
  spdlog::info("Stats: {} roads", roads.size());

  open_semap::SplitRoad(FLAGS_input, FLAGS_output, junctions, roads, useful_nodes);

  return 0;
}
