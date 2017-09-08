#include "mapgen/Biom.hpp"

const std::vector<Biom> BIOMS = {{
    {
      -2.0000,
      sf::Color( 23,  23,  40),
      "Abyss", 1
    },
    {
      -1.0000,
      sf::Color( 39,  39,  70),
      "Deep", 1
    },
    {
      -0.2500,
      sf::Color( 51,  51,  91),
      "Shallow", 1
    },
    {
      0.0000,
      sf::Color( 68, 99, 130),
      "Shore", 1
    },
    {
      0.0625,
      sf::Color(210, 185, 139),
      "Sand", 0.3
    },
    {
      0.1250,
      sf::Color(136, 170,  85),
      "Grass", 0.5
    },
    {
      0.3750,
      sf::Color( 51, 119,  85),
      "Forrest", 0.6
    },
    {
      0.7500,
      sf::Color(148, 148, 148),
      "Rock", 0.3
    },
    {
      1.0000,
      sf::Color(240, 240, 240),
      "Snow", 0.8
    },
    {
      1.2000,
      sf::Color(220, 220, 255),
      "Ice", 0.9
    },
    {
      999.000,
      sf::Color( 51,  51,  91),
      "Lake", 1
    },
    {
      999.000,
      sf::Color(201, 201, 120),
      "Prairie", 0.1
    },
    {
      999.000,
      sf::Color::Red,
      "Mark"
    }
  }};
const Biom MARK = BIOMS.back();
const Biom LAKE = BIOMS[10];
