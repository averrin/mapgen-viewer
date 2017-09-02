//////////////////////////////////////////////////////////////////////////////
//
// Selba Ward (https://github.com/Hapaxia/SelbaWard)
// --
//
// Console Screen v1
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

#include "ConsoleScreenOld.hpp"

#include <random>
#include <functional>
#include <SFML/Graphics/Texture.hpp>

namespace
{

const std::string exceptionPrefix{ "Console Screen (v1): " };

const sf::Color defaultColor{ sf::Color::White };
const sf::Color defaultBackgroundColor{ sf::Color::Black };
const sf::Color defaultCursorColor{ sf::Color::White };
const sf::Color defaultNewPaletteColor{ sf::Color::Black };
const double unBrightAttributeMultiplier{ 0.5 };

const selbaward::ConsoleScreenV1::Cell defaultCell{ 0u, sf::Color::White, sf::Color::Black, selbaward::ConsoleScreenV1::Stretch::None };

std::mt19937 randomGenerator;
const std::uniform_int_distribution<unsigned short int> randomDistribution(0u, 255u);
std::function <unsigned short int()> getRandomByteValue;

inline void randomSeed()
{
	std::random_device rd;
	randomGenerator.seed(rd());
	getRandomByteValue = std::bind(randomDistribution, randomGenerator);
}

inline unsigned char randomByte()
{
	return static_cast<unsigned char>(getRandomByteValue());
}

inline float linearInterpolation(const float start, const float end, const float alpha)
{
	return start * (1 - alpha) + end * alpha;
}

inline void makeColorUnBright(sf::Color& color)
{
	color.r = static_cast<sf::Uint8>(unBrightAttributeMultiplier * color.r);
	color.g = static_cast<sf::Uint8>(unBrightAttributeMultiplier * color.g);
	color.b = static_cast<sf::Uint8>(unBrightAttributeMultiplier * color.b);
}

inline void swapColors(sf::Color& a, sf::Color& b)
{
	sf::Color temp{ a };
	a = b;
	b = temp;
}

inline sf::Color sepiaColor(const float alpha)
{
	// scaling sepia from grey (1.351, 1.203, 0.937). clamp component to <= 1. scaled to 0-255 range: (344.505, 306.765, 238.935)
	const unsigned int r{ static_cast<unsigned int>(linearInterpolation(0.f, 344.505f, alpha)) };
	const unsigned int g{ static_cast<unsigned int>(linearInterpolation(0.f, 306.765f, alpha)) };
	const unsigned int b{ static_cast<unsigned int>(linearInterpolation(0.f, 238.935f, alpha)) };
	return sf::Color((r > 255 ? 255 : static_cast<sf::Uint8>(r)), (g > 255 ? 255 : static_cast<sf::Uint8>(g)), (b > 255 ? 255 : static_cast<sf::Uint8>(b)));
}

inline float relativeLuminance(const sf::Color& color)
{
	return 0.2126f * (color.r / 255.f) + 0.7152f * (color.g / 255.f) + 0.0722f * (color.b / 255.f);
}

// contrasts RGB (black or white to contrast RGB's luminance) and keeps same alpha
inline sf::Color contrastedColor(const sf::Color& color)
{
	sf::Color result{ sf::Color::Black };
	if (relativeLuminance(color) < 0.33f)
		result = sf::Color::White;
	result.a = color.a;
	return result;
}

// inverts RGB and keeps same alpha
inline sf::Color invertedColor(const sf::Color& color)
{
	sf::Color result{ sf::Color::White - color };
	result.a = color.a;
	return result;
}

inline void addColorToPalette(std::vector<sf::Color>& palette, const sf::Color& color)
{
	palette.emplace_back(color);
}

void addPaletteDefault(std::vector<sf::Color>& palette)
{
	addColorToPalette(palette, sf::Color::Black);
	addColorToPalette(palette, sf::Color(0, 0, 128)); // dark blue
	addColorToPalette(palette, sf::Color(128, 0, 0)); // dark red
	addColorToPalette(palette, sf::Color(128, 0, 128)); // dark magenta
	addColorToPalette(palette, sf::Color(0, 128, 0)); // dark green
	addColorToPalette(palette, sf::Color(0, 128, 128)); // dark cyan
	addColorToPalette(palette, sf::Color(128, 128, 0)); // dark yellow
	addColorToPalette(palette, sf::Color(128, 128, 128)); // dark white/medium grey
	addColorToPalette(palette, sf::Color(64, 64, 64)); // light black/dark grey
	addColorToPalette(palette, sf::Color::Blue);
	addColorToPalette(palette, sf::Color::Red);
	addColorToPalette(palette, sf::Color::Magenta);
	addColorToPalette(palette, sf::Color::Green);
	addColorToPalette(palette, sf::Color::Cyan);
	addColorToPalette(palette, sf::Color::Yellow);
	addColorToPalette(palette, sf::Color::White);
}

void addPalette2ColorBlackWhite(std::vector<sf::Color>& palette)
{
	addColorToPalette(palette, sf::Color::Black);
	addColorToPalette(palette, sf::Color::White);
}

void addPalette2ColorWhiteBlack(std::vector<sf::Color>& palette)
{
	addColorToPalette(palette, sf::Color::White);
	addColorToPalette(palette, sf::Color::Black);
}

void addPalette16ColorGreenscale(std::vector<sf::Color>& palette)
{
	for (sf::Uint8 i{ 0 }; i < 16; ++i)
		addColorToPalette(palette, sf::Color(0, i * 17, 0));
}

void addPalette16ColorGrayscale(std::vector<sf::Color>& palette)
{
	for (sf::Uint8 i{ 0 }; i < 16; ++i)
		addColorToPalette(palette, sf::Color(i * 17, i * 17, i * 17));
}

void addPalette16ColorSepia(std::vector<sf::Color>& palette)
{
	const unsigned int numberOfColors{ 16 };
	for (unsigned int i{ 0 }; i < numberOfColors; ++i)
		addColorToPalette(palette, sepiaColor(static_cast<float>(i) / (numberOfColors - 1)));
}

void addPalette16ColorCga(std::vector<sf::Color>& palette)
{
	addColorToPalette(palette, sf::Color(0, 0, 0));
	addColorToPalette(palette, sf::Color(0, 0, 170));
	addColorToPalette(palette, sf::Color(170, 0, 0));
	addColorToPalette(palette, sf::Color(170, 0, 170));
	addColorToPalette(palette, sf::Color(0, 170, 0));
	addColorToPalette(palette, sf::Color(0, 170, 170));
	addColorToPalette(palette, sf::Color(170, 85, 0));
	addColorToPalette(palette, sf::Color(170, 170, 170));
	addColorToPalette(palette, sf::Color(85, 85, 85));
	addColorToPalette(palette, sf::Color(85, 85, 255));
	addColorToPalette(palette, sf::Color(255, 85, 0));
	addColorToPalette(palette, sf::Color(255, 85, 255));
	addColorToPalette(palette, sf::Color(85, 255, 85));
	addColorToPalette(palette, sf::Color(85, 255, 255));
	addColorToPalette(palette, sf::Color(255, 255, 85));
	addColorToPalette(palette, sf::Color(255, 255, 255));
}

void addPalette16ColorWindows(std::vector<sf::Color>& palette)
{
	addColorToPalette(palette, sf::Color(0, 0, 0));
	addColorToPalette(palette, sf::Color(128, 0, 0));
	addColorToPalette(palette, sf::Color(0, 128, 0));
	addColorToPalette(palette, sf::Color(128, 128, 0));
	addColorToPalette(palette, sf::Color(0, 0, 128));
	addColorToPalette(palette, sf::Color(128, 0, 128));
	addColorToPalette(palette, sf::Color(0, 128, 128));
	addColorToPalette(palette, sf::Color(128, 128, 128));
	addColorToPalette(palette, sf::Color(192, 192, 192));
	addColorToPalette(palette, sf::Color(255, 0, 0));
	addColorToPalette(palette, sf::Color(0, 0, 255));
	addColorToPalette(palette, sf::Color(255, 255, 0));
	addColorToPalette(palette, sf::Color(0, 255, 0));
	addColorToPalette(palette, sf::Color(255, 0, 255));
	addColorToPalette(palette, sf::Color(0, 255, 255));
	addColorToPalette(palette, sf::Color(255, 255, 255));
}

void addPalette16ColorMac(std::vector<sf::Color>& palette)
{
	addColorToPalette(palette, sf::Color(255, 255, 255));	// 0
	addColorToPalette(palette, sf::Color(255, 255, 0));		// 1
	addColorToPalette(palette, sf::Color(255, 102, 0));		// 2
	addColorToPalette(palette, sf::Color(221, 0, 0));		// 3
	addColorToPalette(palette, sf::Color(255, 0, 153));		// 4
	addColorToPalette(palette, sf::Color(51, 0, 153));		// 5
	addColorToPalette(palette, sf::Color(0, 0, 204));		// 6
	addColorToPalette(palette, sf::Color(0, 153, 255));		// 7
	addColorToPalette(palette, sf::Color(0, 170, 0));		// 8
	addColorToPalette(palette, sf::Color(0, 102, 0));		// 9
	addColorToPalette(palette, sf::Color(102, 51, 0));		// 10
	addColorToPalette(palette, sf::Color(153, 102, 51));	// 11
	addColorToPalette(palette, sf::Color(187, 187, 187));	// 12
	addColorToPalette(palette, sf::Color(136, 136, 136));	// 13
	addColorToPalette(palette, sf::Color(68, 68, 68));		// 14
	addColorToPalette(palette, sf::Color(0, 0, 0));			// 15
}

void addPalette16ColorZxSpectrum(std::vector<sf::Color>& palette)
{
	addColorToPalette(palette, sf::Color(0, 0, 0));
	addColorToPalette(palette, sf::Color(0, 0, 128));
	addColorToPalette(palette, sf::Color(128, 0, 0));
	addColorToPalette(palette, sf::Color(128, 0, 128));
	addColorToPalette(palette, sf::Color(0, 128, 0));
	addColorToPalette(palette, sf::Color(0, 128, 128));
	addColorToPalette(palette, sf::Color(128, 128, 0));
	addColorToPalette(palette, sf::Color(128, 128, 128));
	addColorToPalette(palette, sf::Color(0, 0, 0));
	addColorToPalette(palette, sf::Color(0, 0, 255));
	addColorToPalette(palette, sf::Color(255, 0, 0));
	addColorToPalette(palette, sf::Color(255, 0, 255));
	addColorToPalette(palette, sf::Color(0, 255, 0));
	addColorToPalette(palette, sf::Color(0, 255, 255));
	addColorToPalette(palette, sf::Color(255, 255, 0));
	addColorToPalette(palette, sf::Color(255, 255, 255));
}

inline void addPalette16ColorCgaNonIbm(std::vector<sf::Color>& palette)
{
	addPalette16ColorZxSpectrum(palette);
}

void addPalette216ColorWebSafe(std::vector<sf::Color>& palette)
{
	for (sf::Uint8 g{ 0 }; g < 6; ++g)
	{
		for (sf::Uint8 r{ 0 }; r < 6; ++r)
		{
			for (sf::Uint8 b{ 0 }; b < 6; ++b)
				addColorToPalette(palette, sf::Color(r * 51, g * 51, b * 51));
		}
	}
}

void addPalette256ColorGreenscale(std::vector<sf::Color>& palette)
{
	for (unsigned int i{ 0 }; i < 256; ++i)
		addColorToPalette(palette, sf::Color(0, static_cast<sf::Uint8>(i), 0));
}

void addPalette256ColorGrayscale(std::vector<sf::Color>& palette)
{
	for (unsigned int i{ 0 }; i < 256; ++i)
		addColorToPalette(palette, sf::Color(static_cast<sf::Uint8>(i), static_cast<sf::Uint8>(i), static_cast<sf::Uint8>(i)));
}

void addPalette256ColorSepia(std::vector<sf::Color>& palette)
{
	const unsigned int numberOfColors{ 256 };
	for (unsigned int i{ 0 }; i < numberOfColors; ++i)
		addColorToPalette(palette, sepiaColor(static_cast<float>(i) / (numberOfColors - 1)));
}

} // namespace

