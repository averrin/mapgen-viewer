#include "mapgen/MapGenerator.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

class SimulationWindow {
private:
  sf::RenderWindow *window;
  MapGenerator *mapgen;
public:
  SimulationWindow(sf::RenderWindow *w, MapGenerator *m);
  void draw();
};
