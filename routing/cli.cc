#include <iostream>

#include "spdlog/spdlog.h"
#include "osmium/io/pbf_input.hpp"
#include "osmium/io/xml_input.hpp"
#include "osmium/util/progress_bar.hpp"
#include "osmium/handler/dump.hpp"

int main(int argc, char **argv) {
  osmium::io::File input_file("/home/breakds/dataset/osm/kirkwood.osm");
  osmium::io::Reader reader(input_file);

  osmium::handler::Dump dump(std::cout);

  osmium::apply(reader, dump);

  
  // osmium::ProgressBar progress_bar(reader.file_size(), osmium::isatty(2));

  // while (osmium::memory::Buffer buffer = reader.read()) {
  //   progress_bar.update(reader.offset());

  //   // for (auto &item : buffer) {
  //   //   if (item.type() != osmium::item_type::node) {
  //   //     spdlog::info("Item type is {}", osmium::item_type_to_name(item.type()));
  //   //   }
  //   // }
  // }

  // progress_bar.done();
  reader.close();
  
  return 0;
}
