#include "mapgen/Simulator.hpp"
#include "mapgen/Biom.hpp"
#include "mapgen/Economy.hpp"
#include "mapgen/Package.hpp"
#include "mapgen/Region.hpp"
#include "mapgen/Report.hpp"
#include "mapgen/names.hpp"
#include "mapgen/utils.hpp"
#include <cstring>
#include <functional>
#include <mutex>
#include <thread>
#include <numeric>

template <typename T> using filterFunc = std::function<bool(T *)>;
template <typename T> using sortFunc = std::function<bool(T *, T *)>;

template <typename T>
std::vector<T *> filterObjects(std::vector<T *> regions, filterFunc<T> filter,
                               sortFunc<T> sort) {
  std::vector<T *> places;

  std::copy_if(regions.begin(), regions.end(), std::back_inserter(places),
               filter);
  std::sort(places.begin(), places.end(), sort);
  return places;
}

Simulator::Simulator(Map *m, int s) : map(m), _seed(s) {
  _gen = new std::mt19937(_seed);
  vars = new EconomyVars();
  report = nullptr;
}

void Simulator::simulate() {
  report = new Report();
  // TODO: reset all simulation results (caves, cities, etc)
  resetAll();

  makeRoads();
  makeCaves();

  removeBadPorts();
  makeLighthouses();
  makeLocationRoads();
  makeForts();

  fixRoads();
  removeCities();

  simulateEconomy();

  upgradeCities();
}

void Simulator::removeCities() {
  // mapgen->map->cities.erase(std::remove_if(mapgen->map->cities.begin(), mapgen->map->cities.end(), [](City* c){
  //     return c->roads.}), mapgen->map->cities.end());
}

void Simulator::resetAll() {
  map->status = "Reseting simulation results";
  for (auto c : map->cities) {
    c->isCapital = false;
    c->population = 1000;
    c->wealth = 1.f;
  }
  for (auto r : map->regions) {
    if (r->city == nullptr && r->location != nullptr) {
      map->locations.erase(std::remove(map->locations.begin(),
                                       map->locations.end(), r->location));
      r->location = nullptr;
      r->city = nullptr;
    }

    if (r->city != nullptr && r->city->type == FORT) {
      map->cities.erase(
          std::remove(map->cities.begin(), map->cities.end(), r->city));
      r->megaCluster->cities.erase(std::remove(r->megaCluster->cities.begin(),
                                               r->megaCluster->cities.end(),
                                               r->city));
      r->location = nullptr;
      r->city = nullptr;
    }
  }
  if (map->roads.size() > 0) {
    fixRoads();
  }
}

void Simulator::fixRoads() {
  map->roads.erase(
      std::remove_if(map->roads.begin(), map->roads.end(),
                     [&](Road *r) {
                       return r == nullptr || (r->regions.back()->city == nullptr &&
                               r->regions.back()->location == nullptr) ||
                              (r->regions.front()->city == nullptr &&
                               r->regions.front()->location == nullptr);
                     }),
      map->roads.end());

  for (auto c : map->cities) {
    if (c->roads.size() == 0) {
      c->region->city = nullptr;
      c->region->location = nullptr;

      auto cc = c->region->megaCluster->cities;
      cc.erase(std::remove(cc.begin(), cc.end(), c), cc.end());

      auto cl = map->locations;
      cl.erase(std::remove(cl.begin(), cl.end(), c), cl.end());
      // error: V789 Iterators for the 'map->cities' container, used in the range-based for loop, become invalid upon the call of the 'erase' function.
      map->cities.erase(std::remove(map->cities.begin(), map->cities.end(), c), map->cities.end());
      continue;
    }
    c->roads.erase(
        std::remove_if(c->roads.begin(), c->roads.end(),
                       [&](Road *r) {
                        return r == nullptr || (r->regions.back()->city == nullptr &&
                                 r->regions.back()->location == nullptr) ||
                                (r->regions.front()->city == nullptr &&
                                 r->regions.front()->location == nullptr);
                       }),
        c->roads.end());
  }

  for (auto r : map->roads) {
    auto c1 = r->regions.front()->city;
    auto c2 = r->regions.back()->city;
    if (c1 == nullptr || c2 == nullptr) {
      continue;
    }
    if (std::count(c1->roads.begin(), c1->roads.end(), r) == 0) {
      c1->roads.push_back(r);
    }
    if (std::count(c2->roads.begin(), c2->roads.end(), r) == 0) {
      c2->roads.push_back(r);
    }
  }

}

void Simulator::simulateEconomy() {
  int y = 1;
  while (y <= years) {
    char op[100];
    sprintf(op, "Simulate economy [%d/%d]", y * 10, years * 10);
    map->status = op;
    economyTick(y);
    populationTick(y);
    disasterTick(y);
    y++;
  }
}

void Simulator::populationTick(int) {
  int p = 0.f;
  for (auto c : map->cities) {
    c->population *= (float)(1 +
                             vars->POPULATION_GROWS * c->wealth *
                                 vars->POPULATION_GROWS_WEALTH_MODIFIER);
    c->population = std::max(c->population, 0);
    p += c->population;
    if (c->population > report->maxPopulation) {
      report->maxPopulation = c->population;
    }
    if (c->population != 1000 && c->population < report->minPopulation) {
      report->minPopulation = c->population;
    }
  }
  report->population.push_back(p);
}

