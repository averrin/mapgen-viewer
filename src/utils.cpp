#include "mapgen/utils.hpp"

namespace mg {
	double getDistance(Point p, Point p2) {
		double distancex = (p2->x - p->x);
		double distancey = (p2->y - p->y);

		return std::sqrt(distancex * distancex + distancey * distancey);
  }

  template <typename T>
  std::vector<T *> filterObjects(std::vector<T *> regions,
                                      filterFunc<T> filter, sortFunc<T> sort) {
    std::vector<T *> places;

    std::copy_if(regions.begin(), regions.end(), std::back_inserter(places),
                 filter);
    std::sort(places.begin(), places.end(), sort);
    return places;
  }
};
