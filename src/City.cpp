#include "mapgen/City.hpp"
#include "mapgen/Region.hpp"

City::City(Region *r, std::string n, CityType t) : region(r), name(n), type(t) {
  isCapital = false;
  // region->biom = CITY;
  region->city = this;
}
