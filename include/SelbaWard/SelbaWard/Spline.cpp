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

#include "Spline.hpp"

#include <cmath>
#include <assert.h>

namespace
{

const std::string exceptionPrefix = "Spline: ";
constexpr float thicknessEpsilon{ 0.001f };
constexpr float zeroEpsilon{ 0.00001f };
constexpr float pi{ 3.141592653f };
#ifdef USE_SFML_PRE_2_4
const sf::PrimitiveType thickPrimitiveType{ sf::PrimitiveType::TrianglesStrip };
#else // USE_SFML_PRE_2_4
const sf::PrimitiveType thickPrimitiveType{ sf::PrimitiveType::TriangleStrip };
#endif // USE_SFML_PRE_2_4

template <class T>
inline T linearInterpolation(const T start, const T end, const float alpha)
{
	return static_cast<T>(end * alpha + start * (1 - alpha));
}

sf::Color linearInterpolation(const sf::Color start, const sf::Color end, const float alpha)
{
	return sf::Color(linearInterpolation(start.r, end.r, alpha), linearInterpolation(start.g, end.g, alpha), linearInterpolation(start.b, end.b, alpha));
}

inline sf::Vector2f bezierInterpolation(const sf::Vector2f start, const sf::Vector2f end, const sf::Vector2f startHandle, const sf::Vector2f endHandle, const float alpha)
{
	const sf::Vector2f startControl{ start + startHandle };
	const sf::Vector2f endControl{ end + endHandle };
	const float alpha2{ 1.f - alpha };
	const float alphaSquared{ alpha * alpha };
	const float alpha2Squared{ alpha2 * alpha2 };

	return
	{
		start.x * alpha2Squared * alpha2 + startControl.x * 3 * alpha2Squared * alpha + endControl.x * 3 * alpha2 * alphaSquared + end.x * alpha * alphaSquared,
		start.y * alpha2Squared * alpha2 + startControl.y * 3 * alpha2Squared * alpha + endControl.y * 3 * alpha2 * alphaSquared + end.y * alpha * alphaSquared
	};
}

inline float dot(const sf::Vector2f a, const sf::Vector2f b)
{
	return a.x * b.x + a.y * b.y;
}

inline float vectorLength(const sf::Vector2f vector)
{
	return std::sqrt(dot(vector, vector));
}

inline void copyAngle(const sf::Vector2f source, sf::Vector2f destination)
{
	destination = -(source / vectorLength(source)) * vectorLength(destination);
}

template <class T>
inline T abs(const T value)
{
	return value < 0 ? -value : value;
}

inline sf::Vector2f vectorNormal(const sf::Vector2f vector)
{
	return{ vector.y, -vector.x };
}

inline sf::Vector2f vectorUnit(const sf::Vector2f vector)
{
	if ((abs(vector.x) < zeroEpsilon) && (abs(vector.y) < zeroEpsilon))
		return{ 0.f, 0.f };
	else
		return vector / vectorLength(vector);
}

} // namespace

