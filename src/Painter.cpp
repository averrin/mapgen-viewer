#include <map>
#include <memory>
#include <thread>
#include <vector>

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "../MapgenConfig.h"
#include "SelbaWard/SelbaWard.hpp"
#include "mapgen/Biom.hpp"
#include "mapgen/MapGenerator.hpp"
#include "mapgen/utils.hpp"
#include "mapgen/Walker.hpp"
#include "rang.hpp"

// TODO: move ints to utils.cpp
template <typename T> using filterFunc = std::function<bool(T *)>;
template <typename T> using sortFunc = std::function<bool(T *, T *)>;

template <typename T>
std::vector<T *> filterObjects(std::vector<T *> regions, filterFunc<T> filter,
                               sortFunc<T> sort) {
  std::vector<T *> places;

  std::copy_if(regions.begin(), regions.end(), std::back_inserter(places),
               filter);
  std::sort(places.begin(), places.end(), sort);
  return places;
}

class Painter {
public:
  // TODO: use map instead mapgen
  Painter(sf::RenderWindow *w, MapGenerator *m, std::string v)
      : window(w), mapgen(m), VERSION(v) {
    loadIcons();
    initProgressBar();
    // TODO: use whereami
    sffont.loadFromFile("./font.ttf");

    sf::Vector2u windowSize = window->getSize();
    cachedMap.create(windowSize.x, windowSize.y);

    bgColor.r = static_cast<sf::Uint8>(color[0] * 255.f);
    bgColor.g = static_cast<sf::Uint8>(color[1] * 255.f);
    bgColor.b = static_cast<sf::Uint8>(color[2] * 255.f);
    window->clear(bgColor);
  };

  sf::Font sffont;

private:
  std::map<LocationType, sf::Texture *> icons;
  sf::RenderWindow *window;
  sw::ProgressBar progressBar;
  sf::Texture cachedMap;
  sf::Color bgColor;
  float color[3] = {0.12f, 0.12f, 0.12f};
  Map *map;
  MapGenerator *mapgen;
  std::string VERSION;
  bool needUpdate = true;
  sf::Clock clock;
  std::vector<Walker*> walkers;

  void initProgressBar() {
    progressBar.setShowBackgroundAndFrame(true);
    progressBar.setSize(sf::Vector2f(400, 10));
    progressBar.setPosition(
        (sf::Vector2f(window->getSize()) - progressBar.getSize()) / 2.f);
  }

  void loadIcons() {
    std::map<LocationType, std::string> iconMap = {
        {CAPITAL, "images/castle.png"}, {PORT, "images/docks.png"},
        {MINE, "images/mine.png"},      {AGRO, "images/farm.png"},
        {TRADE, "images/trade.png"},    {LIGHTHOUSE, "images/lighthouse.png"},
        {CAVE, "images/cave.png"},      {FORT, "images/fort.png"}};

    for (auto pair : iconMap) {
      sf::Texture *icon = new sf::Texture();
      mg::info("Loading icon:", pair.second);
      icon->loadFromFile(pair.second);
      icon->setSmooth(true);
      icons.insert(std::make_pair(pair.first, icon));
    }
  }

public:
  std::vector<sf::ConvexShape> polygons;
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

  bool isIncreasing{true};

  void invalidate() { needUpdate = true; }

  void fade() {
    sf::RectangleShape rectangle;
    rectangle.setSize(sf::Vector2f(window->getSize().x, window->getSize().y));
    auto color = sf::Color::Black;
    color.a = 150;
    rectangle.setFillColor(color);
    rectangle.setPosition(0, 0);

    window->draw(rectangle);
  }

