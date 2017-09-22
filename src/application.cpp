#include <imgui-SFML.h>
#include <imgui.h>
#include "logger.cpp"
#include "infoWindow.cpp"
#include "objectsWindow.cpp"
#include "Painter.cpp"

class Application {
  std::string VERSION;
  AppLog log;
  MapGenerator *mapgen;
  std::thread generator;
  sf::RenderWindow *window;
  Painter* painter;

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
    painter = new Painter(window, mapgen->map, VERSION);
    generator = std::thread([&]() {});
    regen();

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
    mapgen->setSeed(8701368);
    octaves = mapgen->getOctaveCount();
    freq = mapgen->getFrequency();
    nPoints = mapgen->getPointCount();
    relax = mapgen->getRelax();
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
        painter->hum = !painter->hum;
        painter->update();
        break;
      case sf::Keyboard::T:
        painter->temp = !painter->temp;
        painter->update();
        break;
      case sf::Keyboard::M:
        painter->minerals = !painter->minerals;
        painter->update();
        break;
      case sf::Keyboard::P:
        painter->roads = !painter->roads;
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
      log.AddLog("Update map\n");
      mapgen->update();
      painter->update();
      break;
    case sf::Event::MouseButtonPressed:
      if (event.mouseButton.button == sf::Mouse::Right && painter->info) {
        lock = !lock;
      }
      break;
    }
  }


  void drawMainWindow() {
    ImGui::Begin("Mapgen");
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::Text("Window size: w:%d h:%d", window->getSize().x,
                window->getSize().y);

    ImGui::Text("\n");
    ImGui::Text("Controls:");
    if (ImGui::TreeNode("Settings")) {

      if (ImGui::InputInt("Seed", &seed)) {
        mapgen->setSeed(seed);
        log.AddLog("Update map\n");
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

    infoWindow(window, currentRegion);
    painter->drawInfo(currentRegion);
  }


  void drawObjects() {
    auto op = objectsWindow(window, mapgen->map);
    painter->drawObjects(op);
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
          painter->fade();
          faded = true;
        }
        painter->drawLoading(&clock);
        continue;
      }
      faded = false;

      ImGui::SFML::Update(*window, deltaClock.restart());

      painter->draw();

      if (showUI) {
        drawObjects();
        drawMainWindow();
        log.Draw("Mapgen: Log", &painter->info);

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
        log.AddLog(l);
        showUI = true;
        getScreenshot = false;
      }
    }

    if (generator.joinable())
      generator.join();
    ImGui::SFML::Shutdown();
  }
};
