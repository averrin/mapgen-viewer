#ifndef NAMES_HPP_
#define NAMES_HPP_

namespace names {
  std::string generateRiverName(std::mt19937 *gen);
  std::string generateLandName(std::mt19937 *gen);
  std::string generateSeaName(std::mt19937 *gen);
  std::string generateCityName(std::mt19937 *gen);
};
#endif
