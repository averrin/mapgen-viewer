//////////////////////////////////////////////////////////////////////////////
//
// Selba Ward (https://github.com/Hapaxia/SelbaWard)
// --
//
// Console Screen v2
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

#include "ConsoleScreen.hpp"

#include <random>
#include <functional>
#include <algorithm>
#include <SFML/Config.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace
{

const std::string exceptionPrefix{ "Console Screen: " };

const sf::PrimitiveType primitiveType{ sf::PrimitiveType::Quads };

const selbaward::ConsoleScreen::Color defaultColor(15);
const selbaward::ConsoleScreen::Color defaultBackgroundColor(0);
const selbaward::ConsoleScreen::Color defaultCursorColor(15);
const double unBrightAttributeMultiplier{ 0.5 };
const selbaward::ConsoleScreen::Cell defaultCell{ 0u, selbaward::ConsoleScreen::ColorPair(), selbaward::ConsoleScreen::StretchType::None, selbaward::ConsoleScreen::CellAttributes() };
selbaward::ConsoleScreen::Cell fakeCell = defaultCell;

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
	return sf::Color((r > 255 ? 255u : static_cast<sf::Uint8>(r)), (g > 255 ? 255u : static_cast<sf::Uint8>(g)), (b > 255 ? 255u : static_cast<sf::Uint8>(b)));
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
	addColorToPalette(palette, sf::Color(64, 64, 64)); // light black/dark grey/grey
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
	for (sf::Uint8 i{ 0u }; i < 16u; ++i)
		addColorToPalette(palette, sf::Color(0, i * 17u, 0));
}

