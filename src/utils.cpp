#include "mapgen/utils.hpp"

namespace mg {
	double getDistance(Point p, Point p2) {
		double distancex = (p2->x - p->x);
		double distancey = (p2->y - p->y);
		
		return std::sqrt(distancex * distancex + distancey * distancey);
  }
};
