#include <unordered_set>

#include "gflags/gflags.h"
#include "spdlog/spdlog.h"

#include "utils/extract_junction.h"

DEFINE_string(input, "/home/breakds/dataset/osm/kirkwood.osm",
              "The input OSM file in xml or pbf format.");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::unordered_set<osmium::object_id_type> junctions =
      open_semap::ExtractJunction(FLAGS_input);

  for (const osmium::object_id_type id : junctions) {
    spdlog::info("Junction node {}", id);
  }

  spdlog::info("There are {} junctions in total", junctions.size());

  return 0;
}
