#include <SFML/Graphics.hpp>
#include "mapgen/Region.hpp"
#include <vector>

Region::Region(Biom b, PointList v, HeightMap h, Point s)
  : biom(b), _verticies(v), _heights(h), site(s) {}

PointList Region::getPoints() {
  return _verticies;
};

float Region::getHeight(Point p) {
  return _heights[p];
}
