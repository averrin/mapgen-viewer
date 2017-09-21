#include "mapgen/Simulator.hpp"
#include "names.cpp"
#include "mapgen/utils.hpp"

Simulator::Simulator(Map *m, int s) : map(m), _seed(s) {}

void Simulator::simulate() {
  makeRoads();
  makeCaves();
  upgradeCities();
  removeBadPorts();
  makeLighthouses();
  makeLocationRoads();
  makeForts();
}

void Simulator::makeRoads() {
  map->roads.clear();
  currentOperation = "Making roads...";
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
      currentOperation = op;
      micropather::MPVector<void *> path;
      float totalCost = 0;
      _pather->Reset();
      int result = _pather->Solve(c->region, oc->region, &path, &totalCost);
      if (result != micropather::MicroPather::SOLVED) {
        continue;
      }
      Road *road = new Road(&path, 2);
      map->roads.push_back(road);
    };
    n++;
  };
}

void Simulator::makeCaves() {
  currentOperation = "Digging caves...";
  int i = 0;
  for (auto c : map->clusters) {
    if (c->biom.name != ROCK.name) {
      continue;
    }

    int n = c->regions.size() / 50 + 1;

    while (n != 0) {
      Region *r = *select_randomly(c->regions.begin(), c->regions.end());
      if (r->location != nullptr) {
        continue;
      }
      Location *l = new Location(r, generateCityName(), CAVE);
      map->locations.push_back(l);
      n--;
      i++;
    }
  }
  printf("%d caves\n", i);
}

void Simulator::upgradeCities() {
  currentOperation = "Upgrade cities...";
  std::vector<City *> _cities;
  for (auto state : map->states) {
    _cities = map->filterObjects(
        map->cities,
        (filterFunc<City>)[&](City * c) { return c->region->state == state; },
        (sortFunc<City>)[&](City * c, City * c2) {
          if (c->region->traffic > c2->region->traffic) {
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
  currentOperation = "Abaddon ports...";
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
  printf("%d ports for remove\n", n);
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
  for (auto r : roads) {
    if (r->regions.back()->city->type == CAPITAL) {
      r->weight = 4;
    }
    map->roads.push_back(r);
  }
}

void Simulator::makeLighthouses() {
  currentOperation = "Make lighthouses...";
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
      Location *l = new Location(r, generateCityName(), LIGHTHOUSE);
      map->locations.push_back(l);
      cache.push_back(l->region);
    }
  }
}

void Simulator::makeLocationRoads() {
  currentOperation = "Make small roads...";
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
  currentOperation = "Make forts...";
  // TODO: uncluster it too
  std::vector<Region *> regions;
  for (auto mc : map->megaClusters) {
    if (mc->states.size() < 2) {
      continue;
    }

    for (auto state : mc->states) {
      regions = map->filterObjects(mc->regions,
                              (filterFunc<Region>)[&](Region * region) {
                                return region->stateBorder &&
                                       !region->seaBorder &&
                                       region->state == state;
                              },
                              (sortFunc<Region>)[&](Region * r, Region * r2) {
                                if (r->traffic > r2->traffic) {
                                  return true;
                                }
                                return false;
                              });

      int n = 0;
      while (n < 2) {
        City *c = new City(regions[n], generateCityName(), FORT);
        map->cities.push_back(c);
        mc->cities.push_back(c);
        n++;
      }
    }
  }
}
