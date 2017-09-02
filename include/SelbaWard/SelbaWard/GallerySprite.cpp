//////////////////////////////////////////////////////////////////////////////
//
// Selba Ward (https://github.com/Hapaxia/SelbaWard)
// --
//
// Gallery Sprite
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

#include "GallerySprite.hpp"

namespace
{

#ifdef USE_SFML_PRE_2_4
const sf::PrimitiveType primitiveType{ sf::PrimitiveType::TrianglesStrip };
#else // USE_SFML_PRE_2_4
const sf::PrimitiveType primitiveType{ sf::PrimitiveType::TriangleStrip };
#endif // USE_SFML_PRE_2_4
const unsigned int numberOfVertices{ 4u };

} // namespace

namespace selbaward
{
	
GallerySprite::GallerySprite()
	: m_vertices(4)
	, m_pTexture(nullptr)
	, m_exhibits()
	, m_currentExhibit(0u)
{
	priv_updateVertices();
}

GallerySprite::GallerySprite(const sf::Texture& texture)
	: GallerySprite()
{
	setTexture(texture);
	priv_updateVertices();
}

void GallerySprite::setColor(const sf::Color& color)
{
	for (auto& vertex : m_vertices)
		vertex.color = color;
}

sf::Color GallerySprite::getColor() const
{
	return m_vertices.front().color;
}

void GallerySprite::setTexture(const sf::Texture& texture)
{
	m_pTexture = &texture;
	priv_updateVertices();
}

void GallerySprite::setTexture()
{
	m_pTexture = nullptr;
	priv_updateVertices();
}

const sf::Texture& GallerySprite::getTexture() const
{
	return *m_pTexture;
}

sf::Vector2f GallerySprite::getSize() const
{
	return getSize(m_currentExhibit);
}

sf::Vector2f GallerySprite::getSize(const unsigned int exhibitNumber) const
{
	const sf::FloatRect& exhibitRectangle{ priv_getExhibit(exhibitNumber).rectangle };
	return{ exhibitRectangle.width, exhibitRectangle.height };
}

void GallerySprite::setScaleFromTargetSize(const sf::Vector2f& targetSize)
{
	setScaleFromTargetSize(targetSize, m_currentExhibit);
}

void GallerySprite::setScaleFromTargetSize(const sf::Vector2f& targetSize, const unsigned int exhibitNumber)
{
	const sf::Vector2f exhibitSize{ getSize(exhibitNumber) };
	this->setScale({ targetSize.x / exhibitSize.x, targetSize.y / exhibitSize.y });
}

unsigned int GallerySprite::getNumberOfExhibits() const
{
	return static_cast<unsigned int>(m_exhibits.size());
}

void GallerySprite::set(const unsigned int exhibitNumber)
{
	m_currentExhibit = exhibitNumber;
	priv_updateVertices();
}

unsigned int GallerySprite::get() const
{
	return m_currentExhibit;
}

unsigned int GallerySprite::addExhibit(const Exhibit& exhibit)
{
	m_exhibits.emplace_back(exhibit);
	return static_cast<unsigned int>(m_exhibits.size());
}

void GallerySprite::setExhibit(const unsigned int exhibitNumber, const Exhibit& exhibit)
{
	m_exhibits[exhibitNumber - 1] = exhibit;
	if (exhibitNumber == m_currentExhibit)
		priv_updateVertices();
}

GallerySprite::Exhibit GallerySprite::getExhibit(const unsigned int exhibitNumber) const
{
	return priv_getExhibit(exhibitNumber);
}

GallerySprite::Exhibit GallerySprite::getExhibit() const
{
	return priv_getCurrentExhibit();
}

void GallerySprite::setRect(const unsigned int exhibitNumber, const sf::FloatRect& textureRectangle)
{
	m_exhibits[exhibitNumber - 1].rectangle = textureRectangle;
	if (exhibitNumber == m_currentExhibit)
		priv_updateVertices();
}

sf::FloatRect GallerySprite::getRect(const unsigned int exhibitNumber) const
{
	return priv_getExhibit(exhibitNumber).rectangle;
}

sf::FloatRect GallerySprite::getRect() const
{
	return priv_getCurrentExhibit().rectangle;
}

void GallerySprite::setAnchor(const unsigned int exhibitNumber, const sf::Vector2f& anchor)
{
	m_exhibits[exhibitNumber - 1].anchor = anchor;
	if (exhibitNumber == m_currentExhibit)
		priv_updateVertices();
}

sf::Vector2f GallerySprite::getAnchor(const unsigned int exhibitNumber) const
{
	return priv_getExhibit(exhibitNumber).anchor;
}

sf::Vector2f GallerySprite::getAnchor() const
{
	return priv_getCurrentExhibit().anchor;
}

bool GallerySprite::contains(const sf::Vector2f& point) const
{
	return priv_getAdjustedLocalRectangleFromCurrentExhibit().contains(getInverseTransform().transformPoint(point));
}

sf::FloatRect GallerySprite::getLocalBounds() const
{
	return{ { 0.f, 0.f }, getSize() };
}

sf::FloatRect GallerySprite::getGlobalBounds() const
{
	return getTransform().transformRect({ -priv_getCurrentExhibit().anchor, getSize() });
}

// prefix only
GallerySprite& GallerySprite::operator++()
{
	++m_currentExhibit;
	priv_updateVertices();

	return *this;
}

// prefix only
GallerySprite& GallerySprite::operator--()
{
	if (m_currentExhibit > 0)
	{
		--m_currentExhibit;
		priv_updateVertices();
	}

	return *this;
}

void GallerySprite::operator+=(const unsigned int exhibits)
{
	if (exhibits == 0u)
		return;

	m_currentExhibit += exhibits;
	priv_updateVertices();
}

void GallerySprite::operator-=(const unsigned int exhibits)
{
	if (exhibits == 0u || m_currentExhibit == 0u)
		return;

	if (m_currentExhibit > exhibits)
		m_currentExhibit -= exhibits;
	else
		m_currentExhibit = 0u;
	priv_updateVertices();
}



// PRIVATE

void GallerySprite::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.texture = m_pTexture;
	states.transform *= getTransform();
	target.draw(&m_vertices.front(), numberOfVertices, primitiveType, states);
}

