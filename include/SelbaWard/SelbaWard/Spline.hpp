//////////////////////////////////////////////////////////////////////////////
//
// Selba Ward (https://github.com/Hapaxia/SelbaWard)
// --
//
// Spline
//
// Copyright(c) 2014-2017 M.J.Silk
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions :
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software.If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
// M.J.Silk
// MJSilk2@gmail.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SELBAWARD_SPLINE_HPP
#define SELBAWARD_SPLINE_HPP

#include "Common.hpp"

namespace selbaward
{

// SW Spline v1.3.0
class Spline : public sf::Drawable
{
public:
	struct Vertex
	{
		sf::Vector2f position;
		sf::Vector2f frontHandle; // offset from position
		sf::Vector2f backHandle; // offset from position
		float thickness;
		sf::Color color;
		Vertex() : thickness(1.f), color(sf::Color::White) {}
		Vertex(sf::Vector2f newPosition) : position(newPosition), thickness(1.f), color(sf::Color::White) {}
	};

	Spline(unsigned int vertexCount = 0u, sf::Vector2f initialPosition = { 0.f, 0.f });
	Spline(std::initializer_list<sf::Vector2f> list); // pass vertices' positions (sf::Vector2f) to the constructor (sets size automatically)
	void update();

	Vertex& operator[] (unsigned int index); // direct access to the spline's vertices (sw::Spline::Vertex) using the [] operator. no checks are performed. using with an invalid index results in undefined behaviour

	void setClosed(bool isClosed);
	bool getClosed() const;

	unsigned int getVertexCount() const;
	unsigned int getLastVertexIndex() const;
	float getLength() const; // returns the length of (linear) spline - the sum of the lengths of all of the lines between consecutive vertices
	float getInterpolatedLength() const; // returns the actual length of the spline - the sum of the lengths of all of the line segments drawn

	void reserveVertices(unsigned int numberOfVertices);

	void addVertices(const std::vector<sf::Vector2f>& positions);
	void addVertices(unsigned int index, const std::vector<sf::Vector2f>& positions); // inserts the new vertices before the vertex at the specified index
	void addVertices(unsigned int numberOfVertices, sf::Vector2f position = { 0.f, 0.f });
	void addVertices(unsigned int numberOfVertices, unsigned int index, sf::Vector2f position = { 0.f, 0.f }); // inserts the new vertices before the vertex at the specified index
	void addVertex(sf::Vector2f position = { 0.f, 0.f });
	void addVertex(unsigned int index, sf::Vector2f position = { 0.f, 0.f }); // inserts the new vertex before the vertex at the specified index
	void removeVertex(unsigned int index);
	void removeVertices(unsigned int index, unsigned int numberOfVertices = 0u); // if numberOfVertices is zero (the default), removes all vertices from specified index until the end
	void reverseVertices();
	
	void setPosition(unsigned int index, sf::Vector2f position = { 0.f, 0.f });
	void setPositions(unsigned int index, unsigned int numberOfVertices = 0u, sf::Vector2f position = { 0.f, 0.f }); // if numberOfVertices is zero (the default), sets positions of all vertices from specified index until the end
	void setPositions(const std::vector<sf::Vector2f>& positions, unsigned int index = 0u);
	sf::Vector2f getPosition(unsigned int index) const;

	void setFrontHandle(unsigned int index, sf::Vector2f offset);
	sf::Vector2f getFrontHandle(unsigned int index) const;

	void setBackHandle(unsigned int index, sf::Vector2f offset);
	sf::Vector2f getBackHandle(unsigned int index) const;

	void resetHandles(unsigned int index = 0u, unsigned int numberOfVertices = 0u); // if numberOfVertices is zero (the default), reset handles for all vertices from specified index until the end. if no index is specified, all handles are reset

	void smoothHandles();

	void setHandlesVisible(bool handlesVisible = true);
	bool getHandlesVisible() const;

	template <class T>
	void setThickness(T thickness);
	float getThickness() const;
	template <class T>
	void setThickness(unsigned int index, T thickness);
	float getThickness(unsigned int index) const;

