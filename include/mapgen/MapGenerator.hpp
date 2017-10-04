#ifndef MAPGEN_H_
#define MAPGEN_H_

#include "noise/noise.h"
#include "noise/noiseutils.h"
#include <SFML/Graphics.hpp>
#include <VoronoiDiagramGenerator.h>
#include <functional>
#include <memory>
#include <random>

#include "Region.hpp"
#include "Simulator.hpp"
#include "State.hpp"
#include "micropather.h"

typedef std::function<bool(Region *, Region *)> sameFunc;
typedef std::function<void(Region *, Cluster *)> assignFunc;
typedef std::function<void(Region *, Cluster *,
                           std::map<Region *, Cluster *> *)>
    reassignFunc;
typedef std::function<Cluster *(Region *)> createFunc;

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
  int getPointCount();
  int getOctaveCount();
  int getRelax();
  float getFrequency();
  int getSeed();
  Region *getRegion(sf::Vector2f pos);
  std::vector<sf::ConvexShape> *getPolygons();
  void seed();
  std::vector<Region *> getRegions();
  void setMapTemplate(const char *t);
  void startSimulation();

  bool simpleRivers;
  bool ready;
  float temperature;
  Map *map;
  Simulator *simulator;

  template <typename Iter> Iter select_randomly(Iter start, Iter end);

  std::mt19937 *_gen;

private:
  void makeHeights();
  void makeDiagram();
  void makeRegions();
  void makeFinalRegions();
  void makeRivers();
  void makeClusters();
  void makeMegaClusters();
  void makeRelax();
  void makeRiver(Region *r);
  void calcHumidity();
  void calcTemp();
  void simplifyRivers();
  void makeBorders();
  void makeMinerals();
  void makeCities();
  void makeStates();

  void getSea(std::vector<Region *> *seas, Region *base, Region *r);
  int _seed;
  VoronoiDiagramGenerator _vdg;
  int _pointsCount;
  int _w;
  int _h;
  int _relax;
  int _octaves;
  float _freq;
  sf::Rect<double> _bbox;
  std::vector<sf::Vector2<double>> *_sites;
  std::map<Cell *, Region *> _cells;
  std::unique_ptr<Diagram> _diagram;
  Cell *_highestCell;
  std::map<Region *, Cell *> cellsMap;
  std::vector<State *> states;

  micropather::MicroPather *_pather;
  module::Perlin _perlin;
  utils::NoiseMap _heightMap;
  utils::NoiseMap _mineralsMap;
  std::string _terrainType;

  void genRandomSites(std::vector<sf::Vector2<double>> &sites,
                      sf::Rect<double> &bbox, unsigned int dx, unsigned int dy,
                      unsigned int numSites);

  std::vector<Cluster *> clusterize(std::vector<Region *> regions,
                                    sameFunc isSame, assignFunc assignCluster,
                                    reassignFunc reassignCluster,
                                    createFunc createCluster);
};

#endif
