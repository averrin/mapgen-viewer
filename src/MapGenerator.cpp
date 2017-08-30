#include "mapgen/MapGenerator.hpp"

void MapGenerator::build() {
  seed();
  regenHeight();
  regenDiagram();
}

void MapGenerator::seed() {
  _seed = std::clock();
}

void MapGenerator::regenHeight() {
}

void MapGenerator::regenDiagram() {
}
