#include <unordered_set>

#include "gflags/gflags.h"
#include "osmium/osm.hpp"
#include "spdlog/spdlog.h"

#include "utils/extract_junction.h"
#include "utils/filter_isolated_roads.h"

DEFINE_string(input, "/home/breakds/dataset/osm/kirkwood.osm",
              "The input OSM file in xml or pbf format.");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::unordered_set<osmium::object_id_type> junctions =
      open_semap::ExtractJunction(FLAGS_input);

  std::unordered_set<osmium::object_id_type> roads =
      open_semap::FilterIsolatedRoads(FLAGS_input, junctions);

  spdlog::info("There are {} junctions in total", junctions.size());
  spdlog::info("There are {} inter-connected roads in total", roads.size());

  return 0;
}