namespace selbaward
{

Spline::Spline(const unsigned int vertexCount, const sf::Vector2f initialPosition)
	: m_throwExceptions(true)
	, m_isClosed(false)
	, m_vertices(vertexCount, Vertex(initialPosition))
	, m_color(sf::Color::White)
	, m_thickness(0.f)
	, m_sfmlVertices()
	, m_sfmlThickVertices()
#ifdef USE_SFML_PRE_2_4
	, m_primitiveType(sf::PrimitiveType::LinesStrip)
#else // USE_SFML_PRE_2_4
	, m_primitiveType(sf::PrimitiveType::LineStrip)
#endif // USE_SFML_PRE_2_4
	, m_interpolationSteps(0u)
	, m_handlesVertices()
	, m_showHandles(false)
	, m_useBezier(false)
	, m_lockHandleMirror(true)
	, m_lockHandleAngle(true)
{
}

Spline::Spline(std::initializer_list<sf::Vector2f> list)
	: Spline(static_cast<unsigned int>(list.size()))
{
	unsigned int index{ 0u };
	for (auto& position : list)
		m_vertices[index++].position = position;
}

float Spline::getLength() const
{
	if (m_vertices.size() < 2)
		return 0.f;

	float total{ 0.f };
	for (unsigned int v{ 0 }; v < getLastVertexIndex(); ++v)
		total += vectorLength(m_vertices[v + 1].position - m_vertices[v].position);
	return total;
}

float Spline::getInterpolatedLength() const
{
	if (m_sfmlVertices.size() < 2)
		return 0.f;

	float total{ 0.f };
	for (unsigned int v{ 0 }; v < m_sfmlVertices.size() - 1; ++v)
		total += vectorLength(m_sfmlVertices[v + 1].position - m_sfmlVertices[v].position);
	return total;
}

void Spline::update()
{
	if (m_vertices.size() < 2)
	{
		m_sfmlVertices.clear();
		m_sfmlThickVertices.clear();
		m_handlesVertices.clear();
		m_sfmlVerticesUnitTangents.clear();
		return;
	}

	const unsigned int pointsPerVertex{ priv_getNumberOfPointsPerVertex() };
	if (m_isClosed)
		m_sfmlVertices.resize((m_vertices.size() * pointsPerVertex) + 1);
	else
		m_sfmlVertices.resize(((m_vertices.size() - 1) * pointsPerVertex) + 1);
	m_sfmlVerticesUnitTangents.resize(m_sfmlVertices.size());

	m_handlesVertices.resize((m_vertices.size()) * 4);
	std::vector<sf::Vertex>::iterator itHandle{ m_handlesVertices.begin() };

	for (std::vector<Vertex>::iterator begin{ m_vertices.begin() }, end{ m_vertices.end() }, it{ begin }; it != end; ++it)
	{
		itHandle->color = sf::Color(255, 255, 128, 32);
		itHandle++->position = it->position;
		itHandle->color = sf::Color(0, 255, 0, 128);
		itHandle++->position = it->position + it->backHandle;
		itHandle->color = sf::Color(255, 255, 128, 32);
		itHandle++->position = it->position;
		itHandle->color = sf::Color(0, 255, 0, 128);
		itHandle++->position = it->position + it->frontHandle;

		std::vector<sf::Vertex>::iterator itSfml{ m_sfmlVertices.begin() + (it - begin) * pointsPerVertex };
		if (m_isClosed || it != end - 1)
		{
			for (unsigned int i{ 0u }; i < pointsPerVertex; ++i)
			{
				std::vector<Vertex>::iterator nextIt{ m_vertices.begin() };
				if (it != end - 1)
					nextIt = it + 1;
				if (m_useBezier)
					itSfml->position = bezierInterpolation(it->position, nextIt->position, it->frontHandle, nextIt->backHandle, static_cast<float>(i) / pointsPerVertex);
				else
					itSfml->position = linearInterpolation(it->position, nextIt->position, static_cast<float>(i) / pointsPerVertex);
				itSfml++->color = m_color;
			}
		}
		else
		{
			itSfml->position = it->position;
			itSfml->color = m_color;
		}
	}
	if (m_isClosed)
	{
		std::vector<sf::Vertex>::iterator itSfml{ m_sfmlVertices.begin() + m_vertices.size() * pointsPerVertex };
		itSfml->position = m_vertices.front().position;
		itSfml->color = m_color;
	}

	// calculate tangents
	for (std::vector<sf::Vertex>::iterator begin{ m_sfmlVertices.begin() }, end{ m_sfmlVertices.end() }, current{ begin }; current != end; ++current)
	{
		std::vector<sf::Vertex>::iterator next{ current };
		std::vector<sf::Vertex>::iterator previous{ current };
		if (current != begin)
			previous = current - 1;
		else if (m_isClosed)
			previous = end - 2;
		if (current != end - 1)
			next = current + 1;
		else if (m_isClosed)
			next = begin + 1;

		const sf::Vector2f nextVectorUnit{ vectorUnit(next->position - current->position) };
		const sf::Vector2f previousVectorUnit{ vectorUnit(current->position - previous->position) };

		sf::Vector2f& tangent{ *(m_sfmlVerticesUnitTangents.begin() + (current - begin)) };
		tangent = vectorUnit(previousVectorUnit + nextVectorUnit);
	}

	if (!priv_isThick())
		m_sfmlThickVertices.clear();
	else
	{
		m_sfmlThickVertices.resize(m_sfmlVertices.size() * 2); // required number of vertices for a triangle strip (two triangles between each pair of points)

		std::vector<sf::Vertex>::iterator itThick{ m_sfmlThickVertices.begin() };
		for (std::vector<sf::Vertex>::iterator begin{ m_sfmlVertices.begin() }, end{ m_sfmlVertices.end() }, it{ begin }; it != end; ++it)
		{
			const unsigned int sfmlIndex{ static_cast<unsigned int>(it - begin) };
			const unsigned int index{ sfmlIndex / priv_getNumberOfPointsPerVertex() };
			const unsigned int interpolatedIndex{ index * priv_getNumberOfPointsPerVertex() };
			const float vertexRatio{ static_cast<float>(sfmlIndex - interpolatedIndex) / priv_getNumberOfPointsPerVertex() };
			const std::vector<Vertex>::iterator currentVertex{ m_vertices.begin() + (index % m_vertices.size()) };
			std::vector<Vertex>::iterator nextVertex{ currentVertex };
			if (currentVertex != m_vertices.end() - 1)
				nextVertex = currentVertex + 1;
			else if (m_isClosed)
				nextVertex = m_vertices.begin();
			const float thickness{ m_thickness * linearInterpolation(currentVertex->thickness, nextVertex->thickness, vertexRatio) };
			const float halfWidth{ thickness / 2.f };

			sf::Color color{ m_color * linearInterpolation(currentVertex->color, nextVertex->color, vertexRatio) };

			sf::Vector2f tangentUnit{ *(m_sfmlVerticesUnitTangents.begin() + (it - begin)) };
			if (m_isClosed || it != end - 1)
			{
				const sf::Vector2f forwardLine{ ((it != end - 1) ? (it + 1)->position - it->position : (begin + 1)->position - begin->position) };
				const sf::Vector2f forwardUnit{ forwardLine / vectorLength(forwardLine) };
				float dotUnits{ dot(forwardUnit , tangentUnit) };
				tangentUnit = tangentUnit / ((abs(dotUnits) > zeroEpsilon) ? dotUnits : zeroEpsilon);
			}
			const sf::Vector2f scaledNormal{ vectorNormal(tangentUnit) * halfWidth };
			itThick->color = color;
			itThick++->position = it->position + scaledNormal;
			itThick->color = color;
			itThick++->position = it->position - scaledNormal;
		}
	}
}

void Spline::setClosed(const bool isClosed)
{
	m_isClosed = isClosed;
}

void Spline::reserveVertices(const unsigned int numberOfVertices)
{
	if (numberOfVertices == 0)
		return;

	m_vertices.reserve(numberOfVertices);
	m_sfmlVertices.reserve(numberOfVertices * priv_getNumberOfPointsPerVertex() + 1);
	m_sfmlThickVertices.reserve(m_sfmlVertices.size() * 2);
	m_handlesVertices.reserve(numberOfVertices * 4);
}

void Spline::addVertices(const std::vector<sf::Vector2f>& positions)
{
	for (auto& position : positions)
		addVertex(position);
}

void Spline::addVertices(const unsigned int index, const std::vector<sf::Vector2f>& positions)
{
	for (auto& position : positions)
		addVertex(index, position);
}

void Spline::addVertices(const unsigned int numberOfVertices, const sf::Vector2f position)
{
	for (unsigned int i{ 0u }; i < numberOfVertices; ++i)
		addVertex(position);
}

void Spline::addVertices(const unsigned int numberOfVertices, const unsigned int index, const sf::Vector2f position)
{
	for (unsigned int i{ 0u }; i < numberOfVertices; ++i)
		addVertex(index + i, position);
}

void Spline::addVertex(const sf::Vector2f position)
{
	m_vertices.emplace_back(Vertex(position));
}

void Spline::addVertex(const unsigned int index, const sf::Vector2f position)
{
	if (index < getVertexCount())
		m_vertices.insert(m_vertices.begin() + index, Vertex(position));
	else
		addVertex(position);
}

void Spline::removeVertex(const unsigned int index)
{
	if (!priv_testVertexIndex(index, "Cannot remove vertex."))
		return;

	m_vertices.erase(m_vertices.begin() + index);
}

void Spline::removeVertices(const unsigned int index, const unsigned int numberOfVertices)
{
	if (!priv_testVertexIndex(index, "Cannot remove vertices") || ((numberOfVertices > 1) && (!priv_testVertexIndex(index + numberOfVertices - 1, "Cannot remove vertices"))))
		return;

	if (numberOfVertices == 0)
		m_vertices.erase(m_vertices.begin() + index, m_vertices.end());
	else
		m_vertices.erase(m_vertices.begin() + index, m_vertices.begin() + index + numberOfVertices);
}

void Spline::reverseVertices()
{
	std::reverse(m_vertices.begin(), m_vertices.end());

	// swap all handles
	sf::Vector2f tempHandle;
	for (auto& vertex : m_vertices)
	{
		tempHandle = vertex.frontHandle;
		vertex.frontHandle = vertex.backHandle;
		vertex.backHandle = tempHandle;
	}
}

void Spline::setPosition(const unsigned int index, const sf::Vector2f position)
{
	if (!priv_testVertexIndex(index, "Cannot set vertex position."))
		return;

	m_vertices[index].position = position;
}

void Spline::setPositions(const unsigned int index, unsigned int numberOfVertices, const sf::Vector2f position)
{
	if (!priv_testVertexIndex(index, "Cannot set vertices' positions") || (numberOfVertices > 1 && !priv_testVertexIndex(index + numberOfVertices - 1, "Cannot set vertices' positions")))
		return;

	if (numberOfVertices == 0)
		numberOfVertices = static_cast<unsigned int>(m_vertices.size()) - index;

	for (unsigned int v{ 0u }; v < numberOfVertices; ++v)
		m_vertices[index + v].position = position;
}

void Spline::setPositions(const std::vector<sf::Vector2f>& positions, unsigned int index)
{
	const unsigned int numberOfVertices{ static_cast<unsigned int>(positions.size()) };
	if ((numberOfVertices < 1) || (!priv_testVertexIndex(index, "Cannot set vertices' positions")) || ((numberOfVertices > 1) && (!priv_testVertexIndex(index + numberOfVertices - 1, "Cannot set vertices' positions"))))
		return;

	for (auto& position : positions)
	{
		m_vertices[index].position = position;
		++index;
	}
}

sf::Vector2f Spline::getPosition(const unsigned int index) const
{
	if (!priv_testVertexIndex(index, "Cannot get vertex position."))
		return{ 0.f, 0.f };

	return m_vertices[index].position;
}

void Spline::setFrontHandle(const unsigned int index, const sf::Vector2f offset)
{
	if (!priv_testVertexIndex(index, "Cannot set vertex front handle."))
		return;

	m_vertices[index].frontHandle = offset;
	if (m_lockHandleMirror)
		m_vertices[index].backHandle = -offset;
	else if (m_lockHandleAngle)
		copyAngle(m_vertices[index].frontHandle, m_vertices[index].backHandle);
}

sf::Vector2f Spline::getFrontHandle(const unsigned int index) const
{
	if (!priv_testVertexIndex(index, "Cannot get vertex front handle."))
		return{ 0.f, 0.f };

	return m_vertices[index].frontHandle;
}

void Spline::setBackHandle(const unsigned int index, const sf::Vector2f offset)
{
	if (!priv_testVertexIndex(index, "Cannot set vertex back handle."))
		return;

	m_vertices[index].backHandle = offset;
	if (m_lockHandleMirror)
		m_vertices[index].frontHandle = -offset;
	else if (m_lockHandleAngle)
		copyAngle(m_vertices[index].backHandle, m_vertices[index].frontHandle);
}

sf::Vector2f Spline::getBackHandle(const unsigned int index) const
{
	if (!priv_testVertexIndex(index, "Cannot get vertex back handle."))
		return{ 0.f, 0.f };

	return m_vertices[index].backHandle;
}

void Spline::resetHandles(const unsigned int index, unsigned int numberOfVertices)
{
	if ((!priv_testVertexIndex(index, "Cannot reset vertices' handles")) || ((numberOfVertices > 1) && (!priv_testVertexIndex(index + numberOfVertices - 1, "Cannot reset vertices' handles"))))
		return;

	if (numberOfVertices == 0)
		numberOfVertices = static_cast<unsigned int>(m_vertices.size()) - index;

	for (unsigned int v{ 0u }; v < numberOfVertices; ++v)
	{
		m_vertices[index + v].frontHandle = { 0.f, 0.f };
		m_vertices[index + v].backHandle = { 0.f, 0.f };
	}
}

void Spline::smoothHandles()
{
	for (unsigned int v{ 0 }; v < m_vertices.size() - 1; ++v)
	{
		const sf::Vector2f p1{ m_vertices[v].position };
		const sf::Vector2f p2{ m_vertices[v + 1].position };
		sf::Vector2f p0{ p1 };
		sf::Vector2f p3{ p2 };
		if (v > 0)
			p0 = m_vertices[v - 1].position;
		if (v < m_vertices.size() - 2)
			p3 = m_vertices[v + 2].position;

		const sf::Vector2f m0{ linearInterpolation(p0, p1, 0.5f) };
		const sf::Vector2f m1{ linearInterpolation(p1, p2, 0.5f) };
		const sf::Vector2f m2{ linearInterpolation(p2, p3, 0.5f) };

		const float p01{ vectorLength(p1 - p0) };
		const float p12{ vectorLength(p2 - p1) };
		const float p23{ vectorLength(p3 - p2) };
		float proportion0{ 0.f };
		float proportion1{ 0.f };
		if (p01 + p12 != 0.f)
			proportion0 = p01 / (p01 + p12);
		if (p12 + p23 != 0.f)
			proportion1 = p12 / (p12 + p23);

		const sf::Vector2f q0{ linearInterpolation(m0, m1, proportion0) };
		const sf::Vector2f q1{ linearInterpolation(m1, m2, proportion1) };

		m_vertices[v].frontHandle = m1 - q0;
		m_vertices[v + 1].backHandle = m1 - q1;
	}
	m_vertices.front().backHandle = { 0.f, 0.f };
	m_vertices.back().frontHandle = { 0.f, 0.f };
}

void Spline::setHandlesVisible(const bool handlesVisible)
{
	m_showHandles = handlesVisible;
}

void Spline::setColor(const sf::Color color)
{
	m_color = color;
}

void Spline::setColor(const unsigned int index, const sf::Color color)
{
	m_vertices[index].color = color;
}

void Spline::setInterpolationSteps(const unsigned int interpolationSteps)
{
	m_interpolationSteps = interpolationSteps;
}

void Spline::setHandleAngleLock(const bool handleAngleLock)
{
	m_lockHandleAngle = handleAngleLock;
}

void Spline::setHandleMirrorLock(const bool handleMirrorLock)
{
	m_lockHandleMirror = handleMirrorLock;
}

void Spline::setBezierInterpolation(const bool bezierInterpolation)
{
	m_useBezier = bezierInterpolation;
}

void Spline::setPrimitiveType(const sf::PrimitiveType primitiveType)
{
	m_primitiveType = primitiveType;
}

sf::Vector2f Spline::getInterpolatedPosition(const unsigned int interpolationOffset, const unsigned int index) const
{
	return m_sfmlVertices[priv_getInterpolatedIndex(interpolationOffset, index)].position;
}

unsigned int Spline::getInterpolatedPositionCount() const
{
	return ((m_isClosed) ? (static_cast<unsigned int>(m_vertices.size()) * priv_getNumberOfPointsPerVertex()) : ((static_cast<unsigned int>(m_vertices.size()) - 1u) * priv_getNumberOfPointsPerVertex() + 1u));
}

sf::Vector2f Spline::getInterpolatedPositionTangent(const unsigned int interpolationOffset, const unsigned int index) const
{
	return m_sfmlVerticesUnitTangents[priv_getInterpolatedIndex(interpolationOffset, index)];
}

sf::Vector2f Spline::getInterpolatedPositionNormal(const unsigned int interpolationOffset, const unsigned int index) const
{
	return vectorNormal(getInterpolatedPositionTangent(interpolationOffset, index));
}

float Spline::getInterpolatedPositionThickness(const unsigned int interpolationOffset, const unsigned int index) const
{
	if (!priv_isThick())
		return 0.f;

	const unsigned int interpolatedIndex{ priv_getInterpolatedIndex(interpolationOffset, index) };

	const unsigned int vertexIndex{ interpolatedIndex / priv_getNumberOfPointsPerVertex() };
	const unsigned int interpolatedVertexIndex{ vertexIndex * priv_getNumberOfPointsPerVertex() };
	const float vertexRatio{ static_cast<float>(interpolatedIndex - interpolatedVertexIndex) / priv_getNumberOfPointsPerVertex() };
	const std::vector<Vertex>::const_iterator currentVertex{ m_vertices.begin() + (vertexIndex % m_vertices.size()) };
	std::vector<Vertex>::const_iterator nextVertex{ currentVertex };
	if (currentVertex != m_vertices.end() - 1)
		nextVertex = currentVertex + 1;
	else if (m_isClosed)
		nextVertex = m_vertices.begin();

	return m_thickness * linearInterpolation(currentVertex->thickness, nextVertex->thickness, vertexRatio);
}

float Spline::getInterpolatedPositionThicknessCorrectionScale(const unsigned int interpolationOffset, const unsigned int index) const
{
	if (!priv_isThick())
		return 0.f;

	const unsigned int interpolatedIndex{ priv_getInterpolatedIndex(interpolationOffset, index) };

	const sf::Vector2f sideOffset{ m_sfmlThickVertices[interpolatedIndex * 2].position - m_sfmlVertices[interpolatedIndex].position };
	const float sideOffsetLength{ vectorLength(sideOffset) };

	return sideOffsetLength * 2 / getInterpolatedPositionThickness(interpolationOffset, index);
}



// PRIVATE

void Spline::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.texture = nullptr;
	if (priv_isThick())
	{
		if (m_sfmlThickVertices.size() > 0)
			target.draw(&m_sfmlThickVertices.front(), m_sfmlThickVertices.size(), thickPrimitiveType, states);
	}
	else if (m_sfmlVertices.size() > 0)
		target.draw(&m_sfmlVertices.front(), m_sfmlVertices.size(), m_primitiveType, states);
	if (m_showHandles && m_handlesVertices.size() > 1)
		target.draw(&m_handlesVertices.front(), m_handlesVertices.size(), sf::PrimitiveType::Lines, states);
}

