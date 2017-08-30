#ifndef _VORONOI_DIAGRAM_GENERATOR_H_
#define _VORONOI_DIAGRAM_GENERATOR_H_

#include "../src/RBTree.h"
#include "../src/CircleEventQueue.h"
#include "../src/BeachLine.h"
#include "Diagram.h"
#include <vector>

class VoronoiDiagramGenerator {
public:
	VoronoiDiagramGenerator() : circleEventQueue(nullptr), siteEventQueue(nullptr), beachLine(nullptr) {};
	~VoronoiDiagramGenerator() {};

	Diagram* compute(std::vector<sf::Vector2<double>>& sites, sf::Rect<double> bbox);
	Diagram* relax();
private:
	Diagram* diagram;
	CircleEventQueue* circleEventQueue;
	std::vector<sf::Vector2<double>*>* siteEventQueue;
	sf::Rect<double>	boundingBox;

	void printBeachLine();

	//BeachLine
	RBTree<BeachSection>* beachLine;
	treeNode<BeachSection>* addBeachSection(Site* site);
	inline void detachBeachSection(treeNode<BeachSection>* section);
	void removeBeachSection(treeNode<BeachSection>* section);
	double leftBreakpoint(treeNode<BeachSection>* section, double directrix);
	double rightBreakpoint(treeNode<BeachSection>* section, double directrix);
};

inline void VoronoiDiagramGenerator::detachBeachSection(treeNode<BeachSection>* section) {
	circleEventQueue->removeCircleEvent(section);
	beachLine->removeNode(section);
}

#endif