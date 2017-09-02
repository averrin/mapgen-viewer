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

#ifndef SELBAWARD_CROSSHAIR_HPP
#define SELBAWARD_CROSSHAIR_HPP

#include "Common.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace selbaward
{

// SW Crosshair v1.0.0
class Crosshair : public sf::Drawable
{
public:
	Crosshair(const sf::RenderWindow& window);
	Crosshair();
	Crosshair(const sf::Color& color, const sf::RenderWindow& window);
	Crosshair(const sf::Color& color);
	Crosshair(const sf::Color& horizontalColor, const sf::Color& verticalColor, const sf::RenderWindow& window);
	Crosshair(const sf::Color& horizontalColor, const sf::Color& verticalColor);
	void setWindow(const sf::RenderWindow& window);
	void setWindow();
	void setColor(const sf::Color& color = sf::Color::White);
	void setHorizontalColor(const sf::Color& horizontalColor = sf::Color::White);
	void setVerticalColor(const sf::Color& verticalColor = sf::Color::White);
	sf::Vector2f getPosition() const;

private:
	const sf::RenderWindow* m_window;
	sf::Color m_horizontalColor;
	sf::Color m_verticalColor;
	mutable sf::VertexArray m_vertices;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

} // namespace selbaward
#endif // SELBAWARD_CROSSHAIR_HPP