	void setColor(sf::Color color);
	sf::Color getColor() const;
	void setColor(unsigned int index, sf::Color color);
	sf::Color getColor(unsigned int index) const;

	void setInterpolationSteps(unsigned int interpolationSteps);
	unsigned int getInterpolationSteps() const;

	void setHandleMirrorLock(bool handleMirrorLock = true);
	void setHandleAngleLock(bool handleAngleLock = true);

	void setBezierInterpolation(bool bezierInterpolation = true);
	bool getBezierInterpolation() const;

	void setPrimitiveType(sf::PrimitiveType primitiveType);
	sf::PrimitiveType getPrimitiveType() const;

	sf::Vector2f getInterpolatedPosition(unsigned int interpolationOffset, unsigned int index = 0u) const; // index is control vertex offset
	sf::Vector2f getInterpolatedPositionTangent(unsigned int interpolationOffset, unsigned int index = 0u) const; // index is control vertex offset
	sf::Vector2f getInterpolatedPositionNormal(unsigned int interpolationOffset, unsigned int index = 0u) const; // index is control vertex offset
	float getInterpolatedPositionThickness(unsigned int interpolationOffset, unsigned int index = 0u) const; // index is control vertex offset
	float getInterpolatedPositionThicknessCorrectionScale(unsigned int interpolationOffset, unsigned int index = 0u) const; // index is control vertex offset
	unsigned int getInterpolatedPositionCount() const;


private:
	bool m_throwExceptions;

	bool m_isClosed;

	std::vector<Vertex> m_vertices;
	sf::Color m_color;
	float m_thickness;

	std::vector<sf::Vector2f> m_sfmlVerticesUnitTangents;
	std::vector<sf::Vertex> m_sfmlVertices;
	std::vector<sf::Vertex> m_sfmlThickVertices;
	sf::PrimitiveType m_primitiveType;
	unsigned int m_interpolationSteps;
	bool m_useBezier;

	std::vector<sf::Vertex> m_handlesVertices;
	bool m_showHandles;
	bool m_lockHandleMirror;
	bool m_lockHandleAngle;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	bool priv_isValidVertexIndex(unsigned int vertexIndex) const;
	bool priv_testVertexIndex(unsigned int vertexIndex, const std::string& exceptionMessage) const;
	bool priv_isThick() const;
	unsigned int priv_getNumberOfPointsPerVertex() const;
	unsigned int priv_getInterpolatedIndex(const unsigned int interpolationOffset, const unsigned int index) const;
};

template <class T>
inline void Spline::setThickness(const T thickness)
{
	m_thickness = static_cast<float>(thickness);
}

inline float Spline::getThickness() const
{
	return m_thickness;
}

template <class T>
inline void Spline::setThickness(const unsigned int index, const T thickness)
{
	m_vertices[index].thickness = static_cast<float>(thickness);
}

inline float Spline::getThickness(const unsigned int index) const
{
	return m_vertices[index].thickness;
}

inline Spline::Vertex& Spline::operator[] (const unsigned int index)
{
	return m_vertices[index];
}

inline bool Spline::getClosed() const
{
	return m_isClosed;
}

inline unsigned int Spline::getVertexCount() const
{
	return static_cast<unsigned int>(m_vertices.size());
}

inline unsigned int Spline::getLastVertexIndex() const
{
	return static_cast<unsigned int>(m_vertices.size()) - 1u;
}

inline bool Spline::getHandlesVisible() const
{
	return m_showHandles;
}

inline sf::Color Spline::getColor() const
{
	return m_color;
}

inline sf::Color Spline::getColor(const unsigned int index) const
{
	return m_vertices[index].color;
}

inline unsigned int Spline::getInterpolationSteps() const
{
	return m_interpolationSteps;
}

inline sf::PrimitiveType Spline::getPrimitiveType() const
{
	return m_primitiveType;
}

inline bool Spline::getBezierInterpolation() const
{
	return m_useBezier;
}

} // namespace selbaward
#endif // SELBAWARD_SPLINE_HPP
