#include "../include/Diagram.h"
#include "../include/VoronoiDiagramGenerator.h"
#include "Epsilon.h"
#include <iostream>
#include <algorithm>
using std::cout;
using std::endl;

sf::Vector2<double>* Diagram::createVertex(double x, double y) {
	sf::Vector2<double>* vert = vertexPool.newElement(sf::Vector2<double>(x, y));
	tmpVertices.insert(vert);

	return vert;
}

Cell* Diagram::createCell(sf::Vector2<double> site) {
	Cell* cell = cellPool.newElement(site);
	tmpCells.insert(cell);

	return cell;
}

Edge* Diagram::createEdge(Site* lSite, Site* rSite, sf::Vector2<double>* vertA, sf::Vector2<double>* vertB) {
	Edge* edge = edgePool.newElement(Edge(lSite, rSite));
	tmpEdges.insert(edge);

	if (vertA) edge->setStartPoint(lSite, rSite, vertA);
	if (vertB) edge->setEndPoint(lSite, rSite, vertB);

	lSite->cell->halfEdges.push_back(halfEdgePool.newElement(edge, lSite, rSite));
	rSite->cell->halfEdges.push_back(halfEdgePool.newElement(edge, rSite, lSite));

	return edge;
}

Edge* Diagram::createBorderEdge(Site* lSite, sf::Vector2<double>* vertA, sf::Vector2<double>* vertB) {
	Edge* edge = edgePool.newElement(Edge(lSite, nullptr, vertA, vertB));
	tmpEdges.insert(edge);

	return edge;
}

// connect dangling edges (not if a cursory test tells us
// it is not going to be visible.
// return value:
//   false: the dangling endpoint couldn't be connected
//   true: the dangling endpoint could be connected
bool Diagram::connectEdge(Edge* edge, sf::Rect<double> bbox) {
	// skip if end point already connected
	sf::Vector2<double>* va = edge->vertA;
	sf::Vector2<double>* vb = edge->vertB;
	if (vb) { return true; }

	// make local copy for speed
	Site* lSite = edge->lSite;
	Site* rSite = edge->rSite;
	double lx = lSite->p.x;
	double ly = lSite->p.y;
	double rx = rSite->p.x;
	double ry = rSite->p.y;
	double fx = (lx + rx) / 2.0;
	double fy = (ly + ry) / 2.0;
	double fm, fb;

	// if we reach here, this means cells which use this edge will need
	// to be closed, whether because the edge was removed, or because it
	// was connected to the bounding box.
	lSite->cell->closeMe = true;
	rSite->cell->closeMe = true;

	// get the line equation of the bisector if line is not vertical
	if (ry != ly) {
		fm = (lx - rx) / (ry - ly);
		fb = fy - fm*fx;
	}

	// remember, direction of line (relative to left site):
	// upward: left.x < right.x
	// downward: left.x > right.x
	// horizontal: left.x == right.x
	// upward: left.x < right.x
	// rightward: left.y < right.y
	// leftward: left.y > right.y
	// vertical: left.y == right.y

	// depending on the direction, find the best side of the
	// bounding box to use to determine a reasonable start point
	// While at it, since we have the values which define the line,
	// clip the end of va if it is outside the bbox.

	// special case: vertical line
	if (ry == ly) {
		// doesn't intersect with viewport
		if (fx < bbox.left || fx >= bbox.left + bbox.width) { return false; }
		// downward
		if (lx > rx) {
			if (!va || va->y < bbox.top) {
				va = createVertex(fx, bbox.top);
			}
			else if (va->y >= bbox.top + bbox.height) {
				return false;
			}
			vb = createVertex(fx, bbox.top + bbox.height);
		}
		// upward
		else {
			if (!va || va->y > bbox.top + bbox.height) {
				va = createVertex(fx, bbox.top + bbox.height);
			}
			else if (va->y < bbox.top) {
				return false;
			}
			vb = createVertex(fx, bbox.top);
		}
	}
	// closer to vertical than horizontal, connect start point to the
	// top or bottom side of the bounding box
	else if (fm < -1.0 || fm > 1.0) {
		// downward
		if (lx > rx) {
			if (!va || va->y < bbox.top) {
				va = createVertex((bbox.top - fb) / fm, bbox.top);
			}
			else if (va->y >= bbox.top + bbox.height) {
				return false;
			}
			vb = createVertex((bbox.top + bbox.height - fb) / fm, bbox.top + bbox.height);
		}
		// upward
		else {
			if (!va || va->y > bbox.top + bbox.height) {
				va = createVertex((bbox.top + bbox.height - fb) / fm, bbox.top + bbox.height);
			}
			else if (va->y < bbox.top) {
				return false;
			}
			vb = createVertex((bbox.top - fb) / fm, bbox.top);
		}
	}
	// closer to horizontal than vertical, connect start point to the
	// left or right side of the bounding box
	else {
		// rightward
		if (ly < ry) {
			if (!va || va->x < bbox.left) {
				va = createVertex(bbox.left, fm*bbox.left + fb);
			}
			else if (va->x >= bbox.left + bbox.width) {
				return false;
			}
			vb = createVertex(bbox.left + bbox.width, fm*bbox.left + bbox.width + fb);
		}
		// leftward
		else {
			if (!va || va->x > bbox.left + bbox.width) {
				va = createVertex(bbox.left + bbox.width, fm*bbox.left + bbox.width + fb);
			}
			else if (va->x < bbox.left) {
				return false;
			}
			vb = createVertex(bbox.left, fm*bbox.left + fb);
		}
	}
	edge->vertA = va;
	edge->vertB = vb;

	return true;
}

