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

#ifndef SELBAWARD_CONSOLESCREEN_V2_HPP
#define SELBAWARD_CONSOLESCREEN_V2_HPP

#include "Common.hpp"

#include <unordered_map>

namespace selbaward
{

// SW Console Screen v2.3.2
class ConsoleScreen : public sf::Drawable, public sf::Transformable
{
public:
	using Location = sf::Vector2u;
	using Offset = sf::Vector2i;

	enum class Direction;
	enum class Palette;
	enum ColorCommand : int;
	enum Direct : int;
	enum TargetBufferCommand : int;
	struct Char;
	struct Fg;
	struct Bg;
	struct MovementControl;
	struct Left;
	struct Right;
	struct Up;
	struct Down;
	struct Wipe;

	enum class StretchType
	{
		None,
		Top,
		Bottom,
		Both
	};
	enum class CursorCommand // cursor command (able to map a character to these)
	{
		None = 0,
		Newline,
		Tab,
		TabReverse,
		Backspace, // affects cell contents
		Delete, // affects cell contents
		Left,
		Right,
		Up,
		Down,
		Home,
		End,
		HomeLine,
		EndLine,
	};
	enum class PrintType
	{
		Cursor,
		Direct,
		Current
	};
	enum Affect
	{
		None = 0,
		Value = 1,
		FgColor = 2,
		BgColor = 4,
		Stretch = 8,
		Inverse = 16,
		Bright = 32,
		FlipX = 64,
		FlipY = 128,
		Print = Value | FgColor | Stretch,
		Default = Print | BgColor,
		Attribs = Inverse | Bright | FlipX | FlipY,
		Paint = FgColor | BgColor,
		All = 255
	};
	enum ColorType
	{
		Foreground,
		Background
	};
	struct Color
	{
		long int id;
		Color() : id(0) { }
		Color(long int newId) : id(newId) { }
		explicit Color(ColorCommand command) : id(static_cast<long int>(command)) { }
	};
	struct ColorPair
	{
		Color foreground;
		Color background;
		ColorPair() : foreground(1), background(0) { }
		ColorPair(Color fg, Color bg) : foreground(fg), background(bg) { }
	};
	struct CellAttributes
	{
		bool inverse;
		bool bright;
		bool flipX;
		bool flipY;
		CellAttributes() : inverse(false), bright(true), flipX(false), flipY(false) { }
		explicit CellAttributes(const bool newInverse, const bool newBright, const bool newFlipX, const bool newFlipY) : inverse(newInverse), bright(newBright), flipX(newFlipX), flipY(newFlipY) { }
		explicit CellAttributes(const unsigned int attributeMask)
			: inverse((attributeMask & Affect::Inverse) == Affect::Inverse)
			, bright((attributeMask & Affect::Bright) == Affect::Bright)
			, flipX((attributeMask & Affect::FlipX) == Affect::FlipX)
			, flipY((attributeMask & Affect::FlipY) == Affect::FlipY)
		{ }
	};
	struct Cell
	{
		unsigned int value;
		ColorPair colors;
		StretchType stretch;
		CellAttributes attributes;
	};



	
















	// setup
	explicit ConsoleScreen(sf::Vector2u mode = { 80u, 45u });
	void setMode(sf::Vector2u mode); // "mode" is the number of cells used to fit the screen
	sf::Vector2u getMode() const;
	unsigned int getNumberOfCells() const;
	sf::Vector2u getNumberOfTilesInTexture2d() const;
	unsigned int getNumberOfTilesInTexture() const;

	// texture
	void setTexture(const sf::Texture& texture);
	void setTexture();
	void setTextureOffset(sf::Vector2u textureOffset = { 0u, 0u });
	void setTextureTileSize(sf::Vector2u tileSize = { 8u, 8u });
	void setNumberOfTextureTilesPerRow(unsigned int numberOfTextureTilesPerRow = 1);

