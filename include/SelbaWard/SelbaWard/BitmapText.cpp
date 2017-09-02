//////////////////////////////////////////////////////////////////////////////
//
// Selba Ward (https://github.com/Hapaxia/SelbaWard)
// --
//
// BitmapText
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

#include "BitmapText.hpp"

namespace selbaward
{

BitmapText::BitmapText()
	: m_pBitmapFont(nullptr)
	, m_vertices(sf::Quads)
	, m_string()
	, m_color(sf::Color::White)
	, m_tracking(1)
{
}

void BitmapText::setBitmapFont(const BitmapFont& bitmapFont)
{
	m_pBitmapFont = &bitmapFont;
	priv_updateVertices();
}

void BitmapText::setBitmapFont()
{
	m_pBitmapFont = nullptr;
	priv_updateVertices();
}

void BitmapText::setString(const std::string& string)
{
	m_string = string;
	priv_updateVertices();
}

const std::string BitmapText::getString() const
{
	return m_string;
}

void BitmapText::setTracking(int tracking)
{
	m_tracking = tracking;
	priv_updateVertices();
}

const int BitmapText::getTracking() const
{
	return m_tracking;
}

void BitmapText::setColor(const sf::Color& color)
{
	m_color = color;
	priv_updateColor();
}

const sf::Color BitmapText::getColor() const
{
	return m_color;
}

void BitmapText::setScale(unsigned int scale)
{
	setScale(scale, scale);
}

void BitmapText::setScale(sf::Vector2u scale)
{
	setScale(scale.x, scale.y);
}

void BitmapText::setScale(unsigned int scaleX, unsigned int scaleY)
{
	this->Transformable::setScale(static_cast<float>(scaleX), static_cast<float>(scaleY));
}

sf::FloatRect BitmapText::getGlobalBounds() const
{
	return getTransform().transformRect(m_bounds);
}

sf::FloatRect BitmapText::getLocalBounds() const
{
	return m_bounds;
}



// PRIVATE

void BitmapText::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (m_vertices.getVertexCount() == 0)
		return;

	states.transform *= getTransform();
	states.texture = m_pBitmapFont->getTexture();
	target.draw(m_vertices, states);
}

void BitmapText::priv_updateVertices()
{
	if (m_pBitmapFont == nullptr)
	{
		m_vertices.clear();
		m_bounds = { 0.f, 0.f, 0.f, 0.f };
	}

	m_vertices.resize(m_string.length() * 4);

	sf::Vector2f penPosition{ 0.f, 0.f };

	float minX(0.f), minY(0.f), maxX(0.f), maxY(0.f);

	for (unsigned int character{ 0 }; character < m_string.length(); ++character)
	{
		const unsigned int glyphNumber{ (m_string[character] >= 0) ? static_cast<unsigned int>(m_string[character]) : static_cast<unsigned int>(m_string[character] + 256) }; // after 125, 126, 127 is -128, -127, -126. this moves them to 128, 129, 130

		const int kerning{ (character < m_string.length() - 1) ? m_pBitmapFont->getKerning(m_string.substr(character, 2)) : 0 };

		BitmapFont::Glyph glyph = m_pBitmapFont->getGlyph(glyphNumber);

		const sf::Vector2f glyphOffset{ 0.f - glyph.startX, (glyph.baseline < 0) ? (0.f - glyph.baseline - glyph.textureRect.height) : (0.f - glyph.baseline) };
		const sf::Vector2f glyphPosition{ penPosition + glyphOffset };

		m_vertices[character * 4 + 0].position = glyphPosition;
		m_vertices[character * 4 + 1].position = glyphPosition + sf::Vector2f(static_cast<float>(glyph.textureRect.width), 0);
		m_vertices[character * 4 + 2].position = glyphPosition + sf::Vector2f(static_cast<float>(glyph.textureRect.width), static_cast<float>(glyph.textureRect.height));
		m_vertices[character * 4 + 3].position = glyphPosition + sf::Vector2f(0, static_cast<float>(glyph.textureRect.height));

		m_vertices[character * 4 + 0].texCoords = sf::Vector2f(
			static_cast<float>(glyph.textureRect.left),
			static_cast<float>(glyph.textureRect.top));
		m_vertices[character * 4 + 1].texCoords = sf::Vector2f(
			static_cast<float>(glyph.textureRect.left + glyph.textureRect.width),
			static_cast<float>(glyph.textureRect.top));
		m_vertices[character * 4 + 2].texCoords = sf::Vector2f(
			static_cast<float>(glyph.textureRect.left + glyph.textureRect.width),
			static_cast<float>(glyph.textureRect.top + glyph.textureRect.height));
		m_vertices[character * 4 + 3].texCoords = sf::Vector2f(
			static_cast<float>(glyph.textureRect.left),
			static_cast<float>(glyph.textureRect.top + glyph.textureRect.height));

		penPosition += sf::Vector2f((glyph.width > 0) ? (0.f + m_tracking + kerning + glyph.width) : (0.f + m_tracking + kerning + glyph.width + glyph.textureRect.width - glyph.startX), 0);

		minX = std::min(minX, m_vertices[character * 4 + 0].position.x);
		maxX = std::max(maxX, m_vertices[character * 4 + 2].position.x);
		minY = std::min(minY, m_vertices[character * 4 + 0].position.y);
		maxY = std::max(maxY, m_vertices[character * 4 + 2].position.y);
	}

	priv_updateColor();

	m_bounds.left = minX;
	m_bounds.top = minY;
	m_bounds.width = maxX - minX;
	m_bounds.height = maxY - minY;
}

void BitmapText::priv_updateColor()
{
	for (unsigned int v{ 0 }; v < m_vertices.getVertexCount(); ++v)
		m_vertices[v].color = m_color;
}

} // namespace selbaward