// line-clipping code taken from:
//   Liang-Barsky function by Daniel White
//   http://www.skytopia.com/project/articles/compsci/clipping.html
// A bit modified to minimize code paths
bool Diagram::clipEdge(Edge* edge, sf::Rect<double> bbox) {
	double ax = edge->vertA->x;
	double ay = edge->vertA->y;
	double bx = edge->vertB->x;
	double by = edge->vertB->y;
	double t0 = 0;
	double t1 = 1;
	double dx = bx - ax;
	double dy = by - ay;
	// left
	double q = ax - bbox.left;
	if (dx == 0 && q<0) { return false; }
	double r = -q / dx;
	if (dx < 0) {
		if (r < t0) { return false; }
		if (r < t1) { t1 = r; }
	}
	else if (dx>0) {
		if (r > t1) { return false; }
		if (r > t0) { t0 = r; }
	}
	// right
	q = bbox.left + bbox.width - ax;
	if (dx == 0 && q<0) { return false; }
	r = q / dx;
	if (dx<0) {
		if (r>t1) { return false; }
		if (r>t0) { t0 = r; }
	}
	else if (dx>0) {
		if (r<t0) { return false; }
		if (r<t1) { t1 = r; }
	}
	// top
	q = ay - bbox.top;
	if (dy == 0 && q<0) { return false; }
	r = -q / dy;
	if (dy<0) {
		if (r<t0) { return false; }
		if (r<t1) { t1 = r; }
	}
	else if (dy>0) {
		if (r>t1) { return false; }
		if (r>t0) { t0 = r; }
	}
	// bottom        
	q = bbox.top + bbox.height - ay;
	if (dy == 0 && q<0) { return false; }
	r = q / dy;
	if (dy<0) {
		if (r>t1) { return false; }
		if (r>t0) { t0 = r; }
	}
	else if (dy>0) {
		if (r<t0) { return false; }
		if (r<t1) { t1 = r; }
	}

	// if we reach this point, Voronoi edge is within bbox

	// if t0 > 0, va needs to change
	// we need to create a new vertex rather
	// than modifying the existing one, since the existing
	// one is likely shared with at least another edge
	if (t0 > 0) {
		edge->vertA = createVertex(ax + t0*dx, ay + t0*dy);
	}

	// if t1 < 1, vb needs to change
	// we need to create a new vertex rather
	// than modifying the existing one, since the existing
	// one is likely shared with at least another edge
	if (t1 < 1) {
		edge->vertB = createVertex(ax + t1*dx, ay + t1*dy);
	}

	// va and/or vb were clipped, thus we will need to close
	// cells which use this edge.
	if (t0 > 0 || t1 < 1) {
		edge->lSite->cell->closeMe = true;
		edge->rSite->cell->closeMe = true;
	}

	return true;
}

// Connect/cut edges at bounding box
void Diagram::clipEdges(sf::Rect<double> bbox) {
	// connect all dangling edges to bounding box
	// or get rid of them if it can't be done
	std::vector<Edge*> toRemove;

	for(Edge* edge : tmpEdges) {
		// edge is removed if:
		//   it is wholly outside the bounding box
		//   it is looking more like a point than a line
		if (!connectEdge(edge, bbox) || !clipEdge(edge, bbox) 
				|| (eq_withEpsilon(edge->vertA->x, edge->vertB->x) && eq_withEpsilon(edge->vertA->y, edge->vertB->y))) {
			edge->vertA = edge->vertB = nullptr;
			toRemove.push_back(edge);
		}
	}
	for (Edge* e : toRemove) {
		std::vector<HalfEdge*>* halfEdges;
		size_t edgeCount;
		HalfEdge* he;

		//remove lSite halfEdges
		halfEdges = &e->lSite->cell->halfEdges;
		edgeCount = halfEdges->size();
		while (edgeCount) {
			he = halfEdges->at(--edgeCount);
			if (he->edge == e) {
				halfEdges->erase(halfEdges->begin() + edgeCount);
				halfEdgePool.deleteElement(he);
			}
		}

		//remove rSite halfEdges
		halfEdges = &e->rSite->cell->halfEdges;
		edgeCount = halfEdges->size();
		while (edgeCount) {
			he = halfEdges->at(--edgeCount);
			if (he->edge == e) {
				halfEdges->erase(halfEdges->begin() + edgeCount);
				halfEdgePool.deleteElement(he);
			}
		}

		//remove edge
		tmpEdges.erase(e);
		edgePool.deleteElement(e);
	}
}

