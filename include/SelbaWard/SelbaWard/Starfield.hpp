//////////////////////////////////////////////////////////////////////////////
//
// Selba Ward (https://github.com/Hapaxia/SelbaWard)
// --
//
// Starfield
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

#ifndef SELBAWARD_STARFIELD_HPP
#define SELBAWARD_STARFIELD_HPP

#include "Common.hpp"

namespace selbaward
{

// SW Starfield v1.1.0
class Starfield : public sf::Drawable, public sf::Transformable
{
public:
	Starfield(sf::Vector2f size = { 0.f, 0.f }, unsigned int numberOfStars = 100u, sf::Color color = sf::Color(160, 160, 160));
	void regenerate();
	void regenerate(sf::Vector2f size);
	void regenerate(sf::Vector2f size, unsigned int numberOfStars);
	void regenerate(unsigned int numberOfStars);

	void setColor(sf::Color color);

	void move(sf::Vector2f movement);

private:
	sf::PrimitiveType m_primitiveType;
	std::vector<sf::Vertex> m_vertices;
	sf::Vector2f m_size;
	sf::Color m_color;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

} // namespace selbaward
#endif // SELBAWARD_STARFIELD_HPP
