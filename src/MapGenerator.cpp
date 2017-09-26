#include "mapgen/MapGenerator.hpp"
#include "mapgen/Biom.hpp"
#include "mapgen/Map.hpp"
#include "mapgen/names.hpp"
#include "mapgen/utils.hpp"
#include "rang.hpp"
#include <VoronoiDiagramGenerator.h>
#include <iterator>
#include <random>

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

const int DEFAULT_RELAX = 5;

bool cellsOrdered(Cell *c1, Cell *c2) {
  sf::Vector2<double> s1 = c1->site.p;
  sf::Vector2<double> s2 = c2->site.p;
  if (s1.y < s2.y)
    return true;
  if (s1.y == s2.y && s1.x < s2.x)
    return true;
  return false;
}

bool sitesOrdered(const sf::Vector2<double> &s1,
                  const sf::Vector2<double> &s2) {
  if (s1.y < s2.y)
    return true;
  if (s1.y == s2.y && s1.x < s2.x)
    return true;
  return false;
}

bool clusterOrdered(Cluster *s1, Cluster *s2) {
  if (s1->regions.size() > s2->regions.size())
    return true;
  return false;
}

template <typename Iter>
Iter MapGenerator::select_randomly(Iter start, Iter end) {
  std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
  std::advance(start, dis(*_gen));
  return start;
}

MapGenerator::MapGenerator(int w, int h) : _w(w), _h(h) {
  _vdg = VoronoiDiagramGenerator();
  _pointsCount = 10000;
  _octaves = 4;
  _freq = 0.3;
  _relax = DEFAULT_RELAX;
  simpleRivers = true;
  _terrainType = "basic";
  temperature = biom::DEFAULT_TEMPERATURE;
  map = nullptr;
  simulator = nullptr;
  _gen = new std::mt19937(_seed);
}

void MapGenerator::makeStates() {
  map->status = "Making states...";
  map->stateClusters.clear();

  _bbox = sf::Rect<double>(0, 0, _w, _h);
  VoronoiDiagramGenerator vdg;
  auto sites = new std::vector<sf::Vector2<double>>();
  genRandomSites(*sites, _bbox, _w, _h, 2);
  std::unique_ptr<Diagram> diagram;
  diagram.reset(vdg.compute(*sites, _bbox));

  int n = 0;
  for (auto c : diagram->cells) {
    State *s = new State((n == 0 ? "Blue empire" : "Red lands"),
                         (n == 0 ? sf::Color::Blue : sf::Color::Red), c);
    map->states.push_back(s);
    n++;
  }

  for (auto r : map->regions) {
    if (!r->megaCluster->isLand) {
      continue;
    }
    for (auto s : map->states) {
      if (s->cell->pointIntersection(r->site->x, r->site->y) != -1) {
        r->state = s;
        auto ms = r->megaCluster->states;
        if (std::count(ms.begin(), ms.end(), s) == 0) {
          r->megaCluster->states.push_back(s);
        }
        auto cs = r->cluster->states;
        if (std::count(cs.begin(), cs.end(), s) == 0) {
          r->cluster->states.push_back(s);
        }
        break;
      }
    }
  }

  auto sc = clusterize(
      [&](Region *r, Region *rn) { return r->state != rn->state; },
      [&](Region *r, Cluster *knownCluster) {
        r->stateCluster = knownCluster;
      },
      [&](Region *rn, Cluster *knownCluster,
          std::map<Region *, Cluster *> *_clusters) {
        Cluster *oldCluster = rn->stateCluster;
        if (oldCluster != knownCluster) {
          rn->stateCluster = knownCluster;
          // rn->biom.color = sf::Color::Black;
          auto kcrn = knownCluster->regions;
          for (Region *orn : oldCluster->regions) {
            orn->stateCluster = knownCluster;
            if (std::find(kcrn.begin(), kcrn.end(), orn) == kcrn.end()) {
              knownCluster->regions.push_back(orn);
            }
            (*_clusters)[orn] = knownCluster;
          }
          oldCluster->regions.clear();
        }
      },
      [&](Region *r) {
        auto cluster = new Cluster();
        if (r->state != nullptr) {
          cluster->states.push_back(r->state);
        }
        return cluster;
      }

      );

  sc.erase(std::remove_if(sc.begin(), sc.end(),
                          [&](Cluster *c) { return c->regions.size() == 0 || c->states.size() == 0; }),
           sc.end());

  map->stateClusters.assign(sc.begin(), sc.end());

  for (auto sc : map->stateClusters) {
    if (sc->regions.size() < 200) {
      if (sc->regions[0]->megaCluster->regions.size() >
          4 * sc->regions.size()) {
        auto oldState = sc->states[0];
        auto newState =
            oldState == map->states[0] ? map->states[1] : map->states[0];
        sc->states.clear();
        sc->states.push_back(newState);
        for (auto r : sc->regions) {
          r->state = newState;
        }
      }
    }
  }

  map->stateClusters.erase(std::remove_if(map->stateClusters.begin(), map->stateClusters.end(),
                          [&](Cluster *c) { return c->regions.size() == 0 || c->states.size() == 0; }),
                           map->stateClusters.end());

  for (auto r : map->regions) {
    if (!r->megaCluster->isLand) {
      continue;
    }

    int sn = 0;
    int en = 0;
    for (auto n : r->neighbors) {
      if (n->state != r->state) {
        r->stateBorder = true;
        en++;
        if (!n->megaCluster->isLand) {
          sn++;
        }
      }
    }
    if (sn == en) {
      r->seaBorder = true;
    }
  }
}

