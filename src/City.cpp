#include "mapgen/City.hpp"
#include "mapgen/Economy.hpp"
#include "mapgen/Package.hpp"
#include "mapgen/Region.hpp"
#include "mapgen/utils.hpp"

City::City(Region *r, std::string n, LocationType t)
    : Location::Location(r, n, t), isCapital(false) {
  region->city = this;
}

Package *City::makeGoods(int y) {
  Package *goods;
  uint p;
  switch (type) {
  case AGRO:
    p = region->nice * Economy::PACKAGES_PER_NICE * population / 1000 *
        Economy::PACKAGES_AGRO_POPULATION_MODIFIER;
    goods = new Package(this, AGROCULTURE, p);
    break;
  case MINE:
    p = region->minerals * Economy::PACKAGES_PER_MINERALS * population / 1000 *
        Economy::PACKAGES_MINERALS_POPULATION_MODIFIER;
    for (int n = 0; n < p; n++) {
      goods = new Package(this, MINERALS, p);
    }
    break;
  }
  return goods;
}

int City::buyGoods(std::vector<Package *> *goods) {
  uint mineralsNeeded =
      population * Economy::CONSUME_MINERALS_POPULATION_MODIFIER;
  uint agroNeeded = population * Economy::CONSUME_AGRO_POPULATION_MODIFIER;

  std::vector<Package *> mineralsCandidates;
  std::vector<Package *> agroCandidates;

  std::copy_if(goods->begin(), goods->end(),
               std::back_inserter(mineralsCandidates),
               [&](Package *p) { return p->type == MINERALS; });
  std::sort(mineralsCandidates.begin(), mineralsCandidates.end(),
            [&](Package *p1, Package *p2) {
              if (getPrice(p1) < getPrice(p2)) {
                return true;
              }
              return false;
            });
  std::copy_if(goods->begin(), goods->end(), std::back_inserter(agroCandidates),
               [&](Package *p) { return p->type == AGROCULTURE; });
  std::sort(agroCandidates.begin(), agroCandidates.end(),
            [&](Package *p1, Package *p2) {
              if (getPrice(p1) < getPrice(p2)) {
                return true;
              }
              return false;
            });

  if (agroCandidates.size() > 0) {
    int n = 0;
    auto p = agroCandidates[n];
    uint c = 0;
    while (agroNeeded > 0 && n < agroCandidates.size()) {
      p = agroCandidates[n];
      c = std::min((int)agroNeeded, (int)p->count);
      agroNeeded -= c;
      p->buy(this, getPrice(p), c);
      goods->erase(std::remove(goods->begin(), goods->end(), p));
      n++;
    }
  }

  if (mineralsCandidates.size() > 0) {
    int n = 0;
    auto p = mineralsCandidates[n];
    uint c = 0;
    while (mineralsNeeded > 0 && n < mineralsCandidates.size()) {
      p = mineralsCandidates[n];
      c = std::min((int)mineralsNeeded, (int)p->count);
      mineralsNeeded -= c;
      p->buy(this, getPrice(p), c);
      goods->erase(std::remove(goods->begin(), goods->end(), p));
      n++;
    }
  }

  this->wealth -= Economy::CANT_BUY_AGRO * agroNeeded / (float)this->population;
  this->wealth -=
      Economy::CANT_BUY_MINERALS * mineralsNeeded / (float)this->population;
  wealth = std::max(wealth, 0.f);
  return agroNeeded + mineralsNeeded;
}

float City::getPrice(Package* p) {
  float price = 1.f;
  if (cache.find(p->owner) != cache.end()) {
    price = cache[p->owner];
  } else {
    auto path =
      std::find_if(roads.begin(), roads.end(), [&](Road *r) {
          return r->regions.back()->city == p->owner ||
          r->regions.front()->city == p->owner;
        });

    if (path != roads.end()) {
      price *= 1 + ((*path)->cost / 10000.f);
    } else {
      mg::warn("Strange situation:", "ghost road");
      price *= 1.5;
    }
    if (p->owner->region->state != region->state) {
      price *= 1.5;
    }
    cache.insert(std::make_pair(p->owner, price));
  }
  return price;
}