	// switches
	void setThrowExceptions(bool exceptions);
	void setUpdateAutomatically(bool automaticUpdate);
	void setShowCursor(bool showCursor);
	void setInvertCursor(bool invertCursor);
	void setUseCursorColor(bool useCursorColor);
	void setShowBackground(bool showBackground);
	void setScrollAutomatically(bool scroll);
	void setWrapOnManualScroll(bool wrapOnManualScroll);
	void setAddNewColorToPalette(bool addNewColorToPalette);
	bool getThrowExceptions() const;
	bool getUpdateAutomatically() const;
	bool getShowCursor() const;
	bool getInvertCursor() const;
	bool getUseCursorColor() const;
	bool getShowBackground() const;
	bool getScrollAutomatically() const;
	bool getWrapOnManualScroll() const;
	bool getAddNewColorToPalette() const;

	// visual representation
	void setSize(sf::Vector2f size);
	sf::Vector2f getSize() const;
	sf::Vector2f getPerfectSize() const;
	sf::FloatRect getLocalBounds() const;
	sf::FloatRect getGlobalBounds() const;

	// global
	void update();
	void fill(Cell cell);
	void clear(ColorPair colors);
	void clear(Color backgroundColor);
	void clear(ColorCommand colorCommand);
	void clear();
	void crash();

	// stream
	ConsoleScreen& operator<<(const std::string& string);
	ConsoleScreen& operator<<(const Char& csChar);
	ConsoleScreen& operator<<(const Direct& direct);
	ConsoleScreen& operator<<(const Location& location);
	ConsoleScreen& operator<<(const Offset& offset);
	ConsoleScreen& operator<<(const Affect& affect);
	ConsoleScreen& operator<<(unsigned int affectMask);
	ConsoleScreen& operator<<(const ColorType& colorType);
	ConsoleScreen& operator<<(const Color& color);
	ConsoleScreen& operator<<(const sf::Color& newColor); // action depends on action flag. either adds this colour to the palette or finds the closest match in the palette. either way, sets the colour to the resulting palette id.
	ConsoleScreen& operator<<(const ColorPair& colorPair);
	ConsoleScreen& operator<<(const Fg& fg);
	ConsoleScreen& operator<<(const Bg& bg);
	ConsoleScreen& operator<<(const Wipe& length);
	ConsoleScreen& operator<<(const StretchType& stretchType);
	ConsoleScreen& operator<<(const CellAttributes& cellAttributes);
	ConsoleScreen& operator<<(const MovementControl& movementControl);
	ConsoleScreen& operator<<(const CursorCommand& cursorCommand);

	ConsoleScreen& operator>>(std::string& string);
	ConsoleScreen& operator>>(unsigned int length);
	ConsoleScreen& operator>>(const Direct& direct);
	ConsoleScreen& operator>>(const Location& location);
	ConsoleScreen& operator>>(const Offset& offset);
	ConsoleScreen& operator>>(const MovementControl& movementControl);
	ConsoleScreen& operator>>(const CursorCommand& cursorCommand);

	// print properties
	void resetPrintProperties(PrintType printType = PrintType::Current);
	Location getLocation(PrintType printType = PrintType::Current);
	unsigned int getIndex(PrintType printType = PrintType::Current);
	ColorPair getColors(PrintType printType = PrintType::Current);
	StretchType getStretchType(PrintType printType = PrintType::Current);
	CellAttributes getCellAttributes(PrintType printType = PrintType::Current);
	unsigned int getAffectBitmask(PrintType printType = PrintType::Current);
	ColorType getColorType(PrintType printType = PrintType::Current);

	// print
	void print(char character);
	void print(const std::string& string);
	void print(const Location& location, char character);
	void print(const Location& location, const std::string& string);

