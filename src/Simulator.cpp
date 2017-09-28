#include "mapgen/Simulator.hpp"
#include "mapgen/Region.hpp"
#include "mapgen/names.hpp"
#include "mapgen/utils.hpp"
#include "mapgen/Biom.hpp"
#include "mapgen/Package.hpp"
#include "mapgen/Economy.hpp"
#include <functional>
#include <cstring>

	template <typename T> using filterFunc = std::function<bool(T *)>;
	template <typename T> using sortFunc = std::function<bool(T *, T *)>;

  template <typename T>
  std::vector<T *> filterObjects(std::vector<T *> regions,
                                      filterFunc<T> filter, sortFunc<T> sort) {
    std::vector<T *> places;

    std::copy_if(regions.begin(), regions.end(), std::back_inserter(places),
                 filter);
    std::sort(places.begin(), places.end(), sort);
    return places;
  }

Simulator::Simulator(Map *m, int s) : map(m), _seed(s) {
  _gen = new std::mt19937(_seed);
}

void Simulator::simulate() {
  makeRoads();
  makeCaves();

  removeBadPorts();
  makeLighthouses();
  makeLocationRoads();
  makeForts();

  simulateEconomy();

  upgradeCities();
}

void Simulator::simulateEconomy() {
  int y = 1;
  while (y <= years) {
    char op[100];
    sprintf(op, "Simulate economy [%d/%d]", y, years);
    map->status = op;
    economyTick(y);
    populationTick(y);
    y++;
  }
}

void Simulator::populationTick(int) {
  for (auto c : map->cities) {
    c->population *= (float)(1 + Economy::POPULATION_GROWS * c->wealth * Economy::POPULATION_GROWS_WEALTH_MODIFIER);
  }
}

void Simulator::economyTick(int y) {
  mg::info("Economy year:", y);
  std::vector<Package*> goods;
  for (auto c : map->cities) {
    auto lg = c->makeGoods(y);
    goods.insert(goods.end(), lg.begin(), lg.end());
  }
  mg::info("Goods for sale:", goods.size());
  for (auto c : map->cities) {
    c->buyGoods(&goods);
  }
  mg::info("Goods not sold:", goods.size());
}

void Simulator::makeRoads() {
  map->roads.clear();
  map->status = "Making roads...";
  _pather = new micropather::MicroPather(map);

  MegaCluster *biggestCluster;
  for (auto c : map->megaClusters) {
    if (c->isLand) {
      biggestCluster = c;
      break;
    }
  }

  int tc = (map->cities.size() * map->cities.size() - map->cities.size()) / 2;
  int k = 0;
  int n = 1;
  for (auto c : map->cities) {
    for (auto oc :
         std::vector<City *>(map->cities.begin() + n, map->cities.end())) {
      k++;
      char op[100];
      sprintf(op, "Making roads [%d/%d]", k, tc);
      map->status = op;
      micropather::MPVector<void *> path;
      float totalCost = 0;
      _pather->Reset();
      int result = _pather->Solve(c->region, oc->region, &path, &totalCost);
      if (result != micropather::MicroPather::SOLVED) {
        continue;
      }
      Road *road = new Road(&path, result);
      map->roads.push_back(road);
      c->roads.push_back(road);
      oc->roads.push_back(road);
    };
    n++;
  };
}

void Simulator::makeCaves() {
  map->status = "Digging caves...";
  int i = 0;
  for (auto c : map->clusters) {
    if (c->biom.name != biom::ROCK.name) {
      continue;
    }

    int n = c->regions.size() / 50 + 1;

    while (n != 0) {
      Region *r = *select_randomly(c->regions.begin(), c->regions.end());
      if (r->location != nullptr) {
        continue;
      }
      Location *l = new Location(r, names::generateCityName(_gen), CAVE);
      map->locations.push_back(l);
      n--;
      i++;
    }
  }
  mg::info("Caves created:", i);
}

void Simulator::upgradeCities() {
  map->status = "Upgrade cities...";
  std::vector<City *> _cities;
  for (auto state : map->states) {
    _cities = filterObjects(
        map->cities,
        (filterFunc<City>)[&](City * c) { return c->region->state == state; },
        (sortFunc<City>)[&](City * c, City * c2) {
          if (c->wealth > c2->wealth) {
            return true;
          }
          return false;
        });

    _cities[0]->type = CAPITAL;
    printf("%s is capital\n", _cities[0]->name.c_str());
    int n = 1;
    while (n < 4) {
      _cities[n]->type = TRADE;
      printf("%s is trade\n", _cities[n]->name.c_str());
      n++;
    }
  }
}

