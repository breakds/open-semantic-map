#pragma once

#include "osmium/osm.hpp"

namespace open_semap {
namespace predicate {

bool IsValidRoad(const osmium::Way& way);

}  // namespace predicate
}  // namespace open_semap
