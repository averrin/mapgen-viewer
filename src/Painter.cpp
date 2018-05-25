#include <map>
#include <memory>
#include <thread>
#include <vector>
#include <cmath>
#include <chrono>
#include <fmt/format.h>

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include "../MapgenConfig.h"
#include "SelbaWard/SelbaWard.hpp"
#include "mapgen/Biom.hpp"
#include "mapgen/Layers.hpp"
#include "mapgen/MapGenerator.hpp"
#include "mapgen/Painter.hpp"
#include "mapgen/utils.hpp"
#include "mapgen/hslColor.hpp"
#include "rang.hpp"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

  // TODO: rewrite this horror
  std::string get_selfpath() {
    int bl;
#ifdef _WIN32
    char buff[MAX_PATH];
    GetModuleFileName(NULL, buff, sizeof(buff));
#else
    char buff[PATH_MAX];
    ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff) - 1);
    if (len != -1) {
      buff[len] = '\0';
    }
#endif
    auto path = fs::path(buff);
    std::cout << path << std::endl;
    return path.parent_path().string();
  }


std::map<Biom, sf::Color> biomColors = {
    {biom::ABYSS, sf::Color(23, 23, 40)},
    {biom::DEEP, sf::Color(39, 39, 70)},
    {biom::SHALLOW, sf::Color(51, 51, 91)},
    {biom::SHORE, sf::Color(68, 99, 130)},

    {biom::SAND, sf::Color(210, 185, 139)},
    {biom::GRASS, sf::Color(136, 170, 85)},
    {biom::FORREST, sf::Color(51, 119, 85)},
    {biom::ROCK, sf::Color(148, 148, 148)},
    {biom::SNOW, sf::Color(240, 240, 240)},
    {biom::ICE, sf::Color(220, 220, 255)},
    {biom::PRAIRIE, sf::Color(239, 220, 124)},
    {biom::MEADOW, sf::Color(126, 190, 75)},
    {biom::DESERT, sf::Color(244, 164, 96)},
    {biom::CITY, sf::Color(220, 220, 220)},
    {biom::RAIN_FORREST, sf::Color(51, 90, 75)},
    {biom::LAKE, sf::Color(66, 66, 96)},
    {biom::MARK, sf::Color::Red},
    {biom::MARK2, sf::Color::Black},
};

std::map<std::string, sf::Color> stateColors = {
    {"Blue empire", sf::Color(70, 70, 170)},
    {"Red lands", sf::Color(170, 70, 70)},
};

std::map<Road*, sw::Spline*> splines = {};


