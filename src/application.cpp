#include <imgui-SFML.h>
#include <imgui.h>

#include "mapgen/Painter.hpp"
#include "mapgen/InfoWindow.hpp"
#include "mapgen/ObjectsWindow.hpp"
#include "mapgen/SimulationWindow.hpp"
#include "mapgen/WeatherWindow.hpp"
#include <memory>

class Application {
  std::string VERSION;
  std::shared_ptr<MapGenerator> mapgen;
  std::unique_ptr<Painter> painter;
  std::thread generator;
  sf::RenderWindow *window;
  InfoWindow *infoWindow;
  ObjectsWindow *objectsWindow;
  SimulationWindow *simulationWindow;
  WeatherWindow *weatherWindow;

  int relax = 0;
  int octaves;
  float freq;
  int nPoints;
  int seed;
  int t = 0;
  bool showUI = true;
  bool getScreenshot = false;
  bool ready = false;
  Region *lockedRegion = nullptr;
  Region *rulerRegion = nullptr;
  bool lock = false;

public:
  Application(std::string v) : VERSION(v) {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->AddFontFromFileTTF("./font.ttf", 15.0f);

#ifdef _WIN32
    window = new sf::RenderWindow(sf::VideoMode(1600, 900), "",
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
    painter = std::make_unique<Painter>(window, mapgen, VERSION);
    generator = std::thread([&]() {});
    regen();

    infoWindow = new InfoWindow();
    objectsWindow = new ObjectsWindow(mapgen);
    simulationWindow = new SimulationWindow(mapgen);
    weatherWindow = new WeatherWindow(mapgen);
  }

  void regen() {
    if (generator.joinable())
      generator.join();
    generator = std::thread([&]() {
      lockedRegion = nullptr;
      rulerRegion = nullptr;
      lock = false;
      ready = false;
      mapgen->update();
      seed = mapgen->getSeed();
      relax = mapgen->getRelax();
      ready = mapgen->ready;
      painter->invalidate(true);
    });
  }

  void resetSimulation() {
    if (generator.joinable()) {
      generator.join();
    }
    generator = std::thread([&]() {
      ready = false;
      mapgen->simulator->resetAll();
      ready = mapgen->ready;
      painter->invalidate(true);
    });
  }

  void simulate() {
    if (generator.joinable()) {
      generator.join();
    }
    generator = std::thread([&]() {
      ready = false;
      mapgen->startSimulation();
      painter->invalidate();
      ready = mapgen->ready;
      painter->invalidate(true);
    });
  }

  void initMapGen() {
    seed = std::chrono::system_clock::now().time_since_epoch().count();
    mapgen = std::make_shared<MapGenerator>(window->getSize().x, window->getSize().y);
    // mapgen->setSeed(38007851);
	mapgen->setSeed(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    octaves = mapgen->getOctaveCount();
    freq = mapgen->getFrequency();
    nPoints = mapgen->getPointCount();
    relax = mapgen->getRelax();
  }

  void processEvent(sf::Event event) {
    sf::Vector2<float> pos =
        window->mapPixelToCoords(sf::Mouse::getPosition(*window));
    ImGui::SFML::ProcessEvent(event);

    switch (event.type) {
    case sf::Event::KeyPressed:
      switch (event.key.code) {
      case sf::Keyboard::R:
        mapgen->seed();
        regen();
        break;
      case sf::Keyboard::Escape:
        window->close();
        break;
      case sf::Keyboard::L:
        simulate();
        break;
      case sf::Keyboard::M:
        rulerRegion = rulerRegion == nullptr ? mapgen->getRegion(nullptr, pos) : nullptr;
        break;
      case sf::Keyboard::P:
        painter->roads = !painter->roads;
        painter->invalidate();
        break;
      case sf::Keyboard::I:
        painter->info = !painter->info;
        break;
      case sf::Keyboard::V:
        painter->verbose = !painter->verbose;
        painter->invalidate(true);
        break;
      case sf::Keyboard::U:
        showUI = !showUI;
        break;
      case sf::Keyboard::H:
        // painter->heights = true;
        // painter->hum = true;
        painter->temp = true;
        painter->labels = false;
        painter->locations = false;
        showUI = false;
        painter->invalidate(true);
        break;
      case sf::Keyboard::S:
        showUI = false;
        getScreenshot = true;
        break;
      case sf::Keyboard::W:
        // painter->showWalkers = !painter->showWalkers;
        // painter->layers->getLayer("water")->damaged = true;
        painter->wind = !painter->wind;
        painter->invalidate(true);
        break;
      case sf::Keyboard::N:
        painter->labels = !painter->labels;
        painter->invalidate();
        break;
      case sf::Keyboard::B:
        painter->blur = !painter->blur;
        painter->invalidate();
        break;
      case sf::Keyboard::T:
        painter->useTextures = !painter->useTextures;
        painter->invalidate();
        break;
      }
      break;
    case sf::Event::Closed:
      window->close();
      break;
    case sf::Event::Resized:
      mapgen->setSize(window->getSize().x, window->getSize().y);
      // mapgen->update();
      painter->invalidate();
      break;
    case sf::Event::MouseButtonPressed:
      if (event.mouseButton.button == sf::Mouse::Right && painter->info) {
        lock = !lock;
      }
      break;
    }
  }

  void drawMainWindow() {
    ImGui::Begin("MapGen");
    // ImGui::BeginTabBar("##MainTabBar");

    // ImGui::DrawTabsBackground();

    // TODO: move to separate file
    // if (ImGui::AddTab("Generation")) {
      ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                  1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

      ImGui::Text("Window size: w:%d h:%d", window->getSize().x,
                  window->getSize().y);

      ImGui::Text("\n");
      ImGui::Text("Controls:");
      if (ImGui::TreeNode("Settings")) {

        if (ImGui::InputInt("Seed", &seed)) {
          mapgen->setSeed(seed);
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

        if (ImGui::Button("Random")) {
          mapgen->seed();
          regen();
        }
        ImGui::SameLine(120);
        if (ImGui::Button("Update")) {
          regen();
        }

        ImGui::TreePop();
      }
      if (ImGui::TreeNode("Special layers")) {
        if (ImGui::Checkbox("Edges", &painter->edges)) {
          painter->invalidate(true);
        }
        ImGui::SameLine(120);
        if (ImGui::Checkbox("Heights", &painter->heights)) {
          painter->invalidate(true);
        }

        if (ImGui::Checkbox("Minerals", &painter->minerals)) {
          painter->invalidate(true);
        }
        ImGui::TreePop();
      }

      if (ImGui::TreeNode("Info toggles")) {
        if (ImGui::Checkbox("Cities & Locations*", &painter->locations)) {
          painter->invalidate();
        }
        if (ImGui::Checkbox("States", &painter->states)) {
          painter->invalidate();
        }
        if (ImGui::Checkbox("Roads and sea pathes*", &painter->roads)) {
          painter->invalidate();
        }
        ImGui::Checkbox("Walkers*", &painter->showWalkers);
        if (ImGui::Checkbox("Labels", &painter->labels)) {
          painter->invalidate();
        }
        if (ImGui::Checkbox("Experimental textures", &painter->useTextures)) {
          painter->invalidate(true);
        }
        if (ImGui::Checkbox("Use cache map", &painter->useCacheMap)) {
          painter->invalidate();
        }
        if (ImGui::Checkbox("Show sea pathes", &painter->showSeaPathes)) {
          painter->layers->getLayer("roads")->damaged = true;
          painter->invalidate();
        }
        if (ImGui::Checkbox("Water blur", &painter->blur)) {
          painter->invalidate();
        }

        ImGui::TreePop();
      }
      ImGui::Text("\n");

      if (ImGui::Checkbox("Show verbose info", &painter->info)) {
        // painter->invalidate();
      }
      ImGui::Text("\n");

      ImGui::Text(
          "\n[ESC] for exit\n[S] for save screenshot\n[R] for random "
          "map\n[U] toggle ui\n[T] toggle textures\n[I] toggle info\n[P] "
          "toggle pathes\n[RCLICK] toggle selection lock\n"
          "[M] for distance ruler\n"
          "[W] toggle walkers\n"
          "[B] toggle water blur\n"
          "[N] toggle labels\n"
          "[A] show state clusters\n");
    // }
    ImGui::End();

    ImGui::Begin("Weather");
    weatherWindow->draw(std::move(mapgen->weather), std::move(painter));
    ImGui::End();


    ImGui::Begin("Simulation");
    // if (ImGui::AddTab("Simulation")) {
      simulationWindow->draw();

      if (mapgen->simulator->report != nullptr) {
        if (ImGui::Button("Reset simulation")) {
          resetSimulation();
        }
      }
      ImGui::Text("\n");
      if (ImGui::Button("Start simulation")) {
        simulate();
      }
    ImGui::End();
    // }
    ImGui::Begin("Objects");
    // if (ImGui::AddTab("Objects")) {
      drawObjects();
    // }
    // ImGui::EndTabBar();
    ImGui::End();

    // ImGui::Begin("Visual layers [DEBUG]");
    //   ImGui::SliderInt("Hue delta", &painter->hueDelta, 2, 26);
    //   ImGui::SliderInt("Land border", &painter->landBorderHeight, 0, 12);
    //   ImGui::SliderInt("Forrest border", &painter->forrestBorderHeight, 0, 12);


    //   ImGui::DragFloat("Lum delta", &painter->lumDelta, 2.f, 0.5f, 50.f);
    //   if (ImGui::Button("Apply")) painter->invalidate(true);

    //   for (auto l : painter->layers->layers) {
    //     char ln[200];
    //     sprintf(ln, "%s [%s%s]", l->name.c_str(), l->shader == nullptr ? "_" : "#", l->mask == nullptr ? "_" : "#");
    //     if (ImGui::Checkbox(ln, &l->enabled)) {
    //       painter->invalidate();
    //     }
    //   }
    // ImGui::End();

  }
  
  Region *currentRegionCache = nullptr;

  void drawInfo() {
    sf::Vector2<float> pos =
        window->mapPixelToCoords(sf::Mouse::getPosition(*window));

    Region *currentRegion = mapgen->getRegion(currentRegionCache, pos);
    currentRegionCache = currentRegion;
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

    infoWindow->draw(*currentRegion);
    painter->drawInfo(currentRegion);
    // painter->layers->getLayer("roads")->damaged = true;

    if (rulerRegion != nullptr) {
      sf::ConvexShape rPolygon;
      PointList points = rulerRegion->getPoints();
      rPolygon.setPointCount(int(points.size()));
      for (int pi = 0; pi < int(points.size()); pi++) {
        Point p = points[pi];
        rPolygon.setPoint(pi, sf::Vector2f(static_cast<float>(p->x),
                                           static_cast<float>(p->y)));
      }
      rPolygon.setFillColor(sf::Color::Transparent);
      rPolygon.setOutlineColor(sf::Color::Black);
      rPolygon.setOutlineThickness(2);

      sf::CircleShape site(2.f);
      site.setFillColor(sf::Color::Red);
      site.setPosition(static_cast<float>(rulerRegion->site->x - 1),
                       static_cast<float>(rulerRegion->site->y - 1));
      window->draw(rPolygon);
      window->draw(site);

      sf::Vertex line[2];
      line[0].position = sf::Vector2f(static_cast<float>(rulerRegion->site->x),
                                      static_cast<float>(rulerRegion->site->y));
      line[0].color = sf::Color::Red;
      line[1].position =
          sf::Vector2f(static_cast<float>(currentRegion->site->x),
                       static_cast<float>(currentRegion->site->y));
      line[1].color = sf::Color::Black;

      window->draw(line, 2, sf::Lines);

      sf::RectangleShape bg;
      bg.setFillColor(sf::Color::Black);
      bg.setOutlineColor(sf::Color(30,30,30));
      bg.setOutlineThickness(1);

      char mt[40];
      sprintf(mt, "%f",
              mg::getDistance(rulerRegion->site, currentRegion->site));
      sf::Text mark(mt, painter->sffont);
      mark.setCharacterSize(15);
      mark.setFillColor(sf::Color::White);
      // mark.setColor(sf::Color::White);
      mark.setPosition(line[1].position + sf::Vector2f(15.f, 15.f));

      bg.setSize(sf::Vector2f(mark.getGlobalBounds().width + 8, 18));
      bg.setPosition(
                     sf::Vector2f(line[1].position + sf::Vector2f(11.f, 15.f)));

      window->draw(bg);
      window->draw(mark);
    }
  }

  void drawObjects() {
    objectsWindow->draw();
    painter->drawObjects(objectsWindow->objectPolygons);
  }

  void serve() {
    sf::Clock deltaClock;

    bool faded = false;
    while (window->isOpen()) {
      sf::Event event;
      while (window->pollEvent(event)) {
        processEvent(event);
      }

      if (!ready) {
        if (!faded) {
          painter->fade();
          faded = true;
        }
        painter->drawLoading();
        continue;
      }
      faded = false;

      ImGui::SFML::Update(*window, deltaClock.restart());

      painter->draw();

      if (showUI) {
        drawMainWindow();

        if (painter->info) {
          drawInfo();
        }
      }

      ImGui::SFML::Render(*window);
      window->display();

      // if (getScreenshot) {
      //   char s[100];
      //   if (!painter->hum && !painter->temp) {
      //     sprintf(s, "%d.png", seed);
      //   } else if (painter->hum) {
      //     sprintf(s, "%d-hum.png", seed);
      //   } else {
      //     sprintf(s, "%d-temp.png", seed);
      //   }
      //   auto texture = painter->getScreenshot();
      //   sf::Image screenshot = texture.copyToImage();
      //   screenshot.saveToFile(s);
      //   char l[255];
      //   sprintf(l, "Screenshot created: %s\n", s);
      //   showUI = true;
      //   getScreenshot = false;
      // }
    }

    if (generator.joinable())
      generator.join();
    ImGui::SFML::Shutdown();
  }
};
