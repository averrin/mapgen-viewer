#include "mapgen/City.hpp"
#include "mapgen/Region.hpp"

City::City(Region *r, std::string n, LocationType t) : Location::Location(r, n, t), isCapital(false) {
  region->city = this;
}