void addPalette16ColorGrayscale(std::vector<sf::Color>& palette)
{
	for (sf::Uint8 i{ 0u }; i < 16u; ++i)
		addColorToPalette(palette, sf::Color(i * 17u, i * 17u, i * 17u));
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
	addColorToPalette(palette, sf::Color(255, 255, 255));
	addColorToPalette(palette, sf::Color(255, 255, 0));
	addColorToPalette(palette, sf::Color(255, 102, 0));
	addColorToPalette(palette, sf::Color(221, 0, 0));
	addColorToPalette(palette, sf::Color(255, 0, 153));
	addColorToPalette(palette, sf::Color(51, 0, 153));
	addColorToPalette(palette, sf::Color(0, 0, 204));
	addColorToPalette(palette, sf::Color(0, 153, 255));
	addColorToPalette(palette, sf::Color(0, 170, 0));
	addColorToPalette(palette, sf::Color(0, 102, 0));
	addColorToPalette(palette, sf::Color(102, 51, 0));
	addColorToPalette(palette, sf::Color(153, 102, 51));
	addColorToPalette(palette, sf::Color(187, 187, 187));
	addColorToPalette(palette, sf::Color(136, 136, 136));
	addColorToPalette(palette, sf::Color(68, 68, 68));
	addColorToPalette(palette, sf::Color(0, 0, 0));
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

void addPalette16Color16Html(std::vector<sf::Color>& palette)
{
	addColorToPalette(palette, sf::Color(0, 0, 0));
	addColorToPalette(palette, sf::Color(196, 196, 196));
	addColorToPalette(palette, sf::Color(128, 128, 128));
	addColorToPalette(palette, sf::Color(255, 255, 255));
	addColorToPalette(palette, sf::Color(128, 0, 0));
	addColorToPalette(palette, sf::Color(255, 0, 0));
	addColorToPalette(palette, sf::Color(128, 0, 128));
	addColorToPalette(palette, sf::Color(255, 0, 255));
	addColorToPalette(palette, sf::Color(0, 128, 0));
	addColorToPalette(palette, sf::Color(0, 255, 0));
	addColorToPalette(palette, sf::Color(128, 128, 0));
	addColorToPalette(palette, sf::Color(255, 255, 0));
	addColorToPalette(palette, sf::Color(0, 0, 128));
	addColorToPalette(palette, sf::Color(0, 0, 255));
	addColorToPalette(palette, sf::Color(0, 128, 128));
	addColorToPalette(palette, sf::Color(0, 255, 255));
}

void addPalette216ColorWebSafe(std::vector<sf::Color>& palette)
{
	for (sf::Uint8 r{ 0u }; r < 6; ++r)
	{
		for (sf::Uint8 g{ 0u }; g < 6; ++g)
		{
			for (sf::Uint8 b{ 0u }; b < 6; ++b)
				addColorToPalette(palette, sf::Color(r * 51u, g * 51u, b * 51u));
		}
	}
}

void addPalette256ColorGreenscale(std::vector<sf::Color>& palette)
{
	for (unsigned int i{ 0u }; i < 256; ++i)
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

ConsoleScreen& ConsoleScreen::operator<<(const std::string& string)
{
	print(string);
	return *this;
}

ConsoleScreen& ConsoleScreen::operator<<(const Char& csChar)
{
	print(csChar.character);
	return *this;
}

ConsoleScreen& ConsoleScreen::operator<<(const Direct& direct)
{
	if (direct == Direct::Begin)
	{
		m_is.directPrinting = true;
		//m_directPrintProperties = m_cursorPrintProperties;
	}
	else
	{
		m_is.directPrinting = false;
		resetPrintProperties(PrintType::Direct);
	}
	return *this;
}

ConsoleScreen& ConsoleScreen::operator<<(const Location& location)
{
	const unsigned int currentIndex{ priv_getCurrentPrintProperties().index };
	priv_getCurrentPrintProperties().index = priv_getPrintIndex(location);
	if (m_do.updateAutomatically)
	{
		priv_updateCell(currentIndex);
		if (priv_getCurrentPrintProperties().stretch == Cs::StretchType::Both)
			priv_updateCell(currentIndex + m_mode.x);
		if (priv_getCurrentPrintProperties().index != currentIndex)
		{
			priv_updateCell(priv_getPrintIndex(location));
			if (priv_getCurrentPrintProperties().stretch == Cs::StretchType::Both)
				priv_updateCell(priv_getPrintIndex(location) + m_mode.x);
		}
	}
	return *this;
}

ConsoleScreen& ConsoleScreen::operator<<(const Offset& offset)
{
	const Location location{ priv_cellLocation(priv_getCurrentPrintProperties().index) };
	sf::Vector2i targetLocation{ sf::Vector2i(location) + offset };
	while (targetLocation.y >= 0 && targetLocation.x < 0)
		--targetLocation.y;
	if (targetLocation.y < 0)
		targetLocation = { 0, 0 };
	return *this << Location(targetLocation);
}

ConsoleScreen& ConsoleScreen::operator<<(const Affect& affectMask)
{
	priv_getCurrentPrintProperties().affectBitmask = affectMask;
	return *this;
}

ConsoleScreen& ConsoleScreen::operator<<(const unsigned int affectMask)
{
	priv_getCurrentPrintProperties().affectBitmask = affectMask;
	return *this;
}

ConsoleScreen& ConsoleScreen::operator<<(const ColorType& colorType)
{
	priv_getCurrentPrintProperties().colorType = colorType;
	return *this;
}

ConsoleScreen& ConsoleScreen::operator<<(const Color& color)
{
	priv_getActiveColor() = color;
	return *this;
}

ConsoleScreen& ConsoleScreen::operator<<(const sf::Color& newColor)
{
	if (m_is.rgbMode)
		return *this << Color(newColor.r * 65536L + newColor.g * 256L + newColor.b);

	Color color(static_cast<unsigned long int>(m_palette.size()));
	if (m_do.addNewColorToPalette)
		addColorToPalette(newColor);
	else
	{
		color.id = priv_getIndexOfClosestPaletteColor(newColor);
		if (color.id < 0)
			color.id = 0;
	}
	return *this << color;
}

ConsoleScreen& ConsoleScreen::operator<<(const ColorPair& colorPair)
{
	priv_getCurrentPrintProperties().colors = colorPair;
	return *this;
}

ConsoleScreen& ConsoleScreen::operator<<(const Fg& foregroundColor)
{
	priv_getCurrentPrintProperties().colors.foreground = foregroundColor.color;
	return *this;
}

ConsoleScreen& ConsoleScreen::operator<<(const Bg& backgroundColor)
{
	priv_getCurrentPrintProperties().colors.background = backgroundColor.color;
	return *this;
}

ConsoleScreen& ConsoleScreen::operator<<(const Wipe& length)
{
	return *this << length.string;
}

ConsoleScreen& ConsoleScreen::operator<<(const StretchType& stretchType)
{
	priv_getCurrentPrintProperties().stretch = stretchType;
	if (stretchType == Cs::StretchType::Both)
		priv_testCursorForScroll();
	if (m_do.updateAutomatically)
		update();
	return *this;
}

ConsoleScreen& ConsoleScreen::operator<<(const CellAttributes& cellAttributes)
{
	priv_getCurrentPrintProperties().attributes = cellAttributes;
	return *this;
}

ConsoleScreen& ConsoleScreen::operator<<(const MovementControl& movementControl)
{
	PrintProperties& printProperties{ priv_getCurrentPrintProperties() };
	const unsigned int currentIndex{ printProperties.index };
	switch (movementControl.direction)
	{
	case Direction::Left:
		if (printProperties.index > movementControl.amount)
			printProperties.index -= movementControl.amount;
		else
			printProperties.index = 0;
		break;
	case Direction::Right:
		printProperties.index += movementControl.amount;
		break;
	case Direction::Up:
	{
		const unsigned int lefts{ m_mode.x * movementControl.amount };
		if (printProperties.index > lefts)
			printProperties.index -= lefts;
		else
			printProperties.index = 0;
	}
		break;
	case Direction::Down:
		printProperties.index += m_mode.x * movementControl.amount;
		break;
	default:
		;
	}
	if (!priv_isCellIndexInRange(printProperties.index))
	{
		if (m_is.directPrinting)
			printProperties.index = static_cast<unsigned int>(m_cells.size()) - 1u;
		else
			priv_testCursorForScroll();
	}
	if (m_do.updateAutomatically)
	{
		priv_updateCell(currentIndex);
		priv_updateCell(printProperties.index);
		if (priv_getCurrentPrintProperties().stretch == Cs::StretchType::Both)
		{
			priv_updateCell(currentIndex + m_mode.x);
			priv_updateCell(printProperties.index + m_mode.x);
		}
	}
	return *this;
}

ConsoleScreen& ConsoleScreen::operator<<(const CursorCommand& cursorCommand)
{
	PrintProperties& printProperties{ priv_getCurrentPrintProperties() };
	const unsigned int currentIndex{ printProperties.index };
	unsigned int maxIndex{ static_cast<unsigned int>(m_cells.size()) - 1 };
	if (printProperties.stretch == StretchType::Both)
		maxIndex = (static_cast<int>(maxIndex) - m_mode.x) < 0 ? 0u : maxIndex - m_mode.x;
	const unsigned int rowIndexOffset{ printProperties.stretch == StretchType::Both ? m_mode.x * 2 : m_mode.x };
	bool belowCellAltered{ false };
	switch (cursorCommand)
	{
	case CursorCommand::Newline:
		printProperties.index += rowIndexOffset;
		printProperties.index -= printProperties.index % m_mode.x;
		break;
	case CursorCommand::Left:
		if ((printProperties.stretch == StretchType::Both) && (printProperties.index % m_mode.x == 0))
		{
			if (printProperties.index > m_mode.x)
				printProperties.index -= m_mode.x;
			else
				printProperties.index = 0;
		}
		if (printProperties.index > 0)
			--printProperties.index;
		break;
	case CursorCommand::Right:
		++printProperties.index;
		if ((printProperties.stretch == StretchType::Both) && (printProperties.index % m_mode.x == 0))
			printProperties.index += m_mode.x;
		break;
	case CursorCommand::Up:
		if (printProperties.index > rowIndexOffset)
			printProperties.index -= rowIndexOffset;
		else
			printProperties.index = 0;
		break;
	case CursorCommand::Down:
		printProperties.index += rowIndexOffset;
		break;
	case CursorCommand::Home:
		printProperties.index = 0u;
		break;
	case CursorCommand::End:
		printProperties.index = maxIndex;
		break;
	case CursorCommand::HomeLine:
		printProperties.index -= printProperties.index % m_mode.x;
		break;
	case CursorCommand::EndLine:
		printProperties.index += (m_mode.x - 1) - (printProperties.index % m_mode.x);
		break;
	case CursorCommand::Tab:
	{
		const unsigned int col{ printProperties.index % m_mode.x };
		const unsigned int rowStartIndex{ printProperties.index - col };
		const unsigned int targetIndex{ rowStartIndex + (col / m_tabSize + 1) * m_tabSize };
		if (targetIndex < rowStartIndex + m_mode.x)
			printProperties.index = targetIndex;
		else
			printProperties.index = rowStartIndex + rowIndexOffset;
	}
		break;
	case CursorCommand::TabReverse:
	{
		const unsigned int col{ printProperties.index % m_mode.x };
		const unsigned int rowStartIndex{ printProperties.index - col };
		int targetIndex{ static_cast<int>(rowStartIndex + ((static_cast<long int>(col) - 1) / m_tabSize) * m_tabSize) };
		if ((printProperties.stretch == StretchType::Both) && (printProperties.index % m_mode.x == 0))
			targetIndex -= m_mode.x;
		if (targetIndex >= 0)
			printProperties.index = static_cast<unsigned int>(targetIndex);
	}
		break;
	case CursorCommand::Backspace:
		// affects cell contents
		if (printProperties.stretch == StretchType::Both && printProperties.index >= m_mode.x && printProperties.index % m_mode.x == 0)
			printProperties.index -= m_mode.x;
		if (printProperties.index > 0)
			--printProperties.index;
		priv_clearCell(printProperties.index, false, false);
		if (printProperties.stretch == StretchType::Both)
		{
			const unsigned int belowIndex{ printProperties.index + m_mode.x };
			if (priv_isCellIndexInRange(belowIndex))
			{
				belowCellAltered = true;
				priv_clearCell(belowIndex, false, false);
			}
		}
		break;
	case CursorCommand::Delete:
		// affects cell contents
		priv_clearCell(printProperties.index, false, false);
		if (printProperties.stretch == StretchType::Both)
		{
			const unsigned int belowIndex{ printProperties.index + m_mode.x };
			if (priv_isCellIndexInRange(belowIndex))
			{
				belowCellAltered = true;
				priv_clearCell(belowIndex, false, false);
			}
		}
		break;
	case CursorCommand::None:
	default:
		;
	}
	if (printProperties.index > maxIndex)
	{
		if (m_is.directPrinting)
			printProperties.index = maxIndex;
		else
			priv_testCursorForScroll();
	}
	if (m_do.updateAutomatically)
		update();
	return *this;
}

ConsoleScreen& ConsoleScreen::operator>>(std::string& string)
{
	if (priv_isCellIndexInRange(priv_getCurrentPrintProperties().index))
		string = priv_read(priv_getCurrentPrintProperties().index);
	else
		string = "";
	return *this;
}

ConsoleScreen& ConsoleScreen::operator>>(const unsigned int length)
{
	m_readLength = length;
	return *this;
}

ConsoleScreen& ConsoleScreen::operator>>(const Direct& direct)
{
	return *this << direct;
}

ConsoleScreen& ConsoleScreen::operator>>(const Location& location)
{
	return *this << location;
}

ConsoleScreen& ConsoleScreen::operator>>(const Offset& offset)
{
	return *this << offset;
}

ConsoleScreen& ConsoleScreen::operator>>(const MovementControl& movementControl)
{
	return *this << movementControl;
}

ConsoleScreen& ConsoleScreen::operator>>(const CursorCommand& cursorCommand)
{
	return *this << cursorCommand;
}

ConsoleScreen::ConsoleScreen(const sf::Vector2u mode)
	: m_cells()
	, m_mode(mode)
	, m_buffers()
	, m_do()
	, m_is()
	, m_display()
	, m_backgroundDisplay()
	, m_overDisplay()
	, m_underDisplay()
	, m_size{ 100.f, 100.f }
	, m_texture(nullptr)
	, m_textureOffset{ 0u, 0u }
	, m_tileSize{ 8u, 8u }
	, m_numberOfTilesPerRow{ 8u }
	, m_palette()
	, m_cursor{ static_cast<int>('_'), defaultCursorColor, true, false, false }
	, m_tabSize{ 4u }
	, m_readLength{ 1u }
	, m_characterMap()
	, m_defaultPrintProperties{ 0u, ColorPair(defaultColor, defaultBackgroundColor), StretchType::None, CellAttributes(), Affect::Default, ColorType::Foreground }
{
	m_cursorPrintProperties = m_defaultPrintProperties;
	m_directPrintProperties = m_defaultPrintProperties;
	randomSeed();
	loadPalette(Palette::Default);
	setMode(m_mode);
}

void ConsoleScreen::setMode(sf::Vector2u mode)
{
	if (mode.x == 0 || mode.y == 0)
		mode = { 0u, 0u };

	m_mode = mode;
	m_cells.resize(m_mode.x * m_mode.y, defaultCell);
	m_display.resize(m_cells.size() * 4);
	m_backgroundDisplay = m_display;

	clear(Color(0));
	clearStack();
}

void ConsoleScreen::setTexture(const sf::Texture& texture)
{
	m_texture = &texture;
}

void ConsoleScreen::setTexture()
{
	m_texture = nullptr;
}

void ConsoleScreen::setSize(const sf::Vector2f size)
{
	m_size = size;

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreen::setTextureOffset(const sf::Vector2u textureOffset)
{
	m_textureOffset = textureOffset;

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreen::setTextureTileSize(const sf::Vector2u tileSize)
{
	m_tileSize = tileSize;

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreen::setNumberOfTextureTilesPerRow(const unsigned int numberOfTextureTilesPerRow)
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

void ConsoleScreen::setThrowExceptions(const bool throwExceptions)
{
	m_do.throwExceptions = throwExceptions;
}

void ConsoleScreen::setUpdateAutomatically(const bool updateAutomatically)
{
	m_do.updateAutomatically = updateAutomatically;
}

void ConsoleScreen::setShowCursor(const bool showCursor)
{
	m_cursor.visible = showCursor;

	if (m_do.updateAutomatically)
	{
		priv_updateCell(m_cursorPrintProperties.index);
		if (priv_getCurrentPrintProperties().stretch == Cs::StretchType::Both)
			priv_updateCell(m_cursorPrintProperties.index + m_mode.x);
	}
}

void ConsoleScreen::setInvertCursor(const bool invertCursor)
{
	m_cursor.inverse = invertCursor;

	if (m_do.updateAutomatically)
	{
		priv_updateCell(m_cursorPrintProperties.index);
		if (priv_getCurrentPrintProperties().stretch == Cs::StretchType::Both)
			priv_updateCell(m_cursorPrintProperties.index + m_mode.x);
	}
}

void ConsoleScreen::setUseCursorColor(const bool useCursorColor)
{
	m_cursor.useOwnColour = useCursorColor;

	if (m_do.updateAutomatically)
	{
		priv_updateCell(m_cursorPrintProperties.index);
		if (priv_getCurrentPrintProperties().stretch == Cs::StretchType::Both)
			priv_updateCell(m_cursorPrintProperties.index + m_mode.x);
	}
}

void ConsoleScreen::setShowBackground(const bool showBackground)
{
	m_do.showBackround = showBackground;
}

void ConsoleScreen::setScrollAutomatically(const bool scrollAutomatically)
{
	m_do.scrollAutomatically = scrollAutomatically;
}

void ConsoleScreen::setWrapOnManualScroll(const bool wrapOnManualScroll)
{
	m_do.wrapOnManualScroll = wrapOnManualScroll;
}

void ConsoleScreen::setAddNewColorToPalette(const bool addNewColorToPalette)
{
	m_do.addNewColorToPalette = addNewColorToPalette;
}

void ConsoleScreen::update()
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

	for (unsigned int i{ 0u }; i < m_cells.size(); ++i)
		priv_updateCell(i);

	priv_updateUnderCells();
	priv_updateOverCells();
}

sf::Vector2f ConsoleScreen::getSize() const
{
	return m_size;
}

sf::Vector2f ConsoleScreen::getPerfectSize() const
{
	return{ static_cast<float>(m_tileSize.x * m_mode.x), static_cast<float>(m_tileSize.y * m_mode.y) };
}

sf::FloatRect ConsoleScreen::getLocalBounds() const
{
	return{ { 0.f, 0.f }, m_size };
}

sf::FloatRect ConsoleScreen::getGlobalBounds() const
{
	return getTransform().transformRect(getLocalBounds());
}

sf::Vector2u ConsoleScreen::getMode() const
{
	return m_mode;
}

unsigned int ConsoleScreen::getNumberOfCells() const
{
	return static_cast<unsigned int>(m_cells.size());
}

sf::Vector2u ConsoleScreen::getNumberOfTilesInTexture2d() const
{
	return{ m_numberOfTilesPerRow, m_texture->getSize().y / m_tileSize.y };
}

unsigned int ConsoleScreen::getNumberOfTilesInTexture() const
{
	const sf::Vector2u numberOfTiles{ getNumberOfTilesInTexture2d() };
	return numberOfTiles.x * numberOfTiles.y;
}

bool ConsoleScreen::getThrowExceptions() const
{
	return m_do.throwExceptions;
}

bool ConsoleScreen::getUpdateAutomatically() const
{
	return m_do.updateAutomatically;
}

bool ConsoleScreen::getShowCursor() const
{
	return m_cursor.visible;
}

bool ConsoleScreen::getInvertCursor() const
{
	return m_cursor.inverse;
}

bool ConsoleScreen::getUseCursorColor() const
{
	return m_cursor.useOwnColour;
}

bool ConsoleScreen::getShowBackground() const
{
	return m_do.showBackround;
}

bool ConsoleScreen::getScrollAutomatically() const
{
	return m_do.scrollAutomatically;
}

bool ConsoleScreen::getWrapOnManualScroll() const
{
	return m_do.wrapOnManualScroll;
}

bool ConsoleScreen::getAddNewColorToPalette() const
{
	return m_do.addNewColorToPalette;
}

void ConsoleScreen::setCursor(const int cellValue)
{
	m_cursor.value = cellValue;

	if (m_do.updateAutomatically)
	{
		priv_updateCell(m_cursorPrintProperties.index);
		if (priv_getCurrentPrintProperties().stretch == Cs::StretchType::Both)
			priv_updateCell(m_cursorPrintProperties.index + m_mode.x);
	}
}

void ConsoleScreen::setCursor(const char cellCharacter, bool mapCharacter)
{
	m_cursor.value = mapCharacter ? priv_getCellValueFromCharacter(cellCharacter) : static_cast<int>(cellCharacter);

	if (m_do.updateAutomatically)
	{
		priv_updateCell(m_cursorPrintProperties.index);
		if (priv_getCurrentPrintProperties().stretch == Cs::StretchType::Both)
			priv_updateCell(m_cursorPrintProperties.index + m_mode.x);
	}
}

void ConsoleScreen::setCursorColor(const Color& color)
{
	m_cursor.color = color;
}

int ConsoleScreen::getCursorValue() const
{
	return m_cursor.value;
}

char ConsoleScreen::getCursorChar(const bool mapCharacter) const
{
	return mapCharacter ? priv_getCharacterFromCellValue(m_cursor.value) : static_cast<char>(m_cursor.value);
}

ConsoleScreen::Color ConsoleScreen::getCursorColor() const
{
	return m_cursor.color;
}

void ConsoleScreen::print(const char character)
{
	if (getIsMappedCursorCommandCharacter(character))
	{
		*this << getMappedCursorCommandCharacter(character);
		return;
	}

	PrintProperties& printProperties{ priv_getCurrentPrintProperties() };

	if (!priv_isCellIndexInRange(printProperties.index))
		return;

	bool doAlterValue{ (printProperties.affectBitmask & Affect::Value) == Affect::Value };
	const unsigned int currentIndex{ printProperties.index };
	if (doAlterValue)
		m_cells[currentIndex].value = priv_getCellValueFromCharacter(character);

	const unsigned int belowIndex{ currentIndex + m_mode.x };
	bool isBelowCellInRange{ priv_isCellIndexInRange(belowIndex) };
	if (printProperties.stretch != StretchType::Both)
		priv_modifyCellUsingPrintProperties(currentIndex, PrintType::Current, printProperties.stretch);
	else
	{
		priv_modifyCellUsingPrintProperties(currentIndex, PrintType::Current, StretchType::Top);
		if (isBelowCellInRange)
		{
			priv_modifyCellUsingPrintProperties(belowIndex, PrintType::Current, StretchType::Bottom);
			if (doAlterValue)
				m_cells[belowIndex].value = m_cells[currentIndex].value;
		}
	}
	
	++printProperties.index;
	if ((printProperties.stretch == StretchType::Both) && (printProperties.index % m_mode.x == 0))
		printProperties.index += m_mode.x;
	if (!m_is.directPrinting)
		priv_testCursorForScroll();
	else if (!priv_isCellIndexInRange(printProperties.index))
		printProperties.index = static_cast<unsigned int>(m_cells.size()) - 1u;

	if (m_do.updateAutomatically)
	{
		priv_updateCell(currentIndex);
		if (priv_getCurrentPrintProperties().stretch == Cs::StretchType::Both)
			priv_updateCell(currentIndex + m_mode.x);
		if (currentIndex != printProperties.index)
		{
			priv_updateCell(printProperties.index);
			if (priv_getCurrentPrintProperties().stretch == Cs::StretchType::Both)
				priv_updateCell(printProperties.index + m_mode.x);
		}
	}
}

void ConsoleScreen::print(const std::string& string)
{
	for (auto& character : string)
		print(character);
}

void ConsoleScreen::print(const Location& location, char character)
{
	PrintProperties backupPrintProperties = priv_getCurrentPrintProperties();
	priv_getCurrentPrintProperties().index = priv_getPrintIndex(location);
	print(character);
	priv_getCurrentPrintProperties() = backupPrintProperties;
}

void ConsoleScreen::print(const Location& location, const std::string& string)
{
	PrintProperties backupPrintProperties = priv_getCurrentPrintProperties();
	priv_getCurrentPrintProperties().index = priv_getPrintIndex(location);
	print(string);
	priv_getCurrentPrintProperties() = backupPrintProperties;
}

void ConsoleScreen::addOverAt(const Location& location, const char character, const sf::Vector2f offset)
{
	if (getIsMappedCursorCommandCharacter(character))
	{
		*this << getMappedCursorCommandCharacter(character);
		return;
	}

	PrintProperties& printProperties{ priv_getCurrentPrintProperties() };

	const unsigned int cellValue{ priv_getCellValueFromCharacter(character) };
	Cell cell;
	cell.value = cellValue;
	cell.colors.foreground = printProperties.colors.foreground.id < 0 ? 0 : printProperties.colors.foreground;
	cell.attributes.inverse = printProperties.attributes.inverse;
	cell.attributes.bright = printProperties.attributes.bright;
	cell.attributes.flipX = printProperties.attributes.flipX;
	cell.attributes.flipY = printProperties.attributes.flipY;
	if (printProperties.stretch == StretchType::Both)
		cell.stretch = StretchType::Top;
	else
		cell.stretch = printProperties.stretch;
	addOverAt(location, cell, offset);
	if (printProperties.stretch == StretchType::Both)
	{
		const Location belowLocation{ location.x, location.y + 1 };
		if (priv_isCellLocationInRange(belowLocation))
		{
			cell.stretch = StretchType::Bottom;
			addOverAt(belowLocation, cell, offset);
		}
	}
}

void ConsoleScreen::addOverAt(const Location& location, const std::string& string, const sf::Vector2f offset)
{
	Location currentLocation{ location };
	for (auto& character : string)
	{
		addOverAt(currentLocation, character, offset);
		++currentLocation.x;
		if (currentLocation.x >= m_mode.x)
		{
			currentLocation.x = 0;
			++currentLocation.y;
			if (currentLocation.y >= m_mode.y)
				return;
		}
	}
}

void ConsoleScreen::addOverAt(const Location& location, const Cell& cell, const sf::Vector2f offset)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception("Cannot add stack-over cell.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	const unsigned int index{ priv_cellIndex(location) };

	m_overCells.emplace_back(StackCell{ index, offset, cell });
	if (m_overCells.back().cell.colors.foreground.id < 0)
		m_overCells.back().cell.colors.foreground = 0;

	if (m_do.updateAutomatically)
		priv_updateOverCells();
}

void ConsoleScreen::addUnderAt(const Location& location, const char character, const sf::Vector2f offset)
{
	if (getIsMappedCursorCommandCharacter(character))
	{
		*this << getMappedCursorCommandCharacter(character);
		return;
	}

	PrintProperties& printProperties{ priv_getCurrentPrintProperties() };

	const unsigned int cellValue{ priv_getCellValueFromCharacter(character) };
	Cell cell;
	cell.value = cellValue;
	cell.colors.foreground = printProperties.colors.foreground.id < 0 ? 0 : printProperties.colors.foreground;
	cell.attributes.inverse = printProperties.attributes.inverse;
	cell.attributes.bright = printProperties.attributes.bright;
	cell.attributes.flipX = printProperties.attributes.flipX;
	cell.attributes.flipY = printProperties.attributes.flipY;
	if (printProperties.stretch == StretchType::Both)
		cell.stretch = StretchType::Top;
	else
		cell.stretch = printProperties.stretch;
	addUnderAt(location, cell, offset);
	if (printProperties.stretch == StretchType::Both)
	{
		const Location belowLocation{ location.x, location.y + 1 };
		if (priv_isCellLocationInRange(belowLocation))
		{
			cell.stretch = StretchType::Bottom;
			addUnderAt(belowLocation, cell, offset);
		}
	}
}

void ConsoleScreen::addUnderAt(const Location& location, const std::string& string, const sf::Vector2f offset)
{
	Location currentLocation{ location };
	for (auto& character : string)
	{
		addUnderAt(currentLocation, character, offset);
		++currentLocation.x;
		if (currentLocation.x >= m_mode.x)
		{
			currentLocation.x = 0;
			++currentLocation.y;
			if (currentLocation.y >= m_mode.y)
				return;
		}
	}
}

void ConsoleScreen::addUnderAt(const Location& location, const Cell& cell, const sf::Vector2f offset)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception("Cannot add stack-under cell.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	const unsigned int index{ priv_cellIndex(location) };

	m_underCells.emplace_back(StackCell{ index, offset, cell });
	if (m_underCells.back().cell.colors.foreground.id < 0)
		m_underCells.back().cell.colors.foreground = 0;

	if (m_do.updateAutomatically)
		priv_updateUnderCells();
}

void ConsoleScreen::clearOversAt(const Location& location)
{
	const unsigned int index{ priv_cellIndex(location) };
	m_overCells.erase(std::remove_if(m_overCells.begin(), m_overCells.end(), [&index](const StackCell& stackCell) { return stackCell.index == index; }), m_overCells.end());

	if (m_do.updateAutomatically)
		priv_updateOverCells();
}

void ConsoleScreen::clearUndersAt(const Location& location)
{
	const unsigned int index{ priv_cellIndex(location) };
	m_underCells.erase(std::remove_if(m_underCells.begin(), m_underCells.end(), [&index](const StackCell& stackCell) { return stackCell.index == index; }), m_underCells.end());

	if (m_do.updateAutomatically)
		priv_updateUnderCells();
}

void ConsoleScreen::clearStackAt(const Location& location)
{
	clearOversAt(location);
	clearUndersAt(location);
}

void ConsoleScreen::clearOvers()
{
	m_overCells.clear();

	if (m_do.updateAutomatically)
		m_overDisplay.clear();
}

void ConsoleScreen::clearUnders()
{
	m_underCells.clear();

	if (m_do.updateAutomatically)
		m_underDisplay.clear();
}

void ConsoleScreen::clearStack()
{
	clearOvers();
	clearUnders();
}

void ConsoleScreen::pasteOver(const sf::Vector2i offset)
{
	if (m_buffers.size() == 0)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot stack-paste buffer.\nNo buffer exists.");
		return;
	}

	pasteOver(static_cast<unsigned int>(m_buffers.size()) - 1u, offset);
}

void ConsoleScreen::pasteUnder(const sf::Vector2i offset)
{
	if (m_buffers.size() == 0)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot stack-paste buffer.\nNo buffer exists.");
		return;
	}

	pasteUnder(static_cast<unsigned int>(m_buffers.size()) - 1u, offset);
}

void ConsoleScreen::pasteOver(const unsigned int index, const sf::Vector2i offset)
{
	if (!priv_isScreenBufferIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot stack-paste buffer.\nBuffer index (" + std::to_string(index) + ") out of range.");
		return;
	}

	Buffer& buffer{ m_buffers[index] };

	for (unsigned int i{ 0 }; i < buffer.cells.size(); ++i)
	{
		const sf::Vector2i location{ static_cast<int>(i % buffer.width) + offset.x, static_cast<int>(i / buffer.width) + offset.y };
		if (location.x < 0 || location.y < 0)
			continue;
		const sf::Vector2u cellLocation{ static_cast<unsigned int>(location.x), static_cast<unsigned int>(location.y) };
		if (priv_isCellLocationInRange(cellLocation))
			addOverAt(cellLocation, buffer.cells[i]);
	}

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreen::pasteUnder(const unsigned int index, const sf::Vector2i offset)
{
	if (!priv_isScreenBufferIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot stack-paste buffer.\nBuffer index (" + std::to_string(index) + ") out of range.");
		return;
	}

	Buffer& buffer{ m_buffers[index] };

	for (unsigned int i{ 0 }; i < buffer.cells.size(); ++i)
	{
		const sf::Vector2i location{ static_cast<int>(i % buffer.width) + offset.x, static_cast<int>(i / buffer.width) + offset.y };
		if (location.x < 0 || location.y < 0)
			continue;
		const sf::Vector2u cellLocation{ static_cast<unsigned int>(location.x), static_cast<unsigned int>(location.y) };
		if (priv_isCellLocationInRange(cellLocation))
			addUnderAt(cellLocation, buffer.cells[i]);
	}

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreen::resetPrintProperties(const PrintType printType)
{
	priv_getPrintProperties(printType) = m_defaultPrintProperties;
}

ConsoleScreen::Location ConsoleScreen::getLocation(const PrintType printType)
{
	return priv_cellLocation(priv_getPrintProperties(printType).index);
}

unsigned int ConsoleScreen::getIndex(const PrintType printType)
{
	return priv_getPrintProperties(printType).index;
}

ConsoleScreen::ColorPair ConsoleScreen::getColors(const PrintType printType)
{
	return priv_getPrintProperties(printType).colors;
}

ConsoleScreen::StretchType ConsoleScreen::getStretchType(const PrintType printType)
{
	return priv_getPrintProperties(printType).stretch;
}

ConsoleScreen::CellAttributes ConsoleScreen::getCellAttributes(const PrintType printType)
{
	return priv_getPrintProperties(printType).attributes;
}

unsigned int ConsoleScreen::getAffectBitmask(const PrintType printType)
{
	return priv_getPrintProperties(printType).affectBitmask;
}

ConsoleScreen::ColorType ConsoleScreen::getColorType(const PrintType printType)
{
	return priv_getPrintProperties(printType).colorType;
}

void ConsoleScreen::clearCellAt(const sf::Vector2u location)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot clear cell.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	priv_clearCell(priv_cellIndex(location), m_cursorPrintProperties.colors.foreground, m_cursorPrintProperties.colors.background);

	if (m_do.updateAutomatically)
		priv_updateCell(priv_cellIndex(location));
}

void ConsoleScreen::setCellAt(const sf::Vector2u location, const Cell& cell)
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

void ConsoleScreen::setValueAt(const sf::Vector2u location, const unsigned int value)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set cell value.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	m_cells[priv_cellIndex(location)].value = value;

	if (m_do.updateAutomatically)
		priv_updateCell(priv_cellIndex(location));
}

void ConsoleScreen::setColorsAt(const sf::Vector2u location, const Color color, const Color backgroundColor)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set cell colors.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	const unsigned int index{ priv_cellIndex(location) };
	m_cells[index].colors.foreground = color;
	m_cells[index].colors.background = backgroundColor;

	if (m_do.updateAutomatically)
		priv_updateCell(index);

}

void ConsoleScreen::setForegroundColorAt(const sf::Vector2u location, const Color color)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set cell foreground color.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	const unsigned int index{ priv_cellIndex(location) };
	m_cells[index].colors.foreground = color;

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

void ConsoleScreen::setBackgroundColorAt(const sf::Vector2u location, const Color backgroundColor)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set cell background colors.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	const unsigned int index{ priv_cellIndex(location) };
	m_cells[index].colors.background = backgroundColor;

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

void ConsoleScreen::setStretchAt(const sf::Vector2u location, const StretchType& stretch)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set cell stretch.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	m_cells[priv_cellIndex(location)].stretch = stretch;

	if (m_do.updateAutomatically)
		priv_updateCell(priv_cellIndex(location));
}