void GallerySprite::priv_updateVertices()
{
	const Exhibit exhibit{ priv_getCurrentExhibit() };
	m_vertices[0].position = { -exhibit.anchor.x, exhibit.rectangle.height - exhibit.anchor.y };
	m_vertices[1].position = -exhibit.anchor;
	m_vertices[2].position = { exhibit.rectangle.width - exhibit.anchor.x, exhibit.rectangle.height - exhibit.anchor.y };
	m_vertices[3].position = { exhibit.rectangle.width - exhibit.anchor.x, -exhibit.anchor.y };

	if (m_pTexture == nullptr)
		return;

	m_vertices[0].texCoords = { exhibit.rectangle.left, exhibit.rectangle.top + exhibit.rectangle.height };
	m_vertices[1].texCoords = { exhibit.rectangle.left, exhibit.rectangle.top };
	m_vertices[2].texCoords = { exhibit.rectangle.left + exhibit.rectangle.width, exhibit.rectangle.top + exhibit.rectangle.height };
	m_vertices[3].texCoords = { exhibit.rectangle.left + exhibit.rectangle.width, exhibit.rectangle.top };
}

GallerySprite::Exhibit GallerySprite::priv_getCurrentExhibit() const
{
	return priv_getExhibit(m_currentExhibit);
}

GallerySprite::Exhibit GallerySprite::priv_getExhibit(unsigned int exhibitNumber) const
{
	if (m_pTexture == nullptr || exhibitNumber > m_exhibits.size())
		return Exhibit();

	if (exhibitNumber == 0u)
		return{ { 0.f, 0.f, static_cast<float>(m_pTexture->getSize().x), static_cast<float>(m_pTexture->getSize().y) }, { 0.f, 0.f } };

	return m_exhibits[exhibitNumber - 1];
}

sf::FloatRect GallerySprite::priv_getAdjustedLocalRectangleFromCurrentExhibit() const
{
	return priv_getAdjustedLocalRectangleFromExhibit(m_currentExhibit);
}

sf::FloatRect GallerySprite::priv_getAdjustedLocalRectangleFromExhibit(unsigned int exhibitNumber) const
{
	return{ -priv_getExhibit(exhibitNumber).anchor, getSize(exhibitNumber) };
}

} // namespace selbaward
