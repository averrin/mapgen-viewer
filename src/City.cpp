#include "mapgen/City.hpp"
#include "mapgen/Region.hpp"

City::City(Region *r, std::string n) : region(r), name(n) {
  isCapital = false;
  region->biom = CITY;
  region->city = this;
}
