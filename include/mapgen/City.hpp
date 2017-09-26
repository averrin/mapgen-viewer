#ifndef CITY_H_
#define CITY_H_

#include "Region.hpp"
#include "Location.hpp"

class City : public Location {
public:
  City(Region* r, std::string n, LocationType t);
  bool isCapital = false;

  int population = 1000;
  float wealth = 1;
};

#endif
