#ifndef RIVER_H_
#define RIVER_H_

#include "Region.hpp"

struct River {
  std::string name;
  PointList* points;
  std::vector<Region*> regions;
};

#endif
