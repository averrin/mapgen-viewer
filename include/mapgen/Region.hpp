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
  bool hasRiver;
  Cluster *cluster;
  MegaCluster *megaCluster;
  bool border;
  float humidity;
  Cell* cell;
  float temperature;
  float minerals;
  float nice;
  City* city = nullptr;
  float distanceFormCapital;
  bool coast;
  std::vector<Region*> neighbors;
  bool hasRoad = false;
  int traffic = 0;
  Location* location = nullptr;
  State* state = nullptr;
  bool stateBorder = false;
  bool seaBorder = false;
private:
	PointList _verticies;
  HeightMap _heights;
};

struct Cluster {
  std::string name;
  std::vector<Region*> regions;
  std::vector<Cluster*> neighbors;
  MegaCluster* megaCluster;
  Biom biom;
  bool hasRiver;
  bool isLand;
  Point* center;
  PointList border;
  std::vector<Region*> resourcePoints;
  std::vector<Region*> goodPoints;
  std::vector<City*> cities;
  bool hasPort = false;
};

#endif
