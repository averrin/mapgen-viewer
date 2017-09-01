#include <SFML/Graphics.hpp>
#include "mapgen/Region.hpp"
#include <vector>

Region::Region(Biom b, PointList v, HeightMap h, Point s)
  : biom(b), _verticies(v), _heights(h), _site(s) {}

PointList Region::getPoints() {
  return _verticies;
};
