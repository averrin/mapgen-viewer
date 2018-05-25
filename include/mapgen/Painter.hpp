#include <thread>

#include <SFML/Graphics.hpp>

#include "mapgen/Region.hpp"
#include "mapgen/Walker.hpp"
#include "mapgen/Layers.hpp"
#include "mapgen/utils.hpp"

#include "SelbaWard/SelbaWard.hpp"

struct DrawableRegion {
public:
  sf::ConvexShape shape ;
  Region* region;
  int zIndex;
};

class Painter {

public:
  Painter(sf::RenderWindow *w, MapGenerator *m, std::string v);
  sf::Font sffont;

  std::vector<DrawableRegion> polygons;
  std::vector<DrawableRegion> secondLayer;
  std::vector<DrawableRegion> waterPolygons;
  std::vector<sf::ConvexShape> infoPolygons;
  std::vector<sf::CircleShape> poi;
  std::vector<sf::Sprite> sprites;

  bool borders = false;
  bool edges = false;
  bool info = false;
  bool verbose = true;
  bool heights = false;
  bool hum = false;
  bool temp = false;
  bool minerals = false;
  bool roads = true;
  bool cities = true;
  bool locations = true;
  bool states = true;
  bool areas = false;
  bool showWalkers = true;
  bool labels = true;
  bool blur = false;
  bool showSeaPathes = false;
  bool wind = false;


  bool useTextures = false;
  bool useCacheMap = true;
  int hueDelta = 7;
  float lumDelta = 18.f;
  int landBorderHeight = 4;
  int forrestBorderHeight = 5;

  LayersManager *layers;
  sf::Shader shader_lesser_blur;
  sf::Shader shader_mask;

  bool isIncreasing{true};

        void initProgressBar();
        void loadImages();
        void invalidate(bool force = false);
        void fade();
        void drawLoading();
        void drawInfo(Region *currentRegion);
        void drawRivers();
        sw::Spline* drawRoad(Road *r);
        void drawRoads();
        void drawLabels();
        void drawMap();
        void drawBorders();
        void nextBorder(Region *r, std::vector<Region *> *used, sw::Spline *line,
                  std::vector<Region *> *ends, std::vector<Region *> *exclude);
        void drawMark();
        void drawObjects(std::vector<sf::ConvexShape> op);
        void update();
sf::Texture getScreenshot();
        void draw();
        void drawWalkers();
    void drawPolygons();
    void drawWind();
    void drawLakes();
    void drawLocations();
sf::ConvexShape *getPolygon(Region *region);

private:
  std::map<LocationType, sf::Texture *> locationIcons;
  std::map<std::string, sf::Texture *> images;
  sf::RenderWindow *window;
  sw::ProgressBar progressBar;
  sf::RenderTexture cachedMap;
  sf::Color bgColor;
  float color[3] = {0.12f, 0.12f, 0.12f};
  Map *map;
  MapGenerator *mapgen;
  std::string VERSION;
  bool needUpdate = true;
  sf::Clock clock;
  std::vector<Walker *> walkers;
  sf::Shader shader_blur;
  float iconSize = 24.f;

  Region *currentRegionCache = nullptr;
};