inline bool ends_with(std::string const &value, std::string const &ending) {
  if (ending.size() > value.size())
    return false;
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

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

  // TODO: use map instead mapgen
  Painter::Painter(sf::RenderWindow *w, MapGenerator *m, std::string v)
      : window(w), mapgen(m), VERSION(v) {

    auto dir = get_selfpath();
    char path[100];
    sprintf(path, "%s/font.ttf", dir.c_str());
    // mg::info("Loading font:", path.string());
    sffont.loadFromFile(sf::String(path));
    loadImages();
    initProgressBar();

    sf::Vector2u windowSize = window->getSize();
    cachedMap.create(windowSize.x, windowSize.y);

    bgColor = sf::Color(23, 23, 23);
    window->clear(bgColor);

    char spath[100];
    sprintf(spath, "%s/blur.frag", dir.c_str());

    if(shader_blur.loadFromFile(spath, sf::Shader::Type::Fragment)) {
      fmt::print("Blur shader loaded\n");
    }
    shader_blur.setUniform("blur_radius", 0.004f);
    shader_lesser_blur.loadFromFile(spath, sf::Shader::Type::Fragment);
    shader_lesser_blur.setUniform("blur_radius", 0.002f);
    // shader_blur.setParameter("blur_radius", 0.004f);
    sprintf(spath, "%s/mask.frag", dir.c_str());
    if(shader_mask.loadFromFile(spath, sf::Shader::Type::Fragment)) {
      fmt::print("Mask shader loaded\n ");
    }

    layers = new LayersManager(window, &shader_mask);
  };

  void Painter::initProgressBar() {
    progressBar.setShowBackgroundAndFrame(true);
    progressBar.setSize(sf::Vector2f(400, 10));
    progressBar.setPosition(
        (sf::Vector2f(window->getSize()) - progressBar.getSize()) / 2.f);
  }

  void Painter::loadImages() {
    std::map<LocationType, std::string> iconMap = {
        {CAPITAL, "castle"}, {PORT, "docks"},  {MINE, "mine"},
        {AGRO, "farm"},      {TRADE, "trade"}, {LIGHTHOUSE, "lighthouse"},
        {CAVE, "cave"},      {FORT, "fort"}};

    auto dir = get_selfpath();
    char path[100];

    sprintf(path, "%s/images", dir.c_str());
    for (auto &d : fs::directory_iterator(path)) {
      fs::path path = d.path();
      if (!ends_with(path.string(), ".png")) {
        continue;
      }
      sf::Texture *icon = new sf::Texture();
      mg::info("Loading image:", path);
      icon->loadFromFile(path.string());
      icon->setSmooth(true);
      images[path.stem().string()] = icon;
    }

    for (auto pair : iconMap) {
      locationIcons.insert(std::make_pair(pair.first, images[pair.second]));
    }
  }

  void Painter::invalidate(bool force) {
    needUpdate = true;
    if (force) {
      for (auto l : layers->layers) {
        l->damaged = true;
      }
    }
  }

  void Painter::fade() {
    sf::RectangleShape rectangle;
    rectangle.setSize(sf::Vector2f(window->getSize().x, window->getSize().y));
    auto color = sf::Color::Black;
    color.a = 150;
    rectangle.setFillColor(color);
    rectangle.setPosition(0, 0);

    window->draw(rectangle);
  }

  void Painter::drawLoading() {

#ifndef _WIN32
    window->clear();
#endif

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
      operation.setFillColor(sf::Color::White);
      // operation.setColor(sf::Color::White);

      auto middle = (sf::Vector2f(window->getSize())) / 2.f;
      operation.setPosition(sf::Vector2f(
          middle.x - operation.getGlobalBounds().width / 2.f, middle.y + 25.f));
      window->draw(operation);
    }
    window->display();
  }


  void Painter::drawInfo(Region *currentRegion) {
    sf::ConvexShape selectedPolygon;

    if (currentRegion != currentRegionCache) {
      currentRegionCache = currentRegion;

      infoPolygons.clear();
      // if (currentRegion->city != nullptr && !roads) {
      //   for (auto r : currentRegion->city->roads) {
      //     drawRoad(r);
      //   }
      // }
      PointList points = currentRegion->getPoints();
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
    }
    sf::CircleShape site(2.f);

    selectedPolygon.setPointCount(int(currentRegion->getPoints().size()));
    int pi = 0;
    for (auto p : currentRegion->getPoints()) {
      selectedPolygon.setPoint(
          pi, sf::Vector2f(static_cast<float>(p->x), static_cast<float>(p->y)));
      ++pi;
    }
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

  void Painter::drawRivers() {
    int rn = 0;
    for (auto r : mapgen->map->rivers) {
      PointList *rvr = r->points;
      auto river = new sw::Spline();
      river->setThickness(3);
      int i = 0;
      int c = rvr->size();
      for (PointList::iterator it = rvr->begin(); it < rvr->end(); it++, i++) {
        Point p = (*rvr)[i];
        river->addVertex(i,
                         {static_cast<float>(p->x), static_cast<float>(p->y)});
        float t = float(i) / c * 2.f;
        river->setThickness(i, t);
        river->setColor(sf::Color(66, 66, 96, float(i) / c * 255.f));
      }
      river->setBezierInterpolation();
      river->setInterpolationSteps(10);
      river->smoothHandles();
      river->update();
      layers->getLayer("rivers")->add(river);
      rn++;
    }
  }

  sw::Spline* Painter::drawRoad(Road *r) {
    sw::Spline *road = nullptr;
    if (splines.find(r) != splines.end()) {
      road = splines[r];
    } else {
      int i = 0;
      road = new sw::Spline();
      if (r->seaPath) {
        road->setColor(sf::Color(120, 120, 200, 180));
      } else {
        road->setColor(sf::Color(70, 20, 0, 180));
      }
      road->setThickness(1);
      for (auto reg : r->regions) {
        Point p = reg->site;
        road->addVertex(i,
                        {static_cast<float>(p->x), static_cast<float>(p->y)});
        if (reg->megaCluster->isLand) {
          // road->setColor(i, sf::Color(70, 50, 0));
          float w = std::min(3.f, 1.f + reg->traffic / 200.f);
          road->setThickness(i, w);
        } else {
          // road->setColor(i, sf::Color(80, 80, 255, 180));
          road->setThickness(i, 2);
        }
        i++;
      }
      road->setBezierInterpolation();
      road->setInterpolationSteps(10);
      road->smoothHandles();
      road->update();
      splines[r] = road;
    }
    layers->getLayer("roads")->add(road);
    return road;
  }

  void Painter::drawRoads() {
    for (auto p : mapgen->map->roadMap) {

      if (!showSeaPathes && p.second->seaPath) continue;
      drawRoad(p.second);
    }
  }

  void Painter::drawLabels() {
    for (auto c : mapgen->map->cities) {

      auto bg = new sf::RectangleShape();
      bg->setFillColor(sf::Color(50, 30, 22, 200));
      // if (c->isCapital) {
      if (states) {
        bg->setOutlineColor(stateColors[c->region->state->name]);
      } else {
        bg->setOutlineColor(sf::Color(200, 200, 180, 180));
      }
      bg->setOutlineThickness(1);

      auto label = new sf::Text(c->name, sffont);
      label->setCharacterSize(10);
      label->setFillColor(sf::Color(255, 255, 220));
      // label.setColor(sf::Color(255, 255, 220));
      label->setPosition(
          sf::Vector2f(c->region->site->x, c->region->site->y + 10));

      bg->setSize(sf::Vector2f(label->getGlobalBounds().width + 8, 18));
      bg->setPosition(
          sf::Vector2f(c->region->site->x - 4, c->region->site->y + 7));

      layers->getLayer("labels")->add(bg);
      layers->getLayer("labels")->add(label);
    }
  }

  void Painter::drawMap() {
    if (needUpdate) {
      auto t0 = std::chrono::system_clock::now();

      cachedMap.clear(sf::Color::Transparent);

      std::vector<std::string> order = {
        "water", "waterClear",
        "landBorder", "land",
        "roads", "rivers",
        "forrest",
        "lakes",
        "borders",
        "labels",
        "locations",
        "watermark"
      };
      for (auto name : order) {
        auto l = layers->getLayer(name);
        // if (l->damaged && l->enabled) l->clear();
        l->clear();
      }

	  // Its a horrible hack. But renderTexture have no antialiasing=(
	  // layers->getLayer("roads")->direct = true;
      Layer* l;
      l = layers->getLayer("borders"); 
      if (l->enabled != states || l->damaged) {
        l->enabled = states;
        l->damaged = true;
        if (l->enabled) {
          l->clear();
          drawBorders();
        }
      }

      l = layers->getLayer("water"); 
      if (l->enabled != blur || l->damaged) {
        l->enabled = blur;
        l->clear();
        l->damaged = true;
      }

      l = layers->getLayer("waterClear");
      if (l->enabled != !blur || l->damaged) {
        l->enabled = !blur;
        l->clear();
        l->damaged = true;
      }

      l = layers->getLayer("labels");
      if (l->enabled != labels || l->damaged) {
        l->enabled = labels;
        l->damaged = true;
        if (l->enabled) {
          l->clear();
          drawLabels();
        }
      }

      l = layers->getLayer("locations");
      if (l->enabled != locations || l->damaged) {
        l->enabled = locations;
        l->damaged = true;
        if (l->enabled) {
          l->clear();
          drawLocations();
        }
      }

      l = layers->getLayer("roads");
      if (l->enabled != roads || l->damaged) {
        l->enabled = roads;
        l->damaged = true;
        l->clear();
        drawRoads();
      }

      drawPolygons();
      drawRivers();
      drawLakes();
      drawMark();


      layers->setShader("water", &shader_blur);
      layers->setMask("rivers", layers->getLayer("land"));

      for (auto name : order) {
        if (layers->getLayer(name)->damaged) {
          layers->invalidateLayer(name);
        }
      }

      needUpdate = false;
      if (useCacheMap) {
        cachedMap.draw(*layers);
      }
      drawMap();

      auto t1 = std::chrono::system_clock::now();
        using milliseconds = std::chrono::duration<double, std::milli>;
        milliseconds ms = t1 - t0;
        std::cout << "time taken by drawMap[needUpdate]: " << rang::fg::green << ms.count() << rang::style::reset << '\n';
    } else {
      if (useCacheMap) {
        sf::RectangleShape rectangle;
        rectangle.setSize(sf::Vector2f(window->getSize().x,
        window->getSize().y));
        rectangle.setPosition(0, 0);
        cachedMap.display();
        rectangle.setTexture(&(cachedMap.getTexture()));

        window->draw(rectangle);
      } else {
      window->draw(*layers);
      }
    }
  }

  void Painter::drawBorders() {
    auto layer = layers->getLayer("borders");
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
      sf::ConvexShape polygon;
      PointList points = r->getPoints();
      polygon.setPointCount(points.size());
      int n = 0;
      for (auto p : points) {
        polygon.setPoint(n, sf::Vector2f(p->x, p->y));
        n++;
      }

      sf::Color col(sf::Color::Black);
      polygon.setFillColor(col);
      // window->draw(polygon);

      if (std::count(used.begin(), used.end(), r) == 0) {
        auto line = new sw::Spline();
        sf::Color col = stateColors[r->state->name];
        // col.a = 150;
        line->setColor(col);
        line->setThickness(4);
        nextBorder(r, &used, line, &ends, &exclude);

        line->setBezierInterpolation();
        line->setInterpolationSteps(20);
        line->smoothHandles();
        line->update();
        layer->shapes.push_back(line);
      }
    }
  }

  void Painter::nextBorder(Region *r, std::vector<Region *> *used, sw::Spline *line,
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

  void Painter::drawMark() {
    sf::Vector2u windowSize = window->getSize();
    char mt[40];
    sprintf(mt, "Mapgen [%s] by Averrin", VERSION.c_str());
    sf::Text *mark = new sf::Text(mt, sffont);
    mark->setCharacterSize(15);
    mark->setFillColor(sf::Color::White);
    mark->setOutlineColor(sf::Color(23, 23, 23));
    mark->setOutlineThickness(1);
    mark->setPosition(sf::Vector2f(windowSize.x - 240, windowSize.y - 25));

    layers->getLayer("watermark")->add(mark);
  }

  void Painter::drawObjects(std::vector<sf::ConvexShape> op) {
    for (auto obj : op) {
      window->draw(obj);
    }
  }

  void Painter::drawLocations() {
    for (Region *region : mapgen->map->regions) {
      if (region->location != nullptr) {
        auto sprite = new sf::Sprite();

        if ((region->city != nullptr) || (region->city == nullptr)) {
          // auto texture = images["village"];
          auto texture = locationIcons[region->location->type];
          if (region->city != nullptr) {
            if (region->city->isCapital) {
              texture = locationIcons[CAPITAL];
            } else if (region->city->population > 2000) {
              // texture = icons["castle"];
            }
          }
          sprite->setTexture(*texture);
          auto p = region->site;
          // auto size = texture->getSize();
          // sprite.setScale(0.05, 0.05);
          sprite->setPosition(
              sf::Vector2f(p->x - iconSize / 2.f, p->y - iconSize / 2.f));
          layers->getLayer("locations")->add(sprite);
        }
      }
    }
  }

  void Painter::drawLakes() {
    std::vector<Region *> regions = mapgen->map->regions;
    for (auto region : regions) {
      if (region->biom != biom::LAKE) {
        continue;
      }
      auto polygon = getPolygon(region);

      if (region->isCoast()) {
        layers->getLayer("lakes")->add(polygon);
      }
      layers->getLayer("lakes")->add(polygon);
    }
  }

  sf::ConvexShape* Painter::getPolygon(Region *region) {
    auto polygon = new sf::ConvexShape();
    PointList points = region->getPoints();
    polygon->setPointCount(points.size());
    int n = 0;
    for (PointList::iterator it2 = points.begin(); it2 < points.end();
         it2++, n++) {
      sf::Vector2<double> *p = points[n];
      polygon->setPoint(n, sf::Vector2f(p->x, p->y));
    }

      sf::Color col(biomColors[region->biom]);

    if (region->border && !region->megaCluster->isLand) {
      int r = col.r;
      int g = col.g;
      int b = col.b;
      int s = 1;
      for (auto n : region->neighbors) {
        if (n->megaCluster->isLand) {
          continue;
        }
          r += biomColors[n->biom].r;
          g += biomColors[n->biom].g;
          b += biomColors[n->biom].b;
        s++;
      }
      col.r = r / s;
      col.g = g / s;
      col.b = b / s;
    }
    auto hsl = TurnToHSL(col);
    if (region->megaCluster->isLand) {
      float h = region->getHeight(region->site);
      hsl.Luminance -= 20;
      hsl.Luminance += lumDelta * h;
      hsl.Luminance = std::max(hsl.Luminance, double(0));
      hsl.Luminance = std::min(hsl.Luminance, double(100));
    }
    hsl.Hue += (rand() % (hueDelta*2)) - hueDelta;
    col = hsl.TurnToRGB();

    if (useTextures) {
      if (region->biom == biom::FORREST ||
          region->biom == biom::RAIN_FORREST) {
        // auto sp = sf::ConvexShape(*polygon);
        // sp.setFillColor(sf::Color(10, 10, 10));
        polygon->move(0, -forrestBorderHeight);
        layers->getLayer("forrest")->add(polygon);
        // polygons.push_back(DrawableRegion{sp, region, 2});
        polygon->setTexture(images["tt"]);
        // polygons.push_back(DrawableRegion{polygon, region, 2});
      } else if (region->biom == biom::SAND || region->biom == biom::DESERT)
      {
        if (region->biom == biom::SAND) {
          polygon->setFillColor(sf::Color(240,240,240));
        }
        polygon->setTexture(images["st"]);
      } else if (region->biom == biom::GRASS ||
                 region->biom == biom::MEADOW) {
        polygon->setTexture(images["snow"]);
      } else if (region->biom == biom::PRAIRIE) {
        polygon->setTexture(images["pt"]);
      } else if (region->biom == biom::ICE || region->biom == biom::SNOW) {
        if (region->biom == biom::ICE) {
          auto sp = sf::ConvexShape(*polygon);
          sp.setFillColor(sf::Color(200, 200, 240));
          sp.move(0, 5);
          // polygons.push_back(DrawableRegion{sp, region, 2});
          // polygon->setTexture(images["snow"]); //
          // polygons.push_back(DrawableRegion{polygon, region, 2});
        }
        polygon->setTexture(images["snow"]);
      }
    }

    polygon->setFillColor(col);
    if (edges && (region->megaCluster->isLand || !blur)) {
      polygon->setOutlineColor(sf::Color(100, 100, 100));
      polygon->setOutlineThickness(1);
    }
    if (heights) {
        sf::Color col(biomColors[region->biom]);
      col.r = 255 * (region->getHeight(region->site) + 1.6) / 3.2;
      col.a = 20 + 255 * (region->getHeight(region->site) + 1.6) / 3.2;
      col.b = col.b / 3;
      col.g = col.g / 3;
      polygon->setFillColor(col);
    }

    if (minerals && (region->megaCluster->isLand || !blur)) {
        sf::Color col(biomColors[region->biom]);
      col.g = 255 * (region->minerals) / 1.2;
      col.b = col.b / 3;
      col.r = col.g / 3;
      polygon->setFillColor(col);
    }

    if (hum && region->humidity != 1) {
        sf::Color col(biomColors[region->biom]);
      col.b = 255 * region->humidity;
      col.a = 255 * region->humidity;
      col.r = col.b / 3;
      col.g = col.g / 3;
      polygon->setFillColor(col);
    }

    if (temp) {
      if (region->temperature < biom::DEFAULT_TEMPERATURE) {
        sf::Color col(255, 0, 255);
        col.r = std::min(
            255.f, 255 * (biom::DEFAULT_TEMPERATURE / region->temperature));
        col.b =
            std::min(255.f, 255 * std::abs(1.f - (biom::DEFAULT_TEMPERATURE /
                                                  region->temperature)));

        polygon->setFillColor(col);
      }
    }
    return polygon;
  }

  void Painter::drawPolygons() {
    infoPolygons.clear();
    poi.clear();
    walkers.clear();
    currentRegionCache = nullptr;

    std::vector<Region *> regions = mapgen->map->regions;
    for (Region *region : regions) {
      if (region->biom == biom::LAKE) {
        continue;
      }
      auto polygon = getPolygon(region);
      if (region->megaCluster->isLand) {
        layers->getLayer("land")->add(polygon);
        auto p2 = new sf::ConvexShape(*polygon);
        auto nc = polygon->getFillColor();
        auto hsl = TurnToHSL(biomColors[region->biom]);
        hsl.Luminance -= 20;
        nc = hsl.TurnToRGB();
        nc.a = 180;
        p2->setFillColor(nc);
        p2->move(0, landBorderHeight);
        layers->getLayer("landBorder")->add(p2);
        if (region->isCoast()) {
          layers->getLayer("water")->add(polygon);
        }
      } else {
        layers->getLayer("water")->add(polygon);
        layers->getLayer("waterClear")->add(polygon);
      }
    }

    needUpdate = true;
  }

  sf::Texture Painter::getScreenshot() {
    sf::Vector2u windowSize = window->getSize();
    sf::Texture texture;
    texture.create(windowSize.x, windowSize.y);
    texture.update(*window);
    return texture;
  }

  void Painter::draw() {
    window->clear(bgColor);
    drawMap();

    if (showWalkers) {
      if (mapgen->map->roads.size() != 0) {
        drawWalkers();
      }
    }
  }

  void Painter::drawWalkers() {
    if (walkers.size() == 0) {
      int n = 0;
      while (n < mapgen->map->cities.size()) {
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
