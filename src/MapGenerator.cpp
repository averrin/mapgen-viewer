#include "mapgen/MapGenerator.hpp"
#include "Biom.cpp"
#include "City.cpp"
#include "micropather.cpp"
#include "names.cpp"
#include <VoronoiDiagramGenerator.h>
#include <iterator>
#include <random>

const int DEFAULT_RELAX = 5;

double normalize(double in, int dimension) {
  return in / (float)dimension * 1.8 - 0.9;
}

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
  std::mt19937 gen(_seed);
  std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
  std::advance(start, dis(gen));
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
  currentOperation = "";
  temperature = DEFAULT_TEMPERATURE;
  map = nullptr;
}

double getDistance(Point p, Point p2) {
  double distancex = (p2->x - p->x);
  double distancey = (p2->y - p->y);

  return std::sqrt(distancex * distancex + distancey * distancey);
}

void MapGenerator::simplifyRivers() {
  currentOperation = "Simplify rivers...";
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

      double d1 = getDistance(p, p2);
      double d2 = getDistance(p, p3);

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

void MapGenerator::makeRelax() { _diagram.reset(_vdg.relax()); }

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
  }

  map = new Map();
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
  makeRoads();

  ready = true;
}

void  MapGenerator::makeRoads() {
  map->roads.clear();
  currentOperation = "Making roads...";
  _pather = new micropather::MicroPather( map );

  MegaCluster *biggestCluster;
  for (auto c : map->megaClusters) {
    if (c->isLand) {
      biggestCluster = c;
      break;
    }
  }

  // for (auto c: biggestCluster->cities){
  //   micropather::MPVector< void* > path;
  //   float totalCost = 0;
  //   _pather->Reset();
  //   int result = _pather->Solve( map->cities[0]->region, c->region, &path, &totalCost );
  //   if (result != micropather::MicroPather::SOLVED) {
  //     continue;
  //   }
  //   printf("Path [%d] length: %d with cost: %f", result, path.size(),totalCost);
  //   std::cout<<result<<std::endl<<std::flush;
  //   std::vector<Region*> road;
  //   unsigned size = path.size();
  //   for(int k=0; k<size; ++k ) {
  //     auto ptr = path[k];
  //     road.push_back((Region*)ptr);
  //   }
  //   map->roads.push_back(road);
  // };

  std::map<City*,City*> pathCache;
  int tc = map->cities.size()*map->cities.size();
  int k = 0;
  for (auto c: map->cities){
    for (auto oc: map->cities){
      if (c == oc || (pathCache[c] == oc)) {
        tc--;
        continue;
      }
      k++;
      char op[100];
      sprintf(op, "Making roads [%d/%d]", k, tc);
      currentOperation = op;
      pathCache[oc] = c;
      micropather::MPVector< void* > path;
      float totalCost = 0;
      _pather->Reset();
      int result = _pather->Solve( c->region, oc->region, &path, &totalCost );
      if (result != micropather::MicroPather::SOLVED) {
        continue;
      }
      // printf("Path [%d] length: %d with cost: %f", result, path.size(),totalCost);
      // std::cout<<result<<std::endl<<std::flush;
      std::vector<Region*> road;
      unsigned size = path.size();
      for(int k=0; k<size; ++k ) {
        auto ptr = path[k];
        road.push_back((Region*)ptr);
      }
      map->roads.push_back(road);
    };
  };
}

