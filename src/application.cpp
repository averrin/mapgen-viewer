#include <imgui-SFML.h>
#include <imgui.h>
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
#include "infoWindow.cpp"
#include "logger.cpp"
#include "objectsWindow.cpp"

class Application {
  std::string VERSION;
  std::vector<sf::ConvexShape> polygons;
  std::vector<sf::CircleShape> poi;
  std::vector<sf::ConvexShape> infoPolygons;
  std::vector<sf::Sprite> sprites;
  std::map<LocationType, sf::Texture *> icons;
  sf::Color bgColor;
  AppLog log;
  MapGenerator *mapgen;
  std::thread generator;
  sw::ProgressBar progressBar;
  sf::Font sffont;
  sf::RenderWindow *window;
  sf::Texture cachedMap;
  bool isIncreasing{true};
  bool needUpdate = true;

  int relax = 0;
  int seed;
  int octaves;
  float freq;
  int nPoints;
  bool borders = false;
  bool edges = false;
  bool info = false;
  bool verbose = true;
  bool heights = false;
  bool flat = false;
  bool hum = false;
  bool simplifyRivers;
  int t = 0;
  float color[3] = {0.12f, 0.12f, 0.12f};
  bool showUI = true;
  bool getScreenshot = false;
  float temperature;
  bool temp = false;
  bool minerals = false;
  bool roads = false;
  bool ready = false;
  Region *lockedRegion = nullptr;
  bool lock = false;

public:
  Application(std::string v) : VERSION(v) {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("./font.ttf", 15.0f);

#ifdef _WIN32
    window = new sf::RenderWindow(sf::VideoMode(1600,900), "",
                                  sf::Style::Default, settings);
#else
    window = new sf::RenderWindow(sf::VideoMode::getDesktopMode(), "",
                                  sf::Style::Default, settings);
#endif

    window->setVerticalSyncEnabled(true);
    ImGui::SFML::Init(*window);
    char windowTitle[255] = "MapGen";

    window->setTitle(windowTitle);
    window->resetGLStates();

    initMapGen();
    generator = std::thread([&]() {});
    regen();

    bgColor.r = static_cast<sf::Uint8>(color[0] * 255.f);
    bgColor.g = static_cast<sf::Uint8>(color[1] * 255.f);
    bgColor.b = static_cast<sf::Uint8>(color[2] * 255.f);

    progressBar.setShowBackgroundAndFrame(true);
    progressBar.setSize(sf::Vector2f(400, 10));
    progressBar.setPosition(
        (sf::Vector2f(window->getSize()) - progressBar.getSize()) / 2.f);

    sffont.loadFromFile("./font.ttf");

    // TODO: automate it
    sf::Texture *capitalIcon = new sf::Texture();
    capitalIcon->loadFromFile("images/castle.png");
    capitalIcon->setSmooth(true);

    sf::Texture *anchorIcon = new sf::Texture();
    anchorIcon->loadFromFile("images/docks.png");
    anchorIcon->setSmooth(true);

    sf::Texture *mineIcon = new sf::Texture();
    mineIcon->loadFromFile("images/mine.png");
    mineIcon->setSmooth(true);

    sf::Texture *agroIcon = new sf::Texture();
    agroIcon->loadFromFile("images/farm.png");
    agroIcon->setSmooth(true);

    sf::Texture *tradeIcon = new sf::Texture();
    tradeIcon->loadFromFile("images/trade.png");
    tradeIcon->setSmooth(true);

    sf::Texture *lhIcon = new sf::Texture();
    lhIcon->loadFromFile("images/lighthouse.png");
    lhIcon->setSmooth(true);

    sf::Texture *caveIcon = new sf::Texture();
    caveIcon->loadFromFile("images/cave.png");
    caveIcon->setSmooth(true);

    icons.insert(std::make_pair(CAPITAL, capitalIcon));
    icons.insert(std::make_pair(PORT, anchorIcon));
    icons.insert(std::make_pair(MINE, mineIcon));
    icons.insert(std::make_pair(AGRO, agroIcon));
    icons.insert(std::make_pair(TRADE, tradeIcon));
    icons.insert(std::make_pair(LIGHTHOUSE, lhIcon));
    icons.insert(std::make_pair(CAVE, caveIcon));

    sf::Vector2u windowSize = window->getSize();
    cachedMap.create(windowSize.x, windowSize.y);
    log.AddLog("Welcome to Mapgen\n");
  }

