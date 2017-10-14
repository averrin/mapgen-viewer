#ifndef CITY_H_
#define CITY_H_

#include "Region.hpp"
#include "Location.hpp"
#include "Road.hpp"
#include "mapgen/Economy.hpp"

class Package;
class City : public Location {
public:
  City(Region* r, std::string n, LocationType t);
  Package* makeGoods(int y);
  std::pair<int,int> buyGoods(std::vector<Package*>* goods);
  EconomyVars* economyVars = nullptr;

  bool isCapital = false;

  int population = 1000;
  float wealth = 1.f;
  std::vector<Road*> roads;
  std::map<City*,float> cache;
  float getPrice(Package* p);
private:
  friend std::ostream& operator<<(std::ostream &strm, const City &c);
};

#endif