void MapGenerator::makeCities() {
  currentOperation = "Founding cities...";
  // double minDistance = std::numeric_limits<double>::max();
  // Region *betterPlace;
  MegaCluster *biggestCluster;
  for (auto c : map->megaClusters) {
    if (c->isLand) {
      biggestCluster = c;
      break;
    }
  }

  std::vector<Region *> places;
  places = filterRegions(
    biggestCluster->regions,
    [&](Region *r) {
      return r->biom.name != LAKE.name && r->nice >= 0.5 &&
              r->temperature >= DEFAULT_TEMPERATURE / 3 &&
              r->hasRiver;
    },
    [&](Region *r, Region *r2) {
      if (r->nice + r->minerals >= r2->nice + r2->minerals) {
        return true;
      }
      return false;
    });
  if (places.size() == 0) {
    return;
  }

  City *capital = new City(places[0], generateCityName(), CAPITAL);
  capital->isCapital = true;
  map->cities.push_back(capital);
  biggestCluster->cities.push_back(capital);

  for (auto r : map->regions) {
    r->distanceFormCapital = getDistance(r->site, capital->region->site);
  }

  if (!capital->region->coast) {
    places = filterRegions(
        biggestCluster->regions,
        [&](Region *r) {
          return r->border && r->hasRiver && r->coast && r->city == nullptr && r->biom.name!=LAKE.name;
        },
        [&](Region *r, Region *r2) {
          if (r->distanceFormCapital <= r2->distanceFormCapital) {
            return true;
          }
          return false;
        });

    City *port = new City(places[0], generateCityName(), PORT);
    map->cities.push_back(port);
    biggestCluster->cities.push_back(port);
  }

  for (auto mc: map->megaClusters) {
    if (!mc->isLand) {
      continue;
    }
    places = filterRegions(
                           mc->regions,
                           [&](Region *r) {
                             return r->city == nullptr && r->minerals > 1 && r->biom.name!=LAKE.name && r->biom.name!=SNOW.name && r->biom.name!=ICE.name;
                           },
                           [&](Region *r, Region *r2) {
                             if (r->minerals > r2->minerals) {
                               return true;
                             }
                             return false;
                           });
    for (auto r : places) {
      bool canPlace = true;
      for (auto n: r->neighbors) {
        if (n->city != nullptr) {
          canPlace = false;
          break;
        }
      }
      if (!canPlace) {
        continue;
      }
      City *c = new City(r, generateCityName(), MINE);
      map->cities.push_back(c);
      mc->cities.push_back(c);
    }
  }

  for (auto mc: map->megaClusters) {
    if (!mc->isLand) {
      continue;
    }
    places = filterRegions(
                           mc->regions,
                           [&](Region *r) {
                             return r->city == nullptr && r->nice > 0.8 && r->biom.feritlity > 0.7 && r->biom.name!=LAKE.name;
                           },
                           [&](Region *r, Region *r2) {
                             if (r->nice*r->biom.feritlity > r2->nice*r2->biom.feritlity) {
                               return true;
                             }
                             return false;
                           });
    for (auto r : places) {
      bool canPlace = true;
      for (auto n: r->neighbors) {
        if (n->city != nullptr) {
          canPlace = false;
          break;
        }
      }
      if (!canPlace) {
        continue;
      }
      City *c = new City(r, generateCityName(), AGRO);
      map->cities.push_back(c);
      mc->cities.push_back(c);
    }
  }

}

std::vector<Region *> MapGenerator::filterRegions(std::vector<Region *> regions,
                                                  filterFunc filter,
                                                  sortFunc sort) {
  std::vector<Region *> places;

  std::copy_if(regions.begin(), regions.end(), std::back_inserter(places),
               filter);
  std::sort(places.begin(), places.end(), sort);
  return places;
}

void MapGenerator::makeMinerals() {
  currentOperation = "Search for minerals...";
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
  currentOperation = "Making mountains and seas...";
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
  currentOperation = "Making rivers...";
  std::vector<Cell *> visited;
  Cell *c = r->cell;
  float z = r->getHeight(r->site);
  River *rvr = new River();

  rvr->name = generateRiverName();
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
        r->hasRiver = true;
        r->biom.feritlity+=0.2;
      }
      end = c2;
    }
    count++;
    if (count == 100) {
      r->biom = LAKE;
      river->push_back(r->site);
      r->humidity = 1;

      for (auto n : end->getNeighbors()) {
        r = _cells[n];
        r->biom = LAKE;
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
  currentOperation = "Making rivers...";
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
          r->getHeight(r->site) > 0.7) {
        localMaximums.push_back(r);
      }
    }
  }

  std::cout << "Before Making rivers" << std::endl << std::flush;
  for (auto lm : localMaximums) {
    makeRiver(lm);
  }

  std::cout << "Finish Making rivers" << std::endl << std::flush;
}

void MapGenerator::makeFinalRegions() {
  currentOperation = "Making forrests and deserts...";
  for (auto r : map->regions) {
    if (r->biom.name == LAKE.name) {
      r->minerals = 0;
      continue;
    }
    r->minerals = _mineralsMap.GetValue(r->site->x, r->site->y);
    r->minerals = r->minerals > 0 ? r->minerals : 0;
    float ht = r->getHeight(r->site);
    Biom b = BIOMS[0];
    for (int i = 0; i < int(BIOMS.size()); i++) {
      if (ht > BIOMS[i].border) {
        int n = (BIOMS_BY_HEIGHT[i].size() - 1) -
                r->humidity * (BIOMS_BY_HEIGHT[i].size() - 1);
        b = BIOMS_BY_HEIGHT[i][n];
        if (BIOMS_BY_TEMP.count(b.name) != 0) {
          if (r->temperature > temperature * 4 / 5 && r->humidity < 0.2) {
            b = BIOMS_BY_TEMP[b.name];
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
          r->biom.name != LAKE.name) {
        cluster->goodPoints.push_back(r);
      }
    }
  }
}

void MapGenerator::makeRegions() {
  currentOperation = "Spliting land and sea...";
  _cells.clear();
  map->regions.clear();
  map->regions.reserve(_diagram->cells.size());
  Biom lastBiom = BIOMS[0];
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
    Biom b = ht < 0.0625 ? SEA : LAND;
    Region *region = new Region(b, verts, h, &p);
    region->city = nullptr;
    region->coast = false;
    region->cell = c;
    region->humidity = DEFAULT_HUMIDITY;
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
  currentOperation = "Making world cool...";
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
      if (_cells[n]->biom.name == LAKE.name) {
        r->temperature += 2;
        r->biom.feritlity+=0.2;
      }
    }
  }
}

