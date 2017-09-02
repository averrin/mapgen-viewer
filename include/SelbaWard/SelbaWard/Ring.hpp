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

#ifndef SELBAWARD_RING_HPP
#define SELBAWARD_RING_HPP

#include "Common.hpp"

namespace selbaward
{

// SW Ring v1.1.2
class Ring : public sf::Drawable, public sf::Transformable
{
public:
	Ring(float radius = 0.f, float hole = 0.5f, unsigned int numberOfSides = 36u);
	void setRadius(float radius);
	float getRadius() const;
	void setHole(float hole);
	float getHole() const;
	void setNumberOfSides(unsigned int numberOfSides);
	unsigned int getNumberOfSides() const;
	void setColor(sf::Color color);
	sf::Color getColor() const;
	void setTexture(const sf::Texture& texture, bool resetRect = false);
	void setTexture();
	void setTextureRect(const sf::IntRect& textureRect);
	sf::FloatRect getLocalBounds() const;
	sf::FloatRect getGlobalBounds() const;
	float getArea() const;
	void setSectorSize(float sectorSize);
	float getSectorSize() const;
	void setSectorOffset(float sectorOffset);
	float getSectorOffset() const;

private:
	float m_radius;
	float m_hole;
	float m_sectorSize;
	float m_sectorOffset;
	unsigned int m_numberOfSides;
	sf::Color m_color;
	sf::PrimitiveType m_primitiveType;
	std::vector<sf::Vertex> m_vertices;
	const sf::Texture* m_texture;
	sf::IntRect m_textureRect;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	void priv_updateVertices();
};

} // namespace selbaward
#endif // SELBAWARD_RING_HPP
