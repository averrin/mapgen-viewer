#include "mapgen/MapGenerator.hpp"
#include <SFML/Graphics/RenderWindow.hpp>

class SimulationWindow {
private:
  std::shared_ptr<MapGenerator> mapgen;
public:
  SimulationWindow(std::shared_ptr<MapGenerator> m);
  void draw();
};
