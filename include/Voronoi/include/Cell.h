#ifndef _CELL_H_
#define _CELL_H_

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <vector>

struct Cell;
struct Site {
    sf::Vector2<double> p;
	Cell* cell;

	Site() {};
	Site(sf::Vector2<double> _p, Cell* _cell) : p(_p), cell(_cell) {};
};

struct HalfEdge;
struct Cell {
	Site site;
	std::vector<HalfEdge*> halfEdges;
	bool closeMe;

	Cell() : closeMe(false) {};
	Cell(sf::Vector2<double> _site) : site(_site, this), closeMe(false) {};

	std::vector<Cell*> getNeighbors();
	sf::Rect<double> getBoundingBox();

  std::vector<HalfEdge*> getEdges();

	// Return whether a point is inside, on, or outside the cell:
	//   -1: point is outside the perimeter of the cell
	//    0: point is on the perimeter of the cell
	//    1: point is inside the perimeter of the cell
	int pointIntersection(double x, double y);

	static bool edgesCCW(HalfEdge* a, HalfEdge* b);
};

#endif
