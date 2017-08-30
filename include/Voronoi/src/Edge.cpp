#include "../include/Edge.h"
#include "../include/Cell.h"
#include <cmath>

HalfEdge::HalfEdge(Edge* e, Site* lSite, Site* rSite) {
	site = lSite;
	edge = e;
	// 'angle' is a value to be used for properly sorting the
	// halfsegments counterclockwise. By convention, we will
	// use the angle of the line defined by the 'site to the left'
	// to the 'site to the right'.
	// However, border edges have no 'site to the right': thus we
	// use the angle of line perpendicular to the halfsegment (the
	// edge should have both end points defined in such case.)
	if (rSite) {
		angle = atan2(rSite->p.y - lSite->p.y, rSite->p.x - lSite->p.x);
	}
	else {
		sf::Vector2<double>& va = *(e->vertA);
		sf::Vector2<double>& vb = *(e->vertB);

		angle = (e->lSite == lSite) ? atan2(vb.x - va.x, va.y - vb.y) : atan2(va.x - vb.x, vb.y - va.y);
	}
}

void Edge::setStartPoint(Site* _lSite, Site* _rSite, sf::Vector2<double>* vertex) {
	if (!vertA && !vertB) {
		vertA = vertex;
		lSite = _lSite;
		rSite = _rSite;
	}
	else if (lSite == _rSite) {
		vertB = vertex;
	}
	else {
		vertA = vertex;
	}
}

void Edge::setEndPoint(Site* _lSite, Site* _rSite, sf::Vector2<double>* vertex) {
	setStartPoint(_rSite, _lSite, vertex);
}
