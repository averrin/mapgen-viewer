#include "mapgen/Package.hpp"
#include "mapgen/Economy.hpp"
#include "mapgen/Road.hpp"
#include "mapgen/utils.hpp"

Package::Package(City *o, PackageType t, unsigned int c) : owner(o), type(t), count(c) {}

void Package::buy(City *buyer, float price, unsigned int c) {
  count -= c;
  owner->wealth += (float)price / (float)owner->population * c;
  owner->wealth = std::max(owner->wealth, 0.f);

  buyer->wealth -= (float)price / (float)buyer->population * c;
  buyer->wealth = std::max(buyer->wealth, 0.f);
  auto path =
      std::find_if(owner->roads.begin(), owner->roads.end(), [&](Road *r) {
        return r->regions.back()->city == buyer ||
               r->regions.front()->city == buyer;
      });
  if (path == owner->roads.end()) {
    return;
  }
  for (auto r : (*path)->regions) {
    if (r->city != nullptr && r->city->type == PORT && r->city != owner &&
        r->city != buyer) {
      r->city->wealth += price * Economy::PORT_FEE / r->city->population * c;
    }
  }
}