void MapGenerator::simplifyRivers() {
  map->status = "Simplify rivers...";
  for (auto r : map->rivers) {
    PointList *rvr = r->points;
    PointList sr;
    int c = rvr->size();
    int i = 0;
    sr.push_back((*rvr)[0]);
    for (PointList::iterator it = rvr->begin(); it < rvr->end(); it++, i++) {
      Point p = (*rvr)[i];
      Point p2 = (*rvr)[i + 1];
      if (c - i - 1 < 2) {
        break;
      }

      Point p3 = (*rvr)[i + 2];
      Point sp;

      double d1 = mg::getDistance(p, p2);
      double d2 = mg::getDistance(p, p3);

      if (d2 < d1) {
        sp = p2;
      } else {
        sp = p3;
        i++;
      }

      sr.push_back(sp);
    }

    rvr->clear();
    rvr->reserve(sr.size());
    i = 0;
    for (PointList::iterator it = sr.begin(); it < sr.end(); it++, i++) {
      Point p(sr[i]);
      if (p->x <= 0.f || p->x != p->x || p->y <= 0.f || p->y != p->y) {
        continue;
      }
      rvr->push_back(sr[i]);
    }
  }
  std::cout << "end\n" << std::flush;
}

void MapGenerator::makeRelax() {
  try {
    _diagram.reset(_vdg.relax());
  } catch (const std::exception &e) {
    std::cout << "Relax failed" << std::endl << std::flush;
  }
}

void MapGenerator::seed() {
  _seed = std::clock();
  _relax = DEFAULT_RELAX;
  printf("New seed: %d\n", _seed);
}

void MapGenerator::setSeed(int s) {
  _relax = DEFAULT_RELAX;
  _seed = s;
}

int MapGenerator::getSeed() { return _seed; }

int MapGenerator::getOctaveCount() { return _octaves; }

void MapGenerator::setOctaveCount(int o) { _octaves = o; }

int MapGenerator::getPointCount() { return _pointsCount; }

float MapGenerator::getFrequency() { return _freq; }

void MapGenerator::setFrequency(float f) { _freq = f; }

void MapGenerator::setPointCount(int c) { _pointsCount = c; }

void MapGenerator::update() {
  ready = false;
  if (map != nullptr) {
    delete map;
    delete simulator;
  }

  map = new Map();
  simulator = new Simulator(map, _seed);
  makeHeights();
  makeDiagram();

  makeRegions();
  makeMegaClusters();

  makeRivers();
  if (simpleRivers) {
    simplifyRivers();
  }
  calcHumidity();
  calcTemp();

  makeMinerals();
  makeBorders();
  makeFinalRegions();
  makeClusters();

  makeCities();
  makeStates();

  ready = true;
}

void MapGenerator::startSimulation() {
  map->status = "";
  ready = false;
  simulator->simulate();
  ready = true;
}

