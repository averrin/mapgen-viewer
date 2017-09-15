#ifndef MAP_CPP_
#define MAP_CPP_
#include "micropather.h"
#include "mapgen/Region.hpp"
#include "mapgen/River.hpp"
#include "mapgen/City.hpp"
// #include "Biom.cpp"

class Map: public micropather::Graph {
public:
  ~Map(){};

  std::vector<MegaCluster*> megaClusters;
  std::vector<Cluster*> clusters;

  std::vector<Region*> regions;
  std::vector<River*> rivers;
  std::vector<City*> cities;
  std::vector<std::vector<Region*>> roads;

  float getRegionDistance(Region* r, Region* r2) {
    Point p = r->site;
    Point p2 = r2->site;
    double distancex = (p2->x - p->x);
    double distancey = (p2->y - p->y);

    float d = std::sqrt(distancex * distancex + distancey * distancey);
    float hd = (r->getHeight(r->site) - r2->getHeight(r2->site));
    if (hd < 0) {
      d += 10000 * std::abs(hd);
      if (r2->city != nullptr) {
        d -= 5000;
      }
    }
    return d;
  }


  float LeastCostEstimate( void* stateStart, void* stateEnd ){
    return getRegionDistance((Region*)stateStart, (Region*)stateEnd);
  };

  void AdjacentCost( void* state, MP_VECTOR< micropather::StateCost > *neighbors ){
    for (auto n : ((Region*)state)->neighbors) {
      if(!n->megaCluster->isLand || n->biom.name == "Lake"){
        continue;
      }
      micropather::StateCost nodeCost = { (void*)n, getRegionDistance((Region*)state, (Region*)n) };
      neighbors->push_back( nodeCost );
    }
  };
  void PrintStateInfo( void* state ){};
};

#endif
