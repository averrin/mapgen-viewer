#ifndef _EDGE_H_
#define _EDGE_H_

#include <SFML/System/Vector2.hpp>

struct Site;

struct Edge {
	Site* lSite;
	Site* rSite;
	sf::Vector2<double>* vertA;
	sf::Vector2<double>* vertB;

	Edge() : lSite(nullptr), rSite(nullptr), vertA(nullptr), vertB(nullptr) {};
	Edge(Site* _lSite, Site* _rSite) : lSite(_lSite), rSite(_rSite), vertA(nullptr), vertB(nullptr) {};
	Edge(Site* lS, Site* rS, sf::Vector2<double>* vA, sf::Vector2<double>* vB) : lSite(lS), rSite(rS), vertA(vA), vertB(vB) {};

	void setStartPoint(Site* _lSite, Site* _rSite, sf::Vector2<double>* vertex);
	void setEndPoint(Site* _lSite, Site* _rSite, sf::Vector2<double>* vertex);
};

struct HalfEdge {
	Site* site;
	Edge* edge;
	double angle;

	HalfEdge() : site(nullptr), edge(nullptr) {};
	HalfEdge(Edge* e, Site* lSite, Site* rSite);

	inline sf::Vector2<double>* startPoint();
	inline sf::Vector2<double>* endPoint();
};

inline sf::Vector2<double>* HalfEdge::startPoint() {
	return (edge->lSite == site) ? edge->vertA : edge->vertB;
}

inline sf::Vector2<double> * HalfEdge::endPoint() {
	return (edge->lSite == site) ? edge->vertB : edge->vertA;
}

#endif