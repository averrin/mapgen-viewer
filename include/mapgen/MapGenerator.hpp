#ifndef MAPGEN_H_
#define MAPGEN_H_

#include <SFML/Graphics.hpp>
#include <VoronoiDiagramGenerator.h>
#include "noise/noise.h"
#include "noise/noiseutils.h"
#include <memory>
#include <functional>

//TODO: fix it and use header
#include "../../src/Map.cpp"

#include "Region.hpp"
#include "State.hpp"
#include "micropather.h"

template<typename T>
using filterFunc = std::function<bool (T*)>;
template<typename T>
using sortFunc = std::function<bool (T*, T*)>;

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
  float getFrequency();
  int getSeed();
  Region* getRegion(sf::Vector2f pos);
  std::vector<sf::ConvexShape>* getPolygons();
  void seed();
  std::vector<Region*> getRegions();
  void setMapTemplate(const char* t);
  void startSimulation();

  bool simpleRivers;
  bool ready;
  std::string currentOperation;
  float temperature;
  Map* map;
  std::vector<State*> states;

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
  void makeCaves();
  void makeStates();
  void simulation();

  template <typename T>
  std::vector<T*> filterObjects(std::vector<T*> regions, filterFunc<T> filter, sortFunc<T> sort);
  void getSea(std::vector<Region*> *seas, Region* base,Region* r);
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

  template<typename Iter>
  Iter select_randomly_seed(Iter start, Iter end, int s);

  void genRandomSites(std::vector<sf::Vector2<double> >& sites, sf::Rect<double>& bbox, unsigned int dx, unsigned int dy, unsigned int numSites);
};

#endif
