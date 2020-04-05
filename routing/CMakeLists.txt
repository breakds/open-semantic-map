cmake_minimum_required(VERSION 3.10)

project(OpenSemapRouting VERSION 0.1.0)

# C++ Language specific properties
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# FIXME: Explicitly add dependencies when not in nix-shell.

add_executable(osm_routing_cli cli.cc)