void ConsoleScreen::setAttributesAt(const sf::Vector2u location, const CellAttributes& attributes)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set cell attributes.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	m_cells[priv_cellIndex(location)].attributes = attributes;

	if (m_do.updateAutomatically)
		priv_updateCell(priv_cellIndex(location));
}

void ConsoleScreen::setAttributesAt(const sf::Vector2u location, const Affect& attributeMask)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set cell attributes.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	CellAttributes& cellAttributes{ m_cells[priv_cellIndex(location)].attributes };
	cellAttributes.inverse = (attributeMask & Affect::Inverse) == Affect::Inverse;
	cellAttributes.bright = (attributeMask & Affect::Bright) == Affect::Bright;
	cellAttributes.flipX = (attributeMask & Affect::FlipX) == Affect::FlipX;
	cellAttributes.flipY = (attributeMask & Affect::FlipY) == Affect::FlipY;

	if (m_do.updateAutomatically)
		priv_updateCell(priv_cellIndex(location));
}

void ConsoleScreen::setAttributesToAt(const sf::Vector2u location, const bool attributeValue, const Affect& attributeMask)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set cell attributes.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return;
	}

	CellAttributes& cellAttributes{ m_cells[priv_cellIndex(location)].attributes };
	if ((attributeMask & Affect::Inverse) == Affect::Inverse)
		cellAttributes.inverse = attributeValue;
	if ((attributeMask & Affect::Bright) == Affect::Bright)
		cellAttributes.bright = attributeValue;
	if ((attributeMask & Affect::FlipX) == Affect::FlipX)
		cellAttributes.flipX = attributeValue;
	if ((attributeMask & Affect::FlipY) == Affect::FlipY)
		cellAttributes.flipY = attributeValue;

	if (m_do.updateAutomatically)
		priv_updateCell(priv_cellIndex(location));
}

