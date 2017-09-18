#ifndef STATE_H_
#define STATE_H_
#include <SFML/Graphics.hpp>
#include <VoronoiDiagramGenerator.h>

class State {
public:
  State(std::string n, sf::Color cl, Cell* c);
  std::string name;
  sf::Color color;
  Cell* cell;
};

#endif
