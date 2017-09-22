#ifndef BIOM_H_
#define BIOM_H_
#include <SFML/Graphics.hpp>

struct Biom {
  float border;
  sf::Color color;
  std::string name;
  float feritlity;
};

namespace biom {
const float DEFAULT_HUMIDITY = 0.f;
  const float DEFAULT_TEMPERATURE = 30.f;

  const Biom ABYSS = {-2.0000, sf::Color(23, 23, 40), "Abyss", 0};
  const Biom DEEP = {-1.0000, sf::Color(39, 39, 70), "Deep", 0};
  const Biom SHALLOW = {-0.2500, sf::Color(51, 51, 91), "Shallow", 0};
  const Biom SHORE = {0.0000, sf::Color(68, 99, 130), "Shore", 0};
  const Biom SAND = {0.0625, sf::Color(210, 185, 139), "Sand", 0};
  const Biom GRASS = {0.1250, sf::Color(136, 170, 85), "Grass", 0.8};
  const Biom FORREST = {0.3750, sf::Color(51, 119, 85), "Forrest", 0.6};
  const Biom ROCK = {0.7500, sf::Color(148, 148, 148), "Rock", 0};
  const Biom SNOW = {1.0000, sf::Color(240, 240, 240), "Snow", 0};
  const Biom ICE = {1.2000, sf::Color(220, 220, 255), "Ice", 0};
  const Biom PRAIRIE = {999.000, sf::Color(239, 220, 124), "Prairie", 0.6};
  const Biom MEADOW = {999.000, sf::Color(126, 190, 75), "Meadow", 1};
  const Biom DESERT = {999.000, sf::Color(244, 164, 96), "Desert", 0};
  const Biom CITY = {999.000, sf::Color(220, 220, 220), "City", 0};

  const Biom RAIN_FORREST = {0.3750, sf::Color(51, 90, 75), "Rain forrest", 0.6};

  const std::vector<Biom> BIOMS = {
    {ABYSS, DEEP, SHALLOW, SHORE, SAND, GRASS, FORREST, ROCK, SNOW, ICE}};

  const Biom LAKE = {999.000, sf::Color(51, 51, 91), "Lake", 0};
  const Biom MARK = {999.000, sf::Color::Red, "Mark"};
  const Biom MARK2 = {999.000, sf::Color::Black, "Mark"};

  const Biom LAND = {0.500, sf::Color(136, 170, 85), "Land", 0};
  const Biom SEA = {-1.000, sf::Color(39, 39, 70), "Sea", 0};

  const std::vector<std::vector<Biom>> BIOMS_BY_HEIGHT = {{
    {ABYSS},
    {DEEP},
    {SHALLOW},
    {SHORE},
    {SAND},
    {MEADOW, GRASS, PRAIRIE, SAND},
    {RAIN_FORREST, FORREST, GRASS, PRAIRIE, SAND},
    {ROCK},
    {SNOW, ROCK},
    {ICE},
}};

  const std::map<std::string, Biom> BIOMS_BY_TEMP = {{{SAND.name, DESERT},
                                              {PRAIRIE.name, DESERT},
                                              {GRASS.name, PRAIRIE},
                                              {MEADOW.name, GRASS}}};
}

#endif