  void drawLoading() {
    const float frame{clock.restart().asSeconds() * 0.3f};
    const float target{isIncreasing ? progressBar.getRatio() + frame
                                    : progressBar.getRatio() - frame};
    if (target < 0.f)
      isIncreasing = true;
    else if (target > 1.f)
      isIncreasing = false;
    progressBar.setRatio(target);
    window->draw(progressBar);

    sf::RectangleShape bg;
    bg.setSize(sf::Vector2f(420, 40));
    bg.setFillColor(sf::Color::Black);
    bg.setOutlineColor(sf::Color::White);
    bg.setOutlineThickness(1);
    auto middle = (sf::Vector2f(window->getSize()) - bg.getSize()) / 2.f;
    bg.setPosition(sf::Vector2f(middle.x, middle.y + 37.f));
    window->draw(bg);

    if (mapgen->map != nullptr) {
      sf::Text operation(mapgen->map->status, sffont);
      operation.setCharacterSize(20);
      operation.setColor(sf::Color::White);

      auto middle = (sf::Vector2f(window->getSize())) / 2.f;
      operation.setPosition(sf::Vector2f(
          middle.x - operation.getGlobalBounds().width / 2.f, middle.y + 25.f));
      window->draw(operation);
    }
    window->display();
  }

  void drawInfo(Region *currentRegion) {
    infoPolygons.clear();
    sf::ConvexShape selectedPolygon;

    if (currentRegion->location != nullptr && !roads) {
      for (auto r : mapgen->map->roads) {
        if (r->regions[0] != currentRegion->location->region &&
            r->regions.back() != currentRegion->location->region) {
          continue;
        }
        drawRoad(r);
      }
    }
    PointList points = currentRegion->getPoints();
    selectedPolygon.setPointCount(int(points.size()));

    Cluster *cluster = currentRegion->cluster;

    int i = 0;
    for (std::vector<Region *>::iterator
             it = cluster->megaCluster->regions.begin();
         it < cluster->megaCluster->regions.end(); it++, i++) {

      Region *region = cluster->megaCluster->regions[i];
      sf::ConvexShape polygon;
      PointList points = region->getPoints();
      polygon.setPointCount(points.size());
      int n = 0;
      for (PointList::iterator it2 = points.begin(); it2 < points.end();
           it2++, n++) {
        sf::Vector2<double> *p = points[n];
        polygon.setPoint(n, sf::Vector2f(p->x, p->y));
      }
      sf::Color col = sf::Color::Black;
      col.a = 20;
      polygon.setFillColor(col);
      polygon.setOutlineColor(col);
      polygon.setOutlineThickness(1);
      infoPolygons.push_back(polygon);
    }
    i = 0;
    for (std::vector<Region *>::iterator it = cluster->regions.begin();
         it < cluster->regions.end(); it++, i++) {

      Region *region = cluster->regions[i];
      sf::ConvexShape polygon;
      PointList points = region->getPoints();
      polygon.setPointCount(points.size());
      int n = 0;
      for (PointList::iterator it2 = points.begin(); it2 < points.end();
           it2++, n++) {
        sf::Vector2<double> *p = points[n];
        polygon.setPoint(n, sf::Vector2f(p->x, p->y));
      }
      sf::Color col = sf::Color::Red;
      col.a = 50;
      polygon.setFillColor(col);
      polygon.setOutlineColor(col);
      polygon.setOutlineThickness(1);
      infoPolygons.push_back(polygon);
    }

    for (int pi = 0; pi < int(points.size()); pi++) {
      Point p = points[pi];
      selectedPolygon.setPoint(
          pi, sf::Vector2f(static_cast<float>(p->x), static_cast<float>(p->y)));
    }

    sf::CircleShape site(2.f);

    selectedPolygon.setFillColor(sf::Color::Transparent);
    selectedPolygon.setOutlineColor(sf::Color::Red);
    selectedPolygon.setOutlineThickness(2);
    site.setFillColor(sf::Color::Red);
    site.setPosition(static_cast<float>(currentRegion->site->x - 1),
                     static_cast<float>(currentRegion->site->y - 1));

    if (verbose) {
      for (auto p : infoPolygons) {
        window->draw(p);
      }
    }

    window->draw(selectedPolygon);
    window->draw(site);
  }

  void drawRivers() {
    int rn = 0;
    for (auto r : mapgen->map->rivers) {
      PointList *rvr = r->points;
      sw::Spline river;
      river.setThickness(3);
      int i = 0;
      int c = rvr->size();
      for (PointList::iterator it = rvr->begin(); it < rvr->end(); it++, i++) {
        Point p = (*rvr)[i];
        river.addVertex(i,
                        {static_cast<float>(p->x), static_cast<float>(p->y)});
        float t = float(i) / c * 2.f;
        river.setThickness(i, t);
        river.setColor(sf::Color(46, 46, 76, float(i) / c * 255.f));
      }
      river.setBezierInterpolation();
      river.setInterpolationSteps(10);
      river.smoothHandles();
      river.update();
      window->draw(river);
      rn++;
    }
  }

