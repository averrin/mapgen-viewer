//////////////////////////////////////////////////////////////////////////////
//
// Selba Ward (https://github.com/Hapaxia/SelbaWard)
// --
//
// NinePatch
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

#include "NinePatch.hpp"

namespace
{

const sf::Vector2f trimAmount{ 1.f, 1.f };

void extractScalePositionsAndContentAreaFromTexture(const sf::Texture* const pTexture, const sf::IntRect textureRectangle, sf::Vector2f& topLeft, sf::Vector2f& bottomRight, sf::Vector2f& contentTopLeft, sf::Vector2f& contentBottomRight)
{
	sf::Image textureImage{ pTexture->copyToImage() };
	sf::Image image;
	image.create(textureRectangle.width, textureRectangle.height);
	image.copy(textureImage, 0, 0, textureRectangle);

	// scale positions
	topLeft = { 0.f, 0.f };
	bottomRight = { image.getSize().x - 2.f, image.getSize().y - 2.f };
	bool foundStart{ false }, foundEnd{ false };
	for (unsigned int x{ 1 }; x < image.getSize().x; ++x)
	{
		if (!foundStart)
		{
			if (image.getPixel(x, 0) == sf::Color::Black)
			{
				foundStart = true;
				topLeft.x = x - 1.f;
			}
			else
				continue;
		}
		if (foundStart)
		{
			if (image.getPixel(x, 0) == sf::Color::Black)
				bottomRight.x = x - 1.f;
			else
				break;
		}
	}
	foundStart = false;
	foundEnd = false;
	for (unsigned int y{ 1 }; y < image.getSize().y; ++y)
	{
		if (!foundStart)
		{
			if (image.getPixel(0, y) == sf::Color::Black)
			{
				foundStart = true;
				topLeft.y = y - 1.f;
			}
			else
				continue;
		}
		if (foundStart)
		{
			if (image.getPixel(0, y) == sf::Color::Black)
				bottomRight.y = y - 1.f;
			else
				break;
		}
	}

	// content area
	contentTopLeft = { 0.f, 0.f };
	contentBottomRight = { image.getSize().x - 2.f, image.getSize().y - 2.f };
	foundStart = false;
	foundEnd = false;
	const sf::Vector2u textureBottomRightPixel(image.getSize() - sf::Vector2u(1, 1));
	for (unsigned int x{ 1 }; x < image.getSize().x; ++x)
	{
		if (!foundStart)
		{
			if (image.getPixel(x, textureBottomRightPixel.y) == sf::Color::Black)
			{
				foundStart = true;
				contentTopLeft.x = x - 1.f;
			}
			else
				continue;
		}
		if (foundStart)
		{
			if (image.getPixel(x, textureBottomRightPixel.y) == sf::Color::Black)
				contentBottomRight.x = x - 1.f;
			else
				break;
		}
	}
	foundStart = false;
	foundEnd = false;
	for (unsigned int y{ 1 }; y < image.getSize().y; ++y)
	{
		if (!foundStart)
		{
			if (image.getPixel(textureBottomRightPixel.x, y) == sf::Color::Black)
			{
				foundStart = true;
				contentTopLeft.y = y - 1.f;
			}
			else
				continue;
		}
		if (foundStart)
		{
			if (image.getPixel(textureBottomRightPixel.x, y) == sf::Color::Black)
				contentBottomRight.y = y - 1.f;
			else
				break;
		}
	}
}

} // namespace

