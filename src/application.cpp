#include "Painter.cpp"
#include "mapgen/InfoWindow.hpp"
#include "mapgen/ObjectsWindow.hpp"
#include <imgui-SFML.h>
#include <imgui.h>

class Application {
  std::string VERSION;
  MapGenerator *mapgen;
  std::thread generator;
  sf::RenderWindow *window;
  Painter *painter;
  InfoWindow *infoWindow;
  ObjectsWindow *objectsWindow;

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
    painter = new Painter(window, mapgen, VERSION);
    generator = std::thread([&]() {});
    regen();

    infoWindow = new InfoWindow(window);
    objectsWindow = new ObjectsWindow(window, mapgen);
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
      painter->update();
      ready = mapgen->ready;
    });
  }

  void simulate() {
    if (generator.joinable()) {
      generator.join();
    }
    generator = std::thread([&]() {
      ready = false;
      mapgen->startSimulation();
      painter->update();
      ready = mapgen->ready;
    });
  }

  void initMapGen() {
    seed = std::chrono::system_clock::now().time_since_epoch().count();
    mapgen = new MapGenerator(window->getSize().x, window->getSize().y);
    mapgen->setSeed(5836471);
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
      case sf::Keyboard::H:
        painter->hum = !painter->hum;
        painter->update();
        break;
      case sf::Keyboard::T:
        painter->temp = !painter->temp;
        painter->update();
        break;
      case sf::Keyboard::M:
        rulerRegion = rulerRegion == nullptr ? mapgen->getRegion(pos) : nullptr;
        break;
      case sf::Keyboard::P:
        painter->roads = !painter->roads;
        painter->invalidate();
        break;
      case sf::Keyboard::A:
        painter->areas = !painter->areas;
        painter->invalidate();
        break;
      case sf::Keyboard::I:
        painter->info = !painter->info;
        painter->invalidate();
        break;
      case sf::Keyboard::V:
        painter->verbose = !painter->verbose;
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
      // mapgen->update();
      painter->update();
      break;
    case sf::Event::MouseButtonPressed:
      if (event.mouseButton.button == sf::Mouse::Right && painter->info) {
        lock = !lock;
      }
      break;
    }
  }

  void drawSimWindow() {
    ImGui::Begin("Simulation");

    ImGui::Text("Total cities count: %zu", mapgen->map->cities.size());
    ImGui::Text("Ports count: %zu", std::count_if(mapgen->map->cities.begin(), mapgen->map->cities.end(), [](City* c) {return c->type == PORT;}));
    ImGui::Text("Mines count: %zu", std::count_if(mapgen->map->cities.begin(), mapgen->map->cities.end(), [](City* c) {return c->type == MINE;}));
    ImGui::Text("Agro count: %zu", std::count_if(mapgen->map->cities.begin(), mapgen->map->cities.end(), [](City* c) {return c->type == AGRO;}));
    ImGui::Text("Forts count: %zu", std::count_if(mapgen->map->cities.begin(), mapgen->map->cities.end(), [](City* c) {return c->type == FORT;}));
    ImGui::Text("\n");

    if (ImGui::TreeNode("Economy variables")) {
      ImGui::Columns(2, "vars");

      ImGui::Text("POPULATION_GROWS");
      ImGui::NextColumn();
      ImGui::DragFloat("##POPULATION_GROWS", &mapgen->simulator->vars->POPULATION_GROWS,
                      0.01, 0.01, 0.5);
      ImGui::NextColumn();

      ImGui::Text("POPULATION_GROWS_WEALTH_MODIFIER");
      ImGui::NextColumn();
      ImGui::DragFloat("##POPULATION_GROWS_WEALTH_MODIFIER",
                      &mapgen->simulator->vars->POPULATION_GROWS_WEALTH_MODIFIER,
                      0.01, 0.01, 1.f);
      ImGui::NextColumn();

      ImGui::Text("PACKAGES_PER_NICE");
      ImGui::NextColumn();
      ImGui::DragFloat("##PACKAGES_PER_NICE",
                      &mapgen->simulator->vars->PACKAGES_PER_NICE,
                      1.f, 1.f, 1000.f);
      ImGui::NextColumn();

      ImGui::Text("PACKAGES_AGRO_POPULATION_MODIFIER");
      ImGui::NextColumn();
      ImGui::DragFloat("##PACKAGES_AGRO_POPULATION_MODIFIER",
                    &mapgen->simulator->vars->PACKAGES_AGRO_POPULATION_MODIFIER,
                    0.01, 0.01, 1.f);
      ImGui::NextColumn();

      ImGui::Text("PACKAGES_PER_MINERALS");
      ImGui::NextColumn();
      ImGui::DragFloat("##PACKAGES_PER_MINERALS",
                      &mapgen->simulator->vars->PACKAGES_PER_MINERALS,
                      1.f, 1.f, 1000.f);
      ImGui::NextColumn();

      ImGui::Text("PACKAGES_MINERALS_POPULATION_MODIFIER");
      ImGui::NextColumn();
      ImGui::DragFloat("##PACKAGES_MINERALS_POPULATION_MODIFIER",
                      &mapgen->simulator->vars->PACKAGES_MINERALS_POPULATION_MODIFIER,
                      0.01, 0.01, 1.f);
      ImGui::NextColumn();

      ImGui::Text("CONSUME_AGRO_POPULATION_MODIFIER");
      ImGui::NextColumn();
      ImGui::DragFloat("##CONSUME_AGRO_POPULATION_MODIFIER",
                      &mapgen->simulator->vars->CONSUME_AGRO_POPULATION_MODIFIER,
                      0.01, 0.01, 1.f);
      ImGui::NextColumn();

      ImGui::Text("CONSUME_MINERALS_POPULATION_MODIFIER");
      ImGui::NextColumn();
      ImGui::DragFloat("##CONSUME_MINERALS_POPULATION_MODIFIER",
                      &mapgen->simulator->vars->CONSUME_MINERALS_POPULATION_MODIFIER,
                      0.01, 0.01, 1.f);
      ImGui::NextColumn();

      ImGui::Text("CONSUME_AGRO_WEALTH_MODIFIER");
      ImGui::NextColumn();
      ImGui::DragFloat("##CONSUME_AGRO_WEALTH_MODIFIER",
                      &mapgen->simulator->vars->CONSUME_AGRO_WEALTH_MODIFIER,
                      0.01, 0.01, 1.f);
      ImGui::NextColumn();

      ImGui::Text("CONSUME_MINERALS_WEALTH_MODIFIER");
      ImGui::NextColumn();
      ImGui::DragFloat("##CONSUME_MINERALS_WEALTH_MODIFIER",
                      &mapgen->simulator->vars->CONSUME_MINERALS_WEALTH_MODIFIER,
                      0.01, 0.01, 1.f);
      ImGui::NextColumn();

      ImGui::Text("CANT_BUY_AGRO");
      ImGui::NextColumn();
      ImGui::DragFloat("##CANT_BUY_AGRO",
                      &mapgen->simulator->vars->CANT_BUY_AGRO,
                      0.01, 0.01, 1.f);
      ImGui::NextColumn();

      ImGui::Text("CANT_BUY_MINERALS");
      ImGui::NextColumn();
      ImGui::DragFloat("##CANT_BUY_MINERALS",
                      &mapgen->simulator->vars->CANT_BUY_MINERALS,
                      0.01, 0.01, 1.f);
      ImGui::NextColumn();

      ImGui::Text("MINERALS_POPULATION_PRODUCE");
      ImGui::NextColumn();
      ImGui::DragFloat("##MINERALS_POPULATION_PRODUCE",
                      &mapgen->simulator->vars->MINERALS_POPULATION_PRODUCE,
                      0.01, 0.01, 1.f);
      ImGui::NextColumn();

      ImGui::Text("AGRO_POPULATION_PRODUCE");
      ImGui::NextColumn();
      ImGui::DragFloat("##AGRO_POPULATION_PRODUCE",
                      &mapgen->simulator->vars->AGRO_POPULATION_PRODUCE,
                      0.01, 0.01, 1.f);
      ImGui::NextColumn();

      ImGui::Text("PORT_FEE");
      ImGui::NextColumn();
      ImGui::DragFloat("##PORT_FEE",
                      &mapgen->simulator->vars->PORT_FEE,
                      0.01, 0.01, 1.f);
      ImGui::NextColumn();

      ImGui::Columns(1);
      ImGui::TreePop();
    }
    ImGui::Text("\n");

    if (mapgen->simulator->report != nullptr) {
      ImGui::Text("Last simulation report");
      auto pop = &mapgen->simulator->report->population;
      if (pop->size() > 0) {
        char t[1000];
        sprintf(t, "Total: %f\nAvg: %f", pop->back(), pop->back()/mapgen->map->cities.size());
        ImGui::PlotLines(
            t,
            [](void *data, int idx) {
              return ((std::vector<float> *)data)->at(idx) / 1000.f;
            },
            (void *)pop, pop->size(), 0.f, "Total Population (k)",
            pop->front() / 1000.f, pop->back() / 1000.f, ImVec2(0, 100));
      }

      auto w = &mapgen->simulator->report->wealth;
      if (pop->size() > 0) {
        char tw[1000];
        sprintf(tw, "Avg: %f", w->back()/mapgen->map->cities.size());
        ImGui::PlotLines(tw,
                         [](void *data, int idx) {
                           return ((std::vector<float> *)data)->at(idx);
                         },
                         (void *)w, pop->size(), 0.f, "Total Wealth",
                         w->front(), w->back(), ImVec2(0, 100));
      }
      ImGui::Text("\n");
      if (ImGui::Button("Reset simulation")) {
        mapgen->simulator->resetAll();
        painter->update();
      }
      ImGui::Text("\n");
    }

    if (ImGui::Button("Start simulation")) {
      simulate();
    }

    ImGui::End();
  }

  void drawMainWindow() {
    ImGui::Begin("Generation");
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
      ImGui::SameLine(320);

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

      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Special layers")) {
      if (ImGui::Checkbox("Edges", &painter->edges)) {
        painter->update();
      }
      ImGui::SameLine(120);
      if (ImGui::Checkbox("Heights", &painter->heights)) {
        painter->update();
      }

      ImGui::SameLine(220);
      if (ImGui::Checkbox("Humidity", &painter->hum)) {
        painter->update();
      }
      if (ImGui::Checkbox("Temp", &painter->temp)) {
        painter->update();
      }
      ImGui::SameLine(120);
      if (ImGui::Checkbox("Minerals", &painter->minerals)) {
        painter->update();
      }
      ImGui::TreePop();
    }

    if (ImGui::TreeNode("Info toggles")) {
      if (ImGui::Checkbox("Cities", &painter->cities)) {
        painter->update();
      }
      if (ImGui::Checkbox("Locations*", &painter->locations)) {
        painter->update();
      }
      if (ImGui::Checkbox("States", &painter->states)) {
        painter->update();
      }
      if (ImGui::Checkbox("Roads and sea pathes*", &painter->roads)) {
        painter->update();
      }
      ImGui::TreePop();
    }
    ImGui::Text("\n");

    if (ImGui::Checkbox("Show verbose info", &painter->info)) {
      painter->update();
    }

    ImGui::Text("\n[ESC] for exit\n[S] for save screenshot\n[R] for random "
                "map\n[U] toggle ui\n[H] toggle humidity\n[I] toggle info\n[P] "
                "toggle pathes\n[RCLICK] toggle selection lock\n"
                "[M] for distance ruler");

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

    infoWindow->draw(currentRegion);
    painter->drawInfo(currentRegion);

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

      char mt[40];
      sprintf(mt, "%f",
              mg::getDistance(rulerRegion->site, currentRegion->site));
      sf::Text mark(mt, painter->sffont);
      mark.setCharacterSize(15);
      mark.setColor(sf::Color::Black);
      mark.setPosition(line[1].position + sf::Vector2f(15.f, 15.f));
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
        drawObjects();
        drawMainWindow();
        drawSimWindow();

        if (painter->info) {
          drawInfo();
        }
      }

      ImGui::SFML::Render(*window);
      window->display();

      if (getScreenshot) {
        char s[100];
        if (!painter->hum && !painter->temp) {
          sprintf(s, "%d.png", seed);
        } else if (painter->hum) {
          sprintf(s, "%d-hum.png", seed);
        } else {
          sprintf(s, "%d-temp.png", seed);
        }
        auto texture = painter->getScreenshot();
        sf::Image screenshot = texture.copyToImage();
        screenshot.saveToFile(s);
        char l[255];
        sprintf(l, "Screenshot created: %s\n", s);
        showUI = true;
        getScreenshot = false;
      }
    }

    if (generator.joinable())
      generator.join();
    ImGui::SFML::Shutdown();
  }
};