  sw::Spline *drawRoad(Road *r) {
    sw::Spline *road = new sw::Spline();
    if (r->spline != nullptr) {
      road = r->spline;
    } else {
      int i = 0;
      road->setColor(sf::Color(200, 160, 100, 70));
      road->setThickness(1);
      for (auto reg : r->regions) {
        Point p = reg->site;
        road->addVertex(i, {static_cast<float>(p->x), static_cast<float>(p->y)});
        if (reg->megaCluster->isLand) {
          road->setColor(i, sf::Color(70, 50, 0));
          float w = std::min(3.f, 1.f + reg->traffic / 200.f);
          road->setThickness(i, w);
        } else {
          road->setColor(i, sf::Color(80, 80, 255, 180));
          road->setThickness(i, 2);
        }
        i++;
      }
      road->setBezierInterpolation();
      road->setInterpolationSteps(10);
      road->smoothHandles();
      road->update();
      r->spline = road;
    }
    window->draw(*road);
    return road;
  }

  void drawRoads() {
    for (auto r : mapgen->map->roads) {
      drawRoad(r);
    }
  }

  void drawMap() {
    if (needUpdate) {
      for (auto p : polygons) {
        window->draw(p);
      }

      drawRivers();
      if (roads) {
        drawRoads();
      }
      for (auto sprite : sprites) {
        window->draw(sprite);
      }

      // if (info) {
      //   for (auto p : poi) {
      //     window->draw(p);
      //   }
      // }
      if (states) {
        drawBorders();
      }

      if (areas) {
        for (auto c : mapgen->map->stateClusters) {
          for (auto region : c->regions) {
            sf::ConvexShape polygon;
            PointList points = region->getPoints();
            polygon.setPointCount(points.size());
            int n = 0;
            for (auto p : points) {
              polygon.setPoint(n, sf::Vector2f(p->x, p->y));
              n++;
            }

            sf::Color col(region->stateCluster->states[0]->color);
            polygon.setFillColor(col);
            window->draw(polygon);
          }
        }
      }
      drawMark();

      sf::Vector2u windowSize = window->getSize();
      cachedMap.create(windowSize.x, windowSize.y);
      cachedMap.update(*window);
      needUpdate = false;
    } else {
      sf::RectangleShape rectangle;
      rectangle.setSize(sf::Vector2f(window->getSize().x, window->getSize().y));
      rectangle.setPosition(0, 0);
      rectangle.setTexture(&cachedMap);
      window->draw(rectangle);
    }
  }

  void drawBorders() {
    auto ends = filterObjects(
        mapgen->map->regions,
        (filterFunc<Region>)[&](Region * r) {
          if (r->stateBorder && !r->seaBorder &&
              std::count_if(r->neighbors.begin(), r->neighbors.end(),
                            [&](Region *n) {
                              return n->stateBorder && !n->seaBorder &&
                                     n->state == r->state;
                            }) == 1) {
            return true;
          }
          return false;
        },
        (sortFunc<Region>)[&](Region * r, Region * r2) { return false; });

    std::vector<Region *> used;
    std::vector<Region *> exclude;
    for (auto r : ends) {
      if (std::count(used.begin(), used.end(), r) == 0) {
        sw::Spline line;
        sf::Color col = r->state->color;
        col.a = 150;
        line.setColor(col);
        line.setThickness(4);
        nextBorder(r, &used, &line, &ends, &exclude);

        line.setBezierInterpolation();
        line.setInterpolationSteps(20);
        line.smoothHandles();
        line.update();
        window->draw(line);
      }
    }
  }