	// stack printing (over- and under-printing)
	void addOverAt(const Location& location, char character, sf::Vector2f offset = { 0.f, 0.f });
	void addOverAt(const Location& location, const std::string& string, sf::Vector2f offset = { 0.f, 0.f });
	void addOverAt(const Location& location, const Cell& cell, sf::Vector2f offset = { 0.f, 0.f });
	void addUnderAt(const Location& location, char character, sf::Vector2f offset = { 0.f, 0.f });
	void addUnderAt(const Location& location, const std::string& string, sf::Vector2f offset = { 0.f, 0.f });
	void addUnderAt(const Location& location, const Cell& cell, sf::Vector2f offset = { 0.f, 0.f });
	void clearOversAt(const Location& location);
	void clearUndersAt(const Location& location);
	void clearStackAt(const Location& location);
	void clearOvers();
	void clearUnders();
	void clearStack();

	// stack printing from buffers
	void pasteOver(sf::Vector2i offset = sf::Vector2i(0, 0));
	void pasteUnder(sf::Vector2i offset = sf::Vector2i(0, 0));
	void pasteOver(unsigned int index, sf::Vector2i offset = sf::Vector2i(0, 0));
	void pasteUnder(unsigned int index, sf::Vector2i offset = sf::Vector2i(0, 0));

	// read
	std::string read();
	std::string read(unsigned int length);
	std::string read(const Location& location);
	std::string read(const Location& location, unsigned int length);

	// tabs
	void setCursorTab(unsigned int tabSize);
	unsigned int getCursorTab() const;

	// cursor
	void setCursor(int cellValue); // value (tile used to represent cursor)
	void setCursor(char cellChar, bool mapCharacter = true); // converts from character to cell value. this can map the character if the character has an associated mapping
	void setCursorColor(const Color& color);
	int getCursorValue() const;
	char getCursorChar(bool mapCharacter = true) const;
	Color getCursorColor() const;

	// cell manipulation
	void clearCellAt(sf::Vector2u location);
	void setCellAt(sf::Vector2u location, const Cell& cell);
	void setValueAt(sf::Vector2u location, unsigned int value);
	void setForegroundColorAt(sf::Vector2u location, Color color);
	void setBackgroundColorAt(sf::Vector2u location, Color backgroundColor);
	void setColorsAt(sf::Vector2u location, Color color, Color backgroundColor);
	void setStretchAt(sf::Vector2u location, const StretchType& stretch);
	void setAttributesAt(sf::Vector2u location, const CellAttributes& attributes);
	void setAttributesAt(sf::Vector2u location, const Affect& attributeMask);
	void setAttributesToAt(sf::Vector2u location, bool attributeValue, const Affect& attributeMask);

	// cell information
	Cell getCellAt(sf::Vector2u location) const;
	unsigned int getValueAt(sf::Vector2u location) const;
	Color getColorAt(sf::Vector2u location) const;
	Color getBackgroundColorAt(sf::Vector2u location) const;
	StretchType getStretchAt(sf::Vector2u location) const;
	CellAttributes getAttributesAt(sf::Vector2u location) const;
	bool getAttributeAt(sf::Vector2u location, const Affect& attributeMask);

	// manual scrolling
	void scrollUp(unsigned int amount = 1);
	void scrollDown(unsigned int amount = 1);
	void scrollLeft(unsigned int amount = 1);
	void scrollRight(unsigned int amount = 1);
	void scrollUp(unsigned int amount, sf::IntRect selectionRectangle);
	void scrollDown(unsigned int amount, sf::IntRect selectionRectangle);
	void scrollLeft(unsigned int amount, sf::IntRect selectionRectangle);
	void scrollRight(unsigned int amount, sf::IntRect selectionRectangle);

	// colour palette
	void loadPalette(Palette palette);
	void addColorToPalette(sf::Color color);
	void setPaletteColor(Color color, sf::Color newColor);
	sf::Color getPaletteColor(Color color) const;
	void setPaletteSize(unsigned long int size);
	unsigned long int getPaletteSize() const;
	void removePaletteColor(Color color);
	void cyclePaletteUp(long amount = 1);
	void cyclePaletteDown(long amount = 1);
	void cyclePaletteUp(long amount, Color firstColor, Color lastColor); // first colour and last colour of range to cycle
	void cyclePaletteDown(long amount, Color firstColor, Color lastColor); // first colour and last colour of range to cycle