namespace selbaward
{

NinePatch::NinePatch()
	: m_primitiveType(sf::PrimitiveType::Quads)
	, m_vertices(36, sf::Vertex({ 0.f, 0.f }))
	, m_texture(nullptr)
	, m_textureRectangle({ 0, 0, 3, 3 })
	, m_trimmedSize({ 0.f, 0.f })
	, m_size({ 0.f, 0.f })
	, m_scaleTopLeft({ 0.f, 0.f })
	, m_scaleBottomRight({ 0.f, 0.f })
	, m_contentTopLeft({ 0.f, 0.f })
	, m_contentBottomRight({ 0.f, 0.f })
{
}

void NinePatch::setTexture(const sf::Texture& texture, const bool resetSize, const bool resetRect)
{
	m_texture = &texture;
	if (resetRect)
		m_textureRectangle = { { 0, 0 }, sf::Vector2i(m_texture->getSize()) };
	//m_trimmedSize = sf::Vector2f(m_texture->getSize()) - trimAmount * 2.f;
	m_trimmedSize = sf::Vector2f{ static_cast<float>(m_textureRectangle.width), static_cast<float>(m_textureRectangle.height) } -trimAmount * 2.f;
	if (resetSize)
		m_size = m_trimmedSize;
	extractScalePositionsAndContentAreaFromTexture(m_texture, m_textureRectangle, m_scaleTopLeft, m_scaleBottomRight, m_contentTopLeft, m_contentBottomRight);
	priv_updateVertices();
}

void NinePatch::setSize(sf::Vector2f size)
{
	m_size = size;
	const sf::Vector2f minimumSize{ m_scaleTopLeft + (m_trimmedSize - m_scaleBottomRight) };
	if (m_size.x < minimumSize.x)
		m_size.x = minimumSize.x;
	if (m_size.y < minimumSize.y)
		m_size.y = minimumSize.y;
	priv_updateVerticesPositions();
}

void NinePatch::resetSize()
{
	setSize(m_trimmedSize);
}

void NinePatch::setTextureRect(const sf::IntRect textureRectangle)
{
	m_textureRectangle = textureRectangle;
	m_trimmedSize = sf::Vector2f{ static_cast<float>(m_textureRectangle.width), static_cast<float>(m_textureRectangle.height) } -trimAmount * 2.f;
	if (m_texture != nullptr)
	{
		extractScalePositionsAndContentAreaFromTexture(m_texture, m_textureRectangle, m_scaleTopLeft, m_scaleBottomRight, m_contentTopLeft, m_contentBottomRight);
		priv_updateVertices();
	}
}

void NinePatch::setColor(const sf::Color& color)
{
	for (auto& vertex : m_vertices)
		vertex.color = color;
}

sf::Color NinePatch::getColor() const
{
	return m_vertices[0].color;
}

sf::FloatRect NinePatch::getLocalBounds() const
{
	return{ { 0.f, 0.f }, m_size };
}

sf::FloatRect NinePatch::getGlobalBounds() const
{
	return getTransform().transformRect(getLocalBounds());
}

sf::FloatRect NinePatch::getLocalContentArea() const
{
	const sf::Vector2f topLeft{ priv_getResultingPositionOfTextureCoord(m_contentTopLeft) };
	return{ topLeft, priv_getResultingPositionOfTextureCoord(m_contentBottomRight) - topLeft + sf::Vector2f(1.f, 1.f) };
}

sf::FloatRect NinePatch::getGlobalContentArea() const
{
	return getTransform().transformRect(getLocalContentArea());
}

bool NinePatch::isPointInsideTransformedContentArea(const sf::Vector2f point) const
{
	return getLocalContentArea().contains(getInverseTransform().transformPoint(point));
}



// PRIVATE

void NinePatch::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.texture = m_texture;
	states.transform *= getTransform();
	target.draw(&m_vertices.front(), 36, m_primitiveType, states);
}

void NinePatch::priv_updateVertices()
{
	priv_updateVerticesPositions();
	priv_updateVerticesTexCoords();
}

