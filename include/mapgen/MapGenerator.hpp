#include <SFML/Graphics.hpp>
#include <VoronoiDiagramGenerator.h>
#include <libnoise/noise.h>
#include "noise/noiseutils.h"

#include "Region.hpp"

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
  std::vector<Region*>* getRegions();
  std::vector<PointList*> rivers;

private:
  void regenHeight();
  void regenDiagram();
  void regenRegions();
  void regenRivers();
  void regenClusters();
  void makeRelax();
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
  std::vector<sf::ConvexShape>* _polygons;
  Cell* _highestCell;

  module::Perlin _perlin;
  utils::NoiseMap _heightMap;
  std::vector<Region*>* _regions;

  void genRandomSites(std::vector<sf::Vector2<double> >& sites, sf::Rect<double>& bbox, unsigned int dx, unsigned int dy, unsigned int numSites);
};
