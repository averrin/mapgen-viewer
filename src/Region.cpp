#include <SFML/Graphics.hpp>
#include "mapgen/Region.hpp"
#include <vector>

Region::Region(Biom b, PointList v)
  : biom(b), _verticies(v)
{
}

PointList Region::getPoints() {
  return _verticies;
};
