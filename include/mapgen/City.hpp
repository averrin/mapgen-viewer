#ifndef CITY_H_
#define CITY_H_

#include "Region.hpp"

enum CityType {
  CAPITAL,
  PORT,
  MINE,
  AGRO,
};

class City {
public:
  City(Region* r, std::string n, CityType t);
  Region* region;
  std::string name;
  bool isCapital;
  CityType type;
};

#endif
