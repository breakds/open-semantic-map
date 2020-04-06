cmake_minimum_required(VERSION 3.10)

project(OpenSemapRouting VERSION 0.1.0)

# C++ Language specific properties

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(${PROJECT_SOURCE_DIR})

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