void MapGenerator::getSea(std::vector<Region *> *seas, Region *base,
                          Region *r) {
  for (auto n : r->neighbors) {
    if (!n->megaCluster->isLand &&
        std::find(seas->begin(), seas->end(), n) == seas->end()) {
      seas->push_back(n);
      float d = mg::getDistance(base->site, n->site);
      if (d < 100) {
        getSea(seas, base, n);
      }
    }
  }
};

void MapGenerator::makeCities() {
  map->status = "Founding cities...";
  // MegaCluster *biggestCluster;
  // for (auto c : map->megaClusters) {
  //   if (c->isLand) {
  //     biggestCluster = c;
  //     break;
  //   }
  // }

  std::vector<Region *> places;

  std::vector<Region *> cache;
  for (auto mc : map->megaClusters) {
    if (!mc->isLand) {
      continue;
    }
    places = filterObjects(mc->regions,
                           (filterFunc<Region>)[&](Region * r) {
                             bool cond = r->city == nullptr &&
                                         r->minerals > 1 &&
                                         r->biom.name != biom::LAKE.name &&
                                         r->biom.name != biom::SNOW.name &&
                                         r->biom.name != biom::ICE.name;
                             // if (cond && std::none_of(cache.begin(), cache.end(), [&](Region *ri){
                             //       for (auto rn : cache) {
                             //         if (mg::getDistance(ri->site, rn->site) < 0) {
                             //           return true;
                             //         }
                             //       }
                             //       return false;
                             //     })) {
                             //   cache.push_back(r);
                             // } else {
                             //   return false;
                             // }
                             return cond;
                           },
                           (sortFunc<Region>)[&](Region * r, Region * r2) {
                             if (r->minerals > r2->minerals) {
                               return true;
                             }
                             return false;
                           });
    for (auto r : places) {
      bool canPlace = true;
      for (auto n : r->neighbors) {
        if (n->city != nullptr) {
          canPlace = false;
          break;
        }
      }
      if (!canPlace) {
        continue;
      }
      // TODO: decluster it.
      City *c = new City(r, names::generateCityName(_gen), MINE);
      map->cities.push_back(c);
      mc->cities.push_back(c);
    }
  }

  for (auto mc : map->megaClusters) {
    if (!mc->isLand) {
      continue;
    }
    places = filterObjects(
        mc->regions,
        (filterFunc<Region>)[&](Region * r) {
          return r->city == nullptr && r->nice > 0.8 &&
                 r->biom.feritlity > 0.7 && r->biom.name != biom::LAKE.name;
        },
        (sortFunc<Region>)[&](Region * r, Region * r2) {
          if (r->nice * r->biom.feritlity > r2->nice * r2->biom.feritlity) {
            return true;
          }
          return false;
        });
    for (auto r : places) {
      bool canPlace = true;
      for (auto n : r->neighbors) {
        if (n->city != nullptr) {
          canPlace = false;
          break;
        }
      }
      if (!canPlace) {
        continue;
      }
      City *c = new City(r, names::generateCityName(_gen), AGRO);
      map->cities.push_back(c);
      mc->cities.push_back(c);
    }
  }

  for (auto mc : map->megaClusters) {
    if (!mc->isLand) {
      continue;
    }

    int b = 200;

    std::vector<Region *> cache;
    places = filterObjects(
        mc->regions,
        (filterFunc<Region>)[&](Region * r) {
          if (r->megaCluster->cities.size() == 0) {
            return false;
          }

          bool deep = false;
          for (auto n : r->neighbors) {
            if (n->getHeight(n->site) < 0.01) {
              deep = true;
              break;
            }
          }
          if (!deep || r->city != nullptr) {
            return false;
          }
          std::vector<Region *> seas;
          getSea(&seas, r, r);
          int sc = int(seas.size());

          bool cond = (int)sc < b;

          if (cond) {
            for (auto cc : cache) {
              if (mg::getDistance(r->site, cc->site) < 200) {
                return false;
              }
            }

            cache.push_back(r);
          }

          return cond;
        },
        (sortFunc<Region>)[&](Region * r, Region * r2) { return false; });

    for (auto r : places) {
      bool canPlace = true;
      for (auto n : r->neighbors) {
        if (n->city != nullptr) {
          canPlace = false;
          break;
        }
      }
      if (!canPlace) {
        continue;
      }

      City *c = new City(r, names::generateCityName(_gen), PORT);
      map->cities.push_back(c);
      mc->cities.push_back(c);
      mc->hasPort = true;
    }
  }

  for (auto mc : map->megaClusters) {
    if (!mc->hasPort && mc->cities.size() > 0) {
      places = filterObjects(
          mc->regions,
          (filterFunc<Region>)[&](Region * r) {

            bool deep = false;
            for (auto n : r->neighbors) {
              if (!n->megaCluster->isLand) {
                deep = true;
                break;
              }
            }
            if (!deep || r->city != nullptr) {
              return false;
            }
            return true;
          },
          (sortFunc<Region>)[&](Region * r, Region * r2) { return false; });
      std::cout << places.size() << std::endl << std::flush;

      if (places.size() == 0) {
        continue;
      }

      auto r = *select_randomly(places.begin(), places.end());

      City *c = new City(r, names::generateCityName(_gen), PORT);
      map->cities.push_back(c);
      mc->cities.push_back(c);
      mc->hasPort = true;
    }
  }
}

