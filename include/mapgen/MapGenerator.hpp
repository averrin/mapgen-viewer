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
  void relax();
  void setSeed(int seed);
  void setOctaveCount(int octaveCount);
  void setSize(int w, int h);
  void setFrequency(int freq);
  void setPointCount(int count);
  int  getPointCount();
  int  getOctaves();
  float  getFrequency();
  int getSeed();
  Region* getRegion(sf::Vector2f pos);
  void seed();


	std::unique_ptr<Diagram> diagram;

private:
  void regenHeight();
  void regenDiagram();
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
  std::map<sf::Vector2<double>*,float> _heights;

  module::Perlin _perlin;
  utils::NoiseMap _heightMap;

  void genRandomSites(std::vector<sf::Vector2<double> >& sites, sf::Rect<double>& bbox, unsigned int dx, unsigned int dy, unsigned int numSites);
};
