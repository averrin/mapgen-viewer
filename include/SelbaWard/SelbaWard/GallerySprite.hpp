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

#ifndef SELBAWARD_GALLERYSPRITE_HPP
#define SELBAWARD_GALLERYSPRITE_HPP

#include "Common.hpp"
#include <SFML/Graphics/Texture.hpp>

namespace selbaward
{

// Gallery Sprite v1.1.1
class GallerySprite : public sf::Drawable, public sf::Transformable
{
public:
	struct Exhibit
	{
		sf::FloatRect rectangle;
		sf::Vector2f anchor;

		Exhibit(const sf::FloatRect& newRectangle = { 0.f, 0.f, 0.f, 0.f }, const sf::Vector2f& newAnchor = { 0u, 0u }) : rectangle(newRectangle), anchor(newAnchor) { }
	};

	GallerySprite();
	explicit GallerySprite(const sf::Texture& texture);
	void setColor(const sf::Color& color);
	sf::Color getColor() const;
	void setTexture(const sf::Texture& texture);
	void setTexture();
	const sf::Texture& getTexture() const;
	sf::Vector2f getSize() const; // size of current exhibit
	sf::Vector2f getSize(unsigned int exhibitNumber) const; // size of selected exhibit
	void setScaleFromTargetSize(const sf::Vector2f& targetSize); // note: this is based on the current exhibit. scale value will remain even if exhibit changes unless fixed by again calling this method
	void setScaleFromTargetSize(const sf::Vector2f& targetSize, unsigned int exhibitNumber); // uses a specific exhibit rather than the current exhibit. sets the scale so that the exhibit specified would be the target size provided
	unsigned int getNumberOfExhibits() const;
	void set(unsigned int exhibitNumber);
	unsigned int get() const;

	unsigned int addExhibit(const Exhibit& exhibit);
	void setExhibit(unsigned int exhibitNumber, const Exhibit& exhibit);
	Exhibit getExhibit(unsigned int exhibitNumber) const;
	Exhibit getExhibit() const;
	void setRect(unsigned int exhibitNumber, const sf::FloatRect& rect);
	sf::FloatRect getRect(unsigned int exhibitNumber) const;
	sf::FloatRect getRect() const;
	void setAnchor(unsigned int exhibitNumber, const sf::Vector2f& anchor);
	sf::Vector2f getAnchor(unsigned int exhibitNumber) const;
	sf::Vector2f getAnchor() const;

	bool contains(const sf::Vector2f& point) const;

	sf::FloatRect getLocalBounds() const;
	sf::FloatRect getGlobalBounds() const;

	GallerySprite& operator++(); // prefix only
	GallerySprite& operator--(); // prefix only

	void operator+=(unsigned int numberOfExhibits);
	void operator-=(unsigned int numberOfExhibits);

private:
	const sf::Texture* m_pTexture;
	std::vector<sf::Vertex> m_vertices;
	unsigned int m_currentExhibit;
	std::vector<Exhibit> m_exhibits;

	virtual void draw(sf::RenderTarget&, sf::RenderStates) const;
	void priv_updateVertices();
	Exhibit priv_getCurrentExhibit() const;
	Exhibit priv_getExhibit(unsigned int exhibitNumber) const;
	sf::FloatRect priv_getAdjustedLocalRectangleFromCurrentExhibit() const;
	sf::FloatRect priv_getAdjustedLocalRectangleFromExhibit(unsigned int exhibitNumber) const;
};

} // namespace selbaward
#endif // SELBAWARD_GALLERYSPRITE_HPP
