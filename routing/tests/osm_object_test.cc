#include "osmium/handler.hpp"
#include "osmium/io/pbf_input.hpp"
#include "osmium/io/reader_with_progress_bar.hpp"
#include "osmium/io/xml_input.hpp"
#include "osmium/osm.hpp"
#include "osmium/visitor.hpp"

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "spdlog/spdlog.h"

class ReadWayHandler : public osmium::handler::Handler {
 public:
  ReadWayHandler() = default;

  void way(const osmium::Way& way) {
    spdlog::info("Size: {}", way.nodes().size());
  }
};

TEST(OsmObjectTest, ReadWay) {
  osmium::io::File input_file("/home/breakds/dataset/osm/kirkwood.osm");
  osmium::io::ReaderWithProgressBar reader(true, input_file,
                                           osmium::osm_entity_bits::way);
  ReadWayHandler handler;
  osmium::apply(reader, handler);
  reader.close();
}