	// buffers/clipboards/"screenshots"/captures
	unsigned int copy(); // returns index of buffer
	unsigned int copy(sf::IntRect selectionRectangle); // returns index of buffer
	void paste(sf::Vector2i offset = sf::Vector2i(0, 0)); // replace screen with last saved buffer (recalls actual cell data, not the state of the console screen e.g. cursor isn't restored)
	void removeBuffer(); // removes last saved buffer
	void copy(unsigned int index); // copies over (replaces) an existing buffer
	void copy(unsigned int index, sf::IntRect selectionRectangle);
	void paste(unsigned int index, sf::Vector2i offset = sf::Vector2i(0, 0)); // replace screen with saved buffer (recalls actual cell data, not the state of the console screen e.g. cursor isn't restored)
	void removeBuffer(unsigned int index); // as usual, when one buffer is removed, the indices of all following buffers are decreased
	void removeAllBuffers();
	unsigned int addBuffer(sf::Vector2u size = { 1u, 1u }); // returns index of new buffer
	void resizeBuffer(unsigned int index, sf::Vector2u size);
	unsigned int getNumberOfBuffers() const;
	sf::Vector2u getSizeOfBuffer(unsigned int index) const;

	// character mapping (to cell values)
	void setMappedCharacter(char character, unsigned int value);
	void setMappedCharacters(const std::string& characters, unsigned int initialValue);
	void removeMappedCharacter(char character);
	void removeMappedCharacters(const std::string& characters);
	bool getIsMappedCharacter(char character) const;
	unsigned int getMappedCharacter(char character) const;

	// character mapping (to cursor commands)
	void setMappedCursorCommandCharacter(char character, CursorCommand cursorCommand);
	void setMappedCursorCommandCharacters(const std::string& characters, const std::vector<CursorCommand>& cursorCommands);
	void removeMappedCursorCommandCharacter(char character);
	void removeMappedCursorCommandCharacters(const std::string& characters);
	bool getIsMappedCursorCommandCharacter(char character) const;
	CursorCommand getMappedCursorCommandCharacter(char character) const;

	// direct manipulation
	void poke(unsigned int index, const Cell& cell);
	void poke(unsigned int index, unsigned int value);
	void poke(unsigned int index, const Color& color);
	void poke(unsigned int index, const Color& color, const Color& backgroundColor);
	void poke(unsigned int index, const StretchType& stretch);
	void poke(unsigned int index, bool attributeValue, const Affect& attributeMask);
	void poke(unsigned int index, const CellAttributes& attributes);
	Cell peek(unsigned int index) const;
	Cell& cell(unsigned int index);
	Cell& bufferCell(unsigned int bufferIndex, unsigned int cellIndex);
	




















private:
	struct StackCell
	{
		unsigned int index;
		sf::Vector2f offset;
		Cell cell;
	};

	struct PrintProperties
	{
		unsigned int index;
		ColorPair colors;
		StretchType stretch;
		CellAttributes attributes;
		unsigned int affectBitmask;
		ColorType colorType;
	};

	struct CursorProperties
	{
		int value; // if -1, doesn't change the cell's value (allows other cursor features to still be displayed)
		Color color;
		bool visible;
		bool inverse;
		bool useOwnColour;
	};

	struct ActionFlags
	{
		bool throwExceptions = true;
		bool showBackround = true;
		bool updateAutomatically = true;
		bool scrollAutomatically = true;
		bool wrapOnManualScroll = false;
		bool addNewColorToPalette = false; // if false, new colours are matched to nearest palette colour
	};
	struct StateFlags
	{
		bool directPrinting = false;
		bool rgbMode = false;
	};

