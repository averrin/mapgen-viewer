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

#ifndef SELBAWARD_CONSOLESCREEN_V1_HPP
#define SELBAWARD_CONSOLESCREEN_V1_HPP

#include "Common.hpp"

#include <unordered_map>

namespace selbaward
{

// SW Console Screen v1.5.1
class ConsoleScreenV1 : public sf::Drawable, public sf::Transformable
{
public:
	struct Cell;
	enum class Palette;
	enum class Stretch
	{
		None,
		Top,
		Bottom
	};
	enum class Attribute
	{
		Inverse,
		Bright,
		FlipX,
		FlipY,
	};
	struct CellAttributes
	{
		bool inverse{ false };
		bool bright{ true };
		bool flipX{ false };
		bool flipY{ false };
	};
	enum Color // colour command
	{
		Current = -1, // use current colour
		Ignore = -2, // ignore colour (leave cell's colour as it is)
		Invert = -3, // invert the opposite colour ("main colour = invert" would set main colour to inverted background colour and vice versa)
		Contrast = -4 // contrast the opposite colour ("main colour = contrast" would set main colour to contrast background colour and vice versa). contrast is black or white based on opposite's luminance
	};

	// setup
	ConsoleScreenV1(sf::Vector2u mode = { 80u, 45u });
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
	bool getThrowExceptions() const;
	bool getUpdateAutomatically() const;
	bool getShowCursor() const;
	bool getInvertCursor() const;
	bool getUseCursorColor() const;
	bool getShowBackground() const;
	bool getScrollAutomatically() const;
	bool getWrapOnManualScroll() const;

	// visual representation
	void setSize(sf::Vector2f size);
	sf::Vector2f getSize() const;
	sf::FloatRect getLocalBounds() const;
	sf::FloatRect getGlobalBounds() const;

	// global
	void update();
	void clear(int backgroundColorId = Color::Current);
	void clear(sf::Color backgroundColor);
	void crash();

	// current colours
	void setColor(sf::Color color);
	void setColor(int colorId);
	void setBackgroundColor(sf::Color backgroundColor);
	void setBackgroundColor(int colorId);
	void setCursorColor(sf::Color cursorColor);
	void setCursorColor(int colorId);
	void setColors(sf::Color color, sf::Color backgroundColor);
	void setColors(sf::Color color, sf::Color backgroundColor, sf::Color cursorColor);
	void setColors(int colorId, int backgroundColorId);
	void setColors(int colorId, int backgroundColorId, int cursorColorId);
	sf::Color getColor() const;
	sf::Color getBackgroundColor() const;
	sf::Color getCursorColor() const;

	// current stretch
	void setStretch(Stretch stretch);
	Stretch getStretch() const;

	// current attributes
	void setAttributes(CellAttributes attributes);
	void setAttribute(bool attributeValue, Attribute attribute = Attribute::Inverse);
	CellAttributes getAttributes() const;
	bool getAttribute(Attribute attribute = Attribute::Inverse);

	// cursor position
	void cursorHome();
	void cursorHomeLine();
	void cursorEnd();
	void cursorEndLine();
	void cursorTab(unsigned int tabSize = 4); // move cursor right in a tab-like way. if the end of a line is passed, the beginning of the next line is the next tab
	void cursorTabReverse(unsigned int tabSize = 4); // move cursor left in a tab-like way. if the beginning of a line is passed, the last tab position of the previous line is the destination
	void cursorNextline();
	void cursorBackspace(); // clears the cursor's destination's cell
	void cursorLeft(unsigned int distance = 1);
	void cursorRight(unsigned int distance = 1);
	void cursorUp(unsigned int distance = 1);
	void cursorDown(unsigned int distance = 1);
	void moveCursor(sf::Vector2i offset);
	void setCursor(sf::Vector2u location);
	sf::Vector2u getCursor() const;

	// cursor value (tile/character used to represent cursor)
	void setCursor(int cellValue);
	void setCursor(char cellChar, bool mapCharacter); // converts from character to cell value. this can map the character if the character has an associated mapping

	// printing (using cursor)
	void print(char character, int colorId = Color::Current, int backgroundColorId = Color::Current);
	void print(char character, const Stretch& stretch, int colorId = Color::Current, int backgroundColorId = Color::Current);
	void print(char character, const CellAttributes& attributes, int colorId = Color::Current, int backgroundColorId = Color::Current);
	void print(const std::string& string, int colorId = Color::Current, int backgroundColorId = Color::Current);
	void print(const std::string& string, const Stretch& stretch, int colorId = Color::Current, int backgroundColorId = Color::Current);
	void print(const std::string& string, const CellAttributes& attributes, int colorId = Color::Current, int backgroundColorId = Color::Current);
	void printLine(const std::string& string, int colorId = Color::Current, int backgroundColorId = Color::Current);

