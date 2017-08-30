#include <SFML/Graphics.hpp>
#include "Region.hpp"

namespace map {
  class MapGenerator {
  public:
    MapGenerator(){};

    void build();
    void update();
    void relax();
    void setSeed(int seed);
    void setOctaveCount(int octaveCount);
    void setFrequency(int freq);
    void setPointCount(int count);
    int  getPointCount();
    Region* getRegion(sf::Vector2f pos);
  };
}
