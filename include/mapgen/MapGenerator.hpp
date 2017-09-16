#ifndef MAPGEN_H_
#define MAPGEN_H_

#include <SFML/Graphics.hpp>
#include <VoronoiDiagramGenerator.h>
#include <libnoise/noise.h>
#include "noise/noiseutils.h"
#include <functional>

//TODO: fix it and use header
#include "../../src/Map.cpp"

#include "Region.hpp"
#include "micropather.h"

typedef std::function<bool (Region*)> filterFunc;
typedef std::function<bool (Region*, Region*)> sortFunc;

class MapGenerator {
public:
  MapGenerator(int w, int h);

  void build();
  void update();
  void forceUpdate();
  void relax();
  void setSeed(int seed);
  void setOctaveCount(int octaveCount);
  void setSize(int w, int h);
  void setFrequency(float freq);
  void setPointCount(int count);
  int  getPointCount();
  int  getOctaveCount();
  int  getRelax();
  float  getFrequency();
  int getSeed();
  Region* getRegion(sf::Vector2f pos);
  std::vector<sf::ConvexShape>* getPolygons();
  void seed();
  std::vector<Region*> getRegions();
  void setMapTemplate(const char* t);

  bool simpleRivers;
  bool ready;
  std::string currentOperation;
  float temperature;
  Map* map;

private:
  void makeHeights();
  void makeDiagram();
  void makeRegions();
  void makeFinalRegions();
  void makeRegions_old();
  void makeRivers();
  void makeClusters();
  void makeMegaClusters();
  void makeMegaClusters_old();
  void makeRelax();
  void makeRiver(Region* r);
  void calcHumidity();
  void calcTemp();
  void simplifyRivers();
  void makeBorders();
  void makeMinerals();
  void makeCities();
  void makeRoads();
  void simulation();
  std::vector<Region*> filterRegions(std::vector<Region*> regions, filterFunc filter, sortFunc sort);
  std::vector<Region*> getSea(Region* r, int &d);
  int _seed;
  VoronoiDiagramGenerator _vdg;
  int _pointsCount;
  int _w;
  int _h;
  int _relax;
  int _octaves;
  float _freq;
  sf::Rect<double> _bbox;
	std::vector<sf::Vector2<double>>* _sites;
  std::map<Cell*,Region*> _cells;
	std::unique_ptr<Diagram> _diagram;
  Cell* _highestCell;
  std::map<Region*,Cell*> cellsMap;

  micropather::MicroPather* _pather;
  module::Perlin _perlin;
  utils::NoiseMap _heightMap;
  utils::NoiseMap _mineralsMap;
  std::string _terrainType;

  template<typename Iter>
  Iter select_randomly(Iter start, Iter end);

  void genRandomSites(std::vector<sf::Vector2<double> >& sites, sf::Rect<double>& bbox, unsigned int dx, unsigned int dy, unsigned int numSites);
};

#endif