	// flags
	ActionFlags m_do;
	StateFlags m_is;

	// print properties
	PrintProperties m_cursorPrintProperties;
	PrintProperties m_directPrintProperties;

	const PrintProperties m_defaultPrintProperties;

	// definition
	using Cells = std::vector<Cell>;
	Cells m_cells;
	sf::Vector2u m_mode;

	// stack (under- and over-drawing) definition
	std::vector<StackCell> m_overCells;
	std::vector<StackCell> m_underCells;

	// buffers
	struct Buffer
	{
		unsigned int width;
		Cells cells;
	};
	std::vector<Buffer> m_buffers;

	// cursor
	CursorProperties m_cursor;

	// tab
	unsigned int m_tabSize;

	// read
	unsigned int m_readLength;
	
	// colour palette
	std::vector<sf::Color> m_palette;

	// character map (character mapped to cell value)
	std::unordered_map<char, unsigned int> m_characterMap;

	// character map for cursor commands (character mapped to a cursor command)
	std::unordered_map<char, CursorCommand> m_characterMapCursorCommand;

	// visual representation
	std::vector<sf::Vertex> m_display;
	std::vector<sf::Vertex> m_backgroundDisplay;
	std::vector<sf::Vertex> m_overDisplay;
	std::vector<sf::Vertex> m_underDisplay;
	sf::Vector2f m_size;
	const sf::Texture* m_texture;
	sf::Vector2u m_textureOffset;
	sf::Vector2u m_tileSize;
	unsigned int m_numberOfTilesPerRow;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	void priv_setVerticesFromCell(unsigned int index, int baseVertex = -1, bool overLayer = true);
	void priv_updateCell(unsigned int index);
	void priv_updateUnderCells();
	void priv_updateOverCells();
	unsigned int priv_cellIndex(sf::Vector2u location) const;
	sf::Vector2u priv_cellLocation(unsigned int index) const;
	bool priv_isCellIndexInRange(unsigned int index) const;
	bool priv_isCellLocationInRange(sf::Vector2u location) const;
	bool priv_isScreenBufferIndexInRange(unsigned int index) const;
	bool priv_isCursorInRange() const;
	bool priv_isColorInPaletteRange(Color color) const;
	void priv_clearCell(unsigned int index, bool overwriteColor, bool overwriteBackgroundColor);
	void priv_clearCell(unsigned int index, const Color& backgroundColor, bool overwriteColor);
	void priv_clearCell(unsigned int index, const Color& color, const Color& backgroundColor);
	void priv_setCursorIndex(unsigned int index);
	void priv_moveCursorToBeginningOfLine();
	void priv_moveCursorUp();
	void priv_moveCursorDown();
	void priv_moveCursorLeft();
	void priv_moveCursorRight();
	bool priv_testCursorForScroll();
	void priv_scroll();
	void priv_copyToBufferFromSelectionRectangle(Buffer& buffer, const sf::IntRect& selectionRectangle);
	void priv_pasteOffsettedBuffer(Buffer& buffer, const sf::Vector2i& offset);
	bool priv_isSelectionRectangleContainedInScreen(const sf::IntRect& selectionRectangle);
	unsigned int priv_getPrintIndex(sf::Vector2u location) const;
	unsigned int priv_getCellValueFromCharacter(char character) const;
	char priv_getCharacterFromCellValue(unsigned int cellValue) const;
	Color priv_getRandomColor() const;
	PrintProperties& priv_getCurrentPrintProperties();
	PrintProperties& priv_getPrintProperties(const PrintType& printType);
	Color priv_getModifiedColorFromColorPairUsingSpecifiedColorType(const ColorPair& colors, const ColorType& colorType);
	Color priv_getModifiedColorFromCellUsingSpecifiedColorType(const unsigned int& cellIndex, const ColorType& colorType);
	Color& priv_getActiveColor();
	Color& priv_getInactiveColor();
	int priv_getIndexOfClosestPaletteColor(const sf::Color& color) const; // returns -1 if the palette is empty
	std::string priv_read(unsigned int index, const bool unmapCharacters = true);
	void priv_modifyCellUsingPrintProperties(unsigned int index, const PrintType& printType, const StretchType stretch);
};




















enum class ConsoleScreen::Palette
{
	Default, // basic 16-colour palette
	Colors2BlackWhite,
	Colors2WhiteBlack,
	Colors16Greenscale,
	Colors16Grayscale,
	Colors16Sepia,
	Colors16Cga,
	Colors16CgaNonIbm,
	Colors16Windows,
	Colors16Mac,
	Colors16ZxSpectrum,
	Colors16Html,
	Colors216Web,
	Colors256Greenscale,
	Colors256Grayscale,
	Colors256Sepia,
	ColorsRgb // 24-bit RGB colours. palette of 16,777,216 (0x1000000) colours that are ordered so that the palette index can be specified like this: RRGGBB in hex e.g. 0x00FF00 for green
};

enum class ConsoleScreen::Direction
{
	Left,
	Right,
	Up,
	Down
};

enum ConsoleScreen::Direct : int
{
	Begin,
	End
};

enum ConsoleScreen::ColorCommand : int // colour command
{
	Unused = -1, // unused. returns a Color created from this ColorCommand if no Color can be determined such as when an exception is thrown from a method that returns a Color but exceptions are switched off
	Opposite = -2, // use opposite colour (i.e. foreground color for background color and background color for foreground color)
	Invert = -3, // invert the opposite colour ("main colour = invert" would set main colour to inverted background colour and vice versa)
	Contrast = -4, // contrast the opposite colour ("main colour = contrast" would set main colour to contrast background colour and vice versa). contrast is black or white based on opposite's luminance
};

enum ConsoleScreen::TargetBufferCommand : int
{
	Screen = -1, // targets the screen directly
	First = -2, // targets the first buffer if one is available, otherwise targets the screen
	Last = -3 // targets the last buffer if one is available, otherwise targets the screen
};

struct ConsoleScreen::Char
{
	char character;
	explicit Char(char singleCharacter) : character(singleCharacter) { }
};

struct ConsoleScreen::Fg
{
	Color color;
	Fg(Color foregroundColor) : color(foregroundColor) { }
};

struct ConsoleScreen::Bg
{
	Color color;
	Bg(Color backgroundColor) : color(backgroundColor) { }
};

struct ConsoleScreen::MovementControl
{
	const unsigned int amount;
	Direction direction;
	explicit MovementControl(const unsigned int distance = 1u) : amount(distance) { }
};

struct ConsoleScreen::Left : public MovementControl
{
	explicit Left(const unsigned int distance = 1u) : MovementControl(distance) { direction = Direction::Left; }
};

struct ConsoleScreen::Right : public MovementControl
{
	explicit Right(const unsigned int distance = 1u) : MovementControl(distance) { direction = Direction::Right; }
};

struct ConsoleScreen::Up : public MovementControl
{
	explicit Up(const unsigned int distance = 1u) : MovementControl(distance) { direction = Direction::Up; }
};

struct ConsoleScreen::Down : public MovementControl
{
	explicit Down(const unsigned int distance = 1u) : MovementControl(distance) { direction = Direction::Down; }
};

struct ConsoleScreen::Wipe
{
	std::string string;
	explicit Wipe(const unsigned int length) : string(length, ' ') { }
};

} // namespace selbaward

#ifndef SELBAWARD_CONSOLESCREEN_NO_CLASSNAME_SHORTCUT
using Cs = selbaward::ConsoleScreen; // create shortcut class name (also reduces need for selbaward's namespace)
#endif // SELBAWARD_CONSOLESCREEN_NO_CLASSNAME_SHORTCUT

#endif // SELBAWARD_CONSOLESCREEN_V2_HPP
