#ifndef BIOM_H_
#define BIOM_H_
#include <SFML/Graphics.hpp>

struct Biom {
  float border;
  sf::Color color;
  std::string name;
  float humidity;
};

#endif
