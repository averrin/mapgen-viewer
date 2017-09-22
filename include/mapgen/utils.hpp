#ifndef UTILS_HPP_
#define UTILS_HPP_
#include <SFML/Graphics.hpp>

typedef sf::Vector2<double>* Point;
namespace mg {
  double getDistance(Point p, Point p2);
};
#endif
