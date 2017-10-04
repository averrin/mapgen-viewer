#ifndef ROAD_H_
#define ROAD_H_

#include "Region.hpp"
#include "micropather.h"
#include "SelbaWard/SelbaWard.hpp"

class Road {
public:
  Road(micropather::MPVector<void *>* path, float c);
  std::vector<Region *> regions;
  float cost;
  sw::Spline* spline = nullptr;
};

#endif