namespace selbaward
{

ConsoleScreenV1::ConsoleScreenV1(const sf::Vector2u mode)
	: m_cells()
	, m_mode(mode)
	, m_buffers()
	, m_do()
	, m_primitiveType(sf::PrimitiveType::Quads)
	, m_display()
	, m_backgroundDisplay()
	, m_size({ 100.f, 100.f })
	, m_texture(nullptr)
	, m_textureOffset({ 0u, 0u })
	, m_tileSize({ 8u, 8u })
	, m_numberOfTilesPerRow(8u)
	, m_colors({ defaultColor, defaultBackgroundColor, defaultCursorColor })
	, m_palette()
	, m_cursor({ 0, static_cast<unsigned int>('_'), true, false, false })
	, m_attributes()
	, m_stretch()
	, m_characterMap()
{
	randomSeed();
	setMode(m_mode);
	loadPalette(Palette::Default);
}

void ConsoleScreenV1::setMode(sf::Vector2u mode)
{
	if (mode.x == 0 || mode.y == 0)
		mode = { 0u, 0u };

	m_mode = mode;
	m_cells.resize(m_mode.x * m_mode.y, defaultCell);
	m_display.resize(m_cells.size() * 4);
	m_backgroundDisplay = m_display;
	m_buffers.clear();

	clear();
}

void ConsoleScreenV1::setTexture(const sf::Texture& texture)
{
	m_texture = &texture;
}

void ConsoleScreenV1::setTexture()
{
	m_texture = nullptr;
}

void ConsoleScreenV1::setSize(const sf::Vector2f size)
{
	m_size = size;

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreenV1::setTextureOffset(const sf::Vector2u textureOffset)
{
	m_textureOffset = textureOffset;

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreenV1::setTextureTileSize(const sf::Vector2u tileSize)
{
	m_tileSize = tileSize;

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreenV1::setNumberOfTextureTilesPerRow(const unsigned int numberOfTextureTilesPerRow)
{
	if (numberOfTextureTilesPerRow < 1)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set number of texture tiles per row to zero.");
		return;
	}

	m_numberOfTilesPerRow = numberOfTextureTilesPerRow;

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreenV1::setThrowExceptions(const bool throwExceptions)
{
	m_do.throwExceptions = throwExceptions;
}

void ConsoleScreenV1::setUpdateAutomatically(const bool updateAutomatically)
{
	m_do.updateAutomatically = updateAutomatically;
}

void ConsoleScreenV1::setShowCursor(const bool showCursor)
{
	m_cursor.visible = showCursor;

	if (m_do.updateAutomatically)
		priv_updateCell(m_cursor.index);
}

void ConsoleScreenV1::setInvertCursor(const bool invertCursor)
{
	m_cursor.inverse = invertCursor;

	if (m_do.updateAutomatically)
		priv_updateCell(m_cursor.index);
}

void ConsoleScreenV1::setUseCursorColor(const bool useCursorColor)
{
	m_cursor.useOwnColour = useCursorColor;

	if (m_do.updateAutomatically)
		priv_updateCell(m_cursor.index);
}

void ConsoleScreenV1::setShowBackground(const bool showBackground)
{
	m_do.showBackround = showBackground;
}

void ConsoleScreenV1::setScrollAutomatically(const bool scrollAutomatically)
{
	m_do.scrollAutomatically = scrollAutomatically;
}

void ConsoleScreenV1::setWrapOnManualScroll(const bool wrapOnManualScroll)
{
	m_do.wrapOnManualScroll = wrapOnManualScroll;
}

void ConsoleScreenV1::update()
{
	if (m_display.size() != (m_mode.x * m_mode.y * 4))
		return;
	if (m_backgroundDisplay.size() != m_display.size())
		m_backgroundDisplay.resize(m_display.size());

	if (m_display.size() < 4)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot update display.\nNo cells available.");
		return;
	}

	for (unsigned int i{ 0 }; i < m_cells.size(); ++i)
		priv_updateCell(i);
}

sf::Vector2f ConsoleScreenV1::getSize() const
{
	return m_size;
}

sf::FloatRect ConsoleScreenV1::getLocalBounds() const
{
	return{ { 0.f, 0.f }, m_size };
}

sf::FloatRect ConsoleScreenV1::getGlobalBounds() const
{
	return getTransform().transformRect(getLocalBounds());
}

sf::Vector2u ConsoleScreenV1::getMode() const
{
	return m_mode;
}

unsigned int ConsoleScreenV1::getNumberOfCells() const
{
	return static_cast<unsigned int>(m_cells.size());
	//return m_mode.x * m_mode.y;
}

sf::Vector2u ConsoleScreenV1::getNumberOfTilesInTexture2d() const
{
	return{ m_numberOfTilesPerRow, m_texture->getSize().y / m_tileSize.y };
}

unsigned int ConsoleScreenV1::getNumberOfTilesInTexture() const
{
	const sf::Vector2u numberOfTiles{ getNumberOfTilesInTexture2d() };
	return numberOfTiles.x * numberOfTiles.y;
}

bool ConsoleScreenV1::getThrowExceptions() const
{
	return m_do.throwExceptions;
}

bool ConsoleScreenV1::getUpdateAutomatically() const
{
	return m_do.updateAutomatically;
}

bool ConsoleScreenV1::getShowCursor() const
{
	return m_cursor.visible;
}

bool ConsoleScreenV1::getInvertCursor() const
{
	return m_cursor.inverse;
}

bool ConsoleScreenV1::getUseCursorColor() const
{
	return m_cursor.useOwnColour;
}

bool ConsoleScreenV1::getShowBackground() const
{
	return m_do.showBackround;
}

bool ConsoleScreenV1::getScrollAutomatically() const
{
	return m_do.scrollAutomatically;
}

bool ConsoleScreenV1::getWrapOnManualScroll() const
{
	return m_do.wrapOnManualScroll;
}

void ConsoleScreenV1::setColor(const sf::Color color)
{
	m_colors.main = color;
}

void ConsoleScreenV1::setColor(const int colorId)
{
	if (priv_isColorIdInPaletteRange(colorId))
		m_colors.main = m_palette[colorId];
	else
	{
		switch (colorId)
		{
		case Color::Invert:
			m_colors.main = invertedColor(m_colors.background);
			break;
		case Color::Ignore:
		case Color::Current:
		default:
			;
		}
	}
}

void ConsoleScreenV1::setBackgroundColor(const sf::Color backgroundColor)
{
	m_colors.background = backgroundColor;
}

void ConsoleScreenV1::setBackgroundColor(const int colorId)
{
	if (priv_isColorIdInPaletteRange(colorId))
		m_colors.background = m_palette[colorId];
	else
	{
		switch (colorId)
		{
		case Color::Invert:
			m_colors.background = invertedColor(m_colors.main);
			break;
		case Color::Ignore:
		case Color::Current:
		default:
			;
		}
	}
}

void ConsoleScreenV1::setCursorColor(const sf::Color cursorColor)
{
	m_colors.cursor = cursorColor;
}

void ConsoleScreenV1::setCursorColor(const int colorId)
{
	if (priv_isColorIdInPaletteRange(colorId))
		m_colors.cursor = m_palette[colorId];
	else
	{
		switch (colorId)
		{
		case Color::Invert:
			m_colors.cursor = invertedColor(m_colors.background);
			break;
		case Color::Current:
			m_colors.cursor = m_colors.main;
			break;
		case Color::Ignore:
		default:
			;
		}
	}
}

void ConsoleScreenV1::setColors(const sf::Color color, const sf::Color backgroundColor)
{
	setColor(color);
	setBackgroundColor(backgroundColor);
}

void ConsoleScreenV1::setColors(const int colorId, const int backgroundColorId)
{
	setColor(colorId);
	setBackgroundColor(backgroundColorId);
}

void ConsoleScreenV1::setColors(const sf::Color color, const sf::Color backgroundColor, const sf::Color cursorColor)
{
	setColors(color, backgroundColor);
	setCursorColor(cursorColor);
}

void ConsoleScreenV1::setColors(const int colorId, const int backgroundColorId, const int cursorColorId)
{
	setColors(colorId, backgroundColorId);
	setCursorColor(cursorColorId);
}

sf::Color ConsoleScreenV1::getColor() const
{
	return m_colors.main;
}

sf::Color ConsoleScreenV1::getBackgroundColor() const
{
	return m_colors.background;
}

sf::Color ConsoleScreenV1::getCursorColor() const
{
	return m_colors.cursor;
}

void ConsoleScreenV1::setStretch(const Stretch stretch)
{
	m_stretch = stretch;
}

ConsoleScreenV1::Stretch ConsoleScreenV1::getStretch() const
{
	return m_stretch;
}

void ConsoleScreenV1::setAttributes(const CellAttributes attributes)
{
	m_attributes = attributes;
}

ConsoleScreenV1::CellAttributes ConsoleScreenV1::getAttributes() const
{
	return m_attributes;
}

void ConsoleScreenV1::setAttribute(const bool attributeValue, const Attribute attribute)
{
	priv_chooseAttribute(m_attributes, attribute) = attributeValue;
}

bool ConsoleScreenV1::getAttribute(const Attribute attribute)
{
	return priv_chooseAttribute(m_attributes, attribute);
}

void ConsoleScreenV1::cursorLeft(const unsigned int distance)
{
	for (unsigned int i{ 0 }; i < distance; ++i)
		priv_moveCursorLeft();
}

void ConsoleScreenV1::cursorRight(const unsigned int distance)
{
	for (unsigned int i{ 0 }; i < distance; ++i)
		priv_moveCursorRight();
}

void ConsoleScreenV1::cursorUp(const unsigned int distance)
{
	for (unsigned int i{ 0 }; i < distance; ++i)
		priv_moveCursorUp();
}

void ConsoleScreenV1::cursorDown(const unsigned int distance)
{
	for (unsigned int i{ 0 }; i < distance; ++i)
		priv_moveCursorDown();
}

void ConsoleScreenV1::moveCursor(const sf::Vector2i offset)
{
	if (offset.x < 0)
		cursorLeft(abs(offset.x));
	else
		cursorRight(offset.x);
	if (offset.y < 0)
		cursorUp(abs(offset.y));
	else
		cursorDown(offset.y);
}

void ConsoleScreenV1::setCursor(const sf::Vector2u location)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set cursor.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	priv_setCursorIndex(priv_cellIndex(location));
}

sf::Vector2u ConsoleScreenV1::getCursor() const
{
	return priv_cellLocation(m_cursor.index);
}

void ConsoleScreenV1::cursorHome()
{
	priv_setCursorIndex(0u);
}

void ConsoleScreenV1::cursorHomeLine()
{
	priv_moveCursorToBeginningOfLine();
}

void ConsoleScreenV1::cursorEnd()
{
	priv_setCursorIndex(static_cast<unsigned int>(m_cells.size()) - 1u);
}

void ConsoleScreenV1::cursorEndLine()
{
	priv_setCursorIndex(m_cursor.index - m_cursor.index % m_mode.x + m_mode.x - 1);
}

void ConsoleScreenV1::cursorTab(const unsigned int tabSize)
{
	const unsigned int targetDistance{ tabSize - (m_cursor.index % tabSize) };
	cursorRight(targetDistance);
}

void ConsoleScreenV1::cursorTabReverse(const unsigned int tabSize)
{
	if (m_cursor.index == 0)
		return;

	const unsigned int targetDistance{ (m_cursor.index - 1) % tabSize + 1 };
	cursorLeft(targetDistance);
}

void ConsoleScreenV1::cursorNextline()
{
	priv_moveCursorDown();
	priv_moveCursorToBeginningOfLine();
}

void ConsoleScreenV1::cursorBackspace()
{
	priv_moveCursorLeft();
	priv_clearCell(m_cursor.index, true, true);
}

void ConsoleScreenV1::setCursor(const int cellValue)
{
	m_cursor.value = cellValue;

	if (m_do.updateAutomatically)
		priv_updateCell(m_cursor.index);
}

void ConsoleScreenV1::setCursor(const char cellCharacter, bool mapCharacter)
{
	m_cursor.value = (mapCharacter && getIsMappedCharacter(cellCharacter)) ? getMappedCharacter(cellCharacter) : cellCharacter;

	if (m_do.updateAutomatically)
		priv_updateCell(m_cursor.index);
}

void ConsoleScreenV1::print(const char character, const int colorId, const int backgroundColorId)
{
	if (!priv_isCursorInRange())
		return;

	print(character, m_cells[m_cursor.index].attributes, colorId, backgroundColorId);
}

void ConsoleScreenV1::print(const char character, const Stretch& stretch, const int colorId, const int backgroundColorId)
{
	if (!priv_isCursorInRange())
		return;

	const unsigned int currentIndex{ m_cursor.index };
	const unsigned int cellValue{ priv_getCellValueFromCharacter(character) };
	if (colorId == Color::Invert || colorId == Color::Contrast)
	{
		poke(currentIndex, Cell{ cellValue, sf::Color::Black, priv_backgroundColorFromColorIdAtIndex(currentIndex, backgroundColorId), stretch });
		poke(currentIndex, priv_colorFromColorIdAtIndex(currentIndex, colorId));
	}
	else
		poke(currentIndex, Cell{ cellValue, priv_colorFromColorIdAtIndex(currentIndex, colorId), priv_backgroundColorFromColorIdAtIndex(currentIndex, backgroundColorId), stretch });
	priv_moveCursorRight();

	if (m_do.updateAutomatically)
	{
		priv_updateCell(currentIndex);
		priv_updateCell(m_cursor.index);
	}
}


void ConsoleScreenV1::print(const char character, const CellAttributes& attributes, const int colorId, const int backgroundColorId)
{
	if (!priv_isCursorInRange())
		return;

	const unsigned int currentIndex{ m_cursor.index };
	const unsigned int cellValue{ priv_getCellValueFromCharacter(character) };
	const Cell existingCell = peek(currentIndex);
	if (colorId == Color::Invert || colorId == Color::Contrast)
	{
		poke(currentIndex, Cell{ cellValue, sf::Color::Black, priv_backgroundColorFromColorIdAtIndex(currentIndex, backgroundColorId), existingCell.stretch, attributes });
		poke(currentIndex, priv_colorFromColorIdAtIndex(currentIndex, colorId));
	}
	else
		poke(currentIndex, Cell{ cellValue, priv_colorFromColorIdAtIndex(currentIndex, colorId), priv_backgroundColorFromColorIdAtIndex(currentIndex, backgroundColorId), existingCell.stretch, attributes });
	priv_moveCursorRight();

	if (m_do.updateAutomatically)
	{
		priv_updateCell(currentIndex);
		priv_updateCell(m_cursor.index);
	}
}

void ConsoleScreenV1::print(const std::string& string, const int colorId, const int backgroundColorId)
{
	for (auto& character : string)
		print(character, colorId, backgroundColorId);

	if (!priv_isCursorInRange())
		priv_setCursorIndex(static_cast<unsigned int>(m_cells.size()) - 1u);
}

void ConsoleScreenV1::print(const std::string& string, const Stretch& stretch, const int colorId, const int backgroundColorId)
{
	for (auto& character : string)
		print(character, stretch, colorId, backgroundColorId);

	if (!priv_isCursorInRange())
		priv_setCursorIndex(static_cast<unsigned int>(m_cells.size()) - 1u);
}

void ConsoleScreenV1::print(const std::string& string, const CellAttributes& attributes, const int colorId, const int backgroundColorId)
{
	for (auto& character : string)
		print(character, attributes, colorId, backgroundColorId);

	if (!priv_isCursorInRange())
		priv_setCursorIndex(static_cast<unsigned int>(m_cells.size()) - 1u);
}

void ConsoleScreenV1::printLine(const std::string& string, const int colorId, const int backgroundColorId)
{
	print(string, colorId, backgroundColorId);
	cursorNextline();
}

void ConsoleScreenV1::clearCellAt(const sf::Vector2u location)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot clear cell.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	priv_clearCell(priv_cellIndex(location), m_colors.main, m_colors.background);

	if (m_do.updateAutomatically)
		priv_updateCell(priv_cellIndex(location));
}

void ConsoleScreenV1::setCellAt(const sf::Vector2u location, const Cell& cell)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set cell.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	m_cells[priv_cellIndex(location)] = cell;

