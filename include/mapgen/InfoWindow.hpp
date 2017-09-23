#include <SFML/Graphics/RenderWindow.hpp>
#include "mapgen/Region.hpp"

class InfoWindow {
public:
  InfoWindow(sf::RenderWindow *w);
  void draw(Region* currentRegion);
  sf::RenderWindow *window;
};
