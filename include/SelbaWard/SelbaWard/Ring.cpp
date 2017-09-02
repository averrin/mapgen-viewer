//////////////////////////////////////////////////////////////////////////////
//
// Selba Ward (https://github.com/Hapaxia/SelbaWard)
// --
//
// Ring
//
// Copyright(c) 2016-2017 M.J.Silk
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

#include "Ring.hpp"

#include <SFML/Graphics/Texture.hpp>
#include <cmath>

namespace
{

const float pi{ 3.14159265358979f };

} // namespace

namespace selbaward
{

Ring::Ring(const float radius, const float hole, const unsigned int numberOfSides)
	: m_radius(radius)
	, m_hole(hole)
	, m_sectorSize(1.f)
	, m_sectorOffset(0.f)
	, m_numberOfSides(numberOfSides)
	, m_color(sf::Color::White)
#ifdef USE_SFML_PRE_2_4
	, m_primitiveType(sf::PrimitiveType::TrianglesStrip)
#else // USE_SFML_PRE_2_4
	, m_primitiveType(sf::PrimitiveType::TriangleStrip)
#endif // USE_SFML_PRE_2_4
	, m_vertices((m_numberOfSides + 1) * 2)
	, m_texture(nullptr)
	, m_textureRect()
{
	priv_updateVertices();
}

void Ring::setRadius(const float radius)
{
	m_radius = radius;
	priv_updateVertices();
}

float Ring::getRadius() const
{
	return m_radius;
}

void Ring::setHole(const float hole)
{
	m_hole = hole;
	priv_updateVertices();
}

float Ring::getHole() const
{
	return m_hole;
}

void Ring::setNumberOfSides(const unsigned int numberOfSides)
{
	m_numberOfSides = numberOfSides;
	priv_updateVertices();
}

unsigned int Ring::getNumberOfSides() const
{
	return m_numberOfSides;
}

void Ring::setColor(const sf::Color color)
{
	m_color = color;
	for (auto& vertex : m_vertices)
		vertex.color = color;
}

sf::Color Ring::getColor() const
{
	return m_color;
}

void Ring::setTexture(const sf::Texture& texture, const bool resetRect)
{
	const bool textureWasNull{ (m_texture == nullptr) };
	m_texture = &texture;
	if (textureWasNull || resetRect)
		setTextureRect({ { 0, 0 }, sf::Vector2i(texture.getSize()) });
}

void Ring::setTexture()
{
	m_texture = nullptr;
}

void Ring::setTextureRect(const sf::IntRect& textureRect)
{
	m_textureRect = textureRect;
	priv_updateVertices();
}

sf::FloatRect Ring::getLocalBounds() const
{
	if (m_vertices.size() == 0)
		return{ { 0.f, 0.f }, { 0.f, 0.f } };

	sf::Vector2f topLeft{ m_vertices[0].position };
	sf::Vector2f bottomRight{ topLeft };
	for (auto& vertex : m_vertices)
	{
		if (vertex.position.x < topLeft.x)
			topLeft.x = vertex.position.x;
		else if (vertex.position.x > bottomRight.x)
			bottomRight.x = vertex.position.x;
		if (vertex.position.y < topLeft.y)
			topLeft.y = vertex.position.y;
		else if (vertex.position.y > bottomRight.y)
			bottomRight.y = vertex.position.y;
	}
	return{ topLeft, bottomRight - topLeft };
}

sf::FloatRect Ring::getGlobalBounds() const
{
	if (m_vertices.size() == 0)
		return{ { 0.f, 0.f }, { 0.f, 0.f } };

	const sf::Transform transform{ getTransform() };
	sf::Vector2f topLeft{ transform.transformPoint(m_vertices[0].position) };
	sf::Vector2f bottomRight{ topLeft };
	sf::Vector2f current;
	for (auto& vertex : m_vertices)
	{
		current = transform.transformPoint(vertex.position);
		if (current.x < topLeft.x)
			topLeft.x = current.x;
		else if (current.x > bottomRight.x)
			bottomRight.x = current.x;
		if (current.y < topLeft.y)
			topLeft.y = current.y;
		else if (current.y > bottomRight.y)
			bottomRight.y = current.y;
	}
	return{ topLeft, bottomRight - topLeft };
}

float Ring::getArea() const
{
	const float holeRadius{ m_radius * m_hole };
	return (m_radius * m_radius - holeRadius * holeRadius) * m_sectorSize * pi;
}

void Ring::setSectorSize(const float sectorSize)
{
	m_sectorSize = sectorSize;
	priv_updateVertices();
}

float Ring::getSectorSize() const
{
	return m_sectorSize;
}

void Ring::setSectorOffset(const float sectorOffset)
{
	m_sectorOffset = sectorOffset;
	priv_updateVertices();
}

float Ring::getSectorOffset() const
{
	return m_sectorOffset;
}



// PRIVATE

void Ring::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.texture = m_texture;
	states.transform *= getTransform();

	if (m_vertices.size() > 0)
		target.draw(&m_vertices.front(), m_vertices.size(), m_primitiveType, states);
}

void Ring::priv_updateVertices()
{
	const float sectorOffset{ 2.f * pi * m_sectorOffset };
	const float sectorSize{ 2.f * pi * m_sectorSize };
	m_vertices.resize((m_numberOfSides + 1) * 2);
	for (std::vector<sf::Vertex>::iterator begin{ m_vertices.begin() }, end{ m_vertices.end() }, it{ begin }; it != end; ++it)
	{
		const unsigned int index{ static_cast<unsigned int>(it - begin) };
		const unsigned int point{ index / 2u };
		const float angle{ sectorOffset + sectorSize * point / m_numberOfSides };
		const bool isInnerPoint{ (index % 2 != 0) };
		it->position = { m_radius + std::sin(angle) * m_radius * (isInnerPoint ? m_hole : 1.f), m_radius - std::cos(angle) * m_radius * (isInnerPoint ? m_hole : 1.f), };
		it->color = m_color;
		const sf::Vector2f scaledPosition{ it->position / (m_radius * 2.f) };
		it->texCoords = { scaledPosition.x * m_textureRect.width + m_textureRect.left, scaledPosition.y * m_textureRect.height + m_textureRect.top };;
	}
}

} // namespace selbaward
