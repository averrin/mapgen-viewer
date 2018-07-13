#include <SFML/Graphics/RenderWindow.hpp>
#include "mapgen/Region.hpp"
#include "mapgen/MapGenerator.hpp"
#include "mapgen/Painter.hpp"

class WeatherWindow {
public:
  WeatherWindow(sf::RenderWindow *w, MapGenerator* m);
  void draw(WeatherManager* weather, Painter* painter);
  sf::RenderWindow *window;
  MapGenerator *mapgen;
};
