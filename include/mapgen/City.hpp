#ifndef CITY_H_
#define CITY_H_

#include "Region.hpp"

class City {
public:
  City(Region* r, std::string n);
  Region* region;
  std::string name;
  bool isCapital;
};

#endif
