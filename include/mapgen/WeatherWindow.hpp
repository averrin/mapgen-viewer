#include <SFML/Graphics/RenderWindow.hpp>
#include "mapgen/Region.hpp"
#include "mapgen/MapGenerator.hpp"
#include "mapgen/Painter.hpp"

class WeatherWindow {
public:
  WeatherWindow(std::shared_ptr<MapGenerator> m);
  void draw(std::shared_ptr<WeatherManager> weather, std::shared_ptr<Painter> painter);
  std::shared_ptr<MapGenerator> mapgen;
};