// Close the cells.
// The cells are bound by the supplied bounding box.
// Each cell refers to its associated site, and a list
// of halfedges ordered counterclockwise.
void Diagram::closeCells(sf::Rect<double> bbox) {
	sf::Vector2<double>* va;
	sf::Vector2<double>* vb;
	sf::Vector2<double>* vz;
	Edge* edge;
	std::vector<HalfEdge*>* halfEdges;

	for (Cell* cell : tmpCells) {
		// prune, order halfedges counterclockwise, then add missing ones
		// required to close cells
		halfEdges = &cell->halfEdges;
		std::sort(halfEdges->begin(), halfEdges->end(), Cell::edgesCCW);
		size_t nHalfEdges = halfEdges->size();
		if (!nHalfEdges) {
			continue;
		}
		if (!cell->closeMe) {
			continue;
		}
		// find first 'unclosed' point.
		// an 'unclosed' point will be the end point of a halfedge which
		// does not match the start point of the following halfedge
		size_t iLeft = 0;
		while (iLeft < nHalfEdges) {
			va = (*halfEdges)[iLeft]->endPoint();
			vz = (*halfEdges)[(iLeft + 1) % nHalfEdges]->startPoint();
			// if end point is not equal to start point, we need to add the missing
			// halfedge(s) up to vz
			if (std::abs(va->x - vz->x) >= EPSILON || std::abs(va->y - vz->y) >= EPSILON) {
				// find entry point:
				bool foundEntryPoint = false;
				bool finished = false;
				bool lastBorderSegment = false;

				// walk downward along left side
				if (eq_withEpsilon(va->x, bbox.left) && lt_withEpsilon(va->y, bbox.top + bbox.height)) {
					foundEntryPoint = true;
					lastBorderSegment = eq_withEpsilon(vz->x, bbox.left);
					vb = lastBorderSegment ? vz : createVertex(bbox.left, lastBorderSegment ? vz->y : bbox.top + bbox.height);
					edge = createBorderEdge(&cell->site, va, vb);
					++iLeft;
					halfEdges->insert(halfEdges->begin() + iLeft, halfEdgePool.newElement(edge, &cell->site, nullptr));
					++nHalfEdges;
					if (lastBorderSegment) { finished = true; }
					va = vb;
				}

				// walk rightward along bottom side
				if ((!finished && foundEntryPoint) || (eq_withEpsilon(va->y, bbox.top + bbox.height) && lt_withEpsilon(va->x, bbox.left + bbox.width))) {
					foundEntryPoint = true;
					lastBorderSegment = eq_withEpsilon(vz->y, bbox.top + bbox.height);
					vb = lastBorderSegment ? vz : createVertex(lastBorderSegment ? vz->x : bbox.left + bbox.width, bbox.top + bbox.height);
					edge = createBorderEdge(&cell->site, va, vb);
					++iLeft;
					halfEdges->insert(halfEdges->begin() + iLeft, halfEdgePool.newElement(edge, &cell->site, nullptr));
					++nHalfEdges;
					if (lastBorderSegment) { finished = true; }
					va = vb;
				}

				// walk upward along right side
				if ((!finished && foundEntryPoint) || (eq_withEpsilon(va->x, bbox.left + bbox.width) && gt_withEpsilon(va->y, bbox.top))) {
					foundEntryPoint = true;
					lastBorderSegment = eq_withEpsilon(vz->x, bbox.left + bbox.width);
					vb = lastBorderSegment ? vz : createVertex(bbox.left + bbox.width, lastBorderSegment ? vz->y : bbox.top);
					edge = createBorderEdge(&cell->site, va, vb);
					++iLeft;
					halfEdges->insert(halfEdges->begin() + iLeft, halfEdgePool.newElement(edge, &cell->site, nullptr));
					++nHalfEdges;
					if (lastBorderSegment) { finished = true; }
					va = vb;
				}

				// walk leftward along top side
				if ((!finished && foundEntryPoint) || (eq_withEpsilon(va->y, bbox.top) && gt_withEpsilon(va->x, bbox.left))) {
					lastBorderSegment = eq_withEpsilon(vz->y, bbox.top);
					vb = lastBorderSegment ? vz : createVertex(lastBorderSegment ? vz->x : bbox.left, bbox.top);
					edge = createBorderEdge(&cell->site, va, vb);
					++iLeft;
					halfEdges->insert(halfEdges->begin() + iLeft, halfEdgePool.newElement(edge, &cell->site, nullptr));
					++nHalfEdges;
					if (lastBorderSegment) { finished = true; }
					va = vb;

					// walk downward along left side
					if (!finished) {
						lastBorderSegment = eq_withEpsilon(vz->x, bbox.left);
						vb = lastBorderSegment ? vz : createVertex(bbox.left, lastBorderSegment ? vz->y : bbox.top + bbox.height);
						edge = createBorderEdge(&cell->site, va, vb);
						++iLeft;
						halfEdges->insert(halfEdges->begin() + iLeft, halfEdgePool.newElement(edge, &cell->site, nullptr));
						++nHalfEdges;
						if (lastBorderSegment) { finished = true; }
						va = vb;
					}

					// walk rightward along bottom side
					if (!finished) {
						lastBorderSegment = eq_withEpsilon(vz->y, bbox.top + bbox.height);
						vb = lastBorderSegment ? vz : createVertex(lastBorderSegment ? vz->x : bbox.left + bbox.width, bbox.top + bbox.height);
						edge = createBorderEdge(&cell->site, va, vb);
						++iLeft;
						halfEdges->insert(halfEdges->begin() + iLeft, halfEdgePool.newElement(edge, &cell->site, nullptr));
						++nHalfEdges;
						if (lastBorderSegment) { finished = true; }
						va = vb;
					}

					// walk upward along right side
					if (!finished) {
						lastBorderSegment = eq_withEpsilon(vz->x, bbox.left + bbox.width);
						vb = lastBorderSegment ? vz : createVertex(bbox.left + bbox.width, lastBorderSegment ? vz->y : bbox.top);
						edge = createBorderEdge(&cell->site, va, vb);
						++iLeft;
						halfEdges->insert(halfEdges->begin() + iLeft, halfEdgePool.newElement(edge, &cell->site, nullptr));
						++nHalfEdges;
					}
				}
			}
			++iLeft;
		}
		cell->closeMe = false;
	}
}

