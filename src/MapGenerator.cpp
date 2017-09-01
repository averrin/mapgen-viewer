#include "mapgen/MapGenerator.hpp"
#include <VoronoiDiagramGenerator.h>

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
      sf::Color( 91, 132, 173),
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
      sf::Color(128, 128, 128),
      "Rock"
    },
    {
      1.0000,
      sf::Color(240, 240, 240),
      "Snow"
    }
  }};

double normalize(double in, int dimension) {
	return in / (float)dimension*1.8 - 0.9;
}

bool sitesOrdered(const sf::Vector2<double>& s1, const sf::Vector2<double>& s2) {
	if (s1.y < s2.y)
		return true;
	if (s1.y == s2.y && s1.x < s2.x)
		return true;

	return false;
}

MapGenerator::MapGenerator(int w, int h): _w(w), _h(h) {
	_vdg = VoronoiDiagramGenerator();
  _pointsCount = 10000;
  _octaves = 3;
  _freq = 0.3;
}

void MapGenerator::build() {
  seed();
  update();
}

void MapGenerator::relax() {
  _diagram.reset(_vdg.relax());
  regenRegions();
}

void MapGenerator::seed() {
  _seed = std::clock();
  printf("New seed: %d\n", _seed);
}

void MapGenerator::setSeed(int s) {
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

void MapGenerator::regenRegions() {
  _cells.clear();
  _regions.clear();
  _regions.reserve(_diagram->cells.size());
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
    sf::Vector2<double>& p = c->site.p;
    h.insert(std::make_pair(&p, ht));
    Biom b = BIOMS[0];
    for (int i = 0; i < int(BIOMS.size()); i++)
      {
        if (ht>BIOMS[i].border) {
          b = BIOMS[i];
        }
      }
    Region region = Region(b, verts, h, &p);
    _regions.push_back(region);
    _cells.insert(std::make_pair(c, region));

  }
  std::cout << "Regions generation: " << _diagram->cells.size() << " finished\n" << std::flush;
}

Region* MapGenerator::getRegion(sf::Vector2f pos) {
  for (auto &pair : _cells)
  {
    Cell* c = pair.first;
    if(c->pointIntersection(pos.x, pos.y) != -1) {
      return &(pair.second);
    }
  }
  return nullptr;
}

std::vector<Region> MapGenerator::getRegions() {
  return _regions;
}

void MapGenerator::setSize(int w, int h) {
  _w = w;
  _h = h;
}

void MapGenerator::regenDiagram() {
  _bbox = sf::Rect<double>(0,0,_w, _h);
  _relax = 0;
  _sites = new std::vector<sf::Vector2<double>>();
  genRandomSites(*_sites, _bbox, _w, _h, _pointsCount);
  _diagram.reset(_vdg.compute(*_sites, _bbox));
  delete _sites;
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

std::vector<sf::ConvexShape> MapGenerator::getPolygons() {
  return _polygons;
}