void Simulator::removeBadPorts() {
  map->status = "Abandonning ports...";
  std::vector<City *> cities;
  int n = 0;
  std::copy_if(map->cities.begin(), map->cities.end(),
               std::back_inserter(cities), [&](City *c) {
                 bool badPort = c->type == PORT &&
                                c->region->traffic <= map->cities.size();
                 if (badPort) {
                   c->region->city = nullptr;
                   c->region->location = nullptr;

                   auto cc = c->region->megaCluster->cities;
                   cc.erase(std::remove(cc.begin(), cc.end(), c), cc.end());

                   auto cl = map->locations;
                   cl.erase(std::remove(cl.begin(), cl.end(), c), cl.end());
                   n++;
                 }
                 return !badPort;
               });
  mg::info("Ports removed:", n);
  map->cities.clear();
  for (auto c : cities) {
    map->cities.push_back(c);
  }

  std::vector<Road *> roads;
  std::copy_if(map->roads.begin(), map->roads.end(), std::back_inserter(roads),
               [&](Road *r) {
                 return r->regions.back()->city != nullptr &&
                        r->regions[0]->city != nullptr;
               });
  map->roads.clear();
  map->roads.assign(roads.begin(), roads.end());
}

void Simulator::makeLighthouses() {
  map->status = "Make lighthouses...";
  std::vector<Region *> cache;
  for (auto r : map->regions) {
    if (r->city != nullptr) {
      continue;
    }
    bool tc = false;
    for (auto cc : cache) {
      if (mg::getDistance(r->site, cc->site) < 100) {
        tc = true;
      }
    }
    if (tc) {
      continue;
    }

    if (!r->megaCluster->isLand) {
      continue;
    }
    int i = 0;
    for (auto n : r->neighbors) {
      if (n->traffic > 50 && !n->megaCluster->isLand) {
        i++;
      }
    }
    if (i >= 3) {
      Location *l = new Location(r, names::generateCityName(_gen), LIGHTHOUSE);
      map->locations.push_back(l);
      cache.push_back(l->region);
    }
  }
}

void Simulator::makeLocationRoads() {
  map->status = "Make small roads...";
  for (auto l : map->locations) {
    auto mc = l->region->megaCluster;
    if (mc->cities.size() == 0) {
      continue;
    }
    std::sort(mc->cities.begin(), mc->cities.end(), [&](City *c, City *c2) {
        if (mg::getDistance(l->region->site, c->region->site) <
            mg::getDistance(l->region->site, c2->region->site)) {
        return true;
      }
      return false;
    });
    int n = 0;
    City *c = mc->cities[n];
    while (c->region->city == nullptr) {
      n++;
      c = mc->cities[n];
    }

    micropather::MPVector<void *> path;
    float totalCost = 0;
    _pather->Reset();
    int result = _pather->Solve(c->region, l->region, &path, &totalCost);
    if (result != micropather::MicroPather::SOLVED) {
      continue;
    }
    Road *road = new Road(&path, 1);
    map->roads.push_back(road);
  }
}

void Simulator::makeForts() {
  map->status = "Make forts...";
  // TODO: uncluster it too
  std::vector<Region *> regions;
  std::vector<Region *> cache;
  for (auto mc : map->megaClusters) {
    if (mc->states.size() < 2) {
      continue;
    }

    for (auto state : mc->states) {
      regions = filterObjects(mc->regions,
                              (filterFunc<Region>)[&](Region * region) {
                                bool cond = region->stateBorder &&
                                       !region->seaBorder &&
                                       region->state == state;
                                if (cond && std::none_of(cache.begin(), cache.end(), [&](Region *ri){
                                      for (auto rn : cache) {
                                        if (mg::getDistance(ri->site, rn->site) < 20 && ri->state == rn->state) {
                                          return true;
                                        }
                                      }
                                      return false;
                                    })) {
                                  cache.push_back(region);
                                } else {
                                  return false;
                                }
                                return cond;
                              },
                              (sortFunc<Region>)[&](Region * r, Region * r2) {
                                if (r->traffic > r2->traffic) {
                                  return true;
                                }
                                return false;
                              });

      int n = 0;
      while (n < std::min(2, int(regions.size()))) {
        City *c = new City(regions[n], names::generateCityName(_gen), FORT);
        map->cities.push_back(c);
        mc->cities.push_back(c);
        n++;
      }
    }
  }
}

template <typename Iter>
Iter Simulator::select_randomly(Iter start, Iter end) {
  std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
  std::advance(start, dis(*_gen));
  return start;
}
