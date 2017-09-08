#include "mapgen/MapGenerator.hpp"
#include <VoronoiDiagramGenerator.h>
#include <random>
#include <iterator>
#include "Biom.cpp"

const int DEFAULT_RELAX = 10;

double normalize(double in, int dimension) {
	return in / (float)dimension*1.8 - 0.9;
}

bool cellsOrdered(Cell* c1, Cell* c2) {
  sf::Vector2<double> s1 = c1->site.p;
  sf::Vector2<double> s2 = c2->site.p;
	if (s1.y < s2.y)
		return true;
	if (s1.y == s2.y && s1.x < s2.x)
		return true;

	return false;
}

bool sitesOrdered(const sf::Vector2<double>& s1, const sf::Vector2<double>& s2) {
	if (s1.y < s2.y)
		return true;
	if (s1.y == s2.y && s1.x < s2.x)
		return true;

	return false;
}

bool clusterOrdered(Cluster* s1, Cluster* s2) {
	if (s1->regions.size() > s2->regions.size())
		return true;

	return false;
}

template<typename Iter>
Iter MapGenerator::select_randomly(Iter start, Iter end) {
  std::mt19937 gen(_seed);
  std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
  std::advance(start, dis(gen));
  return start;
}



MapGenerator::MapGenerator(int w, int h): _w(w), _h(h) {
	_vdg = VoronoiDiagramGenerator();
  _pointsCount = 10000;
  _octaves = 3;
  _freq = 0.3;
  _relax = DEFAULT_RELAX;
  _regions = new std::vector<Region*>();
  simpleRivers = true;
}

void MapGenerator::build() {
  seed();
  update();
}

void MapGenerator::relax() {
  _relax++;
  makeRelax();
  regenRegions();
  regenRivers();
  simplifyRivers();
  calcHumidity();
}

