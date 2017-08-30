#include "../include/VoronoiDiagramGenerator.h"
#include <SFML/System/Vector2.hpp>
#include "BeachLine.h"
#include "../include/Cell.h"
#include "Epsilon.h"
#include <vector>
#include <forward_list>
#include <limits>

treeNode<BeachSection>* VoronoiDiagramGenerator::addBeachSection(Site* site) {
	double x = site->p.x;
	double directrix = site->p.y;

	// find the left and right beach sections which will surround the newly
	// created beach section.
	treeNode<BeachSection>* lSection = nullptr;
	treeNode<BeachSection>* rSection = nullptr;
	treeNode<BeachSection>* node = beachLine->getRoot();

	double dxl, dxr;
	while (node) {
		dxl = leftBreakpoint(node, directrix) - x;
		if (dxl > EPSILON) {
			//falls somewhere before the left edge of the beachsection
			node = node->left;
		}
		else {
			dxr = x - rightBreakpoint(node, directrix);
			if (dxr > EPSILON) {
				//falls somewhere after the right edge of the beachsection
				if (!node->right) {
					lSection = node;
					break;
				}
				node = node->right;
			}
			else {
				if (dxl > -EPSILON) {
					//falls exactly on the left edge of the beachsection
					lSection = node->prev;
					rSection = node;
				}
				else if (dxr > -EPSILON) {
					//exactly on the right edge of the beachsection
					lSection = node;
					rSection = node->next;
				}
				else {
					// falls somewhere in the middle of the beachsection
					lSection = rSection = node;
				}
				break;
			}
		}
	}
	// at this point, lSection and/or rSection could be null.

	// add a new beach section for the site to the RB-tree
	BeachSection section(site);
	treeNode<BeachSection>* newSection = beachLine->insertSuccessor(lSection, section);

	// cases:

	// [lSection,rSection] where lSection == rSection
	// most likely case: new beach section split an existing beach
	// section.
	// This case means:
	//   one new transition appears
	//   the left and right beach section might be collapsing as a result
	//   two new nodes added to the RB-tree
	if (lSection && lSection == rSection) {
		// invalidate circle event of split beach section
		circleEventQueue->removeCircleEvent(lSection);

		// split the beach section into two separate beach sections
		BeachSection copy = BeachSection(lSection->data.site);
		rSection = beachLine->insertSuccessor(newSection, copy);

		// since we have a new transition between two beach sections, a new edge is born
		Edge* newEdge = diagram->createEdge(lSection->data.site, newSection->data.site, nullptr, nullptr);
		newSection->data.edge = rSection->data.edge = newEdge;

		// check whether the left and right beach sections are collapsing 
		// and if so create circle events
		circleEventQueue->addCircleEvent(lSection);
		circleEventQueue->addCircleEvent(rSection);

		return newSection;
	}

	// [lSection,rSection] where lSection != rSection
	// fairly unlikely case: new beach section falls *exactly* in between two
	// existing beach sections
	// This case means:
	//   one transition disappears
	//   two new transitions appear
	//   the left and right beach section might be collapsing as a result
	//   only one new node added to the RB-tree
	if (lSection && rSection && lSection != rSection) {
		// invalidate circle events of left and right sites
		circleEventQueue->removeCircleEvent(lSection);
		circleEventQueue->removeCircleEvent(rSection);

		// an existing transition disappears, meaning a vertex is defined at
		// the disappearance point.
		// since the disappearance is caused by the new beachsection, the
		// vertex is at the center of the circumscribed circle of the left,
		// new and right beachsections.
		// http://mathforum.org/library/drmath/view/55002.html
		Site* lSite = lSection->data.site;
		Site* rSite = rSection->data.site;
		sf::Vector2<double>& lP = lSite->p;
		sf::Vector2<double>& sP = site->p;
		sf::Vector2<double>& rP = rSite->p;
		double ax = lP.x;
		double ay = lP.y;
		double bx = sP.x - ax;
		double by = sP.y - ay;
		double cx = rP.x - ax;
		double cy = rP.y - ay;
		double d = 2 * (bx*cy - by*cx);
		double hb = bx*bx + by*by;
		double hc = cx*cx + cy*cy;
		sf::Vector2<double>* vertex = diagram->createVertex((cy*hb - by*hc) / d + ax, (bx*hc - cx*hb) / d + ay);

		// one transition disappears
		rSection->data.edge->setStartPoint(lSite, rSite, vertex);

		// two new transitions appear at the new vertex location
		newSection->data.edge = diagram->createEdge(lSite, site, nullptr, vertex);
		rSection->data.edge = diagram->createEdge(site, rSite, nullptr, vertex);

		// check whether the left and right beach sections are collapsing
		// and if so create circle events
		circleEventQueue->addCircleEvent(lSection);
		circleEventQueue->addCircleEvent(rSection);

		return newSection;
	}

	// [lSection,null]
	// very unlikely case: new beach section is the *last* beach section
	// on the beachline -- this can happen only if all the previous beach
	// sections currently on the beachline share the same y value as
	// the new beach section.
	// This case means:
	//   one new transition appears
	//   no collapsing beach section as a result
	//   new beach section becomes right-most node of the RB-tree
	if (lSection && !rSection) {
		newSection->data.edge = diagram->createEdge(lSection->data.site, newSection->data.site, nullptr, nullptr);
		return newSection;
	}

	// [null,null]
	// least likely case: new beach section is the first beach section on the
	// beachline.
	// This case means:
	//   no new transition appears
	//   no collapsing beach section
	//   new beachsection becomes root of the RB-tree
	if (!lSection && !rSection) {
		return newSection;
	}

	return nullptr;
}

