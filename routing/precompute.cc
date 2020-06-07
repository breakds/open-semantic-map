// Do contraction on the input osm data and write the contraction result to
// another file. The contraction result includes the shortcuts and the order of
// vertices.

#include "algorithms/contraction.h"
#include "gflags/gflags.h"
#include "spdlog/spdlog.h"

DEFINE_string(input, "", "The input OSM(PBF) file that needs contraction.");
DEFINE_string(output, "", "The output OSM(PBF) file that needs contraction.");

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  return 0;
}