  void regen() {
    if (generator.joinable())
      generator.join();
    generator = std::thread([&]() {
      lockedRegion = nullptr;
      lock = false;
      ready = false;
      mapgen->update();
      seed = mapgen->getSeed();
      relax = mapgen->getRelax();
      updateVisuals();
      ready = mapgen->ready;
    });
  }

  void simulate() {
    if (generator.joinable())
      generator.join();
    generator = std::thread([&]() {
      ready = false;
      mapgen->startSimulation();
      roads = true;
      updateVisuals();
      ready = mapgen->ready;
    });
  }

  void initMapGen() {
    seed = std::chrono::system_clock::now().time_since_epoch().count();
    mapgen = new MapGenerator(window->getSize().x, window->getSize().y);
    // mapgen.setSeed(/*111629613*/ 81238299);
    octaves = mapgen->getOctaveCount();
    freq = mapgen->getFrequency();
    nPoints = mapgen->getPointCount();
    relax = mapgen->getRelax();
    simplifyRivers = mapgen->simpleRivers;
    temperature = mapgen->temperature;
  }

  void processEvent(sf::Event event) {
    ImGui::SFML::ProcessEvent(event);

    switch (event.type) {
    case sf::Event::KeyPressed:
      switch (event.key.code) {
      case sf::Keyboard::R:
        mapgen->seed();
        log.AddLog("Update map\n");
        regen();
        break;
      case sf::Keyboard::Escape:
        window->close();
        break;
      case sf::Keyboard::L:
        simulate();
        break;
      case sf::Keyboard::H:
        hum = !hum;
        updateVisuals();
        break;
      case sf::Keyboard::T:
        temp = !temp;
        updateVisuals();
        break;
      case sf::Keyboard::M:
        minerals = !minerals;
        updateVisuals();
        break;
      case sf::Keyboard::P:
        roads = !roads;
        needUpdate = true;
        break;
      case sf::Keyboard::I:
        info = !info;
        needUpdate = true;
        break;
      case sf::Keyboard::V:
        verbose = !verbose;
        break;
      case sf::Keyboard::U:
        showUI = !showUI;
        break;
      case sf::Keyboard::S:
        showUI = false;
        getScreenshot = true;
        break;
      }
      break;
    case sf::Event::Closed:
      window->close();
      break;
    case sf::Event::Resized:
      mapgen->setSize(window->getSize().x, window->getSize().y);
      log.AddLog("Update map\n");
      mapgen->update();
      updateVisuals();
      break;
    case sf::Event::MouseButtonPressed:
      if (event.mouseButton.button == sf::Mouse::Right && info) {
        lock = !lock;
      }
      break;
    }
  }

  void fade() {
    sf::RectangleShape rectangle;
    rectangle.setSize(sf::Vector2f(window->getSize().x, window->getSize().y));
    auto color = sf::Color::Black;
    color.a = 150;
    rectangle.setFillColor(color);
    rectangle.setPosition(0, 0);
    window->draw(rectangle);
  }

  void drawLoading(sf::Clock *clock) {
    const float frame{clock->restart().asSeconds() * 0.3f};
    const float target{isIncreasing ? progressBar.getRatio() + frame
                                    : progressBar.getRatio() - frame};
    if (target < 0.f)
      isIncreasing = true;
    else if (target > 1.f)
      isIncreasing = false;
    progressBar.setRatio(target);
    sf::Text operation(mapgen->currentOperation, sffont);
    operation.setCharacterSize(20);
    operation.setColor(sf::Color::White);

    auto middle = (sf::Vector2f(window->getSize())) / 2.f;
    operation.setPosition(sf::Vector2f(
        middle.x - operation.getGlobalBounds().width / 2.f, middle.y + 25.f));

    window->draw(progressBar);

    sf::RectangleShape bg;
    bg.setSize(sf::Vector2f(420, 40));
    bg.setFillColor(sf::Color::Black);
    bg.setOutlineColor(sf::Color::White);
    bg.setOutlineThickness(1);
    middle = (sf::Vector2f(window->getSize()) - bg.getSize()) / 2.f;
    bg.setPosition(sf::Vector2f(middle.x, middle.y + 37.f));
    window->draw(bg);
    window->draw(operation);
    window->display();
  }

