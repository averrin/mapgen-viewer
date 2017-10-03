#ifndef REPORT_H_
#define REPORT_H_
#include <vector>
#include <limits>

class Report {
public:
  Report();
  std::vector<float> population;
  std::vector<float> wealth;
  int minPopulation = std::numeric_limits<int>::max();
  int maxPopulation = 0;
  float minWealth = std::numeric_limits<int>::max();
  float maxWealth = 0;
};

#endif