ConsoleScreen::Cell ConsoleScreen::getCellAt(const sf::Vector2u location) const
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot get cell.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return Cell();
	}

	return m_cells[priv_cellIndex(location)];
}

unsigned int ConsoleScreen::getValueAt(const sf::Vector2u location) const
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot get cell value.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return 0u;
	}

	return m_cells[priv_cellIndex(location)].value;
}

ConsoleScreen::Color ConsoleScreen::getColorAt(const sf::Vector2u location) const
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot get cell color.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return Color(ColorCommand::Unused);
	}

	return m_cells[priv_cellIndex(location)].colors.foreground;
}

ConsoleScreen::Color ConsoleScreen::getBackgroundColorAt(const sf::Vector2u location) const
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot get cell background color.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return Color(ColorCommand::Unused);
	}

	return m_cells[priv_cellIndex(location)].colors.background;
}

ConsoleScreen::StretchType ConsoleScreen::getStretchAt(const sf::Vector2u location) const
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot get cell stretch.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return StretchType::None;
	}

	return m_cells[priv_cellIndex(location)].stretch;
}

ConsoleScreen::CellAttributes ConsoleScreen::getAttributesAt(const sf::Vector2u location) const
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot get cell attributes.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return CellAttributes();
	}

	return m_cells[priv_cellIndex(location)].attributes;
}

bool ConsoleScreen::getAttributeAt(const sf::Vector2u location, const Affect& attributeMask)
{
	if (!priv_isCellLocationInRange(location))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot get cell attribute.\nLocation (" + std::to_string(location.x) + ", " + std::to_string(location.y) + ") out of range.");
		return false;
	}

	CellAttributes& cellAttributes{ m_cells[priv_cellIndex(location)].attributes };
	if ((attributeMask & Affect::Inverse) == Affect::Inverse)
		return cellAttributes.inverse;
	if ((attributeMask & Affect::Bright) == Affect::Bright)
		return cellAttributes.bright;
	if ((attributeMask & Affect::FlipX) == Affect::FlipX)
		return cellAttributes.flipX;
	if ((attributeMask & Affect::FlipY) == Affect::FlipY)
		return cellAttributes.flipY;

	if (m_do.throwExceptions)
		throw Exception(exceptionPrefix + "Cannot get cell attribute.\nAttribute mask does not include attribute.");
	return false;
}

void ConsoleScreen::scrollUp(const unsigned int amount)
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

void ConsoleScreen::scrollDown(const unsigned int amount)
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

void ConsoleScreen::scrollLeft(const unsigned int amount)
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

void ConsoleScreen::scrollRight(const unsigned int amount)
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

