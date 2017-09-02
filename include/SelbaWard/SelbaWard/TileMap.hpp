//////////////////////////////////////////////////////////////////////////////
//
// Selba Ward (https://github.com/Hapaxia/SelbaWard)
// --
//
// Tile Map
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

#ifndef SELBAWARD_TILEMAP_HPP
#define SELBAWARD_TILEMAP_HPP

#include "Common.hpp"

#include <SFML/Graphics/RenderTexture.hpp>

namespace selbaward
{

// SW Tile Map v1.3.0
class TileMap : public sf::Drawable, public sf::Transformable
{
public:
	TileMap();
	void setSize(sf::Vector2f size);
	sf::Vector2f getSize() const;
	void setGridSize(sf::Vector2u gridSize);
	sf::Vector2u getGridSize() const;
	unsigned int getTotalGridSize() const;
	void setOutOfBoundsTile(unsigned long int textureTileIndex);
	void setTexture(const sf::Texture& texture);
	void setTexture();
	void setNumberOfTextureTilesPerRow(unsigned int numberOfTextureTilesPerRow);
	void setTextureOffset(sf::Vector2u textureOffset);
	void setTextureTileSize(sf::Vector2u textureTileSize);
	sf::Vector2u getTextureTileSize() const;
	void setSmooth(bool smooth);
	bool getSmooth() const;
	void setSmoothScroll(bool smoothScroll);
	bool getSmoothScroll() const;
	void setCameraTargetTile(sf::Vector2f cameraTargetTile);
	sf::Vector2f getCameraTargetTile() const;
	void setCamera(sf::Vector2f camera);
	sf::Vector2f getCamera() const;
	void setColor(sf::Color color);
	sf::Color getColor() const;
	sf::Vector2i getLevelPositionAtCoord(sf::Vector2f coord) const;
	unsigned long int getTileAtCoord(sf::Vector2f coord) const;
	sf::Vector2f getCoordAtLevelGridPosition(sf::Vector2f levelGridPosition) const; // takes a float vector so the parameter can specify different parts of that tile e.g. (2.5, 1.5) = centre of tile (2, 1)
	sf::Vector2f getTileSize() const; // display size of a tile before any transformations

	void redraw();

	template <class T>
	void update(const T& level, unsigned int width, sf::Vector2f camera); // level can be any const stl container with random access using indices (std::array/std::vector/std::deque) containing any integer type
	template <class T>
	void update(const T& level, unsigned int width); // level can be any const stl container with random access using indices (std::array/std::vector/std::deque) containing any integer type

	template <class T>
	void update(const T* const level, unsigned int size, unsigned int width, sf::Vector2f camera); // level can be a pointer to any integer type
	template <class T>
	void update(const T* const level, unsigned int size, unsigned int width); // level can be a pointer to any integer type









private:
	struct ActionFlags
	{
		bool scrollSmoothly{ false };
	};

	struct StateFlags
	{
		bool smooth{ false };
	};

	// flags
	ActionFlags m_do;
	StateFlags m_is;

	// data
	sf::Vector2u m_gridSize;
	std::vector<unsigned long int> m_grid;
	unsigned long int m_outOfBoundsTile;

	// camera (in tiles)
	sf::Vector2f m_camera;
	sf::Vector2f m_cameraTarget;

	// color
	sf::Color m_color;

	// tiles
	sf::PrimitiveType m_primitiveType;
	sf::Vector2f m_size;
	const sf::Texture* m_texture;
	unsigned int m_numberOfTextureTilesPerRow;
	sf::Vector2u m_textureOffset;
	sf::Vector2u m_textureTileSize;
	mutable std::vector<sf::Vertex> m_vertices;

	// render
	mutable bool m_redrawRequired;
	mutable sf::RenderTexture m_renderTexture;
	mutable std::vector<sf::Vertex> m_render;

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	void priv_updateVertices() const;
	void priv_updateRender() const;
	void priv_recreateRenderTexture();
	sf::Vector2i priv_getGridPositionAtCoord(sf::Vector2f coord) const;
	unsigned int priv_getTileAtGridPosition(sf::Vector2i gridPosition) const;
	sf::Vector2f priv_getActualCamera() const;
	sf::Vector2f priv_getTileOffsetFromVector(sf::Vector2f vector) const;
	sf::Vector2f priv_getVectorFromTileOffset(sf::Vector2f offset) const;
};

} // namespace selbaward
#include "TileMap.inl"
#endif // SELBAWARD_TILEMAP_HPP
