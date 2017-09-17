#ifndef MAP_CPP_
#define MAP_CPP_
#include "mapgen/City.hpp"
#include "mapgen/Location.hpp"
#include "mapgen/Region.hpp"
#include "mapgen/River.hpp"
#include "mapgen/Road.hpp"
#include "micropather.h"

class Map : public micropather::Graph {
public:
  ~Map(){};

  std::vector<MegaCluster *> megaClusters;
  std::vector<Cluster *> clusters;

  std::vector<Region *> regions;
  std::vector<River *> rivers;
  std::vector<City *> cities;
  std::vector<Location *> locations;
  std::vector<Road *> roads;

  float getRegionDistance(Region *r, Region *r2) {
    Point p = r->site;
    Point p2 = r2->site;
    double distancex = (p2->x - p->x);
    double distancey = (p2->y - p->y);
    float d = std::sqrt(distancex * distancex + distancey * distancey);

    if (r->megaCluster->isLand) {
      float hd = (r->getHeight(r->site) - r2->getHeight(r2->site));
      if (hd < 0) {
        d += 10000 * std::abs(hd);
        if (r2->city != nullptr && d >= 5000) {
          d -= 5000;
        }
      }
      if (r->hasRiver) {
        d *= 0.6;
      }
    } else {
      d *= 0.8;
    }
    return d;
  }

  float LeastCostEstimate(void *stateStart, void *stateEnd) {
    return getRegionDistance((Region *)stateStart, (Region *)stateEnd);
  };

  void AdjacentCost(void *state, MP_VECTOR<micropather::StateCost> *neighbors) {
    auto r = ((Region *)state);
    for (auto n : r->neighbors) {

      if (n->biom.name == "Lake") {
        continue;
      }
      if (r->megaCluster->isLand) {
        if (!n->megaCluster->isLand) {
          if (r->city == nullptr || r->city->type != PORT) {
            continue;
          }
        }
      } else {
        if (n->megaCluster->isLand) {
          if (n->city == nullptr || n->city->type != PORT) {
            continue;
          }
        }
      }

      micropather::StateCost nodeCost = {
          (void *)n, getRegionDistance((Region *)state, (Region *)n)};
      neighbors->push_back(nodeCost);
    }
  };
  void PrintStateInfo(void *state){};
};

#endif