	if (m_do.updateAutomatically)
		priv_updateCell(priv_cellIndex(location));
}

void ConsoleScreenV1::printAt(const sf::Vector2u location, const std::string& string, const sf::Color color, const sf::Color backgroundColor)
{
	if (string.size() == 0)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot print.\nString does not exist.");
		return;
	}

	for (unsigned int i{ 0 }; i < string.size(); ++i)
		printAt({ location.x + i, location.y }, string[i], color, backgroundColor); // print single character
}

void ConsoleScreenV1::printAt(const sf::Vector2u location, const std::string& string, const sf::Color color, const int backgroundColorId)
{
	if (string.size() == 0)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot print.\nString does not exist.");
		return;
	}

	for (unsigned int i{ 0 }; i < string.size(); ++i)
		printAt({ location.x + i, location.y }, string[i], color, backgroundColorId); // print single character
}

void ConsoleScreenV1::printAt(const sf::Vector2u location, const std::string& string, const int colorId, const sf::Color backgroundColor)
{
	if (string.size() == 0)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot print.\nString does not exist.");
		return;
	}

	for (unsigned int i{ 0 }; i < string.size(); ++i)
		printAt({ location.x + i, location.y }, string[i], colorId, backgroundColor); // print single character
}

void ConsoleScreenV1::printAt(const sf::Vector2u location, const std::string& string, const int colorId, const int backgroundColorId)
{
	if (string.size() == 0)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot print.\nString does not exist.");
		return;
	}

	for (unsigned int i{ 0 }; i < string.size(); ++i)
		printAt({ location.x + i, location.y }, string[i], colorId, backgroundColorId); // print single character
}

