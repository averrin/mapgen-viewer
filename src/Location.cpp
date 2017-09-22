#include "mapgen/Location.hpp"

Location::Location(Region *r, std::string n, LocationType t) : region(r), name(n), type(t) {
  region->location = this;

  switch (this->type) {
  case CAPITAL:
    typeName = "Capital";
    break;
  case PORT:
    typeName = "Port";
    break;
  case MINE:
    typeName = "Mine";
    break;
  case AGRO:
    typeName = "Agro";
    break;
  case TRADE:
    typeName = "Trade post";
    break;
  case LIGHTHOUSE:
    typeName = "Lighthouse";
    break;
  case CAVE:
    typeName = "Cave";
    break;
  case FORT:
    typeName = "Fort";
    break;
  }

}
