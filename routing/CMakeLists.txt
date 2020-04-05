cmake_minimum_required(VERSION 3.10)

project(OpenSemapRouting VERSION 0.1.0)

# C++ Language specific properties
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Package Dependencies
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

pkg_check_modules(ZLIB REQUIRED zlib)
pkg_check_modules(BZIP2 REQUIRED bzip2)
pkg_check_modules(EXPAT REQUIRED expat)

add_executable(osm_routing_cli cli.cc)
target_link_libraries(
  osm_routing_cli
  ${BZIP2_LIBRARIES}
  ${EXPAT_LIBRARIES}
  ${ZLIB_LIBRARIES}
  Threads::Threads)

add_executable(extract_routes extract_routes.cc)
target_link_libraries(
  extract_routes
  ${BZIP2_LIBRARIES}
  ${EXPAT_LIBRARIES}
  ${ZLIB_LIBRARIES}
  Threads::Threads)