bool Spline::priv_isValidVertexIndex(const unsigned int vertexIndex) const
{
	return vertexIndex < m_vertices.size();
}

bool Spline::priv_testVertexIndex(const unsigned int vertexIndex, const std::string& exceptionMessage) const
{
	if (!priv_isValidVertexIndex(vertexIndex))
	{
		if (m_throwExceptions)
			throw Exception(exceptionPrefix + exceptionMessage + " Vertex index (" + std::to_string(vertexIndex) + ") out of range");
		return false;
	}
	return true;
}

bool Spline::priv_isThick() const
{
	return (m_thickness >= thicknessEpsilon || m_thickness <= -thicknessEpsilon);
}

unsigned int Spline::priv_getNumberOfPointsPerVertex() const
{
	return m_interpolationSteps + 1u;
}

// index is control vertex index and interpolation offset is index of interpolated vertices from that control vertex
// index should be: [0, numberOfVertices)
// interpolation should be (when used with index): [0, numberOfInterpolationSteps] or [0, numberOfInterpolatedPositionsPerVertex)
// (note that higher value of range is 'not' inclusive)
// interpolation can, however, extend beyond the range into the follow vertex/ices so index can be omitted (it defaults to zero).
// index and interpolationOffset values must be valid
unsigned int Spline::priv_getInterpolatedIndex(const unsigned int interpolationOffset, const unsigned int index) const
{
	const unsigned int indexOffset{ index * priv_getNumberOfPointsPerVertex() };
	const unsigned int interpolatedPositionIndex{ indexOffset + interpolationOffset };
	assert(interpolatedPositionIndex < getInterpolatedPositionCount()); // index must be in valid range
	return interpolatedPositionIndex;
}

} // namespace selbaward
