#include "../include/Cell.h"
#include "../include/Edge.h"
#include <algorithm>
#include <limits>

std::vector<Cell*> Cell::getNeighbors() {
	std::vector<Cell*> neighbors;
	Edge* e;

	size_t edgeCount = halfEdges.size();
	while (edgeCount--) {
		e = halfEdges[edgeCount]->edge;
		if (e->lSite && e->lSite != &site) {
			neighbors.push_back(e->lSite->cell);
		}
		else if (e->rSite && e->rSite != &site) {
			neighbors.push_back(e->rSite->cell);
		}
	}

	return neighbors;
}

sf::Rect<double> Cell::getBoundingBox() {
	size_t edgeCount = halfEdges.size();
	double xmin = std::numeric_limits<double>::infinity();
	double ymin = xmin;
	double xmax = -xmin;
	double ymax = xmax;

	sf::Vector2<double>* vert;
	while (edgeCount--) 
    {
		vert = halfEdges[edgeCount]->startPoint();

		double vx = vert->x;
		double vy = vert->y;

		if (vx < xmin) xmin = vx;
		if (vy < ymin) ymin = vy;
		if (vx > xmax) xmax = vx;
		if (vy > ymax) ymax = vy;
	}

    return{ xmin, ymin, xmax - xmin, ymax - ymin };
}

std::vector<HalfEdge*> Cell::getEdges() {
  return halfEdges;
}

// Return whether a point is inside, on, or outside the cell:
//   -1: point is outside the perimeter of the cell
//    0: point is on the perimeter of the cell
//    1: point is inside the perimeter of the cell
//
int Cell::pointIntersection(double x, double y) {
	// Check if point in polygon. Since all polygons of a Voronoi
	// diagram are convex, then:
	// http://paulbourke.net/geometry/polygonmesh/
	// Solution 3 (2D):
	//   "If the polygon is convex then one can consider the polygon
	//   "as a 'path' from the first vertex. A point is on the interior
	//   "of this polygons if it is always on the same side of all the
	//   "line segments making up the path. ...
	//   "(y - y0) (x1 - x0) - (x - x0) (y1 - y0)
	//   "if it is less than 0 then P is to the right of the line segment,
	//   "if greater than 0 it is to the left, if equal to 0 then it lies
	//   "on the line segment"
	HalfEdge* he;
	size_t edgeCount = halfEdges.size();
	sf::Vector2<double> p0;
	sf::Vector2<double> p1;
	double r;

	while (edgeCount--) {
		he = halfEdges[edgeCount];
		p0 = *he->startPoint();
		p1 = *he->endPoint();
		r = (y - p0.y)*(p1.x - p0.x) - (x - p0.x)*(p1.y - p0.y);

		if (r == 0) {
			return 0;
		}
		if (r > 0) {
			return -1;
		}
	}
	return 1;
}

bool Cell::edgesCCW(HalfEdge* a, HalfEdge* b) {
	return a->angle > b->angle;
}
