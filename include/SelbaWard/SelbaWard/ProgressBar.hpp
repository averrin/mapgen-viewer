//////////////////////////////////////////////////////////////////////////////
//
// Selba Ward (https://github.com/Hapaxia/SelbaWard)
// --
//
// Progress Bar
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

#ifndef SELBAWARD_PROGRESSBAR_HPP
#define SELBAWARD_PROGRESSBAR_HPP

#include "Common.hpp"

#include <SFML/Graphics/RectangleShape.hpp> // simple box drawing (for background and outline)
#include <SFML/Graphics/Texture.hpp>

namespace selbaward
{

// SW Progress Bar 1.1.0
class ProgressBar : public sf::Drawable, public sf::Transformable
{
public:
	// creation and size
	ProgressBar(sf::Vector2f size = { 64.f, 8.f });
	void setSize(sf::Vector2f size);
	sf::Vector2f getSize() const;
	
	// progress
	void setRatio(float ratio);
	float getRatio() const;
	void setPercentage(float percentage);
	float getPercentage() const;
	template <class T>
	void setFromValueInRange(const T& value, const T& minimum, const T& maximum);
	template <class T>
	void setFromValueInRange(const T& value, const T& range);
	
	// visual representation
	void setFrameThickness(float frameThickness);
	void setFrameColor(sf::Color frameColor);
	void setBackgroundColor(sf::Color backgroundColor);
	void setColor(sf::Color color);
	void setShowBar(bool showBar);
	void setShowBackgroundAndFrame(bool showBackgroundAndFrame);
	float getFrameThickness() const;
	sf::Color getFrameColor() const;
	sf::Color getBackgroundColor() const;
	sf::Color getColor() const;
	bool getShowBar() const;
	bool getShowBackgroundAndFrame() const;

	// texturing
	void setTexture(const sf::Texture& texture, bool resetRect = false);
	void setTexture(); // clear/nullify texture
	void setTextureRect(const sf::IntRect& textureRectangle);
	const sf::Texture& getTexture() const;
	void setBackgroundTexture(const sf::Texture& backgroundTexture, bool resetRect = false);
	void setBackgroundTexture(); // clear/nullify background texture
	void setBackgroundTextureRect(const sf::IntRect& backgroundTextureRectangle);
	const sf::Texture& getBackgroundTexture() const;

	// bounds
	sf::FloatRect getLocalBounds() const;
	sf::FloatRect getGlobalBounds() const;

	// anchor points
	sf::Vector2f getAnchorProgressTop() const;
	sf::Vector2f getAnchorProgressCenter() const;
	sf::Vector2f getAnchorProgressBottom() const;

private:
	float m_amount;
	bool m_showBar;
	bool m_showBackground;
	sf::Vector2f m_size;
	sf::Color m_color;
	std::vector<sf::Vertex> m_bar;
	sf::RectangleShape m_backgroundAndFrame;
	const sf::Texture* m_texture;
	const sf::Texture* m_backgroundTexture;
	sf::IntRect m_textureRectangle;
	sf::IntRect m_backgroundTextureRectangle;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	void priv_updateGraphics();
};

inline sf::Vector2f ProgressBar::getSize() const
{
	return m_size;
}

inline float ProgressBar::getRatio() const
{
	return m_amount;
}

inline float ProgressBar::getPercentage() const
{
	return m_amount * 100.f;
}

template <class T>
void ProgressBar::setFromValueInRange(const T& value, const T& minimum, const T& maximum)
{
	setRatio(static_cast<float>(value - minimum) / static_cast<float>(maximum - minimum));
}

template <class T>
void ProgressBar::setFromValueInRange(const T& value, const T& range)
{
	setRatio(static_cast<float>(value) / static_cast<float>(range));
}

inline float ProgressBar::getFrameThickness() const
{
	return m_backgroundAndFrame.getOutlineThickness();
}

inline sf::Color ProgressBar::getFrameColor() const
{
	return m_backgroundAndFrame.getOutlineColor();
}

inline sf::Color ProgressBar::getBackgroundColor() const
{
	return m_backgroundAndFrame.getFillColor();
}

inline sf::Color ProgressBar::getColor() const
{
	return m_color;
}

inline bool ProgressBar::getShowBar() const
{
	return m_showBar;
}

inline bool ProgressBar::getShowBackgroundAndFrame() const
{
	return m_showBackground;
}

} // namespace selbaward
#endif // SELBAWARD_PROGRESSBAR_HPP
