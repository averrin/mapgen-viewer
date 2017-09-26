#ifndef MAP_H_
#define MAP_H_
#include "City.hpp"
#include "Region.hpp"
#include "River.hpp"
#include "Road.hpp"
#include "micropather.h"
#include <cstring>

class Map : public micropather::Graph {
public:
  ~Map();

  std::vector<MegaCluster *> megaClusters;
  std::vector<Cluster *> clusters;
  std::vector<Cluster *> stateClusters;

  std::vector<State *> states;
  std::vector<Region *> regions;
  std::vector<River *> rivers;
  std::vector<City *> cities;
  std::vector<Location *> locations;
  std::vector<Road *> roads;

  std::string status = "";

  float getRegionDistance(Region *r, Region *r2);
  float LeastCostEstimate(void *stateStart, void *stateEnd);
  void AdjacentCost(void *state, MP_VECTOR<micropather::StateCost> *adjacent);
  void PrintStateInfo(void *state);

};

#endif
