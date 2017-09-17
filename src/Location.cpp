#include "mapgen/Location.hpp"
#include "mapgen/Region.hpp"

Location::Location(Region *r, std::string n, LocationType t) : region(r), name(n), type(t) {
  region->location = this;
}