  void nextBorder(Region *r, std::vector<Region *> *used, sw::Spline *line,
                  std::vector<Region *> *ends, std::vector<Region *> *exclude) {

    if (std::count(used->begin(), used->end(), r) == 0) {
      int i = line->getVertexCount();
      line->addVertex(
          i, {static_cast<float>(r->site->x), static_cast<float>(r->site->y)});
      used->push_back(r);
    }

    auto ns = filterObjects(
        r->neighbors,
        (filterFunc<Region>)[&](Region * n) {
          if (n->stateBorder && !n->seaBorder &&
              std::count(used->begin(), used->end(), n) == 0 &&
              std::count(exclude->begin(), exclude->end(), n) == 0 &&
              n->state == r->state) {
            return true;
          }
          return false;
        },
        (sortFunc<Region>)[&](Region * r, Region * r2) { return false; });

    if (ns.size() > 0) {
      nextBorder(ns[0], used, line, ends, exclude);
    } else if (std::count(ends->begin(), ends->end(), r) == 0) {
      exclude->push_back(r);
      used->pop_back();
      line->removeVertex(line->getVertexCount() - 1);
      nextBorder(used->back(), used, line, ends, exclude);
    }
  }

  void drawMark() {
    sf::Vector2u windowSize = window->getSize();
    char mt[40];
    sprintf(mt, "Mapgen [%s] by Averrin", VERSION.c_str());
    sf::Text mark(mt, sffont);
    mark.setCharacterSize(15);
    mark.setColor(sf::Color::White);
    mark.setPosition(sf::Vector2f(windowSize.x - 240, windowSize.y - 25));
    window->draw(mark);
  }

  void drawObjects(std::vector<sf::ConvexShape> op) {
    for (auto obj : op) {
      window->draw(obj);
    }
  }

  void update() {
    infoPolygons.clear();
    polygons.clear();
    poi.clear();
    sprites.clear();
    walkers.clear();

    for (auto mc : mapgen->map->megaClusters) {
      for (auto p : mc->resourcePoints) {
        float rad = p->minerals * 3 + 1;
        sf::CircleShape poiShape(rad);
        poiShape.setFillColor(sf::Color::Blue);
        poiShape.setPosition(
            sf::Vector2f(p->site->x - rad / 2.f, p->site->y - rad / 2.f));
        poi.push_back(poiShape);
      }
      for (auto p : mc->goodPoints) {
        float rad = p->minerals * 3 + 1;
        sf::CircleShape poiShape(rad);
        poiShape.setFillColor(sf::Color::Red);
        poiShape.setPosition(
            sf::Vector2f(p->site->x - rad / 2.f, p->site->y - rad / 2.f));
        poi.push_back(poiShape);
      }
    }

    std::vector<Region *> regions = mapgen->map->regions;
    polygons.reserve(regions.size());
    for (Region *region : regions) {
      sf::ConvexShape polygon;
      PointList points = region->getPoints();
      polygon.setPointCount(points.size());
      int n = 0;
      for (PointList::iterator it2 = points.begin(); it2 < points.end();
           it2++, n++) {
        sf::Vector2<double> *p = points[n];
        polygon.setPoint(n, sf::Vector2f(p->x, p->y));
      }

      sf::Color col(region->biom.color);

      if (region->border && !region->megaCluster->isLand) {
        int r = col.r;
        int g = col.g;
        int b = col.b;
        int s = 1;
        for (auto n : region->neighbors) {
          r += n->biom.color.r;
          g += n->biom.color.g;
          b += n->biom.color.b;
          s++;
        }
        col.r = r / s;
        col.g = g / s;
        col.b = b / s;
      }
      int a =
          255 * (region->getHeight(region->site) + 1.6) / 3 + (rand() % 8 - 4);
      if (a > 255) {
        a = 255;
      }
      col.a = a;
      if (region->location != nullptr) {
        sf::Sprite sprite;

        if ((region->city != nullptr && cities) ||
            (region->city == nullptr && locations)) {
          auto texture = icons[region->location->type];
          if (region->city != nullptr && region->city->isCapital) {
            texture = icons[CAPITAL];
          }
          sprite.setTexture(*texture);
          auto p = region->site;
          auto size = texture->getSize();
          sprite.setPosition(
              sf::Vector2f(p->x - size.x / 2.f, p->y - size.y / 2.f));
          sprites.push_back(sprite);
        }

        if (states && region->state != nullptr && region->city != nullptr) {
          col = region->state->color;
        }
      }

      polygon.setFillColor(col);

      if (edges) {
        polygon.setOutlineColor(sf::Color(100, 100, 100));
        polygon.setOutlineThickness(1);
      }
      if (heights) {
        sf::Color col(region->biom.color);
        col.r = 255 * (region->getHeight(region->site) + 1.6) / 3.2;
        col.a = 20 + 255 * (region->getHeight(region->site) + 1.6) / 3.2;
        col.b = col.b / 3;
        col.g = col.g / 3;
        polygon.setFillColor(col);
        color[2] = 1.f;
      }

      if (minerals) {
        sf::Color col(region->biom.color);
        col.g = 255 * (region->minerals) / 1.2;
        col.b = col.b / 3;
        col.r = col.g / 3;
        polygon.setFillColor(col);
        color[0] = 1.f;
      }

      if (hum && region->humidity != 1) {
        sf::Color col(region->biom.color);
        col.b = 255 * region->humidity;
        col.a = 255 * region->humidity;
        col.r = col.b / 3;
        col.g = col.g / 3;
        polygon.setFillColor(col);
      }

      if (temp) {
        if (region->temperature < biom::DEFAULT_TEMPERATURE) {
          sf::Color col(255, 0, 255);
          col.r = std::min(
              255.f, 255 * (biom::DEFAULT_TEMPERATURE / region->temperature));
          col.b =
              std::min(255.f, 255 * std::abs(1.f - (biom::DEFAULT_TEMPERATURE /
                                                    region->temperature)));

          polygon.setFillColor(col);
        }
      }
      polygons.push_back(polygon);
    }

    needUpdate = true;
  }

