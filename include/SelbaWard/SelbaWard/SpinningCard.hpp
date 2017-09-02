//////////////////////////////////////////////////////////////////////////////
//
// Selba Ward (https://github.com/Hapaxia/SelbaWard)
// --
//
// Spinning Card
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

// faux-3D card spin effect

// selbaward::SpinningCard
//  or
// sw::SpinningCard (if namespace shortcut is created i.e. SELBAWARD_NO_NAMESPACE_SHORTCUT is not defined)

// by Hapax (http://github.com/hapaxia)

// Requires:
//   C++11
//   SFML (http://sfml-dev.org)

#ifndef SELBAWARD_SPINNINGCARD_HPP
#define SELBAWARD_SPINNINGCARD_HPP

#include "Common.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/VertexArray.hpp>

namespace selbaward
{

// SpinningCard (v1.2.1)
class SpinningCard : public sf::Drawable, public sf::Transformable
{
public:
	// pass the sprite that it mimics when constructing the card
	SpinningCard(const sf::Sprite& sprite);

	// rotate around the y axis (in degrees)
	void spin(float angleInDegrees);

	// rotate around the y axis (in radians)
	void spinRadians(float angleInRadians);

	// rotate around the x axis (in degrees)
	void spinVertically(float angleInDegrees);

	// rotate around the x axis (in radians)
	void spinVerticallyRadians(float angleInRadians);

	// set depth of fake perspective
	void setDepth(float depth);

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	sf::VertexArray m_vertices;
	const sf::Texture* m_pTexture;
	sf::FloatRect m_initial;
	const float m_pi;
	float m_depth;
};

} // namespace selbaward

#endif // SELBAWARD_SPINNINGCARD_HPP