  void drawMainWindow() {
    ImGui::Begin("Mapgen");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::Text("Window size: w:%d h:%d", window->getSize().x,
                window->getSize().y);

    if (ImGui::Checkbox("Borders", &borders)) {
      updateVisuals();
    }
    ImGui::SameLine(100);
    if (ImGui::Checkbox("Edges", &edges)) {
      updateVisuals();
    }
    ImGui::SameLine(200);
    if (ImGui::Checkbox("Roads", &roads)) {
      updateVisuals();
    }
    if (ImGui::Checkbox("Heights", &heights)) {
      updateVisuals();
    }
    ImGui::SameLine(100);
    if (ImGui::Checkbox("Flat", &flat)) {
      updateVisuals();
    }
    ImGui::SameLine(200);
    if (ImGui::Checkbox("Info", &info)) {
      infoPolygons.clear();
      updateVisuals();
    }

    if (ImGui::Checkbox("Humidity", &hum)) {
      infoPolygons.clear();
      updateVisuals();
    }
    ImGui::SameLine(100);
    if (ImGui::Checkbox("Temp", &temp)) {
      infoPolygons.clear();
      updateVisuals();
    }
    ImGui::SameLine(200);
    if (ImGui::Checkbox("Minerals", &minerals)) {
      infoPolygons.clear();
      updateVisuals();
    }

    if (ImGui::InputInt("Seed", &seed)) {
      mapgen->setSeed(seed);
      log.AddLog("Update map\n");
    }
    ImGui::SameLine(300);

    if (ImGui::Button("Random")) {
      mapgen->seed();
      regen();
    }

    const char *templates[] = {"basic", "archipelago", "new"};
    if (ImGui::Combo("Map template", &t, templates, 3)) {
      mapgen->setMapTemplate(templates[t]);
    }

    if (ImGui::SliderInt("Height octaves", &octaves, 1, 10)) {
      mapgen->setOctaveCount(octaves);
    }

    if (ImGui::SliderFloat("Height freq", &freq, 0.001, 2.f)) {
      mapgen->setFrequency(freq);
    }

    if (ImGui::InputInt("Points", &nPoints)) {
      if (nPoints < 5) {
        nPoints = 5;
      }
      mapgen->setPointCount(nPoints);
    }

    if (ImGui::Button("Update")) {
      regen();
    }
    if (ImGui::Button("Start simulation")) {
      simulate();
    }

    ImGui::Text("\n[ESC] for exit\n[S] for save screenshot\n[R] for random "
                "map\n[U] toggle ui\n[H] toggle humidity\n[I] toggle info\n[P] "
                "toggle pathes\n[RCLICK] toggle selection lock");

    ImGui::End();
  }