void ConsoleScreenV1::printAt(const sf::Vector2u location, const char character, const sf::Color color, const sf::Color backgroundColor)
{
	unsigned int index{ priv_getPrintIndex(location) };
	Cell cell = defaultCell;
	cell.value = priv_getCellValueFromCharacter(character);
	cell.color = color;
	cell.backgroundColor = backgroundColor;
	m_cells[index] = cell;

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

void ConsoleScreenV1::printAt(const sf::Vector2u location, const char character, const sf::Color color, const int backgroundColorId)
{
	const unsigned int currentIndex{ priv_getPrintIndex(location) };
	printAt(location, character, color, priv_backgroundColorFromColorIdAtIndex(currentIndex, backgroundColorId));
}

void ConsoleScreenV1::printAt(const sf::Vector2u location, const char character, const int colorId, const sf::Color backgroundColor)
{
	const unsigned int currentIndex{ priv_getPrintIndex(location) };
	printAt(location, character, priv_colorFromColorIdAtIndex(currentIndex, colorId), backgroundColor);
}

void ConsoleScreenV1::printAt(const sf::Vector2u location, const char character, const int colorId, const int backgroundColorId)
{
	const unsigned int currentIndex{ priv_getPrintIndex(location) };
	printAt(location, character, priv_colorFromColorIdAtIndex(currentIndex, colorId), priv_backgroundColorFromColorIdAtIndex(currentIndex, backgroundColorId));
}

void ConsoleScreenV1::printStretchedAt(const sf::Vector2u location, const std::string& string, const Stretch& stretchAttribute, const sf::Color color, const sf::Color backgroundColor)
{
	if (string.size() == 0)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot print.\nString does not exist.");
		return;
	}

	for (unsigned int i{ 0 }; i < string.size(); ++i)
		printStretchedAt({ location.x + i, location.y }, string[i], stretchAttribute, color, backgroundColor); // print single character
}

void ConsoleScreenV1::printStretchedAt(const sf::Vector2u location, const std::string& string, const Stretch& stretchAttribute, const int colorId, const int backgroundColorId)
{
	if (string.size() == 0)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot print.\nString does not exist.");
		return;
	}

	for (unsigned int i{ 0 }; i < string.size(); ++i)
		printStretchedAt({ location.x + i, location.y }, string[i], stretchAttribute, colorId, backgroundColorId); // print single character
}

void ConsoleScreenV1::printStretchedAt(sf::Vector2u location, const char character, const Stretch& stretchAttribute, const sf::Color color, const sf::Color backgroundColor)
{
	switch (stretchAttribute)
	{
	case Stretch::Bottom:
		if (location.y < 1)
			return;
		--location.y;
	case Stretch::Top:
		if (location.y > m_mode.y - 2)
			return;
		break;
	case Stretch::None:
	default:
		return;
	}
	unsigned int topIndex{ priv_getPrintIndex(location) };
	unsigned int bottomIndex{ topIndex + m_mode.x };
	const unsigned int cellValue{ priv_getCellValueFromCharacter(character) };

	m_cells[topIndex] = defaultCell;
	m_cells[topIndex].value = cellValue;
	m_cells[topIndex].color = color;
	m_cells[topIndex].backgroundColor = backgroundColor;
	m_cells[topIndex].stretch = Stretch::Top;
	m_cells[bottomIndex] = defaultCell;
	m_cells[bottomIndex].value = cellValue;
	m_cells[bottomIndex].color = color;
	m_cells[bottomIndex].backgroundColor = backgroundColor;
	m_cells[bottomIndex].stretch = Stretch::Bottom;

	if (m_do.updateAutomatically)
	{
		priv_updateCell(topIndex);
		priv_updateCell(bottomIndex);
	}
}

void ConsoleScreenV1::printStretchedAt(sf::Vector2u location, const char character, const Stretch& stretchAttribute, const int colorId, const int backgroundColorId)
{
	switch (stretchAttribute)
	{
	case Stretch::Bottom:
		if (location.y < 1)
			return;
		--location.y;
	case Stretch::Top:
		if (location.y > m_mode.y - 2)
			return;
		break;
	case Stretch::None:
	default:
		return;
	}
	unsigned int topIndex{ priv_getPrintIndex(location) };
	unsigned int bottomIndex{ topIndex + m_mode.x };
	const unsigned int cellValue{ priv_getCellValueFromCharacter(character) };

	m_cells[topIndex].value = cellValue;
	m_cells[topIndex].color = priv_colorFromColorIdAtIndex(topIndex, colorId);
	m_cells[topIndex].backgroundColor = priv_backgroundColorFromColorIdAtIndex(topIndex, backgroundColorId);
	m_cells[topIndex].stretch = Stretch::Top;
	m_cells[topIndex].attributes = CellAttributes();
	m_cells[bottomIndex].value = cellValue;
	m_cells[bottomIndex].color = priv_colorFromColorIdAtIndex(bottomIndex, colorId);
	m_cells[bottomIndex].backgroundColor = priv_backgroundColorFromColorIdAtIndex(bottomIndex, backgroundColorId);
	m_cells[bottomIndex].stretch = Stretch::Bottom;
	m_cells[bottomIndex].attributes = CellAttributes();

	if (m_do.updateAutomatically)
	{
		priv_updateCell(topIndex);
		priv_updateCell(bottomIndex);
	}
}