void MapGenerator::simplifyRivers() {
  for (auto rvr : rivers) {
    PointList sr;
    int c = rvr->size();
    int i = 0;
    sr.push_back((*rvr)[0]);
    for(PointList::iterator it=rvr->begin() ; it < rvr->end(); it++, i++) {
      Point p = (*rvr)[i];
      Point p2 = (*rvr)[i+1];
      if (c - i - 1 < 2) {
        break;
      }

      Point p3 = (*rvr)[i+2];
      Point sp;

      double distancex = (p2->x - p->x);
      distancex *= distancex;
      double distancey = (p2->y - p->x);
      distancey *= distancey;

      double d1 = sqrt(distancex + distancey);

      distancex = (p3->x - p->x);
      distancex *= distancex;
      distancey = (p3->y - p->x);
      distancey *= distancey;

      double d2 = sqrt(distancex + distancey);

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
    for(PointList::iterator it=sr.begin() ; it < sr.end(); it++, i++) {
      Point p(sr[i]);
      if (p->x <= 0.f || p->x != p->x ||
          p->y <= 0.f || p->y != p->y
          ) {
        continue;
      }
      rvr->push_back(sr[i]);
    }
  }
  std::cout<<"end\n"<<std::flush;
}

void MapGenerator::makeRelax() {
  _diagram.reset(_vdg.relax());
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

int MapGenerator::getSeed() {
  return _seed;
}

int MapGenerator::getOctaveCount() {
  return _octaves;
}

void MapGenerator::setOctaveCount(int o) {
  _octaves = o;
}

int MapGenerator::getPointCount() {
  return _pointsCount;
}

float MapGenerator::getFrequency() {
  return _freq;
}

void MapGenerator::setFrequency(float f) {
  _freq = f;
}

void MapGenerator::setPointCount(int c) {
  _pointsCount = c;
}


void MapGenerator::update() {
  regenHeight();
  regenDiagram();
  regenRegions();
  regenClusters();
  regenRivers();
  if (simpleRivers) {
    simplifyRivers();
  }
  calcHumidity();
  regenMegaClusters();
}

void MapGenerator::forceUpdate() {
  // regenHeight();
  regenDiagram();
  regenRegions();
  regenClusters();
  regenRivers();
  simplifyRivers();
  calcHumidity();
  regenMegaClusters();
}

void MapGenerator::regenHeight() {
  _perlin.SetSeed(_seed);
  _perlin.SetOctaveCount(_octaves);
  _perlin.SetFrequency(_freq);
  utils::NoiseMapBuilderPlane heightMapBuilder;
  heightMapBuilder.SetSourceModule(_perlin);
  heightMapBuilder.SetDestNoiseMap(_heightMap);
  heightMapBuilder.SetDestSize(_w, _h);
  heightMapBuilder.SetBounds(0.0, 10.0, 0.0, 10.0);
  heightMapBuilder.Build();
  std::cout << "Height generation finished\n" << std::flush;
}

void MapGenerator::makeRiver(Cell* c) {
  std::vector<Cell*> visited;
  printf("First cell: %p\n", c);
  Region* r = _cells[c];
  printf("First biom: %s\n", r->biom.name.c_str());
  // r->biom = MARK;
  float z = r->getHeight(r->site);
  printf("First vert: %f\n", z);
  PointList* river = new PointList();
  rivers.push_back(river);
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
  while (z > 0.f && count < 100) {
    std::vector<Cell*> n = c->getNeighbors();
    Cell* end;
    for (Cell* c2 : n) {
      if (std::find(visited.begin(), visited.end(),c2)!=visited.end()) {
        continue;
      }
      visited.push_back(c2);
      r = _cells[c2];
      r->cluster->hasRiver = true;
      bool f = false;
      for (auto e : c2->getEdges()) {
        // printf("Next candidate: %f < %f\n", r->getHeight(e->startPoint()), z);
        if (r->getHeight(e->startPoint()) < z) {
          next = e->startPoint();
          z = r->getHeight(next);
          c = c2;
          f = true;
        }
      }
      if (f) {
        // river.push_back(r->site);
        river->push_back(r->site);
        r->hasRiver = true;
        // river.push_back(next);
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
    }
  }
}

void MapGenerator::regenRivers() {
  rivers.clear();
  for (auto cluster : clusters){
    if (cluster->biom.name == "Snow" || cluster->biom.name == "Rock") {
      Cell* c = cellsMap[*select_randomly(cluster->regions.begin(), cluster->regions.end())];
      if (c != nullptr) {
        makeRiver(c);
      }
    }
  }
}

void MapGenerator::regenRegions() {
  _cells.clear();
  _regions->clear();
  _regions->reserve(_diagram->cells.size());
  float ch = 0.f;
  Biom lastBiom = BIOMS[0];
  for (auto c : _diagram->cells) {
    if (c == nullptr) {
      continue;
    }
    PointList verts;
    int count = int(c->getEdges().size());
    verts.reserve(count);

    float ht = 0;
    std::map<sf::Vector2<double>*,float> h;
    for (int i = 0; i < count; i++)
      {
        sf::Vector2<double>* p0;
        p0 = c->getEdges()[i]->startPoint();
        verts.push_back(p0);

        h.insert(std::make_pair(p0, _heightMap.GetValue(p0->x, p0->y)));
        ht += h[p0];
      }
    ht = ht/count;
    if (ht > ch) {
      _highestCell = c;
      ch = ht;
    }
    sf::Vector2<double>& p = c->site.p;
    h.insert(std::make_pair(&p, ht));
    Biom b = BIOMS[0];
    for (int i = 0; i < int(BIOMS.size()); i++)
      {
        if (ht>BIOMS[i].border) {
          b = BIOMS[i];
        }
      }
    Region *region = new Region(b, verts, h, &p);
    region->humidity = b.humidity;
    region->border = false;
    region->hasRiver = false;
    _regions->push_back(region);
    _cells.insert(std::make_pair(c, region));

  }
  std::cout << "Regions generation: " << _diagram->cells.size() << " finished\n" << std::flush;
}

bool isDiscard(const Cluster* c)
{
  // return c->discarded;
  return c->regions.size() == 0;
}

void MapGenerator::calcHumidity() {
  for (auto r : *_regions) {
    if(r->hasRiver) {
      r->humidity += 0.1;
    }
  }
  for (auto r : *_regions) {
    Cell* c = cellsMap[r];
    if (c == nullptr) {
      continue;
    }
    for (auto n : c->getNeighbors()) {
      Region* rn = _cells[n];
      if (rn->biom.humidity > r->humidity && r->humidity!=1) {
        r->humidity += (rn->biom.humidity-r->humidity)/2.f;
      }
    }
  }
}

void MapGenerator::regenMegaClusters() {
  megaClusters.clear();
  std::map<Cluster*,MegaCluster*> _megaClusters;
  for (auto c : clusters) {
    for (auto r : c->regions) {
      if (r->border) {
        Cell* cell = cellsMap[r];
        if (cell == nullptr) {
          continue;
        }
        for (auto n : cell->getNeighbors()) {
          Region* rn = _cells[n];
          Cluster* nc = rn->cluster;
          if(nc != c && std::find(c->neighbors.begin(), c->neighbors.end(), nc) == c->neighbors.end()) {
            c->neighbors.push_back(nc);
          }
        }
      }
    }
  }

  for (auto c : clusters) {
    bool cu = true;
    MegaCluster *knownMegaCluster = nullptr;
    for (auto nc : c->neighbors) {
      if (nc->isLand != c->isLand) {
        continue;
      }
      if (_megaClusters.count(nc) != 0) {
        cu = false;
        if (knownMegaCluster == nullptr) {
          knownMegaCluster = _megaClusters[nc];
          c->megaCluster = knownMegaCluster;
          for (auto r : c->regions){
            knownMegaCluster->regions.push_back(r);
          }
          _megaClusters[c] = knownMegaCluster;
        } else {
          MegaCluster *oldCluster = nc->megaCluster;
          if (oldCluster != knownMegaCluster) {
            nc->megaCluster = knownMegaCluster;
            _megaClusters[nc] = knownMegaCluster;
            for (Region* orn : oldCluster->regions) {
              auto kcrn = knownMegaCluster->regions;
              if(std::find(kcrn.begin(), kcrn.end(), orn) == kcrn.end()) {
                knownMegaCluster->regions.push_back(orn);
              }
              // _clusters[cellsMap[orn]] = knownMegaCluster;
            }
            oldCluster->regions.clear();
          }

          nc->megaCluster = knownMegaCluster;
          _megaClusters[c] = knownMegaCluster;
        }
      }
    }

    if(cu) {
      MegaCluster* cluster = new MegaCluster();
      char buff[100];
      snprintf(buff, sizeof(buff), "%p", (void*)cluster);
      std::string buffAsStdStr = buff;
      cluster->name = buffAsStdStr;
      c->megaCluster = cluster;
      cluster->isLand = c->isLand;
      for (auto r : c->regions){
        cluster->regions.push_back(r);
      }
      _megaClusters[c] = cluster;
      megaClusters.push_back(cluster);
    }
  }
  megaClusters.erase(
                     std::remove_if(megaClusters.begin(), megaClusters.end(), isDiscard),
                     megaClusters.end());
  std::sort(megaClusters.begin(), megaClusters.end(), clusterOrdered);
}

void MapGenerator::regenClusters() {
  clusters.clear();
  cellsMap.clear();
  std::map<Cell*,Cluster*> _clusters;
  for (auto c : _diagram->cells) {
    Region* r = _cells[c];
    bool cu = true;
    Cluster *knownCluster = nullptr;
    // printf("New cell: %p\n", r);
    for (auto n : c->getNeighbors()) {
      Region* rn = _cells[n];
      if (r->biom.name != rn->biom.name){
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
            // if (oldCluster->regions.size() > knownCluster->regions.size()) {
            //   Cluster* tmp = knownCluster;
            //   knownCluster = oldCluster;
            //   oldCluster = tmp;
            // }
            // printf("Replace cluster %p with %p\n", oldCluster, knownCluster);
            rn->cluster = knownCluster;
            _clusters[n] = knownCluster;
            for (Region* orn : oldCluster->regions) {
              orn->cluster = knownCluster;
              auto kcrn = knownCluster->regions;
              if(std::find(kcrn.begin(), kcrn.end(), orn) == kcrn.end()) {
                knownCluster->regions.push_back(orn);
              }
              _clusters[cellsMap[orn]] = knownCluster;
              // clusters.erase(std::find(clusters.begin(), clusters.end(), oldCluster));
            }
            // oldCluster->discarded = true;
            oldCluster->regions.clear();
          }

          r->cluster = knownCluster;
          // _clusters[n]->regions.push_back(r);
          _clusters[c] = knownCluster;
          cellsMap[r] = c;
        }
        continue;
      }
    }
    if(cu) {
      Cluster* cluster = new Cluster();
      char buff[100];
      snprintf(buff, sizeof(buff), "%p", (void*)cluster);
      std::string buffAsStdStr = buff;
      cluster->name = buffAsStdStr;
      cluster->hasRiver = false;
      cluster->discarded = false;
      r->cluster = cluster;
      cluster->biom = r->biom;
      cluster->isLand = r->biom.border > 0;
      cluster->regions.push_back(r);
      _clusters[c] = cluster;
      cellsMap[r] = c;
      clusters.push_back(cluster);
    }
  }
  printf("Clusters: %zu\n", clusters.size());
  clusters.erase(
    std::remove_if(clusters.begin(), clusters.end(), isDiscard),
    clusters.end());
  std::sort(clusters.begin(), clusters.end(), clusterOrdered);
  printf("Clusters: %zu\n", clusters.size());
}

Region* MapGenerator::getRegion(sf::Vector2f pos) {
  for (auto &pair : _cells)
  {
    Cell* c = pair.first;
    if(c->pointIntersection(pos.x, pos.y) != -1) {
      return pair.second;
    }
  }
  return nullptr;
}

std::vector<Region*>* MapGenerator::getRegions() {
  return _regions;
  // std::vector<Region*>* r = new std::vector<Region*>();
  // for (auto &pair : _cells)
  //   {
  //     r->push_back(pair.second);
  //   }
  // return r;
}

void MapGenerator::setSize(int w, int h) {
  _w = w;
  _h = h;
}

int MapGenerator::getRelax() {
  return _relax;
}

void MapGenerator::regenDiagram() {
  _bbox = sf::Rect<double>(0,0,_w, _h);
  _sites = new std::vector<sf::Vector2<double>>();
  genRandomSites(*_sites, _bbox, _w, _h, _pointsCount);
  _diagram.reset(_vdg.compute(*_sites, _bbox));
  for (int n = 0; n < _relax; n++) {
    makeRelax();
  }
  delete _sites;
  std::sort(_diagram->cells.begin(), _diagram->cells.end(), cellsOrdered);
  std::cout << "Diagram generation finished: " << _pointsCount << "\n" << std::flush;
}

void MapGenerator::genRandomSites(std::vector<sf::Vector2<double>>& sites, sf::Rect<double>& bbox, unsigned int dx, unsigned int dy, unsigned int numSites) {
	std::vector<sf::Vector2<double>> tmpSites;

	tmpSites.reserve(numSites);
	sites.reserve(numSites);

  sf::Vector2<double> s;

	srand(_seed);
	for (unsigned int i = 0; i < numSites; ++i) {
		s.x = 1 + (rand() / (double)RAND_MAX)*(dx - 2);
		s.y = 1 + (rand() / (double)RAND_MAX)*(dy - 2);
		tmpSites.push_back(s);
	}

	//remove any duplicates that exist
	std::sort(tmpSites.begin(), tmpSites.end(), sitesOrdered);
	sites.push_back(tmpSites[0]);
	for (sf::Vector2<double>& s : tmpSites) {
		if (s != sites.back()) sites.push_back(s);
	}
}

std::vector<sf::ConvexShape>* MapGenerator::getPolygons() {
  return _polygons;
}