void MapGenerator::makeMinerals() {
  map->status = "Search for minerals...";
  utils::NoiseMapBuilderPlane heightMapBuilder;
  heightMapBuilder.SetDestNoiseMap(_mineralsMap);

  module::Billow minerals;
  minerals.SetSeed(_seed + 5);
  heightMapBuilder.SetSourceModule(minerals);

  heightMapBuilder.SetDestSize(_w, _h);
  heightMapBuilder.SetBounds(10.0, 20.0, 10.0, 20.0);
  heightMapBuilder.Build();
}

void MapGenerator::makeBorders() {
  for (auto c : map->megaClusters) {
    for (auto r : c->regions) {
      if (!r->border) {
        continue;
      }

      Cell *c = r->cell;
      for (auto n : c->getNeighbors()) {
        Region *rn = _cells[n];
        if (rn->biom.name != r->biom.name) {
          for (auto e : n->getEdges()) {
            if (c->pointIntersection(e->startPoint()->x, e->startPoint()->y) ==
                0) {
              r->megaCluster->border.push_back(e->startPoint());
            }
          }
        }
      }
    }
    // TODO: sort points by distance;
    // TODO: do something with inner points
    // std::sort(r->megaCluster->border.begin(), r->megaCluster->border.end(),
    // borderOrdered);
  }
}

void MapGenerator::setMapTemplate(const char *templateName) {
  // TODO: make enum
  _terrainType = std::string(templateName);
}

void MapGenerator::makeHeights() {
  map->status = "Making mountains and seas...";
  utils::NoiseMapBuilderPlane heightMapBuilder;
  heightMapBuilder.SetDestNoiseMap(_heightMap);

  _perlin.SetSeed(_seed);
  _perlin.SetOctaveCount(_octaves);
  _perlin.SetFrequency(_freq);

  module::Billow terrainType;
  module::RidgedMulti mountainTerrain;
  module::Select finalTerrain;
  module::ScaleBias flatTerrain;
  module::Turbulence tModule;

  if (_terrainType == "archipelago") {

    terrainType.SetFrequency(0.5);
    terrainType.SetPersistence(0.5);

    terrainType.SetSeed(_seed + 1);
    mountainTerrain.SetSeed(_seed + 2);

    finalTerrain.SetSourceModule(0, _perlin);
    finalTerrain.SetSourceModule(1, mountainTerrain);
    finalTerrain.SetControlModule(terrainType);
    finalTerrain.SetBounds(0.0, 100.0);
    finalTerrain.SetEdgeFalloff(0.125);
    heightMapBuilder.SetSourceModule(terrainType);

  } else if (_terrainType == "new") {
    terrainType.SetFrequency(0.3);
    terrainType.SetPersistence(0.4);

    terrainType.SetSeed(_seed);
    mountainTerrain.SetSeed(_seed);

    flatTerrain.SetSourceModule(0, _perlin);
    flatTerrain.SetScale(0.3);
    flatTerrain.SetBias(0.1);

    mountainTerrain.SetFrequency(0.9);

    tModule.SetFrequency(1.5);
    tModule.SetPower(1.5);

    finalTerrain.SetSourceModule(0, tModule);
    finalTerrain.SetSourceModule(1, flatTerrain);
    finalTerrain.SetSourceModule(2, mountainTerrain);
    finalTerrain.SetControlModule(terrainType);
    finalTerrain.SetBounds(0.0, 100.0);
    finalTerrain.SetEdgeFalloff(0.125);
    heightMapBuilder.SetSourceModule(terrainType);

  } else {
    heightMapBuilder.SetSourceModule(_perlin);
  }

  heightMapBuilder.SetDestSize(_w, _h);
  heightMapBuilder.SetBounds(0.0, 10.0, 0.0, 10.0);
  heightMapBuilder.Build();
  std::cout << "Height generation finished\n" << std::flush;
}