void ConsoleScreenV1::paintAt(const sf::Vector2u location, const unsigned int length, const sf::Color color, const sf::Color backgroundColor)
{
	for (unsigned int i{ 0 }; i < length; ++i)
		priv_paintCell(priv_getPrintIndex({ location.x + i, location.y }), color, backgroundColor);
}

void ConsoleScreenV1::paintAt(const sf::Vector2u location, const unsigned int length, const sf::Color color, const int backgroundColorId)
{
	for (unsigned int i{ 0 }; i < length; ++i)
	{
		const unsigned int index{ priv_getPrintIndex({ location.x + i, location.y }) };
		priv_paintCell(index, color, priv_backgroundColorFromColorIdAtIndex(index, backgroundColorId));
	}
}

void ConsoleScreenV1::paintAt(const sf::Vector2u location, const unsigned int length, const int colorId, const sf::Color backgroundColor)
{
	for (unsigned int i{ 0 }; i < length; ++i)
	{
		const unsigned int index{ priv_getPrintIndex({ location.x + i, location.y }) };
		priv_paintCell(index, priv_colorFromColorIdAtIndex(index, colorId), backgroundColor);
	}
}

void ConsoleScreenV1::paintAt(const sf::Vector2u location, const unsigned int length, const int colorId, const int backgroundColorId)
{
	for (unsigned int i{ 0 }; i < length; ++i)
	{
		const unsigned int index{ priv_getPrintIndex({ location.x + i, location.y }) };
		priv_paintCell(index, priv_colorFromColorIdAtIndex(index, colorId), priv_backgroundColorFromColorIdAtIndex(index, backgroundColorId));
	}
}

void ConsoleScreenV1::paintAttributeAt(const sf::Vector2u location, const unsigned int length, const bool attributeValue, const Attribute attribute)
{
	for (unsigned int i{ 0 }; i < length; ++i)
	{
		unsigned int index{ priv_getPrintIndex({ location.x + i, location.y }) };
		priv_chooseAttribute(m_cells[index].attributes, attribute) = attributeValue;

		if (m_do.updateAutomatically)
			priv_updateCell(index);
	}
}

std::string ConsoleScreenV1::read(const unsigned int length, const bool unmapCharacters)
{
	std::string string;
	
	for (unsigned int i{ 0u }; i < length; ++i)
	{
		if (!priv_isCursorInRange())
			break;;

		const unsigned int currentIndex{ m_cursor.index };
		string += (unmapCharacters ? priv_getCharacterFromCellValue(m_cells[currentIndex].value) : static_cast<char>(m_cells[currentIndex].value));
		priv_moveCursorRight();
	}

	return string;
}

std::string ConsoleScreenV1::readAt(const sf::Vector2u location, const unsigned int length, const bool unmapCharacters) const
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot read.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return "";
	}

	std::string string;
	unsigned int index{ priv_getPrintIndex(location) };

	for (unsigned int i{ 0u }; i < length; ++i)
	{
		string += (unmapCharacters ? priv_getCharacterFromCellValue(m_cells[index].value) : static_cast<char>(m_cells[index].value));
		++index;
		if (!priv_isCellIndexInRange(index))
			break;
	}

	return string;
}

void ConsoleScreenV1::setValueAt(const sf::Vector2u location, const unsigned int value)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set value.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	m_cells[priv_cellIndex(location)].value = value;

	if (m_do.updateAutomatically)
		priv_updateCell(priv_cellIndex(location));
}

void ConsoleScreenV1::setColorsAt(const sf::Vector2u location, const sf::Color color, const sf::Color backgroundColor)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set colors.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	const unsigned int index{ priv_cellIndex(location) };
	m_cells[index].color = color;
	m_cells[index].backgroundColor = backgroundColor;

	if (m_do.updateAutomatically)
		priv_updateCell(index);

}

void ConsoleScreenV1::setColorsAt(const sf::Vector2u location, const int colorId, const int backgroundColorId)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set colors.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	const unsigned int index{ priv_cellIndex(location) };
	m_cells[index].color = priv_colorFromColorIdAtIndex(index, colorId);
	m_cells[index].backgroundColor = priv_backgroundColorFromColorIdAtIndex(index, backgroundColorId);

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

void ConsoleScreenV1::setColorAt(const sf::Vector2u location, const sf::Color color)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set color.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	const unsigned int index{ priv_cellIndex(location) };
	m_cells[index].color = color;

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

void ConsoleScreenV1::setColorAt(const sf::Vector2u location, const int colorId)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set color.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	const unsigned int index{ priv_cellIndex(location) };
	m_cells[index].color = priv_colorFromColorIdAtIndex(index, colorId);

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

void ConsoleScreenV1::setBackgroundColorAt(const sf::Vector2u location, const sf::Color backgroundColor)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set background colors.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	const unsigned int index{ priv_cellIndex(location) };
	m_cells[index].backgroundColor = backgroundColor;

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

void ConsoleScreenV1::setBackgroundColorAt(const sf::Vector2u location, const int backgroundColorId)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set background colors.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	const unsigned int index{ priv_cellIndex(location) };
	m_cells[index].backgroundColor = priv_backgroundColorFromColorIdAtIndex(index, backgroundColorId);

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

void ConsoleScreenV1::setStretchAt(const sf::Vector2u location, const Stretch& stretch)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set stretch.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	m_cells[priv_cellIndex(location)].stretch = stretch;

	if (m_do.updateAutomatically)
		priv_updateCell(priv_cellIndex(location));
}

void ConsoleScreenV1::setAttributesAt(const sf::Vector2u location, const CellAttributes& attributes)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set attributes.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	m_cells[priv_cellIndex(location)].attributes = attributes;

	if (m_do.updateAutomatically)
		priv_updateCell(priv_cellIndex(location));
}

void ConsoleScreenV1::setAttributeAt(const sf::Vector2u location, const bool attributeValue, const Attribute attribute)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set attribute.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	priv_chooseAttribute(m_cells[priv_cellIndex(location)].attributes, attribute) = attributeValue;
	
	if (m_do.updateAutomatically)
		priv_updateCell(priv_cellIndex(location));
}

ConsoleScreenV1::Cell ConsoleScreenV1::getCellAt(const sf::Vector2u location) const
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot get cell.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return Cell();
	}

	return m_cells[priv_cellIndex(location)];
}

unsigned int ConsoleScreenV1::getValueAt(const sf::Vector2u location) const
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot get cell value.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return 0u;
	}

	return m_cells[priv_cellIndex(location)].value;
}

sf::Color ConsoleScreenV1::getColorAt(const sf::Vector2u location) const
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot get cell color.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return sf::Color::Transparent;
	}

	return m_cells[priv_cellIndex(location)].color;
}

sf::Color ConsoleScreenV1::getBackgroundColorAt(const sf::Vector2u location) const
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot get cell background color.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return sf::Color::Transparent;
	}

	return m_cells[priv_cellIndex(location)].backgroundColor;
}

ConsoleScreenV1::Stretch ConsoleScreenV1::getStretchAt(const sf::Vector2u location) const
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot get cell stretch.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return Stretch::None;
	}

	return m_cells[priv_cellIndex(location)].stretch;
}

ConsoleScreenV1::CellAttributes ConsoleScreenV1::getAttributesAt(const sf::Vector2u location) const
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot get cell attributes.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return CellAttributes();
	}

	return m_cells[priv_cellIndex(location)].attributes;
}

bool ConsoleScreenV1::getAttributeAt(const sf::Vector2u location, const Attribute attribute)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot get cell attribute.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return false;
	}

	return priv_chooseAttribute(m_cells[priv_cellIndex(location)].attributes, attribute);
}