  sf::Color adjustLightness(sf::Color color, float d) {
    // R, G and B input range = 0 ÷ 255
    // H, S and L output range = 0 ÷ 1.0

    float var_R = (color.r / 255);
    float var_G = (color.g / 255);
    float var_B = (color.b / 255);

    float del_R;
    float del_G;
    float del_B;

    float var_Min =
        std::min(std::min(var_R, var_G), var_B); // Min. value of RGB
    float var_Max =
        std::min(std::max(var_R, var_G), var_B); // Max. value of RGB
    float del_Max = var_Max - var_Min;           // Delta RGB value

    float H = 0;
    float S;
    float L = (var_Max + var_Min) / 2;

    if (del_Max == 0) // This is a gray, no chroma...
    {
      H = 0;
      S = 0;
    } else // Chromatic data...
    {
      if (L < 0.5) {
        S = del_Max / (var_Max + var_Min);
      } else {
        S = del_Max / (2 - var_Max - var_Min);
      }

      del_R = (((var_Max - var_R) / 6) + (del_Max / 2)) / del_Max;
      del_G = (((var_Max - var_G) / 6) + (del_Max / 2)) / del_Max;
      del_B = (((var_Max - var_B) / 6) + (del_Max / 2)) / del_Max;

      if (var_R == var_Max) {
        H = del_B - del_G;
      } else if (var_G == var_Max) {
        H = (1 / 3) + del_R - del_B;
      } else if (var_B == var_Max) {
        H = (2 / 3) + del_G - del_R;
      }

      if (H < 0) {
        H += 1;
      }
      if (H > 1) {
        H -= 1;
      }
    }
    L += d;
    return color;
  }

  sf::Texture getScreenshot() {
    sf::Vector2u windowSize = window->getSize();
    sf::Texture texture;
    texture.create(windowSize.x, windowSize.y);
    texture.update(*window);
    return texture;
  }

  void draw() {
    window->clear(bgColor);
    drawMap();

    if (mapgen->map->roads.size() != 0) {
      drawWalkers();
    }
  }

  void drawWalkers() {
    if (walkers.size() == 0) {
      int n = 0;
      while (n < mapgen->map->cities.size()/2) {
        auto c = mapgen->map->cities[n];
        if (c->roads.size() == 0) {
          n++;
          continue;
        }
        auto w = new Walker(c, mapgen);
        walkers.push_back(w);
        n++;
      }
    } else {
      for (auto w : walkers) {
        w->tick();
        if (w->shape != nullptr) {
          window->draw(*w->shape);
        }
      }
    }
  }
};
