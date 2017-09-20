#include "Region.hpp"

struct River {
  std::string name;
  PointList* points;
  std::vector<Region*> regions;
};
