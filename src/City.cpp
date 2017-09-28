#include "mapgen/City.hpp"
#include "mapgen/Region.hpp"
#include "mapgen/Package.hpp"
#include "mapgen/Economy.hpp"
#include "mapgen/utils.hpp"

City::City(Region *r, std::string n, LocationType t) : Location::Location(r, n, t), isCapital(false) {
  region->city = this;
}

std::vector<Package*> City::makeGoods(int y) {
  std::vector<Package*> goods;
  int p;
  switch (type) {
  case AGRO:
    p = region->nice*Economy::PACKAGES_PER_NICE * population/1000*Economy::PACKAGES_AGRO_POPULATION_MODIFIER;
    for (int n = 0; n < p; n++) {
      goods.push_back(new Package(this, AGROCULTURE));
    }
    break;
  case MINE:
    p = region->minerals*Economy::PACKAGES_PER_MINERALS * population/1000*Economy::PACKAGES_MINERALS_POPULATION_MODIFIER;
    for (int n = 0; n < p; n++) {
      goods.push_back(new Package(this, MINERALS));
    }
    break;
  default:
    break;
  }
  return goods;
}

void City::buyGoods(std::vector<Package*>* goods) {
  int mineralsNeeded = population * Economy::CONSUME_MINERALS_POPULATION_MODIFIER;
  int agroNeeded = population * Economy::CONSUME_AGRO_POPULATION_MODIFIER;
  std::vector<Package*> mineralsCandidates;
  std::vector<Package*> agroCandidates;

  std::copy_if(goods->begin(), goods->end(), std::back_inserter(mineralsCandidates),
               [&](Package* p) {
                 return p->type == MINERALS;
               });
  std::sort(mineralsCandidates.begin(), mineralsCandidates.end(), [&](Package* p1, Package* p2) {
      if (p1->getPrice(this) < p2->getPrice(this)) {
        return true;
      }
      return false;
    });
  std::copy_if(goods->begin(), goods->end(), std::back_inserter(agroCandidates),
               [&](Package* p) {
                 return p->type == AGROCULTURE;
               });
  std::sort(agroCandidates.begin(), agroCandidates.end(), [&](Package* p1, Package* p2) {
      if (p1->getPrice(this) < p2->getPrice(this)) {
        return true;
      }
      return false;
    });

  goods->erase(std::remove_if(goods->begin(), goods->end(), [&](Package* p) {
        auto ap = std::find(agroCandidates.begin(), agroCandidates.end(), p);
        auto ad = std::distance(agroCandidates.begin(), ap);
        auto mp = std::find(mineralsCandidates.begin(), mineralsCandidates.end(), p);
        auto md = std::distance(mineralsCandidates.begin(), mp);

        bool cond = (ap != agroCandidates.end() && ad < agroNeeded) || (mp != mineralsCandidates.end() && md < mineralsNeeded);
      if (cond) {
        p->buy(this, p->getPrice(this));
      }
      return cond;
      }), goods->end());
}
