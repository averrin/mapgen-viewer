#include "mapgen/City.hpp"
#include "mapgen/Road.hpp"
#include "mapgen/MapGenerator.hpp"

class Walker {
public:
  Walker(City* s, MapGenerator* m);
  void tick();
  sf::CircleShape* shape = nullptr;
private:
  Road* getNextRoad();
  MapGenerator* mapgen;
  Road* road = nullptr;
  int step = 0;
  bool reverse = false;

  template<typename Iter>
  Iter select_randomly(Iter start, Iter end);

};
