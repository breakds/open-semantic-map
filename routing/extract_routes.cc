#include "spdlog/spdlog.h"

#include "osmium/handler.hpp"
#include "osmium/io/pbf_input.hpp"
#include "osmium/io/xml_input.hpp"
#include "osmium/util/progress_bar.hpp"
#include "osmium/visitor.hpp"

struct RoadHandler : public osmium::handler::Handler {
  void way(const osmium::Way& way) {
    const char* highway = way.tags()["highway"];
    if (highway != nullptr) {
      const char* name = way.tags()["name"];
      if (name == nullptr) {
        spdlog::info("Unknown");
      } else {
        spdlog::info("{}", name);
      }
    }
  }

};  // struct RoadLengthHandler

int main(int argc, char** argv) {
  osmium::io::File input_file("/home/breakds/dataset/osm/kirkwood.osm");
  osmium::io::Reader reader(input_file);

  RoadHandler handler;

  osmium::apply(reader, handler);

  reader.close();

  return 0;
}
