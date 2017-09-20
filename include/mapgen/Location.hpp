#ifndef LOCATION_H_
#define LOCATION_H_

#include "Region.hpp"

enum LocationType {
  CAPITAL,
  PORT,
  MINE,
  AGRO,
  TRADE,
  LIGHTHOUSE,
  CAVE,
  FORT
};

class Location {
public:
  Location(Region* r, std::string n, LocationType t);
  Region* region;
  std::string name;
  LocationType type;
  std::string typeName;
};

#endif
