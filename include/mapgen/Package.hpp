#ifndef PACKAGE_H_
#define PACKAGE_H_

#include "City.hpp"

enum PackageType {
  MINERALS,
  AGROCULTURE
};

class Package {
public:
  Package (City* owner, PackageType type);
  float getPrice(City* buyer);
  City* owner;
  std::vector<City*> ports;
  PackageType type;
  void buy(City* buyer, float price);
};

#endif
