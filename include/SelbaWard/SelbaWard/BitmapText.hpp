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

#ifndef SELBAWARD_BITMAPTEXT_HPP
#define SELBAWARD_BITMAPTEXT_HPP

#include "Common.hpp"
#include "BitmapFont.hpp"
#include <SFML/Graphics/VertexArray.hpp>

namespace selbaward
{

// SW Bitmap Text v1.1.1
class BitmapText : public sf::Drawable, public sf::Transformable
{
public:
	BitmapText();
	void setBitmapFont(const BitmapFont& bitmapFont);
	void setBitmapFont();
	void setString(const std::string& string = "");
	const std::string getString() const;
	void setTracking(int tracking);
	const int getTracking() const;
	void setColor(const sf::Color& color);
	const sf::Color getColor() const;
	void setScale(unsigned int scale);
	void setScale(unsigned int scaleX, unsigned int scaleY);
	void setScale(sf::Vector2u scale);
	sf::FloatRect getGlobalBounds() const;
	sf::FloatRect getLocalBounds() const;

private:
	const BitmapFont* m_pBitmapFont;
	sf::VertexArray m_vertices;
	std::string m_string;
	sf::Color m_color;
	int m_tracking;
	sf::FloatRect m_bounds;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	void priv_updateVertices();
	void priv_updateColor();
};

} // namespace selbaward
#endif // SELBAWARD_BITMAPTEXT_HPP