void NinePatch::priv_updateVerticesPositions()
{
	const sf::Vector2f newBottomRightScaled{ m_size - (m_trimmedSize - m_scaleBottomRight) };

	// top row
	m_vertices[0].position = { 0.f, 0.f };
	m_vertices[1].position = { m_scaleTopLeft.x, 0.f };
	m_vertices[2].position = m_scaleTopLeft;
	m_vertices[3].position = { 0.f, m_scaleTopLeft.y };

	m_vertices[4].position = { m_scaleTopLeft.x, 0.f };
	m_vertices[5].position = { newBottomRightScaled.x, 0.f };
	m_vertices[6].position = { newBottomRightScaled.x, m_scaleTopLeft.y };
	m_vertices[7].position = m_scaleTopLeft;

	m_vertices[8].position = { newBottomRightScaled.x, 0.f };
	m_vertices[9].position = { m_size.x, 0.f };
	m_vertices[10].position = { m_size.x, m_scaleTopLeft.y };
	m_vertices[11].position = { newBottomRightScaled.x, m_scaleTopLeft.y };

	// centre row
	m_vertices[12].position = { 0.f, m_scaleTopLeft.y };
	m_vertices[13].position = { m_scaleTopLeft.x, m_scaleTopLeft.y };
	m_vertices[14].position = { m_scaleTopLeft.x, newBottomRightScaled.y };
	m_vertices[15].position = { 0.f, newBottomRightScaled.y };

	m_vertices[16].position = { m_scaleTopLeft.x, m_scaleTopLeft.y };
	m_vertices[17].position = { newBottomRightScaled.x, m_scaleTopLeft.y };
	m_vertices[18].position = { newBottomRightScaled.x, newBottomRightScaled.y };
	m_vertices[19].position = { m_scaleTopLeft.x, newBottomRightScaled.y };

	m_vertices[20].position = { newBottomRightScaled.x, m_scaleTopLeft.y };
	m_vertices[21].position = { m_size.x, m_scaleTopLeft.y };
	m_vertices[22].position = { m_size.x, newBottomRightScaled.y };
	m_vertices[23].position = { newBottomRightScaled.x, newBottomRightScaled.y };

	// bottom row
	m_vertices[24].position = { 0.f, newBottomRightScaled.y };
	m_vertices[25].position = { m_scaleTopLeft.x, newBottomRightScaled.y };
	m_vertices[26].position = { m_scaleTopLeft.x, m_size.y };
	m_vertices[27].position = { 0.f, m_size.y };

	m_vertices[28].position = { m_scaleTopLeft.x, newBottomRightScaled.y };
	m_vertices[29].position = { newBottomRightScaled.x, newBottomRightScaled.y };
	m_vertices[30].position = { newBottomRightScaled.x, m_size.y };
	m_vertices[31].position = { m_scaleTopLeft.x, m_size.y };

	m_vertices[32].position = { newBottomRightScaled.x, newBottomRightScaled.y };
	m_vertices[33].position = { m_size.x, newBottomRightScaled.y };
	m_vertices[34].position = { m_size.x, m_size.y };
	m_vertices[35].position = { newBottomRightScaled.x, m_size.y };

}