  void drawInfo() {
    sf::Vector2<float> pos =
        window->mapPixelToCoords(sf::Mouse::getPosition(*window));

    Region *currentRegion = mapgen->getRegion(pos);
    if (lock) {
      if (lockedRegion == nullptr) {
        lockedRegion = currentRegion;
      } else {
        currentRegion = lockedRegion;
      }
    } else {
      lockedRegion = nullptr;
    }
    if (currentRegion == nullptr) {
      return;
    }

    if (currentRegion->location != nullptr && !roads) {
      for (auto r : mapgen->map->roads) {
        if (r->regions[0] != currentRegion->location->region &&
            r->regions.back() != currentRegion->location->region) {
          continue;
        }
        drawRoad(r);
      }
    }

    sf::ConvexShape selectedPolygon;

    infoWindow(window, currentRegion);

    infoPolygons.clear();

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
      i = 0;
      for (std::vector<sf::ConvexShape>::iterator it = infoPolygons.begin();
           it < infoPolygons.end(); it++, i++) {
        window->draw(infoPolygons[i]);
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
        if (rivers_selection_mask.size() >= mapgen->map->rivers.size() &&
            rivers_selection_mask[rn]) {
          river.setColor(sf::Color(255, 70, 0));
        } else {
          river.setColor(sf::Color(46, 46, 76, float(i) / c * 255.f));
        }
      }
      river.setBezierInterpolation();
      river.setInterpolationSteps(10);
      river.smoothHandles();
      river.update();
      window->draw(river);
      rn++;
    }
  }

  void drawRoad(Road *r) {
    sw::Spline road;
    int i = 0;
    for (auto reg : r->regions) {
      if (reg == nullptr) {
        continue;
      }
      Point p = reg->site;
      road.addVertex(i, {static_cast<float>(p->x), static_cast<float>(p->y)});
      if (reg->megaCluster->isLand) {
        road.setColor(i, sf::Color(70, 50, 0, 40));
      } else {
        road.setColor(i, sf::Color(100, 100, 100, 60));
        road.setThickness(r->weight - 1);
      }
      float w = std::min(4.f, 1.f + reg->traffic / 200.f);
      road.setThickness(w);
    }
    road.setBezierInterpolation();
    road.setInterpolationSteps(10);
    road.smoothHandles();
    road.update();
    window->draw(road);
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

      if (info) {
        for (auto p : poi) {
          window->draw(p);
        }
      }

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

  void drawObjects() {
    auto op = objectsWindow(window, mapgen);

    int i = 0;
    for (std::vector<sf::ConvexShape>::iterator it = op.begin(); it < op.end();
         it++, i++) {
      window->draw(op[i]);
    }
  }

  void serve() {
    sf::Clock deltaClock;
    sf::Clock clock;

    bool faded = false;
    while (window->isOpen()) {
      sf::Event event;
      while (window->pollEvent(event)) {
        processEvent(event);
      }

      if (!ready) {
        if (!faded) {
          fade();
          faded = true;
        }
        drawLoading(&clock);
        continue;
      }
      faded = false;

      ImGui::SFML::Update(*window, deltaClock.restart());

      window->clear(bgColor);

      drawMap();

      sf::Vector2u windowSize = window->getSize();
      char mt[40];
      sprintf(mt, "Mapgen [%s] by Averrin", VERSION.c_str());
      sf::Text mark(mt, sffont);
      mark.setCharacterSize(15);
      mark.setColor(sf::Color::White);
      mark.setPosition(sf::Vector2f(windowSize.x - 240, windowSize.y - 25));
      window->draw(mark);

      if (showUI) {
        drawObjects();
        drawMainWindow();
        log.Draw("Mapgen: Log", &info);

        if (info) {
          drawInfo();
        }
      }

      ImGui::SFML::Render(*window);
      window->display();
      if (getScreenshot) {
        sf::Texture texture;
        texture.create(windowSize.x, windowSize.y);
        texture.update(*window);
        sf::Image screenshot = texture.copyToImage();
        char s[100];
        if (!hum && !temp) {
          sprintf(s, "%d.png", seed);
        } else if (hum) {
          sprintf(s, "%d-hum.png", seed);
        } else {
          sprintf(s, "%d-temp.png", seed);
        }
        screenshot.saveToFile(s);
        char l[255];
        sprintf(l, "Screenshot created: %s\n", s);
        log.AddLog(l);
        showUI = true;
        getScreenshot = false;
      }
    }

    if (generator.joinable())
      generator.join();
    ImGui::SFML::Shutdown();
  }

  void updateVisuals() {
    log.AddLog("Update geometry\n");
    polygons.clear();
    poi.clear();
    sprites.clear();

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

    std::vector<Region *> regions = mapgen->getRegions();
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
          // if (n->biom.name==region->biom.name) {
          //   continue;
          // }
          r += n->biom.color.r;
          g += n->biom.color.g;
          b += n->biom.color.b;
          s++;
        }
        col.r = r / s;
        col.g = g / s;
        col.b = b / s;
      }
      if (!flat) {
        int a = 255 * (region->getHeight(region->site) + 1.6) / 3;
        if (a > 255) {
          a = 255;
        }
        col.a = a;
      }
      if (region->location != nullptr) {
        sf::Sprite sprite;
        auto texture = icons[region->location->type];
        sprite.setTexture(*texture);
        auto p = region->site;
        auto size = texture->getSize();
        sprite.setPosition(
            sf::Vector2f(p->x - size.x / 2.f, p->y - size.y / 2.f));
        sprites.push_back(sprite);


        if (region->state != nullptr && region->city != nullptr) {
          col = region->state->color;
        }
      }

      if (region->state != nullptr && region->stateBorder && !region->seaBorder) {
        polygon.setOutlineColor(region->state->color);
        polygon.setOutlineThickness(1);
      }

      polygon.setFillColor(col);

      if (edges) {
        polygon.setOutlineColor(sf::Color(100, 100, 100));
        polygon.setOutlineThickness(1);
      }
      if (borders) {
        if (region->border) {
          polygon.setOutlineColor(sf::Color(100, 50, 50));
          polygon.setOutlineThickness(1);
        }
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
        if (region->temperature < temperature) {
          sf::Color col(255, 0, 255);
          col.r = std::min(255.f, 255 * (temperature / region->temperature));
          col.b = std::min(
              255.f, 255 * std::abs(1.f - (temperature / region->temperature)));

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

    float var_Min = std::min(std::min(var_R, var_G), var_B); // Min. value of RGB
    float var_Max = std::min(std::max(var_R, var_G), var_B); // Max. value of RGB
    float del_Max = var_Max - var_Min;        // Delta RGB value

    float H;
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
    L+=d;
    return color;
  }
};