void ConsoleScreen::scrollUp(unsigned int amount, sf::IntRect selectionRectangle)
{
	if (m_mode.y == 0 || amount == 0)
		return;

	if (!priv_isSelectionRectangleContainedInScreen(selectionRectangle))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot scroll selection up.\nThe selection rectangle is not valid.");
		return;
	}

	std::vector<Cell> topRow(selectionRectangle.width);
	for (unsigned int repeat{ 0 }; repeat < amount; ++repeat) // lazy way of scrolling multiple times - loop scrolling (entirely by 1 each time)
	{
		for (unsigned int y{ 0 }; y < static_cast<unsigned int>(selectionRectangle.height); ++y)
		{
			for (unsigned int x{ 0 }; x < static_cast<unsigned int>(selectionRectangle.width); ++x)
			{
				if (m_do.wrapOnManualScroll && y == 0)
					topRow[x] = m_cells[priv_cellIndex({ selectionRectangle.left + x, selectionRectangle.top + y })];
				if (y < static_cast<unsigned int>(selectionRectangle.height) - 1)
					m_cells[priv_cellIndex({ selectionRectangle.left + x, selectionRectangle.top + y })] = m_cells[priv_cellIndex({ selectionRectangle.left + x, selectionRectangle.top + y + 1 })];
				else if (m_do.wrapOnManualScroll)
					m_cells[priv_cellIndex({ selectionRectangle.left + x, selectionRectangle.top + y })] = topRow[x];
				else
					priv_clearCell(priv_cellIndex({ selectionRectangle.left + x, selectionRectangle.top + y }), true, true);
			}
		}
	}

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreen::scrollDown(unsigned int amount, sf::IntRect selectionRectangle)
{
	if (m_mode.y == 0 || amount == 0)
		return;

	if (!priv_isSelectionRectangleContainedInScreen(selectionRectangle))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot scroll selection down.\nThe selection rectangle is not valid.");
		return;
	}

	std::vector<Cell> bottomRow(selectionRectangle.width);
	for (unsigned int repeat{ 0 }; repeat < amount; ++repeat) // lazy way of scrolling multiple times - loop scrolling (entirely by 1 each time)
	{
		for (unsigned int y{ 0 }; y < static_cast<unsigned int>(selectionRectangle.height); ++y)
		{
			for (unsigned int x{ 0 }; x < static_cast<unsigned int>(selectionRectangle.width); ++x)
			{
				const unsigned cellY{ selectionRectangle.top + selectionRectangle.height - y - 1 };
				if (m_do.wrapOnManualScroll && y == 0)
					bottomRow[x] = m_cells[priv_cellIndex({ selectionRectangle.left + x, cellY })];
				if (cellY > static_cast<unsigned int>(selectionRectangle.top))
					m_cells[priv_cellIndex({ selectionRectangle.left + x, cellY })] = m_cells[priv_cellIndex({ selectionRectangle.left + x, cellY - 1 })];
				else if (m_do.wrapOnManualScroll)
					m_cells[priv_cellIndex({ selectionRectangle.left + x, cellY })] = bottomRow[x];
				else
					priv_clearCell(priv_cellIndex({ selectionRectangle.left + x, cellY }), true, true);
			}
		}
	}

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreen::scrollLeft(unsigned int amount, sf::IntRect selectionRectangle)
{
	if (m_mode.x == 0 || amount == 0)
		return;

	if (!priv_isSelectionRectangleContainedInScreen(selectionRectangle))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot scroll selection left.\nThe selection rectangle is not valid.");
		return;
	}

	std::vector<Cell> leftColumn(selectionRectangle.height);
	for (unsigned int repeat{ 0 }; repeat < amount; ++repeat) // lazy way of scrolling multiple times - loop scrolling (entirely by 1 each time)
	{
		for (unsigned int x{ 0 }; x < static_cast<unsigned int>(selectionRectangle.width); ++x)
		{
			for (unsigned int y{ 0 }; y < static_cast<unsigned int>(selectionRectangle.height); ++y)
			{
				if (m_do.wrapOnManualScroll && x == 0)
					leftColumn[y] = m_cells[priv_cellIndex({ selectionRectangle.left + x, selectionRectangle.top + y })];
				if (x < static_cast<unsigned int>(selectionRectangle.width) - 1)
					m_cells[priv_cellIndex({ selectionRectangle.left + x, selectionRectangle.top + y })] = m_cells[priv_cellIndex({ selectionRectangle.left + x + 1, selectionRectangle.top + y })];
				else if (m_do.wrapOnManualScroll)
					m_cells[priv_cellIndex({ selectionRectangle.left + x, selectionRectangle.top + y })] = leftColumn[y];
				else
					priv_clearCell(priv_cellIndex({ selectionRectangle.left + x, selectionRectangle.top + y }), true, true);
			}
		}
	}

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreen::scrollRight(unsigned int amount, sf::IntRect selectionRectangle)
{
	if (m_mode.x == 0 || amount == 0)
		return;

	if (!priv_isSelectionRectangleContainedInScreen(selectionRectangle))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot scroll selection right.\nThe selection rectangle is not valid.");
		return;
	}

	std::vector<Cell> rightColumn(selectionRectangle.height);
	for (unsigned int repeat{ 0 }; repeat < amount; ++repeat) // lazy way of scrolling multiple times - loop scrolling (entirely by 1 each time)
	{
		for (unsigned int y{ 0 }; y < static_cast<unsigned int>(selectionRectangle.height); ++y)
		{
			for (unsigned int x{ 0 }; x < static_cast<unsigned int>(selectionRectangle.width); ++x)
			{
				const unsigned cellX{ selectionRectangle.left + selectionRectangle.width - x - 1 };
				if (m_do.wrapOnManualScroll && x == 0)
					rightColumn[y] = m_cells[priv_cellIndex({ cellX, selectionRectangle.top + y, })];
				if (cellX > static_cast<unsigned int>(selectionRectangle.left))
					m_cells[priv_cellIndex({ cellX, selectionRectangle.top + y })] = m_cells[priv_cellIndex({ cellX - 1, selectionRectangle.top + y })];
				else if (m_do.wrapOnManualScroll)
					m_cells[priv_cellIndex({ cellX, selectionRectangle.top + y })] = rightColumn[y];
				else
					priv_clearCell(priv_cellIndex({ cellX, selectionRectangle.top + y }), true, true);
			}
		}
	}

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreen::fill(Cell cell)
{
	if (m_cells.size() == 0)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot fill screen.\nNo cells exist.");
		return;
	}

	if (cell.colors.background.id < 0)
		cell.colors.background = 0;
	if (cell.colors.foreground.id < 0)
		cell.colors.foreground = 0;

	for (auto& singleCell : m_cells)
		singleCell = cell;

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreen::clear(const ColorPair colors)
{
	if (m_cells.size() == 0)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot clear screen.\nNo cells exist.");
		return;
	}

	for (unsigned int i{ 0 }; i < m_cells.size(); ++i)
		priv_clearCell(i, colors.foreground, colors.background);
	m_cursorPrintProperties.index = 0u;

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreen::clear(const Color backgroundColor)
{
	clear({ m_cursorPrintProperties.colors.foreground, backgroundColor });
}

void ConsoleScreen::clear(const ColorCommand backgroundColorCommand)
{
	clear(priv_getModifiedColorFromColorPairUsingSpecifiedColorType({ m_cursorPrintProperties.colors.foreground, backgroundColorCommand }, ColorType::Background));
}

void ConsoleScreen::clear()
{
	clear(m_cursorPrintProperties.colors.background);
}

void ConsoleScreen::crash()
{
	for (auto& cell : m_cells)
	{
		cell = { randomByte(), ColorPair(priv_getRandomColor(), priv_getRandomColor()) };
	}
	m_cursorPrintProperties.index = static_cast<unsigned int>(m_cells.size()) - 1u;

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreen::setCursorTab(const unsigned int tabSize)
{
	m_tabSize = tabSize;
}

unsigned int ConsoleScreen::getCursorTab() const
{
	return m_tabSize;
}

std::string ConsoleScreen::read()
{
	std::string string;
	*this >> string;
	return string;
}

std::string ConsoleScreen::read(const unsigned int length)
{
	m_readLength = length;
	return read();
}

std::string ConsoleScreen::read(const Location& location)
{
	std::string string;
	PrintProperties backupPrintProperties = priv_getCurrentPrintProperties();
	*this >> location >> string;
	priv_getCurrentPrintProperties() = backupPrintProperties;
	return string;
}

std::string ConsoleScreen::read(const Location& location, const unsigned int length)
{
	m_readLength = length;
	return read(location);
}

void ConsoleScreen::loadPalette(const Palette palette)
{
	m_is.rgbMode = false;
	m_palette.clear();
	switch (palette)
	{
	case Palette::Colors2BlackWhite:
		addPalette2ColorBlackWhite(m_palette);
		break;
	case Palette::Colors2WhiteBlack:
		addPalette2ColorWhiteBlack(m_palette);
		break;
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
	case Palette::Colors16Html:
		addPalette16Color16Html(m_palette);
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
	case Palette::ColorsRgb:
		m_is.rgbMode = true;
		break;
	case Palette::Default:
	default:
		addPaletteDefault(m_palette);
	}
	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreen::addColorToPalette(const sf::Color color)
{
	m_palette.emplace_back(color);
}

void ConsoleScreen::setPaletteColor(const Color color, const sf::Color newColor)
{
	if (m_is.rgbMode)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set palette color.\nRGB Mode is currently active.");
		return;
	}
	if (!priv_isColorInPaletteRange(color))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set palette color.\nColor ID (" + std::to_string(color.id) + ") out of range.");
		return;
	}

	m_palette[color.id] = newColor;

	if (m_do.updateAutomatically)
		update();
}

sf::Color ConsoleScreen::getPaletteColor(const Color color) const
{
	if (!priv_isColorInPaletteRange(color))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot get palette color.\nColor ID (" + std::to_string(color.id) + ") out of range.");
		return sf::Color::Transparent;
	}

	return m_is.rgbMode ? sf::Color(static_cast<sf::Uint8>(color.id / 65536), static_cast<sf::Uint8>((color.id % 65536) / 256), static_cast<sf::Uint8>(color.id % 256)) : m_palette[color.id];
}

void ConsoleScreen::setPaletteSize(const unsigned long int size)
{
	if (m_is.rgbMode)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot resize palette.\nRGB Mode is currently active.");
		return;
	}
	if (size < 1)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot set palette size to zero.");
		return;
	}

	m_palette.resize(size);
}

unsigned long int ConsoleScreen::getPaletteSize() const
{
	return m_is.rgbMode ? 16777216L : static_cast<unsigned long int>(m_palette.size());
}

void ConsoleScreen::removePaletteColor(const Color color)
{
	if (m_is.rgbMode)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot remove palette color.\nRGB Mode is currently active.");
		return;
	}
	if (!priv_isColorInPaletteRange(color))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot remove palette color.\nColor ID (" + std::to_string(color.id) + ") out of range.");
		return;
	}
	if (m_palette.size() < 2)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot remove final palette color.");
		return;
	}

	m_palette.erase(m_palette.begin() + color.id);
}

void ConsoleScreen::cyclePaletteUp(const long amount)
{
	if (m_palette.size() < 2)
		return;

	cyclePaletteUp(amount, 0, static_cast<unsigned long int>(m_palette.size()) - 1u);
}

