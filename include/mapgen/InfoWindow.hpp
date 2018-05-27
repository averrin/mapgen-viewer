#include <SFML/Graphics/RenderWindow.hpp>
#include "mapgen/Region.hpp"

class InfoWindow {
public:
  InfoWindow();
  void draw(Region currentRegion);
};
