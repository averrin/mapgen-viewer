#include "mapgen/Map.hpp"

Map::~Map(){};

float Map::getRegionDistance(Region *r, Region *r2) {
  Point p = r->site;
  Point p2 = r2->site;
  double distancex = (p2->x - p->x);
  double distancey = (p2->y - p->y);
  float d = std::sqrt(distancex * distancex + distancey * distancey);

  if (r->megaCluster->isLand) {
    float hd = (r->getHeight(r->site) - r2->getHeight(r2->site));
    if (hd < 0) {
      d += 1000 * std::abs(hd);
      if (r2->city != nullptr && d >= 500) {
        d -= 500;
      }
    }
    if (r2->hasRiver) {
      d *= 0.6f;
    }
    if (r2->state != r->state) {
      d *= 1.2f;
    }
    if (r2->biom == biom::FORREST && r2->biom == biom::RAIN_FORREST) {
      d *= 1.1f;
    }
    if (r2->hasRoad) {
      d *= 0.2f;
    }
  } else {
    d *= 0.8f;
  }
  return d;
}

float Map::LeastCostEstimate(void *stateStart, void *stateEnd) {
  return getRegionDistance((Region *)stateStart, (Region *)stateEnd);
};

void Map::AdjacentCost(void *state,
                       MP_VECTOR<micropather::StateCost> *neighbors) {
  auto r = ((Region *)state);
  for (auto n : r->neighbors) {

    if (n->biom == biom::LAKE) {
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
void Map::PrintStateInfo(void *state){};

