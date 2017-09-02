#include <SFML/Graphics.hpp>
#include <vector>
#include "Biom.hpp"

typedef sf::Vector2<double>* Point;
typedef std::vector<Point> PointList;
typedef std::map<Point,float> HeightMap;

struct Cluster;
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
  bool border;
private:
	PointList _verticies;
  HeightMap _heights;
};

struct Cluster {
  std::string name;
  std::vector<Region*> regions;
  Biom biom;
  bool hasRiver;
  bool discarded;
};