void MapGenerator::makeRiver(Region *r) {
  map->status = "Making rivers...";
  std::vector<Cell *> visited;
  Cell *c = r->cell;
  float z = r->getHeight(r->site);
  River *rvr = new River();

  rvr->name = names::generateRiverName(_gen);
  PointList *river = new PointList();
  rvr->points = river;
  map->rivers.push_back(rvr);
  river->push_back(r->site);
  visited.push_back(c);

  Point next = r->site;
  for (auto e : c->getEdges()) {
    if (r->getHeight(e->startPoint()) < z) {
      next = e->startPoint();
      z = r->getHeight(next);
    }
  }

  int count = 0;
  while (count < 100) {
    std::vector<Cell *> n = c->getNeighbors();
    Cell *end;

    for (Cell *c2 : n) {
      if (std::find(visited.begin(), visited.end(), c2) != visited.end()) {
        continue;
      }
      visited.push_back(c2);
      r = _cells[c2];
      r->megaCluster->hasRiver = true;
      bool f = false;
      for (auto e : c2->getEdges()) {
        if (r->getHeight(e->startPoint()) < z) {
          next = e->startPoint();
          z = r->getHeight(next);
          c = c2;
          f = true;
        }
      }
      if (f) {
        river->push_back(r->site);
        rvr->regions.push_back(r);
        r->hasRiver = true;
        r->biom.feritlity += 0.2;
      }
      end = c2;
    }
    count++;
    if (count == 100) {
      r->biom = biom::LAKE;
      river->push_back(r->site);
      rvr->regions.push_back(r);
      r->humidity = 1;

      for (auto n : end->getNeighbors()) {
        r = _cells[n];
        r->biom = biom::LAKE;
        r->humidity = 1;
      }
      break;
    }
    if (r->getHeight(r->site) < 0.0625) {
      river->push_back(r->site);
      break;
    }
  }
}

void MapGenerator::makeRivers() {
  // TODO: make more rivers
  map->status = "Making rivers...";
  map->rivers.clear();

  std::vector<Region *> localMaximums;
  for (auto cluster : map->megaClusters) {
    if (!cluster->isLand || cluster->regions.size() < 50) {
      continue;
    }
    for (auto r : cluster->regions) {
      Cell *c = r->cell;
      if (c == nullptr) {
        continue;
      }
      auto ns = c->getNeighbors();
      if (std::count_if(ns.begin(), ns.end(),
                        [&](Cell *oc) {
                          Region *reg = _cells[oc];
                          return reg->getHeight(reg->site) >
                                 r->getHeight(r->site);
                        }) == 0 &&
          r->getHeight(r->site) > 0.66) {
        localMaximums.push_back(r);
      }
    }
  }

  for (auto lm : localMaximums) {
    makeRiver(lm);
  }
}

