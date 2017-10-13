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
  Package *goods = nullptr;
  unsigned int p;
  switch (type) {
  case AGRO:
    p = region->nice * economyVars->PACKAGES_PER_NICE * population *
        economyVars->PACKAGES_AGRO_POPULATION_MODIFIER;
    goods = new Package(this, AGROCULTURE, p);
    break;
  case MINE:
    p = region->minerals * economyVars->PACKAGES_PER_MINERALS * population *
        economyVars->PACKAGES_MINERALS_POPULATION_MODIFIER;
    goods = new Package(this, MINERALS, p);
    break;
  }
  return goods;
}

int City::buyGoods(std::vector<Package *> *goods) {
  unsigned int mineralsNeeded =
      population * (economyVars->CONSUME_MINERALS_POPULATION_MODIFIER -
                    region->minerals * economyVars->MINERALS_POPULATION_PRODUCE);
  unsigned int agroNeeded =
      population * (economyVars->CONSUME_AGRO_POPULATION_MODIFIER -
                    region->nice * economyVars->AGRO_POPULATION_PRODUCE);

  std::vector<Package *> mineralsCandidates;
  std::vector<Package *> agroCandidates;

  std::copy_if(
      goods->begin(), goods->end(), std::back_inserter(mineralsCandidates),
      [&](Package *p) { return p->type == MINERALS && p->owner != this; });
  std::sort(mineralsCandidates.begin(), mineralsCandidates.end(),
            [&](Package *p1, Package *p2) {
              if (getPrice(p1) < getPrice(p2)) {
                return true;
              }
              return false;
            });
  std::copy_if(
      goods->begin(), goods->end(), std::back_inserter(agroCandidates),
      [&](Package *p) { return p->type == AGROCULTURE && p->owner != this; });
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
    unsigned int c = 0;
    while (agroNeeded > 0 && n < int(agroCandidates.size())) {
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
    unsigned int c = 0;
    while (mineralsNeeded > 0 && n < int(mineralsCandidates.size())) {
      p = mineralsCandidates[n];
      c = std::min((int)mineralsNeeded, (int)p->count);
      mineralsNeeded -= c;
      p->buy(this, getPrice(p), c);
      goods->erase(std::remove(goods->begin(), goods->end(), p));
      n++;
    }
  }

  this->wealth -= economyVars->CANT_BUY_AGRO * agroNeeded / (float)this->population;
  this->wealth -=
      economyVars->CANT_BUY_MINERALS * mineralsNeeded / (float)this->population;
  wealth = std::max(wealth, 0.f);
  return agroNeeded + mineralsNeeded;
}

float City::getPrice(Package *p) {
  float price = 1.f;
  if (cache.find(p->owner) != cache.end()) {
    price = cache[p->owner];
  } else if (roads.size() != 0) {
    auto path = std::find_if(roads.begin(), roads.end(), [&](Road *r) {
      return r->regions.back()->city == p->owner ||
             r->regions.front()->city == p->owner;
    });

    if (path != roads.end()) {
      price *= 1 + ((*path)->cost / 10000.f);
    } else {
      mg::warn("Road not found: from ", *this);
      mg::warn("Road not found: to ", *p->owner);
      price *= 1.5;
    }
    if (p->owner->region->state != region->state) {
      price *= 1.5;
    }
    cache.insert(std::make_pair(p->owner, price));
  }
  return price;
}

std::ostream& operator<<(std::ostream &strm, const City &c) {
  return strm << "City: " << c.name << " [" << c.typeName << "]";
}
