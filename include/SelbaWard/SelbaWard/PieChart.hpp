//////////////////////////////////////////////////////////////////////////////
//
// Selba Ward (https://github.com/Hapaxia/SelbaWard)
// --
//
// Pie Chart
//
// Copyright(c) 2015-2017 M.J.Silk
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

#ifndef SELBAWARD_PIECHART_HPP
#define SELBAWARD_PIECHART_HPP

#include "Common.hpp"

namespace selbaward
{

// SW Pie Chart v1.0.1
class PieChart : public sf::Drawable, public sf::Transformable
{
public:
	struct Slice
	{
		float size;
		float scale;
		float explode;
		sf::Color color;
		Slice();
	};
	std::vector<Slice> slices;

	PieChart();
	void update();
	void setSize(sf::Vector2f size);
	sf::Vector2f getSize() const;
	void setRadius(float radius);
	void setDiameter(float diameter);

	sf::FloatRect getLocalBounds() const;
	sf::FloatRect getGlobalBounds() const;

private:
	sf::PrimitiveType m_primitive;
	std::vector<sf::Vertex> m_vertices;
	sf::Vector2f m_size;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

inline void PieChart::setSize(sf::Vector2f size)
{
	m_size = size;
}

inline sf::Vector2f PieChart::getSize() const
{
	return m_size;
}

inline void PieChart::setRadius(float radius)
{
	setDiameter(radius * 2.f);
}

inline void PieChart::setDiameter(float diameter)
{
	m_size = { diameter, diameter };
}

} // namespace selbaward
#endif // SELBAWARD_PIECHART_HPP