void Diagram::finalize() {
	cells.reserve(tmpCells.size());
	for (Cell* c : tmpCells) {
		cells.push_back(c);
	}
	tmpCells.clear();

	edges.reserve(tmpEdges.size());
	for (Edge* e : tmpEdges) {
		edges.push_back(e);
	}
	tmpEdges.clear();

	vertices.reserve(tmpVertices.size());
	for (sf::Vector2<double>* v : tmpVertices) {
		vertices.push_back(v);
	}
	tmpVertices.clear();
}

void Diagram::printDiagram() {
	if (cells.size()) {
		for (Cell* c : cells) {
			cout << c->site.p.x << " " << c->site.p.y << "\n" << c << endl;
			for (HalfEdge* e : c->halfEdges) {
				cout << '\t' << e->startPoint()->x << " " << e->startPoint()->y << "\n" << endl;
			}
			cout << endl;
		}
		for (Edge* e : edges) {
			cout << e->vertA->x << " " << e->vertA->y  << "\n" << " -> " << 
                e->vertB->x << " " << e->vertB->y << "\n" << endl;
		}
		cout << endl;
	}
	else {
		for (Cell* c : tmpCells) {
			cout << c->site.p.x << " " << c->site.p.y << "\n" << endl;
			for (HalfEdge* e : c->halfEdges) {
				sf::Vector2<double>* pS = e->startPoint();
				sf::Vector2<double>* pE = e->endPoint();

				cout << '\t';
				if (pS) cout << pS->x << " " << pS->y << "\n";
				else cout << "null";
				cout << " -> ";
				if (pE) cout << pE->x << " " << pE->y << "\n";
				else cout << "null";
				cout << endl;
			}
			cout << endl;
		}
		for (Edge* e : tmpEdges) {
			if (e->vertA)
				cout << e->vertA->x << " " << e->vertA->y << "\n";
			else
				cout << "null";
			cout << " -> ";
			if (e->vertB)
				cout << e->vertB->x << " " << e->vertB->y << "\n";
			else
				cout << "null";
			cout << endl;
		}
		cout << endl;
	}
	cout << "=============================================" << endl;
}