void ConsoleScreen::cyclePaletteDown(const long amount)
{
	if (m_palette.size() < 2)
		return;

	cyclePaletteDown(amount, 0, static_cast<unsigned long int>(m_palette.size()) - 1u);
}

void ConsoleScreen::cyclePaletteUp(long amount, Color firstColor, Color lastColor)
{
	if (m_palette.size() < 2 || amount < 1)
		return;

	if (lastColor.id < firstColor.id)
		std::swap(lastColor, firstColor);

	if (firstColor.id < 0)
		firstColor = 0;
	if (lastColor.id > static_cast<long>(m_palette.size() - 1))
		lastColor = static_cast<unsigned long int>(m_palette.size()) - 1u;

	const long rangeSize{ lastColor.id - firstColor.id + 1 };
	amount %= rangeSize;

	std::vector<sf::Color> buffer;
	for (long i{ 0 }; i < amount; ++i)
		buffer.emplace_back(m_palette[lastColor.id - i]);
	for (long i{ lastColor.id }; i >= firstColor.id; --i)
	{
		if (i - amount >= firstColor.id)
			m_palette[i] = m_palette[i - amount];
		else
			m_palette[i] = buffer[firstColor.id - (i - amount) - 1];
	}

	if (m_do.updateAutomatically)
		update();
}

void ConsoleScreen::cyclePaletteDown(long amount, Color firstColor, Color lastColor)
{
	if (m_palette.size() < 2 || amount < 1)
		return;

	if (lastColor.id < firstColor.id)
		std::swap(lastColor, firstColor);

	if (firstColor.id < 0)
		firstColor = 0;
	if (lastColor.id > static_cast<long>(m_palette.size() - 1))
		lastColor = static_cast<unsigned long int>(m_palette.size()) - 1u;

	const long rangeSize{ lastColor.id - firstColor.id + 1 };
	amount %= rangeSize;

	std::vector<sf::Color> buffer;
	for (long i{ 0 }; i < amount; ++i)
		buffer.emplace_back(m_palette[firstColor.id + i]);
	for (long i{ firstColor.id }; i <= lastColor.id; ++i)
	{
		if (i + amount <= lastColor.id)
			m_palette[i] = m_palette[i + amount];
		else
			m_palette[i] = buffer[i + amount - lastColor.id - 1];
	}

	if (m_do.updateAutomatically)
		update();
}

unsigned int ConsoleScreen::copy()
{
	m_buffers.push_back({ m_mode.x, m_cells });
	return static_cast<unsigned int>(m_buffers.size()) - 1u;
}

void ConsoleScreen::copy(const unsigned int index)
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

unsigned int ConsoleScreen::copy(const sf::IntRect selectionRectangle)
{
	m_buffers.emplace_back();
	priv_copyToBufferFromSelectionRectangle(m_buffers.back(), selectionRectangle);
	return static_cast<unsigned int>(m_buffers.size()) - 1u;
}

void ConsoleScreen::copy(const unsigned int index, const sf::IntRect selectionRectangle)
{
	if (!priv_isScreenBufferIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot replace buffer with copy.\nBuffer index (" + std::to_string(index) + ") does not exist.");
		return;
	}

	priv_copyToBufferFromSelectionRectangle(m_buffers[index], selectionRectangle);
}

void ConsoleScreen::paste(const sf::Vector2i offset)
{
	if (m_buffers.size() == 0)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot paste buffer.\nNo buffer exists.");
		return;
	}

	priv_pasteOffsettedBuffer(m_buffers.back(), offset);
}

void ConsoleScreen::paste(const unsigned int index, const sf::Vector2i offset)
{
	if (!priv_isScreenBufferIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot paste buffer.\nBuffer index (" + std::to_string(index) + ") out of range.");
		return;
	}

	priv_pasteOffsettedBuffer(m_buffers[index], offset);
}

void ConsoleScreen::removeBuffer()
{
	if (m_buffers.size() == 0)
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot remove buffer.\nNo buffer exists.");
		return;
	}

	m_buffers.pop_back();
}

void ConsoleScreen::removeBuffer(const unsigned int index)
{
	if (!priv_isScreenBufferIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot remove buffer.\nBuffer index (" + std::to_string(index) + ") out of range.");
		return;
	}

	m_buffers.erase(m_buffers.begin() + index);
}

void ConsoleScreen::removeAllBuffers()
{
	m_buffers.clear();
}

unsigned int ConsoleScreen::addBuffer(const sf::Vector2u size)
{
	const unsigned int newBufferIndex{ static_cast<unsigned int>(m_buffers.size()) };
	m_buffers.emplace_back();
	resizeBuffer(static_cast<unsigned int>(m_buffers.size()) - 1u, size);
	return newBufferIndex;
}

void ConsoleScreen::resizeBuffer(const unsigned int index, const sf::Vector2u size)
{
	if (!priv_isScreenBufferIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot resize buffer.\nBuffer index (" + std::to_string(index) + ") out of range.");
		return;
	}

	Buffer& buffer{ m_buffers[index] };

	const unsigned int newNumberOfCells{ size.x * size.y };

	if (buffer.cells.empty())
	{
		buffer.cells.resize(newNumberOfCells, defaultCell);
		buffer.width = size.x;
		return;
	}

	const unsigned int currentBufferHeight{ static_cast<unsigned int>(buffer.cells.size()) / buffer.width };

	if (size.x < buffer.width)
	{
		for (unsigned int i{ 0u }; i < size.x * currentBufferHeight; ++i)
		{
			const unsigned int targetIndex{ (i / size.x) * buffer.width + (i % size.x) };
			buffer.cells[i] = buffer.cells[targetIndex];
		}

		buffer.cells.resize(size.x * currentBufferHeight);
		buffer.width = size.x;
	}
	if (size.x > buffer.width)
	{
		buffer.cells.resize(size.x * currentBufferHeight);
		for (int i{ static_cast<int>(size.x * currentBufferHeight - 1) }; i >= 0; --i)
		{
			if (i % size.x >= buffer.width)
			{
				buffer.cells[i] = defaultCell;
				continue;
			}
			const unsigned int targetIndex{ (i / size.x) * buffer.width + (i % size.x) };
			buffer.cells[i] = buffer.cells[targetIndex];
		}

		buffer.width = size.x;
	}

	if (size.y < currentBufferHeight)
	{
		for (unsigned int i{ 0u }; i < newNumberOfCells; ++i)
		{
			const unsigned int targetIndex{ (i / size.x) * buffer.width + (i % size.x) };
			buffer.cells[i] = buffer.cells[targetIndex];
		}

		buffer.cells.resize(newNumberOfCells);
	}
	if (size.y > currentBufferHeight)
	{
		buffer.cells.resize(newNumberOfCells);
		for (int i{ static_cast<int>(newNumberOfCells - 1) }; i >= 0; --i)
		{
			if (i / size.x >= currentBufferHeight)
			{
				buffer.cells[i] = defaultCell;
				continue;
			}
			const unsigned int targetIndex{ (i / size.x) * buffer.width + (i % size.x) };
			buffer.cells[i] = buffer.cells[targetIndex];
		}
	}
}

unsigned int ConsoleScreen::getNumberOfBuffers() const
{
	return static_cast<unsigned int>(m_buffers.size());
}

sf::Vector2u ConsoleScreen::getSizeOfBuffer(const unsigned int index) const
{
	if (!priv_isScreenBufferIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot get size of buffer.\nBuffer index (" + std::to_string(index) + ") out of range.");
		return sf::Vector2u();
	}

	const Buffer& buffer{ m_buffers[index] };
	return sf::Vector2u(static_cast<unsigned int>(buffer.cells.size()) % buffer.width, static_cast<unsigned int>(buffer.cells.size()) / buffer.width);
}

void ConsoleScreen::setMappedCharacter(const char character, const unsigned int value)
{
	m_characterMap[character] = value;
}

void ConsoleScreen::setMappedCharacters(const std::string& characters, unsigned int initialValue)
{
	for (auto& character : characters)
		setMappedCharacter(character, initialValue++);
}

void ConsoleScreen::removeMappedCharacter(const char character)
{
	m_characterMap.erase(character);
}

void ConsoleScreen::removeMappedCharacters(const std::string& characters)
{
	for (auto& character : characters)
		removeMappedCharacter(character);
}

bool ConsoleScreen::getIsMappedCharacter(const char character) const
{
	return m_characterMap.find(character) != m_characterMap.end();
}

unsigned int ConsoleScreen::getMappedCharacter(const char character) const
{
	return m_characterMap.at(character);
}

void ConsoleScreen::setMappedCursorCommandCharacter(const char character, const CursorCommand cursorCommand)
{
	m_characterMapCursorCommand[character] = cursorCommand;
}

void ConsoleScreen::setMappedCursorCommandCharacters(const std::string& characters, const std::vector<CursorCommand>& cursorCommands)
{
	for (unsigned int i{ 0u }; i < characters.size() && i < cursorCommands.size(); ++i)
		setMappedCursorCommandCharacter(characters[i], cursorCommands[i]);
}

void ConsoleScreen::removeMappedCursorCommandCharacter(const char character)
{
	m_characterMapCursorCommand.erase(character);
}

void ConsoleScreen::removeMappedCursorCommandCharacters(const std::string& characters)
{
	for (auto& character : characters)
		removeMappedCursorCommandCharacter(character);
}

bool ConsoleScreen::getIsMappedCursorCommandCharacter(const char character) const
{
	return m_characterMapCursorCommand.find(character) != m_characterMapCursorCommand.end();
}

ConsoleScreen::CursorCommand ConsoleScreen::getMappedCursorCommandCharacter(const char character) const
{
	return m_characterMapCursorCommand.at(character);
}

void ConsoleScreen::poke(const unsigned int index, const Cell& cell)
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

void ConsoleScreen::poke(const unsigned int index, const unsigned int value)
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

