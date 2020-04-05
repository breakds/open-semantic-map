#include <unordered_set>

#include "gflags/gflags.h"
#include "osmium/osm.hpp"
#include "spdlog/spdlog.h"

#include "utils/extract_junction.h"
#include "utils/filter_isolated_roads.h"
#include "utils/split_road.h"

DEFINE_string(input, "/home/breakds/dataset/osm/kirkwood.osm",
              "The input OSM file in xml or pbf format.");

DEFINE_string(output, "/home/breakds/dataset/osm/kirkwood_routes.osm",
              "The output OSM file in xml or pbf format.");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::unordered_set<osmium::object_id_type> junctions =
      open_semap::ExtractJunction(FLAGS_input);

  std::unordered_set<osmium::object_id_type> roads =
      open_semap::FilterIsolatedRoads(FLAGS_input, junctions);

  open_semap::SplitRoad(FLAGS_input, FLAGS_output, junctions, roads);

  spdlog::info("There are {} junctions in total", junctions.size());
  spdlog::info("There are {} inter-connected roads in total", roads.size());

  return 0;
}