void ConsoleScreenV1::scrollUp(const unsigned int amount)
{
	if (m_mode.y == 0 || amount == 0)
		return;

	std::vector<Cell> topRow(m_mode.x);
	for (unsigned int repeat{ 0 }; repeat < amount; ++repeat) // lazy way of scrolling multiple times - loop scrolling (entirely by 1 each time)
	{
		for (unsigned int y{ 0 }; y < m_mode.y; ++y)
		{
			for (unsigned int x{ 0 }; x < m_mode.x; ++x)
			{
				if (m_do.wrapOnManualScroll && y == 0)
					topRow[x] = m_cells[x];
				if (y < m_mode.y - 1)
					m_cells[priv_cellIndex({ x, y })] = m_cells[priv_cellIndex({ x, y + 1 })];
				else if (m_do.wrapOnManualScroll)
					m_cells[priv_cellIndex({ x, y })] = topRow[x];
				else
					priv_clearCell(priv_cellIndex({ x, y }), true, true);
			}
		}
	}

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreenV1::scrollDown(const unsigned int amount)
{
	if (m_mode.y == 0 || amount == 0)
		return;

	std::vector<Cell> bottomRow(m_mode.x);
	for (unsigned int repeat{ 0 }; repeat < amount; ++repeat) // lazy way of scrolling multiple times - loop scrolling (entirely by 1 each time)
	{
		for (unsigned int y{ 0 }; y < m_mode.y; ++y)
		{
			for (unsigned int x{ 0 }; x < m_mode.x; ++x)
			{
				const unsigned cellY{ m_mode.y - y - 1 };
				if (m_do.wrapOnManualScroll && y == 0)
					bottomRow[x] = m_cells[priv_cellIndex({ x, cellY })];
				if (cellY > 0)
					m_cells[priv_cellIndex({ x, cellY })] = m_cells[priv_cellIndex({ x, cellY - 1 })];
				else if (m_do.wrapOnManualScroll)
					m_cells[priv_cellIndex({ x, cellY })] = bottomRow[x];
				else
					priv_clearCell(priv_cellIndex({ x, cellY }), true, true);
			}
		}
	}

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreenV1::scrollLeft(const unsigned int amount)
{
	if (m_mode.x == 0 || amount == 0)
		return;

	Cell leftCell;
	for (unsigned int repeat{ 0 }; repeat < amount; ++repeat) // lazy way of scrolling multiple times - loop scrolling (entirely by 1 each time)
	{
		for (unsigned int cell{ 0 }; cell < m_cells.size(); ++cell)
		{
			if (cell % m_mode.x == 0)
				leftCell = m_cells[cell];
			if (cell % m_mode.x == m_mode.x - 1)
			{
				if (m_do.wrapOnManualScroll)
					m_cells[cell] = leftCell;
				else
					priv_clearCell(cell, true, true);
			}
			else
				m_cells[cell] = m_cells[cell + 1];
		}
	}

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreenV1::scrollRight(const unsigned int amount)
{
	if (m_mode.x == 0 || amount == 0)
		return;

	Cell rightCell;
	for (unsigned int repeat{ 0 }; repeat < amount; ++repeat) // lazy way of scrolling multiple times - loop scrolling (entirely by 1 each time)
	{
		for (unsigned int i{ 0 }; i < m_cells.size(); ++i)
		{
			const unsigned int cell{ static_cast<unsigned int>(m_cells.size()) - i - 1 };
			if (i % m_mode.x == 0)
				rightCell = m_cells[cell];
			if (cell % m_mode.x == 0)
			{
				if (m_do.wrapOnManualScroll)
					m_cells[cell] = rightCell;
				else
					priv_clearCell(cell, true, true);
			}
			else
				m_cells[cell] = m_cells[cell - 1];
		}
	}

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreenV1::clear(const int backgroundColorId)
{
	if (priv_isColorIdInPaletteRange(backgroundColorId))
		clear(m_palette[backgroundColorId]);
	else
	{
		switch (backgroundColorId)
		{
		case Color::Contrast:
			clear(contrastedColor(m_colors.main));
			break;
		case Color::Invert:
			clear(invertedColor(m_colors.main));
			break;
		case Color::Ignore:
		case Color::Current:
		default:
			clear(m_colors.background);
		}
	}
}

void ConsoleScreenV1::clear(const sf::Color backgroundColor)
{
	if (m_cells.size() == 0)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot clear cell.\nNo cells exist.");
		return;
	}

	for (unsigned int i{ 0 }; i < m_cells.size(); ++i)
		priv_clearCell(i, m_colors.main, backgroundColor);
	cursorHome();

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreenV1::crash()
{
	for (auto& cell : m_cells)
	{
		cell = { randomByte(), sf::Color(randomByte(), randomByte(), randomByte()), sf::Color(randomByte(), randomByte(), randomByte()) };
	}
	cursorEnd();

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreenV1::loadPalette(const Palette palette)
{
	m_palette.clear();
	switch (palette)
	{
	case Palette::Colors16Cga:
		addPalette16ColorCga(m_palette);
		break;
	case Palette::Colors16CgaNonIbm:
		addPalette16ColorCgaNonIbm(m_palette);
		break;
	case Palette::Colors16Greenscale:
		addPalette16ColorGreenscale(m_palette);
		break;
	case Palette::Colors16Grayscale:
		addPalette16ColorGrayscale(m_palette);
		break;
	case Palette::Colors16Sepia:
		addPalette16ColorSepia(m_palette);
		break;
	case Palette::Colors16Windows:
		addPalette16ColorWindows(m_palette);
		break;
	case Palette::Colors16Mac:
		addPalette16ColorMac(m_palette);
		break;
	case Palette::Colors16ZxSpectrum:
		addPalette16ColorZxSpectrum(m_palette);
		break;
	case Palette::Colors216Web:
		addPalette216ColorWebSafe(m_palette);
		break;
	case Palette::Colors256Greenscale:
		addPalette256ColorGreenscale(m_palette);
		break;
	case Palette::Colors256Grayscale:
		addPalette256ColorGrayscale(m_palette);
		break;
	case Palette::Colors256Sepia:
		addPalette256ColorSepia(m_palette);
		break;
	case Palette::Default:
	default:
		addPaletteDefault(m_palette);
	}
}

void ConsoleScreenV1::addColorToPalette(const sf::Color color)
{
	m_palette.emplace_back(color);
}

void ConsoleScreenV1::setPaletteColor(const int colorId, const sf::Color color)
{
	if (!priv_isColorIdInPaletteRange(colorId))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set palette color.\nColor ID (" + std::to_string(colorId) + ") out of range.");
		return;
	}

	m_palette[colorId] = color;
}

sf::Color ConsoleScreenV1::getPaletteColor(const int colorId) const
{
	if (!priv_isColorIdInPaletteRange(colorId))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot get palette color.\nColor ID (" + std::to_string(colorId) + ") out of range.");
		return defaultNewPaletteColor;
	}

	return m_palette[colorId];
}

void ConsoleScreenV1::setPaletteSize(const unsigned int size)
{
	if (size < 1)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set palette size to zero.");
		return;
	}

	m_palette.resize(size);
}

unsigned int ConsoleScreenV1::getPaletteSize() const
{
	return static_cast<unsigned int>(m_palette.size());
}

void ConsoleScreenV1::removePaletteColor(const int colorId)
{
	if (!priv_isColorIdInPaletteRange(colorId))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot remove palette color.\nColor ID (" + std::to_string(colorId) + ") out of range.");
		return;
	}
	if (m_palette.size() < 2)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot remove final palette color.");
		return;
	}

	m_palette.erase(m_palette.begin() + colorId);
}

unsigned int ConsoleScreenV1::copy()
{
	m_buffers.push_back({ m_mode.x, m_cells });
	return static_cast<unsigned int>(m_buffers.size()) - 1u;
}

void ConsoleScreenV1::copy(const unsigned int index)
{
	if (!priv_isScreenBufferIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot replace buffer with copy.\nBuffer index (" + std::to_string(index) + ") does not exist.");
		return;
	}

	m_buffers[index] = { m_mode.x, m_cells };

	if (m_do.updateAutomatically)
		update();
}

unsigned int ConsoleScreenV1::copy(const sf::IntRect selectionRectangle)
{
	m_buffers.emplace_back();
	priv_copyToBufferFromSelectionRectangle(m_buffers.back(), selectionRectangle);
	return static_cast<unsigned int>(m_buffers.size()) - 1u;
}

void ConsoleScreenV1::copy(const unsigned int index, const sf::IntRect selectionRectangle)
{
	if (!priv_isScreenBufferIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot replace buffer with copy.\nBuffer index (" + std::to_string(index) + ") does not exist.");
		return;
	}

	priv_copyToBufferFromSelectionRectangle(m_buffers[index], selectionRectangle);
}

void ConsoleScreenV1::paste(const sf::Vector2i offset)
{
	if (m_buffers.size() == 0)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot paste buffer.\nNo buffer exists.");
		return;
	}

	priv_pasteOffsettedBuffer(m_buffers.back(), offset);
}

void ConsoleScreenV1::paste(const unsigned int index, const sf::Vector2i offset)
{
	if (!priv_isScreenBufferIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot paste buffer.\nBuffer index (" + std::to_string(index) + ") out of range.");
		return;
	}

	priv_pasteOffsettedBuffer(m_buffers[index], offset);
}

void ConsoleScreenV1::removeBuffer()
{
	if (m_buffers.size() == 0)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot remove buffer.\nNo buffer exists.");
		return;
	}

	m_buffers.pop_back();
}

