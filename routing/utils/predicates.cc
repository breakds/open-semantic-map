#include "utils/predicates.h"

namespace open_semap {
namespace predicate {

bool IsValidRoad(const osmium::Way& way) {
  const char* highway = way.tags()["highway"];

  // If there is no highway tag, this Way object does not represent a
  // road and we do not care about it.
  if (highway == nullptr) {
    return false;
  }

  // Crosswalks and footways are not valid roads.
  if (strcmp(highway, "footway") == 0) {
    return false;
  }

  // See https://wiki.openstreetmap.org/wiki/Tag:highway%3Dservice
  if (strcmp(highway, "service") == 0) {
    return false;
  }

  return true;
}

}  // namespace predicate
}  // namespace open_semap