void VoronoiDiagramGenerator::removeBeachSection(treeNode<BeachSection>* section) {
	CircleEvent circle = section->data.circleEvent->data;
	double x = circle.x;
	double y = circle.yCenter;
	sf::Vector2<double>* vertex = diagram->createVertex(x, y);
	treeNode<BeachSection>* prev = section->prev;
	treeNode<BeachSection>* next = section->next;
	std::vector<treeNode<BeachSection>*> disappearingTransitions;
	std::forward_list<treeNode<BeachSection>*> toBeDetached;
	disappearingTransitions.push_back(section);

	// save collapsed beachsection to be detached from beachline
	toBeDetached.push_front(section);

	// there could be more than one empty arc at the deletion point, this
	// happens when more than two edges are linked by the same vertex,
	// so we will collect all those edges by looking along both sides of
	// the deletion point.

	// look left
	treeNode<BeachSection>* lSection = prev;
	while (lSection->data.circleEvent 
			&& eq_withEpsilon(x, lSection->data.circleEvent->data.x) 
			&& eq_withEpsilon(y, lSection->data.circleEvent->data.yCenter)) {
		prev = lSection->prev;
		disappearingTransitions.insert(disappearingTransitions.begin(), lSection);
		toBeDetached.push_front(lSection);
		lSection = prev;
	}
	// even though it is not disappearing, I will also add the beach section
	// immediately to the left of the left-most collapsed beach section, for
	// convenience, since we need to refer to it later as this beach section
	// is the 'left' site of an edge for which a start point is set.
	disappearingTransitions.insert(disappearingTransitions.begin(), lSection);
	circleEventQueue->removeCircleEvent(lSection);

	// look right
	treeNode<BeachSection>* rSection = next;
	while (rSection->data.circleEvent 
			&& eq_withEpsilon(x, rSection->data.circleEvent->data.x) 
			&& eq_withEpsilon(y, rSection->data.circleEvent->data.yCenter)) {
		next = rSection->next;
		disappearingTransitions.push_back(rSection);
		toBeDetached.push_front(rSection);
		rSection = next;
	}
	// we also have to add the beach section immediately to the right of the
	// right-most collapsed beach section, since there is also a disappearing
	// transition representing an edge's start point on its left.
	disappearingTransitions.push_back(rSection);
	circleEventQueue->removeCircleEvent(rSection);

	// walk through all the disappearing transitions between beach sections and
	// set the start point of their (implied) edges.
	size_t nSections = disappearingTransitions.size();
	for (size_t i = 1; i < nSections; ++i) {
		rSection = disappearingTransitions[i];
		lSection = disappearingTransitions[i-1];
		rSection->data.edge->setStartPoint(lSection->data.site, rSection->data.site, vertex);
	}

	// create a new edge as we have now a new transition between
	// two beach sections which were previously not adjacent.
	// since this edge appears as a new vertex is defined, the vertex
	// actually defines an end point of the edge (relative to the site
	// on the left)
	lSection = disappearingTransitions[0];
	rSection = disappearingTransitions[nSections - 1];
	rSection->data.edge = diagram->createEdge(lSection->data.site, rSection->data.site, nullptr, vertex);

	for (treeNode<BeachSection>* section : toBeDetached) {
		detachBeachSection(section);
	}

	// create circle events if any for beach sections left in the beachline
	// adjacent to collapsed sections
	circleEventQueue->addCircleEvent(lSection);
	circleEventQueue->addCircleEvent(rSection);
}

// calculate the left break point of a particular beach section,
// given a particular sweep line height
double VoronoiDiagramGenerator::leftBreakpoint(treeNode<BeachSection>* section, double directrix) {
	sf::Vector2<double> site = section->data.site->p;
	double rfocx = site.x;
	double rfocy = site.y;
	double pby2 = rfocy - directrix;
	if (pby2 == 0) {
		// parabola in degenerate case where focus is on directrix
		return rfocx;
	}

	treeNode<BeachSection>* lSection = section->prev;
	if (!lSection) {
		return -std::numeric_limits<double>::infinity();
	}

	site = lSection->data.site->p;
	double lfocx = site.x;
	double lfocy = site.y;
	double plby2 = lfocy - directrix;
	if (plby2 == 0) {
		// parabola in degenerate case where focus is on directrix
		return lfocx;
	}

	double hl = lfocx - rfocx;
	double aby2 = (1 / pby2) - (1 / plby2);
	double b = hl / plby2;
	if (aby2 != 0) {
		return (-b + std::sqrt(b*b - 2 * aby2*(hl*hl / (-2 * plby2) - lfocy + plby2 / 2 + rfocy - pby2 / 2))) / aby2 + rfocx;
	}

	// if we get here, the two parabolas have the same 
	// distance to the directrix, so the break point is midway
	return (rfocx + lfocx) / 2;
}

// calculate the right break point of a particular beach section,
// given a particular sweep line height
double VoronoiDiagramGenerator::rightBreakpoint(treeNode<BeachSection>* section, double directrix) {
	treeNode<BeachSection>* rSection = section->next;
	if (rSection) {
		return leftBreakpoint(rSection, directrix);
	}

	sf::Vector2<double> site = section->data.site->p;
	if (site.y == directrix) {
		return site.x;
	}
	else {
		return std::numeric_limits<double>::infinity();
	}
}
