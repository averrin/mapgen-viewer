//////////////////////////////////////////////////////////////////////////////
//
// Selba Ward (https://github.com/Hapaxia/SelbaWard)
// --
//
// BitmapFont
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

#include "BitmapFont.hpp"

namespace
{

const std::string exceptionPrefix = "Bitmap Font: ";

} // namespace
namespace selbaward
{

BitmapFont::BitmapFont()
	: m_throwExceptions(true)
	, m_useExternalTexture(false)
	, m_texture()
	, m_mExternalTexture(nullptr)
	, m_numberOfTilesPerRow(1)
	, m_tileSize({ 0, 0 })
	, m_defaultTextureRect()
	, m_kernings()
	, m_glyphs(256) // create 256 glyphs to cover the base ascii set
{
	setAllGlyphsToDefault();
}

void BitmapFont::setExternalTexture(const sf::Texture& externalTexture)
{
	m_mExternalTexture = &externalTexture;
	m_useExternalTexture = true;
}

void BitmapFont::loadTexture(const std::string& filename)
{
	if (!m_texture.loadFromFile(filename))
	{
		if (m_throwExceptions)
			throw Exception(exceptionPrefix + "failed to load texture.");
		return;
	}
	m_useExternalTexture = false;
}

void BitmapFont::setSmooth(const bool smooth)
{
	if (!m_useExternalTexture)
		m_texture.setSmooth(smooth);
}

void BitmapFont::setNumberOfTilesPerRow(const unsigned int numberOfTilesPerRow)
{
	m_numberOfTilesPerRow = numberOfTilesPerRow;
}

void BitmapFont::setThrowExceptions(const bool throwExceptions)
{
	m_throwExceptions = throwExceptions;
}

const bool BitmapFont::getThrowExceptions() const
{
	return m_throwExceptions;
}








void BitmapFont::setDefaultTextureRect(const sf::IntRect& defaultTextureRect)
{
	m_defaultTextureRect = defaultTextureRect;
}

void BitmapFont::setTextureRect(const sf::IntRect& textureRect, const unsigned int glyphIndex)
{
	if (!priv_isGlyphIndexValid(glyphIndex))
	{
		if (m_throwExceptions)
			throw Exception(exceptionPrefix + "cannot set texture rect - glyph index (" + std::to_string(glyphIndex) + ") out of range.");
		return;
	}

	m_glyphs[glyphIndex].useDefaultTextureRect = false;;
	m_glyphs[glyphIndex].textureRect = textureRect;
	m_glyphs[glyphIndex].width = textureRect.width;
	m_glyphs[glyphIndex].baseline = textureRect.height - 1;
	m_glyphs[glyphIndex].startX = 0;
}

void BitmapFont::setTextureRects(const std::vector<sf::IntRect>& textureRects, const unsigned int initialGlyphIndex)
{
	for (unsigned int i{ 0 }; i < textureRects.size(); ++i)
		setTextureRect(textureRects[i], initialGlyphIndex + i);
}

void BitmapFont::clearTextureRect(const unsigned int glyphIndex)
{
	if (!priv_isGlyphIndexValid(glyphIndex))
	{
		if (m_throwExceptions)
			throw Exception(exceptionPrefix + "cannot clear texture rect - glyph index (" + std::to_string(glyphIndex) + ") out of range.");
		return;
	}

	m_glyphs[glyphIndex].useDefaultTextureRect = true;
}

void BitmapFont::clearAllTextureRects()
{
	for (unsigned int i{ 0 }; i < m_glyphs.size(); ++i)
		clearTextureRect(i);
}






void BitmapFont::setGlyphToDefault(const unsigned int glyphIndex)
{
	if (!priv_isGlyphIndexValid(glyphIndex))
	{
		if (m_throwExceptions)
			throw Exception(exceptionPrefix + "cannot set glyph to default - glyph index (" + std::to_string(glyphIndex) + ") out of range.");
		return;
	}

	m_glyphs[glyphIndex].useDefaultTextureRect = true;
	m_glyphs[glyphIndex].textureRect = m_defaultTextureRect;
	m_glyphs[glyphIndex].width = 0;
	m_glyphs[glyphIndex].baseline = -1;
	m_glyphs[glyphIndex].startX = 0;
}

void BitmapFont::setGlyphsToDefault(const unsigned int numberOfGlyphs, const unsigned int initialGlyphIndex)
{
	for (unsigned int i{ 0 }; i < numberOfGlyphs; ++i)
		setGlyphToDefault(initialGlyphIndex + i);
}

void BitmapFont::setAllGlyphsToDefault()
{
	setGlyphsToDefault(static_cast<unsigned int>(m_glyphs.size()));
}









void BitmapFont::setBaseline(const int baseline, const unsigned int glyphIndex)
{
	if (!priv_isGlyphIndexValid(glyphIndex))
	{
		if (m_throwExceptions)
			throw Exception(exceptionPrefix + "cannot set glyph baseline - glyph index (" + std::to_string(glyphIndex) + ") out of range.");
		return;
	}

	m_glyphs[glyphIndex].baseline = baseline;
}

void BitmapFont::setWidth(const int width, const unsigned int glyphIndex)
{
	if (!priv_isGlyphIndexValid(glyphIndex))
	{
		if (m_throwExceptions)
			throw Exception(exceptionPrefix + "cannot set glyph width - glyph index (" + std::to_string(glyphIndex) + ") out of range.");
		return;
	}

	m_glyphs[glyphIndex].width = width;
}

void BitmapFont::setStartX(const int startX, const unsigned int glyphIndex)
{
	if (!priv_isGlyphIndexValid(glyphIndex))
	{
		if (m_throwExceptions)
			throw Exception(exceptionPrefix + "cannot set glyph start x - glyph index (" + std::to_string(glyphIndex) + ") out of range.");
		return;
	}

	m_glyphs[glyphIndex].startX = startX;
}

void BitmapFont::setBaselines(const int baseline, const unsigned int numberOfGlyphs, const unsigned int initialGlyphIndex)
{
	for (unsigned int i{ 0 }; i < numberOfGlyphs; ++i)
		setBaseline(baseline, initialGlyphIndex + i);
}

void BitmapFont::setWidths(const int width, const unsigned int numberOfGlyphs, const unsigned int initialGlyphIndex)
{
	for (unsigned int i{ 0 }; i < numberOfGlyphs; ++i)
		setWidth(width, initialGlyphIndex + i);
}

void BitmapFont::setStartXs(const int startX, const unsigned int numberOfGlyphs, const unsigned int initialGlyphIndex)
{
	for (unsigned int i{ 0 }; i < numberOfGlyphs; ++i)
		setStartX(startX, initialGlyphIndex + i);
}

void BitmapFont::setBaselines(const std::vector<int>& baselines, const unsigned int initialGlyphIndex)
{
	for (unsigned int i{ 0 }; i < baselines.size(); ++i)
		setBaseline(baselines[i], initialGlyphIndex + i);
}

void BitmapFont::setWidths(const std::vector<int>& widths, const unsigned int initialGlyphIndex)
{
	for (unsigned int i{ 0 }; i < widths.size(); ++i)
		setWidth(widths[i], initialGlyphIndex + i);
}

void BitmapFont::setStartXs(const std::vector<int>& startXs, const unsigned int initialGlyphIndex)
{
	for (unsigned int i{ 0 }; i < startXs.size(); ++i)
		setStartX(startXs[i], initialGlyphIndex + i);
}

void BitmapFont::setBaseline(const int baseline, const std::string& glyphs)
{
	for (auto& glyph : glyphs)
		setBaseline(baseline, glyph);
}

void BitmapFont::setWidth(const int width, const std::string& glyphs)
{
	for (auto& glyph : glyphs)
		setWidth(width, glyph);
}

void BitmapFont::setStartX(const int startX, const std::string& glyphs)
{
	for (auto& glyph : glyphs)
		setStartX(startX, glyph);
}

void BitmapFont::setKerning(const int kerning, const std::string& glyphs)
{
	if (glyphs.size() < 2)
	{
		if (m_throwExceptions)
			throw Exception(exceptionPrefix + "cannot set kerning - glyph pair not specified.");
		return;
	}

	for (std::size_t i{ 0u }; i < glyphs.size(); i += 2)
	{
		std::string glyphPair{ glyphs.substr(i, 2) };
		if (glyphPair.size() != 2)
		{
			if (m_throwExceptions)
				throw Exception(exceptionPrefix + "cannot set kerning - final glyph pair is missing second glyph.");
			return;
		}
		priv_setKerning(kerning, glyphPair);
	}
}

void BitmapFont::setKerning(const int kerning, const std::vector<std::string>& glyphPairs)
{
	for (auto& glyphPair : glyphPairs)
		priv_setKerning(kerning, glyphPair);
}

const sf::Texture* BitmapFont::getTexture() const
{
	if (m_useExternalTexture)
		return m_mExternalTexture;
	else
		return &m_texture;
}

const BitmapFont::Glyph BitmapFont::getGlyph(const unsigned int glyphIndex) const
{
	if (!priv_isGlyphIndexValid(glyphIndex))
	{
		if (m_throwExceptions)
			throw Exception(exceptionPrefix + "cannot get glyph - glyph index out of range.");
		return priv_getGlyphWithDefaultTextureRect();
	}

	if (m_glyphs[glyphIndex].useDefaultTextureRect)
		return priv_getGlyphWithDefaultTextureRect(glyphIndex);
	else
		return m_glyphs[glyphIndex];
}

const unsigned int BitmapFont::getNumberOfGlyphs() const
{
	return static_cast<unsigned int>(m_glyphs.size());
}

const int BitmapFont::getKerning(const std::string& glyphPair) const
{
	if (glyphPair.size() != 2)
	{
		if (m_throwExceptions)
			throw Exception(exceptionPrefix + "cannot get kerning - glyph pair not valid.");
		return 0;
	}

	std::map<std::string, int>::iterator it;
	it = m_kernings.find(glyphPair);
	if (it == m_kernings.end())
		return 0;
	
	return m_kernings[glyphPair];
}





// PRIVATE

const bool BitmapFont::priv_isGlyphIndexValid(const unsigned int glyphIndex) const
{
	if (glyphIndex < m_glyphs.size())
		return true;
	else if (m_throwExceptions)
		throw Exception(exceptionPrefix + "glyph index (" + std::to_string(glyphIndex) + ") out of range.");
	else
		return false;
}

const BitmapFont::Glyph BitmapFont::priv_getGlyphWithDefaultTextureRect(unsigned int glyphIndex) const
{
	if (!priv_isGlyphIndexValid(glyphIndex))
	{
		if (m_throwExceptions)
			throw Exception(exceptionPrefix + "cannot get default glyph - glyph index (" + std::to_string(glyphIndex) + ") out of range.");
		glyphIndex = 0;
	}

	if (m_glyphs.size() == 0)
		throw Exception(exceptionPrefix + "BUG - no glyphs available.");

	Glyph defaultGlyph;
	defaultGlyph.useDefaultTextureRect = false;
	defaultGlyph.textureRect = m_defaultTextureRect;
	defaultGlyph.textureRect.left = m_defaultTextureRect.width * (glyphIndex % m_numberOfTilesPerRow);
	defaultGlyph.textureRect.top = m_defaultTextureRect.height * (glyphIndex / m_numberOfTilesPerRow);
	defaultGlyph.width = m_glyphs[glyphIndex].width;
	defaultGlyph.baseline = m_glyphs[glyphIndex].baseline;
	defaultGlyph.startX = m_glyphs[glyphIndex].startX;
	return defaultGlyph;
}

void BitmapFont::priv_setKerning(const int kerning, const std::string& glyphPair)
{
	if (glyphPair.size() != 2)
	{
		if (m_throwExceptions)
			throw Exception(exceptionPrefix + "cannot set kerning - glyph pair not valid.");
		return;
	}

	m_kernings[glyphPair] = kerning;
}

} // namespace selbaward