	// printing (directly)
	void printAt(sf::Vector2u location, const std::string& string, sf::Color color, sf::Color backgroundColor);
	void printAt(sf::Vector2u location, const std::string& string, sf::Color color, int backgroundColorId = Color::Ignore);
	void printAt(sf::Vector2u location, const std::string& string, int colorId, sf::Color backgroundColor);
	void printAt(sf::Vector2u location, const std::string& string, int colorId = Color::Ignore, int backgroundColorId = Color::Ignore);
	void printAt(sf::Vector2u location, char character, sf::Color color, sf::Color backgroundColor);
	void printAt(sf::Vector2u location, char character, sf::Color color, int backgroundColorId = Color::Ignore);
	void printAt(sf::Vector2u location, char character, int colorId, sf::Color backgroundColor);
	void printAt(sf::Vector2u location, char character, int colorId = Color::Ignore, int backgroundColorId = Color::Ignore);

	// printing stretched (directly - cannot mix sf::Colors and color IDs therefore cannot supply only foreground colour)
	void printStretchedAt(sf::Vector2u location, const std::string& string, const Stretch& stretch, sf::Color color, sf::Color backgroundColor);
	void printStretchedAt(sf::Vector2u location, const std::string& string, const Stretch& stretch = Stretch::Top, int colorId = Color::Ignore, int backgroundColorId = Color::Ignore);
	void printStretchedAt(sf::Vector2u location, char character, const Stretch& stretch, sf::Color color, sf::Color backgroundColor);
	void printStretchedAt(sf::Vector2u location, char character, const Stretch& stretch = Stretch::Top, int colorId = Color::Ignore, int backgroundColorId = Color::Ignore);

	// painting (directly)
	void paintAt(sf::Vector2u location, unsigned int length, sf::Color color, sf::Color backgroundColor);
	void paintAt(sf::Vector2u location, unsigned int length, sf::Color color, int backgroundColorId = Color::Ignore);
	void paintAt(sf::Vector2u location, unsigned int length, int colorId, sf::Color backgroundColor);
	void paintAt(sf::Vector2u location, unsigned int length, int colorId = Color::Current, int backgroundColorId = Color::Ignore);

	// painting attributes (directly - flags only)
	void paintAttributeAt(sf::Vector2u location, unsigned int length, bool attributeValue, Attribute attribute = Attribute::Inverse);

	// reading
	std::string read(unsigned int length = 1u, bool unmapCharacters = true); // not const because cursor is moved by reading

	// reading (directly)
	std::string readAt(sf::Vector2u location, unsigned int length = 1u, bool unmapCharacters = true) const;

	// cell manipulation
	void clearCellAt(sf::Vector2u location);
	void setCellAt(sf::Vector2u location, const Cell& cell);
	void setValueAt(sf::Vector2u location, unsigned int value);
	void setColorAt(sf::Vector2u location, sf::Color color);
	void setColorAt(sf::Vector2u location, int colorId);
	void setBackgroundColorAt(sf::Vector2u location, sf::Color backgroundColor);
	void setBackgroundColorAt(sf::Vector2u location, int backgroundColorId);
	void setColorsAt(sf::Vector2u location, sf::Color color, sf::Color backgroundColor);
	void setColorsAt(sf::Vector2u location, int colorId, int backgroundColorId);
	void setStretchAt(sf::Vector2u location, const Stretch& stretch);
	void setAttributesAt(sf::Vector2u location, const CellAttributes& attributes);
	void setAttributeAt(sf::Vector2u location, bool attributeValue, Attribute attribute = Attribute::Inverse);

	// cell information
	Cell getCellAt(sf::Vector2u location) const;
	unsigned int getValueAt(sf::Vector2u location) const;
	sf::Color getColorAt(sf::Vector2u location) const;
	sf::Color getBackgroundColorAt(sf::Vector2u location) const;
	Stretch getStretchAt(sf::Vector2u location) const;
	CellAttributes getAttributesAt(sf::Vector2u location) const;
	bool getAttributeAt(sf::Vector2u location, Attribute attribute = Attribute::Inverse);

	// manual scrolling
	void scrollUp(unsigned int amount = 1);
	void scrollDown(unsigned int amount = 1);
	void scrollLeft(unsigned int amount = 1);
	void scrollRight(unsigned int amount = 1);

	// colour palette
	void loadPalette(Palette palette);
	void addColorToPalette(sf::Color color);
	void setPaletteColor(int colorId, sf::Color color);
	sf::Color getPaletteColor(int colorId) const;
	void setPaletteSize(unsigned int size);
	unsigned int getPaletteSize() const;
	void removePaletteColor(int colorId);

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
	unsigned int getNumberOfBuffers() const;

