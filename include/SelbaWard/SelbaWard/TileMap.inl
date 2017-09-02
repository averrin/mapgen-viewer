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

#ifndef SELBAWARD_TILEMAP_INL
#define SELBAWARD_TILEMAP_INL

namespace selbaward
{

template <class T>
// level can be any const stl container with random access using indices (std::array/std::vector/std::deque) containing any integer type
void TileMap::update(const T& level, unsigned int width, const sf::Vector2f camera)
{
	setCamera(camera);
	update(level, width);
}

template <class T>
// level can be a pointer to any integer type. size of level data must also be provided
void TileMap::update(const T* const level, const unsigned int size, unsigned int width, const sf::Vector2f camera)
{
	setCamera(camera);
	update(level, size, width);
}

template <class T>
// level can be any const stl container with random access using indices (std::array/std::vector/std::deque) containing any integer type
void TileMap::update(const T& level, unsigned int width)
{
	if (width > level.size())
		width = level.size();

	// if width is zero, rather than not display anything, choose width automatically to create an approximate square shape from the level data (if level size is zero, width will still be zero)
	if (width == 0)
		width = static_cast<unsigned int>(std::sqrt(level.size()));

	const sf::Vector2f actualCamera{ priv_getActualCamera() };
	const sf::Vector2i levelOffset{ static_cast<int>(floor(actualCamera.x)), static_cast<int>(floor(actualCamera.y)) };
	const unsigned int height{ width > 0u ? level.size() / width : 0u };

	for (unsigned int y{ 0 }; y < m_gridSize.y; ++y)
	{
		for (unsigned int x{ 0 }; x < m_gridSize.x; ++x)
		{
			const unsigned int gridIndex{ y * m_gridSize.x + x };
			const unsigned int levelIndex{ (levelOffset.y + y) * width + levelOffset.x + x };
			if (levelOffset.x + x >= 0 && levelOffset.x + x < width &&
				levelOffset.y + y >= 0 && levelOffset.y + y < height)
				m_grid[gridIndex] = static_cast<unsigned long int>(level[levelIndex]);
			else
				m_grid[gridIndex] = m_outOfBoundsTile;
		}
	}

	m_redrawRequired = true;
}

template <class T>
// level can be a pointer to any integer type. size of level data must also be provided
void TileMap::update(const T* const level, const unsigned int size, unsigned int width)
{
	if (width > size)
		width = size;

	// if width is zero, rather than not display anything, choose width automatically to create an approximate square shape from the level data (if level size is zero, width will still be zero)
	if (width == 0)
		width = static_cast<unsigned int>(std::sqrt(size));

	const sf::Vector2f actualCamera{ priv_getActualCamera() };
	const sf::Vector2i levelOffset{ static_cast<int>(floor(actualCamera.x)), static_cast<int>(floor(actualCamera.y)) };
	const unsigned int height{ width > 0u ? size / width : 0u };

	for (unsigned int y{ 0 }; y < m_gridSize.y; ++y)
	{
		for (unsigned int x{ 0 }; x < m_gridSize.x; ++x)
		{
			const unsigned int gridIndex{ y * m_gridSize.x + x };
			const unsigned int levelIndex{ (levelOffset.y + y) * width + levelOffset.x + x };
			if (levelOffset.x + x >= 0 && levelOffset.x + x < width &&
				levelOffset.y + y >= 0 && levelOffset.y + y < height)
				m_grid[gridIndex] = static_cast<unsigned long int>(level[levelIndex]);
			else
				m_grid[gridIndex] = m_outOfBoundsTile;
		}
	}

	m_redrawRequired = true;
}

} // namespace selbaward
#endif // SELBAWARD_TILEMAP_INL