void Simulator::disasterTick(int) {
  // int r = rand() % 100;
  // if (r <= 10 && !plague) {
  //   for (auto c : map->cities) {
  //     c->population *= 0.5;
  //   }
  //   plague = true;
  // }
}

void Simulator::economyTick(int y) {
  mg::info("Economy year:", y * 10);
  std::vector<Package *> *goods = new std::vector<Package *>();
  for (auto c : map->cities) {
    c->economyVars = vars;
    auto lg = c->makeGoods(y);
	if (lg != nullptr) {
		goods->push_back(lg);
	}
  }
  unsigned int gc = std::accumulate(goods->begin(), goods->end(), 0,
                            [](int s, Package *p2) { return s + p2->count; });
  mg::info("Goods for sale:", gc);
  std::shuffle(map->cities.begin(), map->cities.end(), *_gen);
  unsigned int sn = 0;
  unsigned int ab = 0;
  for (auto c : map->cities) {
    auto r = c->buyGoods(goods);
	sn += r.first;
	ab += r.second;
  }
  mg::info("Bought:", ab);
  mg::info("Still needs:", sn);

  float w = 0.f;
  for (auto c : map->cities) {
    w += c->wealth;

    if (c->wealth > report->maxWealth) {
      report->maxWealth = c->wealth;
    }
    if (c->wealth != 0.f && c->wealth < report->minWealth) {
      report->minWealth = c->wealth;
    }
  }
  report->wealth.push_back(w);
}

Road *makeRoad(Map *map, City *c, City *oc) {
  auto pather = new micropather::MicroPather(map);
  micropather::MPVector<void *> path;
  float totalCost = 0;
  pather->Reset();
  int result = pather->Solve(c->region, oc->region, &path, &totalCost);
  delete pather;
  if (result != micropather::MicroPather::SOLVED) {
    mg::warn("No road from", *c);
    mg::warn("No road to", *oc);
    return nullptr;
  }
  Road *road = new Road(&path, totalCost);
  return road;
}

void Simulator::makeRoads() {
  map->roads.clear();
  map->status = "Making roads...";
  char op[100];

  const int tc = (map->cities.size() * map->cities.size() - map->cities.size()) / 2;
  int k = 0;
  int n = 1;
#ifdef _WIN32
  std::thread threads[10000];
#else
  std::thread threads[tc];
#endif
  std::mutex g_lock;
  for (auto c : map->cities) {
    for (auto oc :
         std::vector<City *>(map->cities.begin() + n, map->cities.end())) {
      sprintf(op, "Making roads [%d/%d]", k, tc);
      if (c == oc) {
        continue;
      }
      map->status = op;
      threads[k] = std::thread([&](City* c, City* oc) {
        auto road = makeRoad(map, c, oc);
        if (road == nullptr) {
          return;
        }
        g_lock.lock();
        map->roads.push_back(road);
        g_lock.unlock();
        }, c, oc);
      k++;
    };
    n++;
  };
  for (int i = 0; i < tc; ++i) {
    sprintf(op, "Finalize roads [%d/%d]", i, tc);
    map->status = op;
    threads[i].join();
  }
  for (auto r : map->roads) {
    auto c1 = r->regions.front()->city;
    auto c2 = r->regions.back()->city;
    if (std::count(c1->roads.begin(), c1->roads.end(), r) == 0) {
      c1->roads.push_back(r);
    }
    if (std::count(c2->roads.begin(), c2->roads.end(), r) == 0) {
      c2->roads.push_back(r);
    }
  }
  std::shuffle(map->roads.begin(), map->roads.end(), *_gen);
}

void Simulator::makeCaves() {
  map->status = "Digging caves...";
  int i = 0;
  for (auto c : map->clusters) {
    if (c->biom != biom::ROCK) {
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

    if (_cities.size() == 0) {
      continue;
    }
    _cities[0]->isCapital = true;
    mg::info("Capital:", *_cities[0]);

    //TODO: refine city tiers
    // int n = 1;
    // while (n < 4) {
    //   _cities[n]->type = TRADE;
    //   printf("%s is trade\n", _cities[n]->name.c_str());
    //   n++;
    // }
  }
}

void Simulator::removeBadPorts() {
  map->status = "Abandonning ports...";
  std::vector<City *> cities;
  int n = 0;
  std::copy_if(map->cities.begin(), map->cities.end(),
               std::back_inserter(cities), [&](City *c) {
                 bool badPort = c->type == PORT &&
                                c->region->traffic <= int(map->cities.size());
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

  auto _pather = new micropather::MicroPather(map);
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
  delete _pather;
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
                                // if (cond && std::none_of(cache.begin(),
                                // cache.end(), [&](Region *ri){
                                //       for (auto rn : cache) {
                                //         if (mg::getDistance(ri->site,
                                //         rn->site) < 20 && ri->state ==
                                //         rn->state) {
                                //           return true;
                                //         }
                                //       }
                                //       return false;
                                //     })) {
                                //   cache.push_back(region);
                                // } else {
                                //   return false;
                                // }
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
        for (auto oc : map->cities) {
          auto road = makeRoad(map, c, oc);
          if (road == nullptr) {
            continue;
          }
          map->roads.push_back(road);
          c->roads.push_back(road);
          oc->roads.push_back(road);
        }
        map->cities.push_back(c);
        mc->cities.push_back(c);
        n++;
      }
    }
  }
}

template <typename Iter> Iter Simulator::select_randomly(Iter start, Iter end) {
  std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
  std::advance(start, dis(*_gen));
  return start;
}