void MapGenerator::makeFinalRegions() {
  map->status = "Making forrests and deserts...";
  for (auto r : map->regions) {
    if (r->biom.name == biom::LAKE.name) {
      r->minerals = 0;
      continue;
    }
    r->minerals = _mineralsMap.GetValue(r->site->x, r->site->y);
    r->minerals = r->minerals > 0 ? r->minerals : 0;
    float ht = r->getHeight(r->site);
    Biom b = biom::BIOMS[0];
    for (int i = 0; i < int(biom::BIOMS.size()); i++) {
      if (ht > biom::BIOMS[i].border) {
        int n = (biom::BIOMS_BY_HEIGHT[i].size() - 1) -
                r->humidity * (biom::BIOMS_BY_HEIGHT[i].size() - 1);
        b = biom::BIOMS_BY_HEIGHT[i][n];
        if (biom::BIOMS_BY_TEMP.count(b.name) != 0) {
          if (r->temperature > temperature * 4 / 5 && r->humidity < 0.2) {
            b = biom::BIOMS_BY_TEMP.at(b.name);
          }
        }
      }
    }
    r->biom = b;
    float hc = (1.f - std::abs(r->humidity - 0.8f));
    hc = hc <= 0 ? 0 : hc / 3.f;
    float hic = (1.f - std::abs(r->getHeight(r->site) - 0.7f));
    hic = hic <= 0 ? 0 : hic / 3.f;
    float tc = (1.f - std::abs(r->temperature - temperature * 2.f / 3.f));
    tc = tc <= 0 ? 0 : tc / 3.f;

    r->nice = hc + hic + tc;
  }

  for (auto cluster : map->megaClusters) {
    if (!cluster->isLand) {
      continue;
    }
    for (auto r : cluster->regions) {
      Cell *c = r->cell;
      if (c == nullptr) {
        continue;
      }
      auto ns = c->getNeighbors();
      if (std::count_if(ns.begin(), ns.end(),
                        [&](Cell *oc) {
                          Region *reg = _cells[oc];
                          return reg->minerals > r->minerals;
                        }) == 0 &&
          r->minerals != 0) {
        cluster->resourcePoints.push_back(r);
      }

      if (std::count_if(ns.begin(), ns.end(),
                        [&](Cell *oc) {
                          Region *reg = _cells[oc];
                          return reg->nice >= r->nice;
                        }) == 0 &&
          r->biom.name != biom::LAKE.name) {
        cluster->goodPoints.push_back(r);
      }
    }
  }
}

void MapGenerator::makeRegions() {
  map->status = "Spliting land and sea...";
  _cells.clear();
  map->regions.clear();
  map->regions.reserve(_diagram->cells.size());
  Biom lastBiom = biom::BIOMS[0];
  for (auto c : _diagram->cells) {
    if (c == nullptr) {
      continue;
    }

    PointList verts;
    int count = int(c->getEdges().size());
    verts.reserve(count);

    float ht = 0;
    std::map<sf::Vector2<double> *, float> h;
    for (int i = 0; i < count; i++) {
      sf::Vector2<double> *p0;
      p0 = c->getEdges()[i]->startPoint();
      verts.push_back(p0);

      h.insert(std::make_pair(p0, _heightMap.GetValue(p0->x, p0->y)));
      ht += h[p0];
    }
    ht = ht / count;
    sf::Vector2<double> &p = c->site.p;
    h.insert(std::make_pair(&p, ht));
    Biom b = ht < 0.0625 ? biom::SEA : biom::LAND;
    Region *region = new Region(b, verts, h, &p);
    region->city = nullptr;
    region->cell = c;
    region->humidity = biom::DEFAULT_HUMIDITY;
    region->border = false;
    region->hasRiver = false;
    map->regions.push_back(region);
    _cells.insert(std::make_pair(c, region));
  }

  for (auto r : map->regions) {
    auto c = r->cell;
    for (auto n : c->getNeighbors()) {
      r->neighbors.push_back(_cells[n]);
    }
  }
}

bool isDiscard(const Cluster *c) { return c->regions.size() == 0; }

void MapGenerator::calcTemp() {
  map->status = "Making world cool...";
  for (auto r : map->regions) {
    if (!r->megaCluster->isLand) {
      r->temperature = temperature + 5;
      continue;
    }
    // TODO: adjust it
    r->temperature = temperature - (temperature / 5 * r->humidity) -
                     (temperature / 1.2 * r->getHeight(r->site));
    Cell *c = r->cell;
    for (auto n : c->getNeighbors()) {
      if (_cells[n]->biom.name == biom::LAKE.name) {
        r->temperature += 2;
        r->biom.feritlity += 0.2;
      }
    }
  }
}