void ConsoleScreen::poke(const unsigned int index, const Color& color)
{
	if (!priv_isCellIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot poke color.\nCell number (" + std::to_string(index) + ") out of range.");
		return;
	}

	m_cells[index].colors.foreground = color;

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

void ConsoleScreen::poke(const unsigned int index, const Color& color, const Color& backgroundColor)
{
	if (!priv_isCellIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot poke colors.\nCell number (" + std::to_string(index) + ") out of range.");
		return;
	}

	m_cells[index].colors.foreground = color;
	m_cells[index].colors.background = backgroundColor;

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

void ConsoleScreen::poke(const unsigned int index, const StretchType& stretch)
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

void ConsoleScreen::poke(const unsigned int index, const CellAttributes& cellAttributes)
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

void ConsoleScreen::poke(const unsigned int index, const bool attributeValue, const Affect& attributeMask)
{
	if (!priv_isCellIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot poke attribute.\nCell number (" + std::to_string(index) + ") out of range.");
		return;
	}

	CellAttributes& cellAttributes{ m_cells[index].attributes };
	if ((attributeMask & Affect::Inverse) == Affect::Inverse)
		cellAttributes.inverse = attributeValue;
	if ((attributeMask & Affect::Bright) == Affect::Bright)
		cellAttributes.bright = attributeValue;
	if ((attributeMask & Affect::FlipX) == Affect::FlipX)
		cellAttributes.flipX = attributeValue;
	if ((attributeMask & Affect::FlipY) == Affect::FlipY)
		cellAttributes.flipY = attributeValue;

	if (m_do.updateAutomatically)
		priv_updateCell(index);
}

ConsoleScreen::Cell ConsoleScreen::peek(const unsigned int index) const
{
	if (!priv_isCellIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot peek cell.\nCell number (" + std::to_string(index) + ") out of range.");
		return Cell();
	}

	return m_cells[index];
}

ConsoleScreen::Cell& ConsoleScreen::cell(const unsigned int index)
{
	if (!priv_isCellIndexInRange(index))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot retrieve cell.\nCell number (" + std::to_string(index) + ") out of range.");
		return m_cells.empty() ? fakeCell : m_cells.back();
	}

	return m_cells[index];
}

ConsoleScreen::Cell& ConsoleScreen::bufferCell(const unsigned int bufferIndex, const unsigned int cellIndex)
{
	if (!priv_isScreenBufferIndexInRange(bufferIndex))
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot retrieve buffer cell.\nBuffer index (" + std::to_string(bufferIndex) + ") out of range.");
		return fakeCell;
	}

	if (cellIndex >= m_buffers[bufferIndex].cells.size())
	{
		if (m_do.throwExceptions)
			throw Exception(exceptionPrefix + "Cannot retrieve buffer cell.\nCell index (" + std::to_string(cellIndex) + ") out of range.");
		return m_buffers[bufferIndex].cells.empty() ? fakeCell : m_buffers[bufferIndex].cells.back();
	}

	return m_buffers[bufferIndex].cells[cellIndex];
}




// PRIVATE

void ConsoleScreen::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();

	if (m_do.showBackround && m_backgroundDisplay.size() > 0)
	{
		states.texture = nullptr;
		target.draw(&m_backgroundDisplay.front(), m_backgroundDisplay.size(), primitiveType, states);
	}
	states.texture = m_texture;
	if (m_underDisplay.size() > 0)
		target.draw(&m_underDisplay.front(), m_underDisplay.size(), primitiveType, states);
	if (m_display.size() > 0)
		target.draw(&m_display.front(), m_display.size(), primitiveType, states);
	if (m_overDisplay.size() > 0)
		target.draw(&m_overDisplay.front(), m_overDisplay.size(), primitiveType, states);
}

void ConsoleScreen::priv_setVerticesFromCell(unsigned int index, int baseVertex, const bool overLayer)
{
	bool mainLayer{ (baseVertex < 0) };

	Cell* pCell;
	sf::Vector2f offset{ 0.f, 0.f };
	if (mainLayer)
		pCell = &m_cells[index];
	else
	{
		if (overLayer)
		{
			pCell = &m_overCells[index].cell;
			offset = m_overCells[index].offset;
			index = m_overCells[index].index;
		}
		else
		{
			pCell = &m_underCells[index].cell;
			offset = m_underCells[index].offset;
			index = m_underCells[index].index;
		}
	}
	offset.x *= static_cast<float>(m_size.x) / m_mode.x;
	offset.y *= static_cast<float>(m_size.y) / m_mode.y;


	Cell& cell{ *pCell };

	unsigned int cellValue{ cell.value };
	sf::Color cellColor;
	sf::Color backgroundColor;

	if (mainLayer)
	{
		cellColor = getPaletteColor(priv_getModifiedColorFromCellUsingSpecifiedColorType(index, ColorType::Foreground).id);
		backgroundColor = getPaletteColor(priv_getModifiedColorFromCellUsingSpecifiedColorType(index, ColorType::Background).id);
	}
	else
	{
		cellColor = getPaletteColor(cell.colors.foreground.id);
		backgroundColor = getPaletteColor(cell.colors.background.id);
	}

	const bool isCursor{ (m_cursor.visible) && (m_cursorPrintProperties.index == index) };
	const bool isCursorBottom{ (m_cursor.visible) && (m_cursorPrintProperties.stretch == Cs::StretchType::Both) && (m_cursorPrintProperties.index + m_mode.x == index) };
	bool useCursorValue{ false };

	if (mainLayer && (isCursor || isCursorBottom))
	{
		if (m_cursor.value >= 0)
		{
			useCursorValue = true;
			cellValue = m_cursor.value;
		}
		if (m_cursor.useOwnColour)
			cellColor = getPaletteColor(m_cursor.color.id);
		if (cell.attributes.inverse != m_cursor.inverse)
			swapColors(cellColor, backgroundColor);
	}
	else if (cell.attributes.inverse)
		swapColors(cellColor, backgroundColor);

	if (!cell.attributes.bright)
	{
		makeColorUnBright(cellColor);
		if (mainLayer)
			makeColorUnBright(backgroundColor);
	}

	const unsigned int cellX{ index % m_mode.x };
	const unsigned int cellY{ index / m_mode.x };
	const float left{ offset.x + linearInterpolation(0.f, m_size.x, static_cast<float>(cellX + (cell.attributes.flipX ? 1 : 0)) / m_mode.x) };
	const float right{ offset.x + linearInterpolation(0.f, m_size.x, static_cast<float>(cellX + (cell.attributes.flipX ? 0 : 1)) / m_mode.x) };
	const float top{ offset.y + linearInterpolation(0.f, m_size.y, static_cast<float>(cellY + (cell.attributes.flipY ? 1 : 0)) / m_mode.y) };
	const float bottom{ offset.y + linearInterpolation(0.f, m_size.y, static_cast<float>(cellY + (cell.attributes.flipY ? 0 : 1)) / m_mode.y) };

	sf::Vector2u textureCell{ cellValue % m_numberOfTilesPerRow, cellValue / m_numberOfTilesPerRow };
	const float textureLeft{ static_cast<float>(m_textureOffset.x + textureCell.x * m_tileSize.x) };
	const float textureTop{ static_cast<float>(m_textureOffset.y + (textureCell.y + (!useCursorValue && cell.stretch == StretchType::Bottom ? 0.5f : 0.f)) * m_tileSize.y) };
	const float textureRight{ static_cast<float>(m_textureOffset.x + (textureCell.x + 1) * m_tileSize.x) };
	const float textureBottom{ static_cast<float>(m_textureOffset.y + (textureCell.y + (!useCursorValue && cell.stretch == StretchType::Top ? 0.5f : 1.f)) * m_tileSize.y) };

	if (mainLayer)
		baseVertex = index * 4;
	
	// main display
	sf::Vertex* const pVertex1{ (mainLayer ? &m_display[baseVertex] : overLayer ? &m_overDisplay[baseVertex] : &m_underDisplay[baseVertex]) };
	sf::Vertex* const pVertex2{ pVertex1 + 1 };
	sf::Vertex* const pVertex3{ pVertex1 + 2 };
	sf::Vertex* const pVertex4{ pVertex1 + 3 };

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
	if (mainLayer)
	{
		sf::Vertex* const pBackgroundVertex1{ &m_backgroundDisplay[baseVertex] };
		sf::Vertex* const pBackgroundVertex2{ pBackgroundVertex1 + 1 };
		sf::Vertex* const pBackgroundVertex3{ pBackgroundVertex1 + 2 };
		sf::Vertex* const pBackgroundVertex4{ pBackgroundVertex1 + 3 };

		pBackgroundVertex1->position = pVertex1->position;
		pBackgroundVertex2->position = pVertex2->position;
		pBackgroundVertex3->position = pVertex3->position;
		pBackgroundVertex4->position = pVertex4->position;

		pBackgroundVertex1->color = backgroundColor;
		pBackgroundVertex2->color = backgroundColor;
		pBackgroundVertex3->color = backgroundColor;
		pBackgroundVertex4->color = backgroundColor;
	}
}

void ConsoleScreen::priv_updateCell(const unsigned int index)
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

	priv_setVerticesFromCell(index);
}

void ConsoleScreen::priv_updateUnderCells()
{
	m_underDisplay.resize(m_underCells.size() * 4);
	unsigned int outOfRangeCells{ 0u };
	unsigned int baseVertex{ 0u };
	for (unsigned int i{ 0u }; i < m_underCells.size(); ++i)
	{
		if (!priv_isCellIndexInRange(m_underCells[i].index))
		{
			++outOfRangeCells;
			continue;
		}

		priv_setVerticesFromCell(i, baseVertex, false);

		baseVertex += 4;
	}
	if (outOfRangeCells > 0)
		m_underDisplay.resize(baseVertex);
}

void ConsoleScreen::priv_updateOverCells()
{
	m_overDisplay.resize(m_overCells.size() * 4);
	unsigned int outOfRangeCells{ 0u };
	unsigned int baseVertex{ 0u };
	for (unsigned int i{ 0u }; i < m_overCells.size(); ++i)
	{
		if (!priv_isCellIndexInRange(m_overCells[i].index))
		{
			++outOfRangeCells;
			continue;
		}

		priv_setVerticesFromCell(i, baseVertex, true);

		baseVertex += 4;
	}
	if (outOfRangeCells > 0)
		m_overDisplay.resize(baseVertex);
}

unsigned int ConsoleScreen::priv_cellIndex(const sf::Vector2u location) const
{
	return location.y * m_mode.x + location.x;
}

sf::Vector2u ConsoleScreen::priv_cellLocation(const unsigned int index) const
{
	return{ index % m_mode.x, index / m_mode.x };
}

bool ConsoleScreen::priv_isCellIndexInRange(const unsigned int index) const
{
	return index < m_cells.size();
}

bool ConsoleScreen::priv_isCellLocationInRange(const sf::Vector2u location) const
{
	return (location.x < m_mode.x && location.y < m_mode.y);
}

bool ConsoleScreen::priv_isScreenBufferIndexInRange(const unsigned int index) const
{
	return index < m_buffers.size();
}

bool ConsoleScreen::priv_isCursorInRange() const
{
	return priv_isCellIndexInRange(m_cursorPrintProperties.index);
}

