#include "mapgen/Walker.hpp"
#include "mapgen/City.hpp"
#include "mapgen/MapGenerator.hpp"
#include "mapgen/utils.hpp"
#include <SFML/Graphics.hpp>
#include <random>

template <typename Iter>
Iter Walker::select_randomly(Iter start, Iter end) {
  std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
  std::random_device gen;
  std::advance(start, dis(gen));
  return start;
}

Walker::Walker(City *c, MapGenerator *m) : mapgen(m) {

  // road = *mapgen->select_randomly(s->roads.begin(), s->roads.end());
  // reverse = rand() % 2 == 0;
  // shape = new sf::CircleShape(3);
  // shape->setFillColor(sf::Color(225,225,190));
  // shape->setOutlineColor(sf::Color::Black);
  // shape->setOutlineThickness(1);
  // road = *select_randomly(c->roads.begin(), c->roads.end());
  // reverse = road->regions.back()->city == c;
  if (reverse) {
    // step = road->spline->getInterpolatedPositionCount() - 1;
  }
}

Road* Walker::getNextRoad() {
  // City* c;
  // if (!reverse) {
  //   c = road->regions.back()->city;
  // } else {
  //   c = road->regions.front()->city;
  // }
  // if (c == nullptr) {
  //   mg::warn("city", "nullptr");
  // } else if (c->roads.size() == 0) {
  //   mg::warn(c->typeName, "without roads");
  // }
  // road = *select_randomly(c->roads.begin(), c->roads.end());
  // reverse = road->regions.back()->city == c;
  // if (reverse) {
  //   // step = road->spline->getInterpolatedPositionCount() - 1;
  // } else {
  //   step = 0;
  // }
  return road;
}

//TODO: fix road->spline
void Walker::tick() {
//   if ((step == road->spline->getInterpolatedPositionCount() && !reverse) || (step == 0 && reverse)) {
//     getNextRoad();
//   }
//   auto p = road->spline->getInterpolatedPosition(step);
//   shape->setPosition(sf::Vector2f(p.x - 1.5, p.y - 1.5));
//   if (!reverse) {
//     step++;
//   } else {
//     step--;
//   }
}