void MapGenerator::calcHumidity() {
  map->status = "Making world moist...";
  for (auto r : map->regions) {
    if (!r->megaCluster->isLand) {
      r->humidity = 1;
      continue;
    }
    if (r->hasRiver) {
      r->humidity += 0.2;
    }
  }

  auto calcRegionsHum = [&]() {
    for (auto r : map->regions) {
      if (!r->megaCluster->isLand || r->humidity >= 0.9) {
        continue;
      }
      Cell *c = r->cell;
      if (c == nullptr) {
        continue;
      }
      for (auto n : c->getNeighbors()) {
        Region *rn = _cells[n];
        if (rn->hasRiver || rn->biom.name == biom::LAKE.name) {
          r->humidity += 0.05;
        }
        float hd = rn->getHeight(rn->site) - r->getHeight(r->site);
        if (rn->humidity > r->humidity && r->humidity != 1 && hd < 0.04) {
          r->humidity += (rn->humidity - r->humidity) / (1.8f - (hd * 2));
        }
      }
      r->humidity = std::min(0.9f, float(r->humidity));
    }
  };

  calcRegionsHum();
  std::reverse(map->regions.begin(), map->regions.end());
  calcRegionsHum();
  std::reverse(map->regions.begin(), map->regions.end());
}

std::vector<Cluster *> MapGenerator::clusterize(sameFunc isNotSame,
                                                assignFunc assignCluster,
                                                reassignFunc reassignCluster,
                                                createFunc createCluster) {
  std::vector<Cluster *> clusters;

  std::map<Region *, Cluster *> _clusters;
  for (auto r : map->regions) {
    Cell *c = r->cell;
    bool cu = true;
    Cluster *knownCluster = nullptr;
    for (auto n : c->getNeighbors()) {
      Region *rn = _cells[n];
      if (isNotSame(r, rn)) {
        r->border = true;
      } else if (_clusters.count(rn) != 0) {
        cu = false;
        if (knownCluster == nullptr) {
          knownCluster = _clusters[rn];
          knownCluster->regions.push_back(r);
          _clusters[r] = knownCluster;

          assignCluster(r, knownCluster);
        } else {
          _clusters[rn] = knownCluster;
          reassignCluster(rn, knownCluster, &_clusters);
          assignCluster(r, knownCluster);
          _clusters[r] = knownCluster;
        }
        continue;
      }
    }

    if (cu) {
      auto cluster = createCluster(r);
      assignCluster(r, cluster);

      _clusters[r] = cluster;
      clusters.push_back(cluster);
    }
  }

  clusters.erase(std::remove_if(clusters.begin(), clusters.end(), isDiscard),
                 clusters.end());
  std::sort(clusters.begin(), clusters.end(), clusterOrdered);
  return clusters;
}

void MapGenerator::makeMegaClusters() {
  map->status = "Finding far lands...";
  map->megaClusters.clear();

  auto mc = clusterize(
      [&](Region *r, Region *rn) { return r->biom.name != rn->biom.name; },
      [&](Region *r, Cluster *knownCluster) {
        r->megaCluster = knownCluster;
        r->cluster = knownCluster;
      },
      [&](Region *rn, Cluster *knownCluster,
          std::map<Region *, Cluster *> *_clusters) {
        Cluster *oldCluster = rn->megaCluster;
        if (oldCluster != knownCluster) {
          rn->cluster = knownCluster;
          for (Region *orn : oldCluster->regions) {
            orn->cluster = knownCluster;
            orn->megaCluster = knownCluster;
            auto kcrn = knownCluster->regions;
            if (std::find(kcrn.begin(), kcrn.end(), orn) == kcrn.end()) {
              knownCluster->regions.push_back(orn);
            }
            (*_clusters)[orn] = knownCluster;
          }
          oldCluster->regions.clear();
        }
      },
      [&](Region *r) {
        Cluster *cluster = new MegaCluster();
        cluster->isLand = r->biom.name == biom::LAND.name;
        cluster->megaCluster = cluster;
        if (cluster->isLand) {
          cluster->name = names::generateLandName(_gen);
        } else {
          cluster->name = names::generateSeaName(_gen);
        }
        return cluster;
      }

      );

  map->megaClusters.assign(mc.begin(), mc.end());
}

