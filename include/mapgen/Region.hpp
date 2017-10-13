#ifndef REGION_H_
#define REGION_H_

#include <SFML/Graphics.hpp>
#include <vector>
#include "Biom.hpp"
#include "State.hpp"
#include <VoronoiDiagramGenerator.h>

typedef sf::Vector2<double>* Point;
typedef std::vector<Point> PointList;
typedef std::map<Point,float> HeightMap;

struct Cluster;
typedef Cluster MegaCluster;
typedef Cluster StateCluster;
class City;
class Location;
class Region {
public:
  Region();
  Region(Biom b, PointList v, HeightMap h, Point s);
  PointList getPoints();
  float getHeight(Point p);
  Biom biom;
  Point site;
  bool hasRiver = false;
  Cluster *cluster = nullptr;
  Cluster *stateCluster = nullptr;
  MegaCluster *megaCluster = nullptr;
  bool border = false;
  float humidity = 0.f;
  Cell* cell = nullptr;
  float temperature = 0.f;
  float minerals = 0.f;
  float nice = 0.f;
  City* city = nullptr;
  std::vector<Region*> neighbors;
  bool hasRoad = false;
  int traffic = 0;
  Location* location = nullptr;
  State* state = nullptr;
  bool stateBorder = false;
  bool seaBorder = false;
  bool isCoast();
private:
	PointList _verticies;
  HeightMap _heights;
};

struct Cluster {
  std::string name = "";
  std::vector<Region*> regions;
  std::vector<Cluster*> neighbors;
  MegaCluster* megaCluster = nullptr;
  Biom biom;
  bool hasRiver = false;;
  bool isLand = false;;
  Point* center = nullptr;
  PointList border;
  std::vector<Region*> resourcePoints;
  std::vector<Region*> goodPoints;
  std::vector<City*> cities;
  bool hasPort = false;
  std::vector<State*> states;
};

#endif