void ConsoleScreenV1::removeBuffer(const unsigned int index)
{
	if (!priv_isScreenBufferIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot remove buffer.\nBuffer index (" + std::to_string(index) + ") out of range.");
		return;
	}

	m_buffers.erase(m_buffers.begin() + index);
}

void ConsoleScreenV1::removeAllBuffers()
{
	m_buffers.clear();
}

unsigned int ConsoleScreenV1::getNumberOfBuffers() const
{
	return static_cast<unsigned int>(m_buffers.size());
}

void ConsoleScreenV1::setMappedCharacter(const char character, const unsigned int value)
{
	m_characterMap[character] = value;
}

void ConsoleScreenV1::setMappedCharacters(const std::string& characters, unsigned int initialValue)
{
	for (auto& character : characters)
		setMappedCharacter(character, initialValue++);
}

void ConsoleScreenV1::removeMappedCharacter(const char character)
{
	m_characterMap.erase(character);
}

void ConsoleScreenV1::removeMappedCharacters(const std::string& characters)
{
	for (auto& character : characters)
		removeMappedCharacter(character);
}

bool ConsoleScreenV1::getIsMappedCharacter(const char character) const
{
	return m_characterMap.find(character) != m_characterMap.end();
}

unsigned int ConsoleScreenV1::getMappedCharacter(const char character) const
{
	return m_characterMap.at(character);
}

