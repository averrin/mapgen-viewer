#include <SFML/Graphics.hpp>
#include <vector>
#include "Biom.hpp"

typedef sf::Vector2<double>* Point;
typedef std::vector<Point> PointList;
typedef std::map<Point,float> HeightMap;

class Region {
public:
  Region(Biom b, PointList v, HeightMap h, Point s);
  PointList getPoints();
  Biom biom;
private:
	PointList _verticies;
  HeightMap _heights;
  Point _site;
};
