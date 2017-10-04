#ifndef PACKAGE_H_
#define PACKAGE_H_

#include "City.hpp"

enum PackageType {
  MINERALS,
  AGROCULTURE
};

class Package {
public:
  Package (City* owner, PackageType type, unsigned int count);
  City* owner;
  std::vector<City*> ports;
  PackageType type;
  unsigned int count = 0;
  void buy(City* buyer, float price, unsigned int c);
};

#endif
