#include "mapgen/Road.hpp"
#include "mapgen/Package.hpp"
#include "mapgen/utils.hpp"
#include "mapgen/Economy.hpp"

Package::Package(City* o, PackageType t): owner(o), type(t) {
  
}

void Package::buy(City* buyer, float price) {
  owner->wealth += (float)price / (float)owner->population;
  buyer->wealth -= (float)price / (float)buyer->population;
  auto path = std::find_if(owner->roads.begin(), owner->roads.end(), [&](Road* r){
      return r->regions.back()->city == buyer || r->regions.front()->city == buyer;
    });
  for (auto r : (*path)->regions) {
    if (r->city != nullptr && r->city->type == PORT && r->city != owner && r->city != buyer){
      r->city->wealth += price * Economy::PORT_FEE / r->city->population;
    }
  }
}

float Package::getPrice(City* buyer) {
  float p = 1;
  if (buyer->region->state != owner->region->state) {
    p *= 1.5;
  }

  return p;
}
