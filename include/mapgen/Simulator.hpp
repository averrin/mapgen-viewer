#ifndef SIM_H_
#define SIM_H_
#include "../../src/Map.cpp"

class Simulator{
public:
  Simulator(Map* m, int s);
  void simulate();

private:
  void makeRoads();
  void makeCaves();
  void upgradeCities();
  void removeBadPorts();
  void makeLighthouses();
  void makeLocationRoads();
  void makeForts();

  Map* map;
  std::mt19937 _gen;
  int _seed;
  micropather::MicroPather* _pather;
};

#endif