void ConsoleScreenV1::poke(const unsigned int index, const Cell& cell)
{
	if (!priv_isCellIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot poke cell.\nCell number (" + std::to_string(index) + ") out of range.");
		return;
	}

	m_cells[index] = cell;

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

void ConsoleScreenV1::poke(const unsigned int index, const unsigned int value)
{
	if (!priv_isCellIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot poke value.\nCell number (" + std::to_string(index) + ") out of range.");
		return;
	}

	m_cells[index].value = value;

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

void ConsoleScreenV1::poke(const unsigned int index, const sf::Color& color)
{
	if (!priv_isCellIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot poke color.\nCell number (" + std::to_string(index) + ") out of range.");
		return;
	}

	m_cells[index].color = color;

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

void ConsoleScreenV1::poke(const unsigned int index, const sf::Color& color, const sf::Color& backgroundColor)
{
	if (!priv_isCellIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot poke colors.\nCell number (" + std::to_string(index) + ") out of range.");
		return;
	}

	m_cells[index].color = color;
	m_cells[index].backgroundColor = backgroundColor;

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

void ConsoleScreenV1::poke(const unsigned int index, const Stretch& stretch)
{
	if (!priv_isCellIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot poke stretch attribute.\nCell number (" + std::to_string(index) + ") out of range.");
		return;
	}

	m_cells[index].stretch = stretch;

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

void ConsoleScreenV1::poke(const unsigned int index, const CellAttributes& cellAttributes)
{
	if (!priv_isCellIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot poke attributes.\nCell number (" + std::to_string(index) + ") out of range.");
		return;
	}

	m_cells[index].attributes = cellAttributes;

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

void ConsoleScreenV1::poke(const unsigned int index, const bool attributeValue, const Attribute attribute)
{
	if (!priv_isCellIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot poke attribute.\nCell number (" + std::to_string(index) + ") out of range.");
		return;
	}

	priv_chooseAttribute(m_cells[index].attributes, attribute) = attributeValue;

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

ConsoleScreenV1::Cell ConsoleScreenV1::peek(const unsigned int index) const
{
	if (!priv_isCellIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot peek cell.\nCell number (" + std::to_string(index) + ") out of range.");
		return Cell();
	}

	return m_cells[index];
}



// PRIVATE

void ConsoleScreenV1::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();

	if (m_do.showBackround && m_backgroundDisplay.size() > 0)
	{
		states.texture = nullptr;
		target.draw(&m_backgroundDisplay.front(), m_backgroundDisplay.size(), m_primitiveType, states);
	}
	if (m_display.size() > 0)
	{
		states.texture = m_texture;
		target.draw(&m_display.front(), m_display.size(), m_primitiveType, states);
	}
}

void ConsoleScreenV1::priv_updateCell(const unsigned int index)
{
	if (!priv_isCellIndexInRange(index))
		return;

	if (m_display.size() != (m_mode.x * m_mode.y * 4))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Bug: display does not match cells.");
		return;
	}

	if (m_display.size() < 4)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Bug: cannot update cell. No cells available.");
		return;
	}

	Cell& cell{ m_cells[index] };

	unsigned int cellValue{ cell.value };
	sf::Color cellColor{ cell.color };
	sf::Color backgroundColor{ cell.backgroundColor };

	const bool isCursor{ m_cursor.visible && m_cursor.index == index };
	bool useCursorValue{ false };

	if (isCursor)
	{
		if (m_cursor.value >= 0)
		{
			useCursorValue = true;
			cellValue = m_cursor.value;
		}
		if (m_cursor.useOwnColour)
			cellColor = m_colors.cursor;
		if (cell.attributes.inverse != m_cursor.inverse)
			swapColors(cellColor, backgroundColor);
	}
	else if (cell.attributes.inverse)
		swapColors(cellColor, backgroundColor);

	if (!cell.attributes.bright)
	{
		makeColorUnBright(cellColor);
		makeColorUnBright(backgroundColor);
	}

	const unsigned int cellX{ index % m_mode.x };
	const unsigned int cellY{ index / m_mode.x };
	const float left{ linearInterpolation(0.f, m_size.x, static_cast<float>(cellX + (cell.attributes.flipX ? 1 : 0)) / m_mode.x) };
	const float right{ linearInterpolation(0.f, m_size.x, static_cast<float>(cellX + (cell.attributes.flipX ? 0 : 1)) / m_mode.x) };
	const float top{ linearInterpolation(0.f, m_size.y, static_cast<float>(cellY + (cell.attributes.flipY ? 1 : 0)) / m_mode.y) };
	const float bottom{ linearInterpolation(0.f, m_size.y, static_cast<float>(cellY + (cell.attributes.flipY ? 0 : 1)) / m_mode.y) };

	sf::Vector2u textureCell{ cellValue % m_numberOfTilesPerRow, cellValue / m_numberOfTilesPerRow };
	const float textureLeft{ static_cast<float>(m_textureOffset.x + textureCell.x * m_tileSize.x) };
	const float textureTop{ static_cast<float>(m_textureOffset.y + (textureCell.y + (!useCursorValue && cell.stretch == Stretch::Bottom ? 0.5f : 0.f)) * m_tileSize.y) };
	const float textureRight{ static_cast<float>(m_textureOffset.x + (textureCell.x + 1) * m_tileSize.x) };
	const float textureBottom{ static_cast<float>(m_textureOffset.y + (textureCell.y + (!useCursorValue && cell.stretch == Stretch::Top ? 0.5f : 1.f)) * m_tileSize.y) };

	const unsigned int baseVertex{ index * 4 };
	sf::Vertex* const pVertex1{ &m_display[baseVertex] };
	sf::Vertex* const pVertex2{ pVertex1 + 1 };
	sf::Vertex* const pVertex3{ pVertex1 + 2 };
	sf::Vertex* const pVertex4{ pVertex1 + 3 };
	sf::Vertex* const pBackgroundVertex1{ &m_backgroundDisplay[baseVertex] };
	sf::Vertex* const pBackgroundVertex2{ pBackgroundVertex1 + 1 };
	sf::Vertex* const pBackgroundVertex3{ pBackgroundVertex1 + 2 };
	sf::Vertex* const pBackgroundVertex4{ pBackgroundVertex1 + 3 };

	// main display
	pVertex1->position = { left, top };
	pVertex2->position = { right, top };
	pVertex3->position = { right, bottom };
	pVertex4->position = { left, bottom };

	pVertex1->texCoords = { textureLeft, textureTop };
	pVertex2->texCoords = { textureRight, textureTop };
	pVertex3->texCoords = { textureRight, textureBottom };
	pVertex4->texCoords = { textureLeft, textureBottom };

	pVertex1->color = cellColor;
	pVertex2->color = cellColor;
	pVertex3->color = cellColor;
	pVertex4->color = cellColor;

	// background display
	pBackgroundVertex1->position = pVertex1->position;
	pBackgroundVertex2->position = pVertex2->position;
	pBackgroundVertex3->position = pVertex3->position;
	pBackgroundVertex4->position = pVertex4->position;

	pBackgroundVertex1->color = backgroundColor;
	pBackgroundVertex2->color = backgroundColor;
	pBackgroundVertex3->color = backgroundColor;
	pBackgroundVertex4->color = backgroundColor;
}

inline unsigned int ConsoleScreenV1::priv_cellIndex(const sf::Vector2u location) const
{
	return location.y * m_mode.x + location.x;
}

inline sf::Vector2u ConsoleScreenV1::priv_cellLocation(const unsigned int index) const
{
	return{ index % m_mode.x, index / m_mode.x };
}

inline bool ConsoleScreenV1::priv_isCellIndexInRange(const unsigned int index) const
{
	return index < m_cells.size();
}

inline bool ConsoleScreenV1::priv_isCellLocationInRange(const sf::Vector2u location) const
{
	return (location.x < m_mode.x && location.y < m_mode.y);
}

inline bool ConsoleScreenV1::priv_isScreenBufferIndexInRange(const unsigned int index) const
{
	return index < m_buffers.size();
}

inline bool ConsoleScreenV1::priv_isCursorInRange() const
{
	return priv_isCellIndexInRange(m_cursor.index);
}

inline bool ConsoleScreenV1::priv_isColorIdInPaletteRange(const int id) const
{
	return (id >= 0) && (id < static_cast<int>(m_palette.size()));
}

inline void ConsoleScreenV1::priv_clearCell(const unsigned int index, const bool overwriteColor, const bool overwriteBackgroundColor)
{
	const sf::Color color{ overwriteColor ? m_colors.main : m_cells[index].color };
	const sf::Color backgroundColor{ overwriteBackgroundColor ? m_colors.background : m_cells[index].backgroundColor };
	poke(index, { 0, color, backgroundColor, Stretch::None, CellAttributes() });
}

inline void ConsoleScreenV1::priv_clearCell(const unsigned int index, const sf::Color& backgroundColor, const bool overwriteColor)
{
	const sf::Color color{ overwriteColor ? m_colors.main : m_cells[index].color };
	poke(index, { 0, color, backgroundColor, Stretch::None, CellAttributes() });
}

inline void ConsoleScreenV1::priv_clearCell(const unsigned int index, const sf::Color& color, const sf::Color& backgroundColor)
{
	poke(index, { 0, color, backgroundColor, Stretch::None, CellAttributes() });
}

inline void ConsoleScreenV1::priv_paintCell(const unsigned int index, const sf::Color& color, const sf::Color& backgroundColor)
{
	poke(index, color, backgroundColor);

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

void ConsoleScreenV1::priv_setCursorIndex(const unsigned int index)
{
	const unsigned int previousIndex{ m_cursor.index };
	m_cursor.index = index;

	if (m_do.updateAutomatically)
	{
		priv_updateCell(previousIndex);
		priv_updateCell(m_cursor.index);
	}
}

void ConsoleScreenV1::priv_moveCursorToBeginningOfLine()
{
	priv_setCursorIndex(m_cursor.index - m_cursor.index % m_mode.x);
}

void ConsoleScreenV1::priv_moveCursorUp()
{
	if (m_cursor.index >= m_mode.x)
		priv_setCursorIndex(m_cursor.index - m_mode.x);
}

void ConsoleScreenV1::priv_moveCursorDown()
{
	priv_setCursorIndex(m_cursor.index + m_mode.x);
	priv_testCursorForScroll();
}

void ConsoleScreenV1::priv_moveCursorLeft()
{
	if (m_cursor.index > 0)
		priv_setCursorIndex(m_cursor.index - 1);
}

void ConsoleScreenV1::priv_moveCursorRight()
{
	priv_setCursorIndex(m_cursor.index + 1);
	priv_testCursorForScroll();
}

void ConsoleScreenV1::priv_testCursorForScroll()
{
	if (m_cursor.index >= m_cells.size())
	{
		if (m_do.scrollAutomatically)
		{
			priv_scroll();
			if (m_do.updateAutomatically)
				update();
		}
		else
		{
			m_cursor.index = static_cast<unsigned int>(m_cells.size()) - 1u;
			if (m_do.updateAutomatically)
				priv_updateCell(m_cursor.index);
		}
	}
}

void ConsoleScreenV1::priv_scroll()
{
	for (unsigned int y{ 0 }; y < m_mode.y; ++y)
	{
		for (unsigned int x{ 0 }; x < m_mode.x; ++x)
		{
			if (y < m_mode.y - 1)
				m_cells[priv_cellIndex({ x, y })] = m_cells[priv_cellIndex({ x, y + 1 })];
			else
				priv_clearCell(priv_cellIndex({ x, y }), true, true);
		}
	}
	priv_moveCursorUp();
}

void ConsoleScreenV1::priv_copyToBufferFromSelectionRectangle(Buffer& buffer, const sf::IntRect& selectionRectangle)
{
	if (selectionRectangle.left >= static_cast<int>(m_mode.x) ||
		selectionRectangle.top >= static_cast<int>(m_mode.y) ||
		selectionRectangle.width <= 0 ||
		selectionRectangle.height <= 0 ||
		(selectionRectangle.left + selectionRectangle.width) < 0 ||
		(selectionRectangle.top + selectionRectangle.height) < 0)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot copy selection.\nSelection does not contain any cells.");
		return;
	}

	buffer.width = 0u;
	buffer.cells.clear();

	for (int y{ 0 }; y < selectionRectangle.height; ++y)
	{
		for (int x{ 0 }; x < selectionRectangle.width; ++x)
		{
			const sf::Vector2i location{ x + selectionRectangle.left, y + selectionRectangle.top };
			if (location.x < 0 || location.y < 0)
				continue;
			const sf::Vector2u cellLocation{ static_cast<unsigned int>(location.x), static_cast<unsigned int>(location.y) };
			if (priv_isCellLocationInRange(cellLocation))
			{
				buffer.cells.push_back(m_cells[priv_cellIndex(cellLocation)]);
				if (y == 0)
					++buffer.width;
			}
		}
	}
}

void ConsoleScreenV1::priv_pasteOffsettedBuffer(Buffer& buffer, const sf::Vector2i& offset)
{
	for (unsigned int i{ 0 }; i < buffer.cells.size(); ++i)
	{
		const sf::Vector2i location{ static_cast<int>(i % buffer.width) + offset.x, static_cast<int>(i / buffer.width) + offset.y };
		if (location.x < 0 || location.y < 0)
			continue;
		const sf::Vector2u cellLocation{ static_cast<unsigned int>(location.x), static_cast<unsigned int>(location.y) };
		if (priv_isCellLocationInRange(cellLocation))
			m_cells[priv_cellIndex(cellLocation)] = buffer.cells[i];
	}

	if (m_do.updateAutomatically)
		update();
}

unsigned int ConsoleScreenV1::priv_getPrintIndex(sf::Vector2u location) const
{
	if (location.x >= m_mode.x)
	{
		location.y += location.x / m_mode.x;
		location.x %= m_mode.x;
	}
	if (location.y >= m_mode.y)
	{
		location.y = m_mode.y - 1;
		location.x = m_mode.x - 1; // push cursor to the end of the line if cursor was below the last line
	}

	return priv_cellIndex(location);
}

inline unsigned int ConsoleScreenV1::priv_getCellValueFromCharacter(const char character) const
{
	if (getIsMappedCharacter(character))
		return m_characterMap.at(character);
	else
		return static_cast<unsigned int>(character);
}

inline char ConsoleScreenV1::priv_getCharacterFromCellValue(const unsigned int cellValue) const
{
	for (auto& pair : m_characterMap)
	{
		if (pair.second == cellValue)
			return pair.first;
	}
	return static_cast<char>(cellValue);
}

sf::Color ConsoleScreenV1::priv_colorFromColorIdAtIndex(const unsigned int index, const int colorId) const
{
	sf::Color color;
	if (priv_isColorIdInPaletteRange(colorId))
		return m_palette[colorId];
	else
	{
		switch (colorId)
		{
		case Color::Contrast:
			return contrastedColor(m_cells[index].backgroundColor);
		case Color::Invert:
			return invertedColor(m_cells[index].backgroundColor);
		case Color::Ignore:
			return m_cells[index].color;
		case Color::Current:
		default:
			return m_colors.main;
		}
	}
}

sf::Color ConsoleScreenV1::priv_backgroundColorFromColorIdAtIndex(const unsigned int index, const int colorId) const
{
	sf::Color color;
	if (priv_isColorIdInPaletteRange(colorId))
		return m_palette[colorId];
	else
	{
		switch (colorId)
		{
		case Color::Contrast:
			return contrastedColor(m_cells[index].color);
		case Color::Invert:
			return invertedColor(m_cells[index].color);
		case Color::Ignore:
			return m_cells[index].backgroundColor;
		case Color::Current:
		default:
			return m_colors.background;
		}
	}
}

bool& ConsoleScreenV1::priv_chooseAttribute(CellAttributes& cellAttributes, const Attribute attribute)
{
	switch (attribute)
	{
	case Attribute::FlipX:
		return cellAttributes.flipX;
	case Attribute::FlipY:
		return cellAttributes.flipY;
	case Attribute::Bright:
		return cellAttributes.bright;
	case Attribute::Inverse:
	default:
		return cellAttributes.inverse;
	}
}

} // namespace selbaward
