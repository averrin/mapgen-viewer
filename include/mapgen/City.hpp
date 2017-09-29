#ifndef CITY_H_
#define CITY_H_

#include "Region.hpp"
#include "Location.hpp"
#include "Road.hpp"

class Package;
class City : public Location {
public:
  City(Region* r, std::string n, LocationType t);
  Package* makeGoods(int y);
  int buyGoods(std::vector<Package*>* goods);

  bool isCapital = false;

  int population = 1000;
  float wealth = 1;
  std::vector<Road*> roads;
  std::map<City*,float> cache;
  float getPrice(Package* p);
};

#endif
