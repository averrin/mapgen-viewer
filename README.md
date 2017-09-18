# mapgen

Map generator based on Voronoi Diagram and Perlin noise

![screenshot](https://raw.githubusercontent.com/averrin/mapgen/master/mapgen_screenshot.png)

## Build from sources

### Linux
* Install dev version of SFML and libnoise
* cmake .
* make

### Windows
* Install [SFML](https://www.sfml-dev.org/files/SFML-2.4.2-windows-vc14-32-bit.zip)
* Install CMake for Windows
* cmake .
* Built created solution with Visual Studio
* Add libnoise.dll and sfml libraries to result folder
* Copy images/ and font.ttf into save folder