// TODO: use clusterize instead this code
void MapGenerator::makeClusters() {
  map->status = "Meeting with neighbors...";
  map->clusters.clear();
  cellsMap.clear();
  std::map<Cell *, Cluster *> _clusters;
  for (auto c : _diagram->cells) {
    Region *r = _cells[c];
    bool cu = true;
    Cluster *knownCluster = nullptr;
    for (auto n : c->getNeighbors()) {
      Region *rn = _cells[n];
      if (r->biom.name != rn->biom.name) {
        r->border = true;
      } else if (_clusters.count(n) != 0) {
        cu = false;
        if (knownCluster == nullptr) {
          r->cluster = _clusters[n];
          _clusters[n]->regions.push_back(r);
          _clusters[c] = _clusters[n];
          knownCluster = _clusters[n];
        } else {
          Cluster *oldCluster = rn->cluster;
          if (oldCluster != knownCluster) {
            rn->cluster = knownCluster;
            _clusters[n] = knownCluster;
            for (Region *orn : oldCluster->regions) {
              orn->cluster = knownCluster;
              auto kcrn = knownCluster->regions;
              if (std::find(kcrn.begin(), kcrn.end(), orn) == kcrn.end()) {
                knownCluster->regions.push_back(orn);
              }
              _clusters[cellsMap[orn]] = knownCluster;
            }
            oldCluster->regions.clear();
          }

          r->cluster = knownCluster;
          _clusters[c] = knownCluster;
          cellsMap[r] = c;
        }
        continue;
      }
    }
    if (cu) {
      Cluster *cluster = new Cluster();
      char buff[100];
      snprintf(buff, sizeof(buff), "%p", (void *)cluster);
      std::string buffAsStdStr = buff;
      cluster->name = buffAsStdStr;
      cluster->hasRiver = false;
      r->cluster = cluster;
      cluster->biom = r->biom;
      cluster->isLand = r->biom.border > 0;
      cluster->regions.push_back(r);
      _clusters[c] = cluster;
      cellsMap[r] = c;
      map->clusters.push_back(cluster);
    }
  }
  map->clusters.erase(
      std::remove_if(map->clusters.begin(), map->clusters.end(), isDiscard),
      map->clusters.end());
  std::sort(map->clusters.begin(), map->clusters.end(), clusterOrdered);
  for (auto c : map->clusters) {
    c->megaCluster = c->regions[0]->megaCluster;
  }
}

Region *MapGenerator::getRegion(sf::Vector2f pos) {
  for (auto &pair : _cells) {
    Cell *c = pair.first;
    if (c->pointIntersection(pos.x, pos.y) != -1) {
      return pair.second;
    }
  }
  return nullptr;
}

void MapGenerator::setSize(int w, int h) {
  _w = w;
  _h = h;
}

int MapGenerator::getRelax() { return _relax; }

bool damagedCell(Cell *c) {
  return c->pointIntersection(c->site.p.x, c->site.p.y) == -1;
}

void MapGenerator::makeDiagram() {
  map->status = "Making nothing...";
  _bbox = sf::Rect<double>(0, 0, _w, _h);
  _sites = new std::vector<sf::Vector2<double>>();
  genRandomSites(*_sites, _bbox, _w, _h, _pointsCount);
  _diagram.reset(_vdg.compute(*_sites, _bbox));
  for (int n = 0; n < _relax; n++) {
    map->status = "Relaxing...";
    makeRelax();
  }

  while (std::count_if(_diagram->cells.begin(), _diagram->cells.end(),
                       damagedCell) != 0) {
    _relax++;
    makeRelax();
  }
  delete _sites;

  std::sort(_diagram->cells.begin(), _diagram->cells.end(), cellsOrdered);
  std::cout << "Diagram generation finished: " << _diagram->cells.size() << "\n"
            << std::flush;
}

void MapGenerator::genRandomSites(std::vector<sf::Vector2<double>> &sites,
                                  sf::Rect<double> &bbox, unsigned int dx,
                                  unsigned int dy, unsigned int numSites) {
  std::vector<sf::Vector2<double>> tmpSites;

  tmpSites.reserve(numSites);
  sites.reserve(numSites);

  sf::Vector2<double> s;

  srand(_seed);
  for (unsigned int i = 0; i < numSites; ++i) {
    s.x = 1 + (rand() / (double)RAND_MAX) * (dx - 2);
    s.y = 1 + (rand() / (double)RAND_MAX) * (dy - 2);
    tmpSites.push_back(s);
  }

  // remove any duplicates that exist
  std::sort(tmpSites.begin(), tmpSites.end(), sitesOrdered);
  sites.push_back(tmpSites[0]);
  for (sf::Vector2<double> &s : tmpSites) {
    if (s != sites.back())
      sites.push_back(s);
  }
}
