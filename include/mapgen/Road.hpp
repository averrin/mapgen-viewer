#ifndef ROAD_H_
#define ROAD_H_

#include "Region.hpp"
#include "micropather.h"

class Road {
public:
  Road(micropather::MPVector<void *>* path, float w);
  std::vector<Region *> regions;
  float weight;
};

#endif
