#include "osmium/geom/haversine.hpp"
#include "osmium/handler.hpp"
#include "osmium/io/pbf_input.hpp"
#include "osmium/io/reader_with_progress_bar.hpp"
#include "osmium/io/xml_input.hpp"
#include "osmium/osm.hpp"
#include "osmium/visitor.hpp"

#include <functional>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "spdlog/spdlog.h"

#include "tests/testdata.h"

namespace open_semap {
namespace testing {

class ReadWayHandler : public osmium::handler::Handler {
 public:
  ReadWayHandler() = default;

  void way(const osmium::Way& way) {
    spdlog::info("Size: {}", way.nodes().size());
  }
};

TEST(OsmObjectTest, ReadWay) {
  osmium::io::File input_file(std::string(TEST_DATA_PATH) + "/adobe_wells.osm");
  osmium::io::ReaderWithProgressBar reader(true, input_file,
                                           osmium::osm_entity_bits::way);
  ReadWayHandler handler;
  osmium::apply(reader, handler);
  reader.close();
}

}  // namespace testing
}  // namespace open_semap