	// character mapping (to cell values)
	void setMappedCharacter(char character, unsigned int value);
	void setMappedCharacters(const std::string& characters, unsigned int initialValue);
	void removeMappedCharacter(char character);
	void removeMappedCharacters(const std::string& characters);
	bool getIsMappedCharacter(char character) const;
	unsigned int getMappedCharacter(char character) const;

	// direct manipulation
	void poke(unsigned int index, const Cell& cell);
	void poke(unsigned int index, unsigned int value);
	void poke(unsigned int index, const sf::Color& color);
	void poke(unsigned int index, const sf::Color& color, const sf::Color& backgroundColor);
	void poke(unsigned int index, const Stretch& stretch);
	void poke(unsigned int index, bool attributeValue, Attribute attribute = Attribute::Inverse);
	void poke(unsigned int index, const CellAttributes& attributes);
	Cell peek(unsigned int index) const;










private:
	struct CurrentColors
	{
		sf::Color main;
		sf::Color background;
		sf::Color cursor;
	};

	struct Cursor
	{
		unsigned int index;
		int value; // if -1, doesn't change the cell's value (allows other cursor features to still be displayed)
		bool visible;
		bool inverse;
		bool useOwnColour;
	};

	struct Flags
	{
		bool throwExceptions = true;
		bool showBackround = true;
		bool updateAutomatically = true;
		bool scrollAutomatically = true;
		bool wrapOnManualScroll = false;
	};

	// flags
	Flags m_do;

	// definition
	using Cells = std::vector<Cell>;
	Cells m_cells;
	sf::Vector2u m_mode;

	// buffers
	struct Buffer
	{
		unsigned int width;
		Cells cells;
	};
	std::vector<Buffer> m_buffers;

	// cursor
	Cursor m_cursor;

	// current colours
	CurrentColors m_colors;

	// current stretch
	Stretch m_stretch;

	// current attributes
	CellAttributes m_attributes;

	// colour palette
	std::vector<sf::Color> m_palette;

	// character map (mapped to cell value)
	std::unordered_map<char, unsigned int> m_characterMap;

	// visual representation
	const sf::PrimitiveType m_primitiveType;
	std::vector<sf::Vertex> m_display;
	std::vector<sf::Vertex> m_backgroundDisplay;
	sf::Vector2f m_size;
	const sf::Texture* m_texture;
	sf::Vector2u m_textureOffset;
	sf::Vector2u m_tileSize;
	unsigned int m_numberOfTilesPerRow;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	void priv_updateCell(unsigned int index);
	unsigned int priv_cellIndex(sf::Vector2u location) const;
	sf::Vector2u priv_cellLocation(unsigned int index) const;
	bool priv_isCellIndexInRange(unsigned int index) const;
	bool priv_isCellLocationInRange(sf::Vector2u location) const;
	bool priv_isScreenBufferIndexInRange(unsigned int index) const;
	bool priv_isCursorInRange() const;
	bool priv_isColorIdInPaletteRange(int id) const;
	//bool priv_isColorIdInExtendedRange(int id) const;
	void priv_clearCell(unsigned int index, bool overwriteColor, bool overwriteBackgroundColor);
	void priv_clearCell(unsigned int index, const sf::Color& backgroundColor, bool overwriteColor);
	void priv_clearCell(unsigned int index, const sf::Color& color, const sf::Color& backgroundColor);
	void priv_paintCell(unsigned int index, const sf::Color& color, const sf::Color& backgroundColor);
	void priv_setCursorIndex(unsigned int index);
	void priv_moveCursorToBeginningOfLine();
	void priv_moveCursorUp();
	void priv_moveCursorDown();
	void priv_moveCursorLeft();
	void priv_moveCursorRight();
	void priv_testCursorForScroll();
	void priv_scroll();
	void priv_copyToBufferFromSelectionRectangle(Buffer& buffer, const sf::IntRect& selectionRectangle);
	void priv_pasteOffsettedBuffer(Buffer& buffer, const sf::Vector2i& offset);
	unsigned int priv_getPrintIndex(sf::Vector2u location) const;
	unsigned int priv_getCellValueFromCharacter(char character) const;
	char priv_getCharacterFromCellValue(unsigned int cellValue) const;
	sf::Color priv_colorFromColorIdAtIndex(unsigned int index, int colorId) const;
	sf::Color priv_backgroundColorFromColorIdAtIndex(unsigned int index, int colorId) const;
	bool& priv_chooseAttribute(CellAttributes& cellAttributes, Attribute attribute = Attribute::Inverse);
};

struct ConsoleScreenV1::Cell
{
	unsigned int value;
	sf::Color color;
	sf::Color backgroundColor;
	Stretch stretch;
	CellAttributes attributes;
};

enum class ConsoleScreenV1::Palette
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
	Colors216Web,
	Colors256Greenscale,
	Colors256Grayscale,
	Colors256Sepia
};

} // namespace selbaward
#endif // SELBAWARD_CONSOLESCREEN_V1_HPP
