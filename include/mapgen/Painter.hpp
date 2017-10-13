#include "mapgen/Region.hpp"
#include <SFML/Graphics.hpp>

struct DrawableRegion {
public:
  sf::ConvexShape shape ;
  Region* region;
  int zIndex;
};
