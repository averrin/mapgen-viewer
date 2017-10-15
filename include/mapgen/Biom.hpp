#ifndef BIOM_H_
#define BIOM_H_
#include <SFML/Graphics.hpp>

struct Biom {
  float border;
  sf::Color color;
  std::string name;
  float feritlity;
  bool operator==(const Biom& b) const {
    return b.name == name;
  }
  bool operator!=(const Biom& b) const {
    return b.name != name;
  }
};

namespace biom {
const float DEFAULT_HUMIDITY = 0.f;
  const float DEFAULT_TEMPERATURE = 30.f;

  const Biom ABYSS = {-2.0000f, sf::Color(23, 23, 40), "Abyss", 0.f};
  const Biom DEEP = {-1.0000f, sf::Color(39, 39, 70), "Deep", 0.f};
  const Biom SHALLOW = {-0.2500f, sf::Color(51, 51, 91), "Shallow", 0.f};
  const Biom SHORE = {0.0000f, sf::Color(68, 99, 130), "Shore", 0.f};
  // const Biom SAND = {0.0625, sf::Color(255,255,255), "Sand", 0};
  const Biom SAND = {0.0625f, sf::Color(210, 185, 139), "Sand", 0.f};
  const Biom GRASS = {0.1250f, sf::Color(136, 170, 85), "Grass", 0.8f};
  const Biom FORREST = {0.3750f, sf::Color(51, 119, 85), "Forrest", 0.6f};
  const Biom ROCK = {0.7500f, sf::Color(108, 108, 108), "Rock", 0.f};
  const Biom SNOW = {1.0000f, sf::Color(240, 240, 240), "Snow", 0.f};
  const Biom ICE = {1.2000f, sf::Color(220, 220, 255), "Ice", 0.f};
  const Biom PRAIRIE = {999.000f, sf::Color(239, 220, 124), "Prairie", 0.6f};
  const Biom MEADOW = {999.000f, sf::Color(126, 190, 75), "Meadow", 1.f};
  const Biom DESERT = {999.000f, sf::Color(244, 164, 96), "Desert", 0.f};
  const Biom CITY = {999.000f, sf::Color(220, 220, 220), "City", 0.f};

  const Biom RAIN_FORREST = {0.3750f, sf::Color(51, 90, 75), "Rain forrest", 0.6f};

  const std::vector<Biom> BIOMS = {
    {ABYSS, DEEP, SHALLOW, SHORE, SAND, GRASS, FORREST, ROCK, SNOW, ICE}};

  const Biom LAKE = {999.000f, sf::Color(51, 51, 91), "Lake", 0.f};
  const Biom MARK = {999.000f, sf::Color::Red, "Mark"};
  const Biom MARK2 = {999.000f, sf::Color::Black, "Mark"};

  const Biom LAND = {0.500f, sf::Color(136, 170, 85), "Land", 0.f};
  const Biom SEA = {-1.000f, sf::Color(39, 39, 70), "Sea", 0.f};

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