void NinePatch::priv_updateVerticesTexCoords()
{
	const sf::Vector2f textureBottomRight{ m_trimmedSize };

	// top row
	m_vertices[0].texCoords = { 0.f, 0.f };
	m_vertices[1].texCoords = { m_scaleTopLeft.x, 0.f };
	m_vertices[2].texCoords = m_scaleTopLeft;
	m_vertices[3].texCoords = { 0.f, m_scaleTopLeft.y };

	m_vertices[4].texCoords = { m_scaleTopLeft.x, 0.f };
	m_vertices[5].texCoords = { m_scaleBottomRight.x, 0.f };
	m_vertices[6].texCoords = { m_scaleBottomRight.x, m_scaleTopLeft.y };
	m_vertices[7].texCoords = m_scaleTopLeft;

	m_vertices[8].texCoords = { m_scaleBottomRight.x, 0.f };
	m_vertices[9].texCoords = { textureBottomRight.x, 0.f };
	m_vertices[10].texCoords = { textureBottomRight.x, m_scaleTopLeft.y };
	m_vertices[11].texCoords = { m_scaleBottomRight.x, m_scaleTopLeft.y };

	// centre row
	m_vertices[12].texCoords = { 0.f, m_scaleTopLeft.y };
	m_vertices[13].texCoords = m_scaleTopLeft;
	m_vertices[14].texCoords = { m_scaleTopLeft.x, m_scaleBottomRight.y };
	m_vertices[15].texCoords = { 0.f, m_scaleBottomRight.y };

	m_vertices[16].texCoords = m_scaleTopLeft;
	m_vertices[17].texCoords = { m_scaleBottomRight.x, m_scaleTopLeft.y };
	m_vertices[18].texCoords = m_scaleBottomRight;
	m_vertices[19].texCoords = { m_scaleTopLeft.x, m_scaleBottomRight.y };

	m_vertices[20].texCoords = { m_scaleBottomRight.x, m_scaleTopLeft.y };
	m_vertices[21].texCoords = { textureBottomRight.x, m_scaleTopLeft.y };
	m_vertices[22].texCoords = { textureBottomRight.x, m_scaleBottomRight.y };
	m_vertices[23].texCoords = m_scaleBottomRight;

	// bottom row
	m_vertices[24].texCoords = { 0.f, m_scaleBottomRight.y };
	m_vertices[25].texCoords = { m_scaleTopLeft.x, m_scaleBottomRight.y };
	m_vertices[26].texCoords = { m_scaleTopLeft.x, textureBottomRight.y };
	m_vertices[27].texCoords = { 0.f, textureBottomRight.y };

	m_vertices[28].texCoords = { m_scaleTopLeft.x, m_scaleBottomRight.y };
	m_vertices[29].texCoords = m_scaleBottomRight;
	m_vertices[30].texCoords = { m_scaleBottomRight.x, textureBottomRight.y };
	m_vertices[31].texCoords = { m_scaleTopLeft.x, textureBottomRight.y };

	m_vertices[32].texCoords = m_scaleBottomRight;
	m_vertices[33].texCoords = { textureBottomRight.x, m_scaleBottomRight.y };
	m_vertices[34].texCoords = textureBottomRight;
	m_vertices[35].texCoords = { m_scaleBottomRight.x, textureBottomRight.y };

	// offset trim and texture rectangle
	const sf::Vector2f textureRectangleOffset{ static_cast<float>(m_textureRectangle.left), static_cast<float>(m_textureRectangle.top) };
	for (auto& vertex : m_vertices)
		vertex.texCoords += textureRectangleOffset + trimAmount;
}

sf::Vector2f NinePatch::priv_getResultingPositionOfTextureCoord(sf::Vector2f textureCoord) const
{
	sf::Vector2f result;

	const sf::Vector2f newBottomRightScaled{ m_size - (m_trimmedSize - m_scaleBottomRight) };
	const sf::Vector2f scaleSize{ m_scaleBottomRight - m_scaleTopLeft };
	const sf::Vector2f newScaleSize{ newBottomRightScaled - m_scaleTopLeft };

	if (textureCoord.x <= m_scaleTopLeft.x)
		result.x = textureCoord.x;
	else if (textureCoord.x >= m_scaleBottomRight.x)
		result.x = newBottomRightScaled.x + (textureCoord.x - m_scaleBottomRight.x); // brackets are to help clarity
	else
		result.x = ((textureCoord.x - m_scaleTopLeft.x) / scaleSize.x) * newScaleSize.x + m_scaleTopLeft.x;
	if (textureCoord.y <= m_scaleTopLeft.y)
		result.y = textureCoord.y;
	else if (textureCoord.y >= m_scaleBottomRight.y)
		result.y = newBottomRightScaled.y + (textureCoord.y - m_scaleBottomRight.y); // brackets are to help clarity
	else
		result.y = ((textureCoord.y - m_scaleTopLeft.y) / (scaleSize.y)) * newScaleSize.y + m_scaleTopLeft.y;

	return result;
}

} // namespace selbaward
