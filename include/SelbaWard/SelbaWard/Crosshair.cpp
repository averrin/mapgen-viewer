//////////////////////////////////////////////////////////////////////////////
//
// Selba Ward (https://github.com/Hapaxia/SelbaWard)
// --
//
// Crosshair
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

#include "Crosshair.hpp"

#include <SFML/Window/Mouse.hpp>

namespace
{

bool updateVertices(sf::VertexArray& vertices, const sf::RenderWindow* const window, const sf::Color& horizontalColor, const sf::Color& verticalColor)
{
	if (window == nullptr)
		return false;
	const sf::View view{ window->getView() };
	vertices[0].position.x = view.getCenter().x - view.getSize().x / 2.f;
	vertices[1].position.x = vertices[0].position.x + view.getSize().x;
	vertices[2].position.y = view.getCenter().y - view.getSize().y / 2.f;
	vertices[3].position.y = vertices[2].position.y + view.getSize().y;
	const sf::Vector2f target{ window->mapPixelToCoords(sf::Mouse::getPosition(*window), view) };
	if ((target.x < vertices[0].position.x) ||
		(target.x > vertices[1].position.x) ||
		(target.y < vertices[2].position.y) ||
		(target.y > vertices[3].position.y))
		return false;
	vertices[0].position.y = target.y + 0.5f;
	vertices[1].position.y = target.y + 0.5f;
	vertices[2].position.x = target.x + 0.5f;
	vertices[3].position.x = target.x + 0.5f;
	vertices[0].color = horizontalColor;
	vertices[1].color = horizontalColor;
	vertices[2].color = verticalColor;
	vertices[3].color = verticalColor;
	return true;
}

} // namespace

namespace selbaward
{

Crosshair::Crosshair(const sf::RenderWindow& window)
	: Crosshair()
{
	m_window = &window;
}

Crosshair::Crosshair()
	: Crosshair(sf::Color::White)
{
}

Crosshair::Crosshair(const sf::Color& color, const sf::RenderWindow& window)
	: Crosshair(color, color, window)
{
}

Crosshair::Crosshair(const sf::Color& color)
	: Crosshair(color, color)
{
}

Crosshair::Crosshair(const sf::Color& horizontalColor, const sf::Color& verticalColor, const sf::RenderWindow& window)
	: Crosshair(horizontalColor, verticalColor)
{
	m_window = &window;
}

Crosshair::Crosshair(const sf::Color& horizontalColor, const sf::Color& verticalColor)
	: m_vertices(sf::PrimitiveType::Lines, 4)
	, m_horizontalColor(horizontalColor)
	, m_verticalColor(verticalColor)
	, m_window(nullptr)
{
}

void Crosshair::setWindow(const sf::RenderWindow& window)
{
	m_window = &window;
}

void Crosshair::setWindow()
{
	m_window = nullptr;
}

void Crosshair::setColor(const sf::Color& color)
{
	m_horizontalColor = color;
	m_verticalColor = color;
}

void Crosshair::setHorizontalColor(const sf::Color& horizontalColor)
{
	m_horizontalColor = horizontalColor;
}

void Crosshair::setVerticalColor(const sf::Color& verticalColor)
{
	m_verticalColor = verticalColor;
}

sf::Vector2f Crosshair::getPosition() const
{
	return{ m_vertices[2].position.x - 0.5f, m_vertices[0].position.y - 0.5f };
}



// PRIVATE

void Crosshair::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (updateVertices(m_vertices, m_window, m_horizontalColor, m_verticalColor))
		target.draw(m_vertices, states);
}

} // namespace selbaward
