#include <SFML/Graphics.hpp>
#include <vector>
#include "Biom.hpp"

typedef std::vector<sf::Vector2<double> *> PointList ;

class Region {
public:
  Region(Biom b, PointList v);
  PointList getPoints();
  Biom biom;
private:
	PointList _verticies;
};
