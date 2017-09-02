//////////////////////////////////////////////////////////////////////////////
//
// Selba Ward (https://github.com/Hapaxia/SelbaWard)
// --
//
// SpinningCard
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

#include "SpinningCard.hpp"

namespace selbaward
{

SpinningCard::SpinningCard(const sf::Sprite& sprite) :
m_pi(3.14159f),
m_depth(0.2f),
#ifdef USE_SFML_PRE_2_4
m_vertices(sf::PrimitiveType::TrianglesFan, 6)
#else // USE_SFML_PRE_2_4
m_vertices(sf::PrimitiveType::TriangleFan, 6)
#endif // USE_SFML_PRE_2_4
{
	// copy sprite's origin, position, rotation and scale in an attempt to look like the sprite
	setOrigin(sprite.getOrigin());
	setPosition(sprite.getPosition());
	setRotation(sprite.getRotation());
	setScale(sprite.getScale());

	// copy sprite's vertices and texture

	// save initial state/starting point
	m_initial = sprite.getLocalBounds();

	// start off by setting all vertices to top-left...
	for (unsigned int i{ 0 }; i < 5; ++i)
		m_vertices[i].position = sprite.getPosition() - sprite.getOrigin(); // top-left

	// ...then, add offsets to individual vertices
	m_vertices[0].position += sf::Vector2f(static_cast<float>(m_initial.width) / 2, static_cast<float>(m_initial.height) / 2); // centre
	m_vertices[2].position += sf::Vector2f(m_initial.width, 0.f); // top-right
	m_vertices[3].position += sf::Vector2f(m_initial.width, m_initial.height); // bottom-right
	m_vertices[4].position += sf::Vector2f(0.f, m_initial.height); // bottom-left

	// set texture coordinates to the same as the sprite's (using the same method for texCoords as position above)
	for (unsigned int i{ 0 }; i < 5; ++i)
		m_vertices[i].texCoords = sf::Vector2f(static_cast<float>(sprite.getTextureRect().left), static_cast<float>(sprite.getTextureRect().top)); // top-left
	m_vertices[0].texCoords += sf::Vector2f(static_cast<float>(sprite.getTextureRect().width) / 2, static_cast<float>(sprite.getTextureRect().height) / 2); // centre
	m_vertices[2].texCoords += sf::Vector2f(static_cast<float>(sprite.getTextureRect().width), 0.f); // top-right
	m_vertices[3].texCoords += sf::Vector2f(static_cast<float>(sprite.getTextureRect().width), static_cast<float>(sprite.getTextureRect().height)); // bottom-right
	m_vertices[4].texCoords += sf::Vector2f(0.f, static_cast<float>(sprite.getTextureRect().height)); // bottom-left

	// set colours of vertices to match sprite's colour
	for (unsigned int i{ 0 }; i < 5; ++i)
		m_vertices[i].color = sprite.getColor();

	// weld last vertex to second vertex to complete the solid (matches its position, texture co-ordinate, and colour)
	m_vertices[5] = m_vertices[1];

	// store a pointer to the sprite's texture
	m_pTexture = sprite.getTexture();
}

void SpinningCard::spin(float angleInDegrees)
{
	spinRadians(angleInDegrees * m_pi / 180);
}

void SpinningCard::spinRadians(float angleInRadians)
{
	angleInRadians -= m_pi / 2;
	m_vertices[0].position = sf::Vector2f(m_initial.left + m_initial.width / 2, m_initial.top + m_initial.height / 2);
	m_vertices[1].position = sf::Vector2f(m_initial.left + (sin(angleInRadians) + 1) * m_initial.width / 2, m_initial.top - cos(angleInRadians) * m_depth * m_initial.height / 2);
	m_vertices[2].position = sf::Vector2f(m_initial.left + (sin(angleInRadians + m_pi) + 1) * m_initial.width / 2, m_initial.top - cos(angleInRadians + m_pi) * m_depth * m_initial.height / 2);
	m_vertices[3].position = sf::Vector2f(m_initial.left + (sin(angleInRadians + m_pi) + 1) * m_initial.width / 2, m_initial.top + (cos(angleInRadians + m_pi) * m_depth + 2) * m_initial.height / 2);
	m_vertices[4].position = sf::Vector2f(m_initial.left + (sin(angleInRadians) + 1) * m_initial.width / 2, m_initial.top + (cos(angleInRadians) * m_depth + 2) * m_initial.height / 2);

	// weld last vertex to second vertex (to complete the solid)
	m_vertices[5] = m_vertices[1];
}

void SpinningCard::spinVertically(float angleInDegrees)
{
	spinVerticallyRadians(angleInDegrees * m_pi / 180);
}

void SpinningCard::spinVerticallyRadians(float angleInRadians)
{
	angleInRadians -= m_pi / 2;
	m_vertices[0].position = sf::Vector2f(m_initial.left + m_initial.width / 2, m_initial.top + m_initial.height / 2);
	m_vertices[1].position = sf::Vector2f(m_initial.left - cos(angleInRadians) * m_depth * m_initial.width / 2, m_initial.top + (sin(angleInRadians) + 1) * m_initial.height / 2);
	m_vertices[4].position = sf::Vector2f(m_initial.left - cos(angleInRadians + m_pi) * m_depth * m_initial.width / 2, m_initial.top + (sin(angleInRadians + m_pi) + 1) * m_initial.height / 2);
	m_vertices[3].position = sf::Vector2f(m_initial.left + (cos(angleInRadians + m_pi) * m_depth + 2) * m_initial.width / 2, m_initial.top + (sin(angleInRadians + m_pi) + 1) * m_initial.height / 2);
	m_vertices[2].position = sf::Vector2f(m_initial.left + (cos(angleInRadians) * m_depth + 2) * m_initial.width / 2, m_initial.top + (sin(angleInRadians) + 1) * m_initial.height / 2);

	// weld last vertex to second vertex (to complete the solid)
	m_vertices[5] = m_vertices[1];
}

void SpinningCard::setDepth(float depth)
{
	m_depth = depth;
}

void SpinningCard::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	states.texture = m_pTexture;
	target.draw(m_vertices, states);
}

} // namespace selbaward
