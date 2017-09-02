#include "mapgen/MapGenerator.hpp"
#include <VoronoiDiagramGenerator.h>
#include  <random>
#include  <iterator>

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
  std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
  std::advance(start, dis(g));
  return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  return select_randomly(start, end, gen);
}

const int DEFAULT_RELAX = 10;

const std::vector<Biom> BIOMS = {{
    {
      -2.0000,
      sf::Color( 23,  23,  40),
      "Abyss"
    },
    {
      -1.0000,
      sf::Color( 39,  39,  70),
      "Deep"
    },
    {
      -0.2500,
      sf::Color( 51,  51,  91),
      "Shallow"
    },
    {
      0.0000,
      sf::Color( 68, 99, 130),
      "Shore"
    },
    {
      0.0625,
      sf::Color(210, 185, 139),
      "Sand"
    },
    {
      0.1250,
      sf::Color(136, 170,  85),
      "Grass"
    },
    {
      0.3750,
      sf::Color( 51, 119,  85),
      "Forrest"
    },
    {
      0.7500,
      sf::Color(148, 148, 148),
      "Rock"
    },
    {
      1.0000,
      sf::Color(240, 240, 240),
      "Snow"
    },
    {
      999.000,
      sf::Color::Red,
      "Mark"
    }
  }};
const Biom MARK = BIOMS[9];

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

MapGenerator::MapGenerator(int w, int h): _w(w), _h(h) {
	_vdg = VoronoiDiagramGenerator();
  _pointsCount = 10000;
  _octaves = 3;
  _freq = 0.3;
  _relax = DEFAULT_RELAX;
  _regions = new std::vector<Region*>();
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
}

void MapGenerator::forceUpdate() {
  // regenHeight();
  regenDiagram();
  regenRegions();
  regenClusters();
  regenRivers();
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
        // river.push_back(next);
      }
      end = c2;
    }
    count++;
    if (count == 100) {
      r->biom = BIOMS[2];
      river->push_back(r->site);

      for (auto n : end->getNeighbors()) {
        r = _cells[n];
        r->biom = BIOMS[3];
      }
      // std::cout << "Cannot create river!\n";
      // river.clear();
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
            oldCluster->discarded = true;
            oldCluster->regions.clear();
          }

          r->cluster = knownCluster;
          // _clusters[n]->regions.push_back(r);
          _clusters[c] = knownCluster;
          cellsMap[r] = c;
        }
        continue;
      }
        // break;
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
      if (r->hasRiver) {
        cluster->hasRiver = true;
      }
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