bool ConsoleScreen::priv_isColorInPaletteRange(const Color color) const
{
	return (color.id >= 0) && ((m_is.rgbMode && color.id < 16777216L) || (color.id < static_cast<long int>(m_palette.size())));
}

void ConsoleScreen::priv_clearCell(const unsigned int index, const bool overwriteColor, const bool overwriteBackgroundColor)
{
	const Color color{ overwriteColor ? m_cursorPrintProperties.colors.foreground : m_cells[index].colors.foreground };
	const Color backgroundColor{ overwriteBackgroundColor ? m_cursorPrintProperties.colors.background : m_cells[index].colors.background };
	poke(index, { 0, ColorPair(color, backgroundColor), StretchType::None, CellAttributes() });
}

void ConsoleScreen::priv_clearCell(const unsigned int index, const Color& backgroundColor, const bool overwriteColor)
{
	const Color color{ overwriteColor ? m_cursorPrintProperties.colors.foreground : m_cells[index].colors.foreground };
	poke(index, { 0, ColorPair(color, backgroundColor), StretchType::None, CellAttributes() });
}

void ConsoleScreen::priv_clearCell(const unsigned int index, const Color& color, const Color& backgroundColor)
{
	poke(index, { 0, ColorPair(color, backgroundColor), StretchType::None, CellAttributes() });
}

void ConsoleScreen::priv_setCursorIndex(const unsigned int index)
{
	const unsigned int previousIndex{ m_cursorPrintProperties.index };
	m_cursorPrintProperties.index = index;

	if (m_do.updateAutomatically)
	{
		priv_updateCell(previousIndex);
		priv_updateCell(m_cursorPrintProperties.index);
		if (priv_getCurrentPrintProperties().stretch == Cs::StretchType::Both)
		{
			priv_updateCell(previousIndex + m_mode.x);
			priv_updateCell(m_cursorPrintProperties.index + m_mode.x);
		}
	}
}

void ConsoleScreen::priv_moveCursorToBeginningOfLine()
{
	priv_setCursorIndex(m_cursorPrintProperties.index - m_cursorPrintProperties.index % m_mode.x);
}

void ConsoleScreen::priv_moveCursorUp()
{
	if (m_cursorPrintProperties.index >= m_mode.x)
		priv_setCursorIndex(m_cursorPrintProperties.index - m_mode.x);
}

void ConsoleScreen::priv_moveCursorDown()
{
	priv_setCursorIndex(m_cursorPrintProperties.index + m_mode.x);
	priv_testCursorForScroll();
}

void ConsoleScreen::priv_moveCursorLeft()
{
	if (m_cursorPrintProperties.index > 0)
		priv_setCursorIndex(m_cursorPrintProperties.index - 1);
}

void ConsoleScreen::priv_moveCursorRight()
{
	priv_setCursorIndex(m_cursorPrintProperties.index + 1);
	priv_testCursorForScroll();
}

bool ConsoleScreen::priv_testCursorForScroll()
{
	bool result{ false };
	if (priv_getCurrentPrintProperties().stretch == Cs::StretchType::Both)
		m_cursorPrintProperties.index += m_mode.x;
	while (m_cursorPrintProperties.index >= m_cells.size())
	{
		if (m_do.scrollAutomatically)
		{
			result = true;
			priv_scroll();
			if (m_do.updateAutomatically)
				update();
		}
		else
		{
			m_cursorPrintProperties.index = static_cast<unsigned int>(m_cells.size()) - 1u;
			if (m_do.updateAutomatically)
				update();
		}
	}
	if (priv_getCurrentPrintProperties().stretch == Cs::StretchType::Both)
	{
		if (m_cursorPrintProperties.index > m_mode.x)
			m_cursorPrintProperties.index -= m_mode.x;
		else
			m_cursorPrintProperties.index = 0;
	}
	return result;
}

void ConsoleScreen::priv_scroll()
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

void ConsoleScreen::priv_copyToBufferFromSelectionRectangle(Buffer& buffer, const sf::IntRect& selectionRectangle)
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

void ConsoleScreen::priv_pasteOffsettedBuffer(Buffer& buffer, const sf::Vector2i& offset)
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

bool ConsoleScreen::priv_isSelectionRectangleContainedInScreen(const sf::IntRect& selectionRectangle)
{
	return (selectionRectangle.left >= 0 &&
		selectionRectangle.top >= 0 &&
		selectionRectangle.width >= 0 &&
		selectionRectangle.height >= 0 &&
		static_cast<unsigned int>(selectionRectangle.left + selectionRectangle.width) <= m_mode.x &&
		static_cast<unsigned int>(selectionRectangle.top + selectionRectangle.height) <= m_mode.y);
}

unsigned int ConsoleScreen::priv_getPrintIndex(sf::Vector2u location) const
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

unsigned int ConsoleScreen::priv_getCellValueFromCharacter(const char character) const
{
	if (getIsMappedCharacter(character))
		return m_characterMap.at(character);
	else
		return static_cast<unsigned int>(character);
}

char ConsoleScreen::priv_getCharacterFromCellValue(const unsigned int cellValue) const
{
	for (auto& pair : m_characterMap)
	{
		if (pair.second == cellValue)
			return pair.first;
	}
	return static_cast<char>(cellValue);
}

ConsoleScreen::Color ConsoleScreen::priv_getRandomColor() const
{
	return Color(std::uniform_int_distribution<unsigned long int>(0u, m_is.rgbMode ? 16777215L : static_cast<unsigned long int>(m_palette.size()) - 1u)(randomGenerator));
}

ConsoleScreen::PrintProperties& ConsoleScreen::priv_getCurrentPrintProperties()
{
	if (m_is.directPrinting)
		return m_directPrintProperties;
	else
		return m_cursorPrintProperties;
}

ConsoleScreen::PrintProperties& ConsoleScreen::priv_getPrintProperties(const PrintType& printType)
{
	switch (printType)
	{
	case PrintType::Cursor:
		return m_cursorPrintProperties;
	case PrintType::Direct:
		return m_directPrintProperties;
	case PrintType::Current:
	default:
		return priv_getCurrentPrintProperties();
	}
}

ConsoleScreen::Color ConsoleScreen::priv_getModifiedColorFromColorPairUsingSpecifiedColorType(const ColorPair& colors, const ColorType& colorType)
{
	const Color& focusColor{ (colorType == ColorType::Foreground ? colors.foreground : colors.background) };
	const Color& supportColor{ (colorType == ColorType::Foreground ? colors.background : colors.foreground) };

	if (focusColor.id >= 0)
		return focusColor;

	Color color;

	switch (focusColor.id)
	{
	case ColorCommand::Invert:
	{
		if (priv_isColorInPaletteRange(supportColor))
			color = priv_getIndexOfClosestPaletteColor(invertedColor(getPaletteColor(supportColor.id)));
	}
	break;
	case ColorCommand::Contrast:
	{
		if (priv_isColorInPaletteRange(supportColor))
			color = priv_getIndexOfClosestPaletteColor(contrastedColor(getPaletteColor(supportColor.id)));
	}
		break;
	case ColorCommand::Opposite:
		color = supportColor;
		break;
	default:
		color = focusColor;
	}

	if (color.id < 0)
		color = 0;

	return color;
}

ConsoleScreen::Color ConsoleScreen::priv_getModifiedColorFromCellUsingSpecifiedColorType(const unsigned int& cellIndex, const ColorType& colorType)
{
	return priv_getModifiedColorFromColorPairUsingSpecifiedColorType(m_cells[cellIndex].colors, colorType);
}

ConsoleScreen::Color& ConsoleScreen::priv_getActiveColor()
{
	PrintProperties& printProperties{ priv_getCurrentPrintProperties() };
	return (printProperties.colorType == ColorType::Foreground) ? printProperties.colors.foreground : printProperties.colors.background;
}

ConsoleScreen::Color& ConsoleScreen::priv_getInactiveColor()
{
	PrintProperties& printProperties{ priv_getCurrentPrintProperties() };
	return (printProperties.colorType == ColorType::Foreground) ? printProperties.colors.background : printProperties.colors.foreground;
}

int ConsoleScreen::priv_getIndexOfClosestPaletteColor(const sf::Color& color) const
{
	int index{ -1 }; // returns -1 if the palette is empty
	int currentDistance{ 195076 }; // 195076 = 255 * 255 * 3 + 1 (one further than the maximum possible distance)

	const unsigned int numberOfColorsInPalette{ static_cast<unsigned int>(m_palette.size()) };
	for (unsigned int i{ 0u }; i < numberOfColorsInPalette; ++i)
	{
		const int r{ static_cast<int>(m_palette[i].r) - color.r };
		const int g{ static_cast<int>(m_palette[i].g) - color.g };
		const int b{ static_cast<int>(m_palette[i].b) - color.b };
		const int distance{ r * r + g * g + b * b };
		if (distance < currentDistance)
		{
			index = i;
			currentDistance = distance;
		}
	}

	return index;
}

std::string ConsoleScreen::priv_read(unsigned int index, const bool unmapCharacters)
{
	std::string string;

	for (unsigned int i{ 0u }; i < m_readLength; ++i)
	{
		string += (unmapCharacters ? priv_getCharacterFromCellValue(m_cells[index].value) : static_cast<char>(m_cells[index].value));
		++index;
		if (!priv_isCellIndexInRange(index))
			break;
		priv_getCurrentPrintProperties().index = index;
	}

	return string;
}

void ConsoleScreen::priv_modifyCellUsingPrintProperties(const unsigned int index, const PrintType& printType, const StretchType stretch)
{
	PrintProperties& printProperties{ priv_getPrintProperties(printType) };
	Cell& currentCell = m_cells[index];
	if ((printProperties.affectBitmask & Affect::FgColor) == Affect::FgColor)
		currentCell.colors.foreground = printProperties.colors.foreground;
	if ((printProperties.affectBitmask & Affect::BgColor) == Affect::BgColor)
		currentCell.colors.background = printProperties.colors.background;
	if ((printProperties.affectBitmask & Affect::Inverse) == Affect::Inverse)
		currentCell.attributes.inverse = printProperties.attributes.inverse;
	if ((printProperties.affectBitmask & Affect::Bright) == Affect::Bright)
		currentCell.attributes.bright = printProperties.attributes.bright;
	if ((printProperties.affectBitmask & Affect::FlipX) == Affect::FlipX)
		currentCell.attributes.flipX = printProperties.attributes.flipX;
	if ((printProperties.affectBitmask & Affect::FlipY) == Affect::FlipY)
		currentCell.attributes.flipY = printProperties.attributes.flipY;
	if ((printProperties.affectBitmask & Affect::Stretch) == Affect::Stretch)
		currentCell.stretch = stretch;
}

} // namespace selbaward