void MapGenerator::calcHumidity() {
  currentOperation = "Making world moist...";
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
        if (rn->hasRiver || rn->biom.name == LAKE.name) {
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

void MapGenerator::makeMegaClusters() {
  currentOperation = "Finding far lands...";
  map->megaClusters.clear();
  std::map<Region *, MegaCluster *> _megaClusters;
  for (auto r : map->regions) {
    Cell *c = r->cell;
    bool cu = true;
    Cluster *knownCluster = nullptr;
    for (auto n : c->getNeighbors()) {
      Region *rn = _cells[n];
      if (r->biom.name != rn->biom.name) {
        r->border = true;
        r->coast = r->biom.name == LAND.name && rn->biom.name == SEA.name;
      } else if (_megaClusters.count(rn) != 0) {
        cu = false;
        if (knownCluster == nullptr) {
          knownCluster = _megaClusters[rn];
          r->megaCluster = knownCluster;
          r->cluster = knownCluster;
          knownCluster->regions.push_back(r);
          _megaClusters[r] = knownCluster;
        } else {
          Cluster *oldCluster = rn->megaCluster;
          if (oldCluster != knownCluster) {
            rn->cluster = knownCluster;
            _megaClusters[rn] = knownCluster;
            for (Region *orn : oldCluster->regions) {
              orn->cluster = knownCluster;
              orn->megaCluster = knownCluster;
              auto kcrn = knownCluster->regions;
              if (std::find(kcrn.begin(), kcrn.end(), orn) == kcrn.end()) {
                knownCluster->regions.push_back(orn);
              }
              _megaClusters[orn] = knownCluster;
            }
            oldCluster->regions.clear();
          }

          r->megaCluster = knownCluster;
          r->cluster = knownCluster;
          _megaClusters[r] = knownCluster;
        }
        continue;
      }
    }

    if (cu) {
      Cluster *cluster = new MegaCluster();
      cluster->isLand = r->biom.name == LAND.name;
      cluster->megaCluster = cluster;
      if (cluster->isLand) {
        cluster->name = generateLandName();
      } else {
        cluster->name = generateSeaName();
      }
      r->megaCluster = cluster;
      cluster->regions.push_back(r);
      _megaClusters[r] = cluster;
      map->megaClusters.push_back(cluster);
    }
  }
  printf("Map->Clusters: %zu\n", map->megaClusters.size());
  map->megaClusters.erase(std::remove_if(map->megaClusters.begin(),
                                         map->megaClusters.end(), isDiscard),
                          map->megaClusters.end());
  std::sort(map->megaClusters.begin(), map->megaClusters.end(), clusterOrdered);
  printf("Map->Clusters: %zu\n", map->megaClusters.size());
}

void MapGenerator::makeClusters() {
  currentOperation = "Meeting with neighbors...";
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
  printf("Map->Clusters: %zu\n", map->clusters.size());
  map->clusters.erase(
      std::remove_if(map->clusters.begin(), map->clusters.end(), isDiscard),
      map->clusters.end());
  std::sort(map->clusters.begin(), map->clusters.end(), clusterOrdered);
  for (auto c : map->clusters) {
    c->megaCluster = c->regions[0]->megaCluster;
  }
  printf("Map->Clusters: %zu\n", map->clusters.size());
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

std::vector<Region *> MapGenerator::getRegions() { return map->regions; }

void MapGenerator::setSize(int w, int h) {
  _w = w;
  _h = h;
}

int MapGenerator::getRelax() { return _relax; }

bool damagedCell(Cell *c) {
  return c->pointIntersection(c->site.p.x, c->site.p.y) == -1;
}

void MapGenerator::makeDiagram() {
  currentOperation = "Making nothing...";
  _bbox = sf::Rect<double>(0, 0, _w, _h);
  _sites = new std::vector<sf::Vector2<double>>();
  genRandomSites(*_sites, _bbox, _w, _h, _pointsCount);
  _diagram.reset(_vdg.compute(*_sites, _bbox));
  for (int n = 0; n < _relax; n++) {
    currentOperation = "Relaxing...";
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
