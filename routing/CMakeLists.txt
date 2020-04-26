cmake_minimum_required(VERSION 3.10)

project(OpenSemapRouting VERSION 0.1.0)

# C++ Language specific properties

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR})
add_compile_options(-Wall -Werror)

# +------------------------------------------------------------+
# | 3rd Party Dependencies                                     |
# +------------------------------------------------------------+

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(ZLIB REQUIRED zlib)
pkg_check_modules(BZIP2 REQUIRED bzip2)
pkg_check_modules(EXPAT REQUIRED expat)
pkg_check_modules(GFLAGS REQUIRED gflags)
pkg_check_modules(SPDLOG REQUIRED spdlog)
pkg_check_modules(GTEST REQUIRED gtest)
pkg_check_modules(GMOCK REQUIRED gmock)

# +------------------------------------------------------------+
# | Targets: Utilities                                         |
# +------------------------------------------------------------+

# No need to add libosmium explicitly because it is header only.

add_library(predicates utils/predicates.cc)

add_library(extract_junction utils/extract_junction.cc)
target_link_libraries(
  extract_junction
  predicates
  ${BZIP2_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${EXPAT_LIBRARIES}
  Threads::Threads)

add_library(filter_bounding_box utils/filter_bounding_box.cc)
target_link_libraries(
  filter_bounding_box
  ${BZIP2_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${EXPAT_LIBRARIES}
  Threads::Threads)

add_library(filter_isolated_roads utils/filter_isolated_roads.cc)
target_link_libraries(
  filter_isolated_roads
  predicates
  ${BZIP2_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${EXPAT_LIBRARIES}
  Threads::Threads)

add_library(split_road utils/split_road.cc)
target_link_libraries(
  split_road
  ${BZIP2_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${EXPAT_LIBRARIES}
  Threads::Threads)

add_library(road_graph graph/road_graph.cc graph/edge.cc)
target_link_libraries(
  road_graph
  ${BZIP2_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${EXPAT_LIBRARIES}
  ${SPDLOG_LIBRARIES}
  Threads::Threads)

add_library(graph_builder graph/builder.cc)
target_link_libraries(
  graph_builder
  road_graph)

add_library(simple_indexer graph/simple_indexer.cc)
target_link_libraries(
  simple_indexer
  road_graph
  ${BZIP2_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${EXPAT_LIBRARIES}
  ${SPDLOG_LIBRARIES}
  Threads::Threads)

add_library(dijkstra algorithms/dijkstra.cc)
target_link_libraries(
  dijkstra
  road_graph
  simple_indexer
  ${SPDLOG_LIBRARIES})

add_library(contraction algorithms/contraction.cc)
target_link_libraries(
  contraction
  dijkstra
  road_graph
  simple_indexer
  ${SPDLOG_LIBRARIES})

# +------------------------------------------------------------+
# | Targets: Binaries                                          |
# +------------------------------------------------------------+

add_executable(extract_routes extract_routes.cc)
target_link_libraries(
  extract_routes
  extract_junction
  split_road
  filter_isolated_roads
  filter_bounding_box
  ${GFLAGS_LIBRARIES}
  ${SPDLOG_LIBRARIES})

# +------------------------------------------------------------+
# | Targets: Tests                                             |
# +------------------------------------------------------------+

set(CMAKE_TEST_DATA_PATH "${CMAKE_CURRENT_BINARY_DIR}/tests/data")
file(COPY tests/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/tests)
configure_file(tests/testdata.h.in tests/testdata.h @ONLY)

add_executable(road_graph_test tests/road_graph_test.cc)
target_include_directories(road_graph_test PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(
  road_graph_test
  road_graph
  graph_builder
  simple_indexer
  gtest_main
  ${GMOCK_LIBRARIES}  
  ${GTEST_LIBRARIES})
set_target_properties(road_graph_test PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY tests)

add_executable(osm_object_test tests/osm_object_test.cc)
target_include_directories(osm_object_test PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(
  osm_object_test
  gtest_main
  ${BZIP2_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${EXPAT_LIBRARIES}
  ${GTEST_LIBRARIES}
  ${SPDLOG_LIBRARIES}
  Threads::Threads)
set_target_properties(osm_object_test PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY tests)

add_executable(dijkstra_test tests/dijkstra_test.cc)
target_include_directories(dijkstra_test PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(
  dijkstra_test
  dijkstra
  graph_builder
  gtest_main
  ${GMOCK_LIBRARIES}
  ${GTEST_LIBRARIES})
set_target_properties(dijkstra_test PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY tests)

add_executable(contraction_test tests/contraction_test.cc)
target_include_directories(contraction_test PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(
  contraction_test
  contraction
  graph_builder
  gtest_main
  ${GMOCK_LIBRARIES}
  ${GTEST_LIBRARIES})
set_target_properties(contraction_test PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY tests)

