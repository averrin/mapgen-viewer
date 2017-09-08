#include <memory>
#include <map>
#include <imgui.h>
#include <imgui-SFML.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics.hpp>

#include "logger.cpp"
#include "../MapgenConfig.h"
#include "SelbaWard/SelbaWard.hpp"
#include "infoWindow.cpp"
#include "objectsWindow.cpp"

int relax = 0;
int seed;
int octaves;
float freq;
int nPoints;
bool borders;
bool sites;
bool edges;
bool info;
bool heights;
bool flat;
bool hum;
bool simplifyRivers;

int main()
{
  seed = std::clock();
  sf::ContextSettings settings;
  settings.antialiasingLevel = 8;

  /* sf::RenderWindow  */
  sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "", sf::Style::Default, settings);

  //the generator
  MapGenerator mapgen = MapGenerator(window.getSize().x, window.getSize().y);
  octaves = mapgen.getOctaveCount();
  freq = mapgen.getFrequency();
  nPoints = mapgen.getPointCount();
  relax = mapgen.getRelax();
  simplifyRivers = mapgen.simpleRivers;

  //used to measure generation time
  sf::Clock timer;

  window.setVerticalSyncEnabled(true);
  ImGui::SFML::Init(window);

  static AppLog log;

  // Demo: add random items (unless Ctrl is held)
  log.AddLog("Welcome to Mapgen\n");

  std::vector<sf::ConvexShape> polygons;
  std::vector<sf::ConvexShape> infoPolygons;
  std::vector<sf::Vertex> verticies;
  sf::Color bgColor;
  float color[3] = { 0.12, 0.12, 0.12 };
  auto updateVisuals = [&](){
    log.AddLog("Update geometry\n");
    polygons.clear();
    verticies.clear();
    int i = 0;
    std::vector<Region*>* regions = mapgen.getRegions();
    polygons.reserve(regions->size());
    verticies.reserve(regions->size());
    for(std::vector<Region*>::iterator it=regions->begin() ; it < regions->end(); it++, i++) {

      Region* region = (*regions)[i];
      sf::ConvexShape polygon;
      PointList points = region->getPoints();
      polygon.setPointCount(points.size());
      int n = 0;
      for(PointList::iterator it2=points.begin() ; it2 < points.end(); it2++, n++) {
        sf::Vector2<double>* p = points[n];
        polygon.setPoint(n, sf::Vector2f(p->x, p->y));
      }

      sf::Color col(region->biom.color);
      if(!flat){
      int a = 255 * (region->getHeight(region->site)+1.6)/3;
      if (a > 255) {
        a = 255;
      }
      col.a = a;
      }
      polygon.setFillColor(col);
      if(edges) {
        polygon.setOutlineColor(sf::Color(100,100,100));
        polygon.setOutlineThickness(1);
      }
      if(borders) {
        if(region->border) {
          polygon.setOutlineColor(sf::Color(100,50,50));
          polygon.setOutlineThickness(1);
        }
      }
      if(heights) {
        sf::Color col(region->biom.color);
        col.r = 255 * (region->getHeight(region->site)+1.6)/3.2;
        col.a = 20 + 255 * (region->getHeight(region->site)+1.6)/3.2;
        col.b = col.b/3;
        col.g = col.g/3;
        polygon.setFillColor(col);
        color[2] = 1.f;
      }

      if(hum && region->humidity != 1) {
        sf::Color col(region->biom.color);
        col.b = 255 * region->humidity;
        col.a = 255 * region->humidity;
        col.r = col.b/3;
        col.g = col.g/3;
        polygon.setFillColor(col);
      }
      polygons.push_back(polygon);
      verticies.push_back(sf::Vertex(sf::Vector2f(region->site->x, region->site->y), sf::Color(100,100,100)));
    }
  };

    bgColor.r = static_cast<sf::Uint8>(color[0] * 255.f);
    bgColor.g = static_cast<sf::Uint8>(color[1] * 255.f);
    bgColor.b = static_cast<sf::Uint8>(color[2] * 255.f);

    char windowTitle[255] = "MapGen";

    window.setTitle(windowTitle);
    window.resetGLStates(); // call it if you only draw ImGui. Otherwise not needed.

    mapgen.setSeed(111629613 /*81238299*/);
    mapgen.update();
    seed = mapgen.getSeed();
    updateVisuals();

    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);

            switch (event.type)
              {
              case sf::Event::KeyPressed:
                switch (event.key.code)
                  {
                  case sf::Keyboard::R:
                    mapgen.seed();
                    seed = mapgen.getSeed();
                    log.AddLog("Update map\n");
                    mapgen.update();
                    updateVisuals();
                    relax = mapgen.getRelax();
                    break;
                  case sf::Keyboard::Escape:
                    window.close();
                    break;
                  case sf::Keyboard::S:
                    sf::Vector2u windowSize = window.getSize();
                    sf::Texture texture;
                    texture.create(windowSize.x, windowSize.y);
                    texture.update(window);
                    sf::Image screenshot = texture.copyToImage();
                    char s[100];
                    sprintf(s, "%d.png", seed);
                    screenshot.saveToFile(s);
                    char l[255];
                    sprintf(l, "Screenshot created: %s\n", s);
                    log.AddLog(l);
                    break;
                  }
                break;
              }

            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::Resized) {
              mapgen.setSize(window.getSize().x, window.getSize().y);
              log.AddLog("Update map\n");
              mapgen.update();
              updateVisuals();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin("Mapgen"); // begin window
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("Polygons: %zu", polygons.size());

        ImGui::Text("Window size: w:%d h:%d",
                    window.getSize().x,
                    window.getSize().y
                    );

        if (ImGui::Checkbox("Borders",&borders)) {
          updateVisuals();
        }
        ImGui::SameLine(100);
        ImGui::Checkbox("Sites",&sites);
        ImGui::SameLine(200);
        if(ImGui::Checkbox("Edges",&edges)){
            updateVisuals();
        }
        if(ImGui::Checkbox("Heights",&heights)){
          updateVisuals();
        }
        ImGui::SameLine(100);
        if(ImGui::Checkbox("Flat",&flat)){
          updateVisuals();
        }
        ImGui::SameLine(200);
        if(ImGui::Checkbox("Info",&info)) {
          infoPolygons.clear();
          updateVisuals();
        }

        if(ImGui::Checkbox("Humidity",&hum)) {
          infoPolygons.clear();
          updateVisuals();
        }
        ImGui::SameLine(100);
        if(ImGui::Checkbox("Simplify rivers",&simplifyRivers)) {
          mapgen.simpleRivers = simplifyRivers;
          mapgen.update();
        }

        if (ImGui::InputInt("Seed", &seed)) {
          mapgen.setSeed(seed);
          log.AddLog("Update map\n");
          mapgen.update();
          updateVisuals();
          relax = mapgen.getRelax();
        }
        if (ImGui::Button("Random")) {
          mapgen.seed();
          seed = mapgen.getSeed();
          log.AddLog("Update map\n");
          mapgen.update();
          updateVisuals();
          relax = mapgen.getRelax();
        }
        ImGui::SameLine(100);
        if (ImGui::Button("Update")) {
          mapgen.forceUpdate();
          updateVisuals();
        }

        if (ImGui::InputInt("Height octaves", &octaves)) {
          if (octaves < 1) {
            octaves = 1;
          }
          mapgen.setOctaveCount(octaves);
          log.AddLog("Update map\n");
          mapgen.update();
          updateVisuals();
          relax = mapgen.getRelax();
        }

        if (ImGui::InputFloat("Height freq", &freq)) {
          if (freq < 0) {
            freq = 0;
          }
          mapgen.setFrequency(freq);
          log.AddLog("Update map\n");
          mapgen.update();
          updateVisuals();
        }

        if (ImGui::InputInt("Points", &nPoints)) {
          if (nPoints < 5) {
            nPoints = 5;
          }
          mapgen.setPointCount(nPoints);
          log.AddLog("Update map\n");
          mapgen.update();
          updateVisuals();
          relax = mapgen.getRelax();
        }

        if (ImGui::Button("Relax")) {
          log.AddLog("Update map\n");
          mapgen.relax();
          updateVisuals();
          relax = mapgen.getRelax();
        }
        ImGui::SameLine(100);
        ImGui::Text("Relax iterations: %d", relax);
        if (ImGui::Button("+1000")) {
          nPoints+=1000;
          mapgen.setPointCount(nPoints);
          log.AddLog("Update map\n");
          mapgen.update();
          updateVisuals();
          relax = mapgen.getRelax();
        }
        ImGui::SameLine(100);
        if (ImGui::Button("-1000")) {
          nPoints-=1000;
          mapgen.setPointCount(nPoints);
          log.AddLog("Update map\n");
          mapgen.update();
          updateVisuals();
          relax = mapgen.getRelax();
        }

        ImGui::Text("\n[ESC] for exit\n[S] for save screensot\n[R] for random map");

        ImGui::End(); // end window

        log.Draw("Mapgen: Log", &info);

        window.clear(bgColor); // fill background with color

        int i = 0;
        for(std::vector<sf::ConvexShape>::iterator it=polygons.begin() ; it < polygons.end(); it++, i++) {
          window.draw(polygons[i]);
        }

        if(info) {

          sf::Vector2<float> pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
          Region* currentRegion =  mapgen.getRegion(pos);
          // char p[100];
          // sprintf(p,"%p\n",currentRegion);
          // std::cout<<p<<std::flush;
          sf::ConvexShape selectedPolygon;

          infoWindow(&window, currentRegion);

          infoPolygons.clear();

          PointList points = currentRegion->getPoints();
          selectedPolygon.setPointCount(int(points.size()));

          Cluster* cluster = currentRegion->cluster;
          int i = 0;
          for(std::vector<Region*>::iterator it=cluster->regions.begin() ; it < cluster->regions.end(); it++, i++) {

            Region* region = cluster->regions[i];
            sf::ConvexShape polygon;
            PointList points = region->getPoints();
            polygon.setPointCount(points.size());
            int n = 0;
            for(PointList::iterator it2=points.begin() ; it2 < points.end(); it2++, n++) {
              sf::Vector2<double>* p = points[n];
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
            selectedPolygon.setPoint(pi, sf::Vector2f(static_cast<float>(p->x), static_cast<float>(p->y)));
          }

          sf::CircleShape site(2.f);

          selectedPolygon.setFillColor(sf::Color::Transparent);
          selectedPolygon.setOutlineColor(sf::Color::Red);
          selectedPolygon.setOutlineThickness(2);
          site.setFillColor(sf::Color::Red);
          site.setPosition(static_cast<float>(currentRegion->site->x-1),static_cast<float>(currentRegion->site->y-1));

          i = 0;
          for(std::vector<sf::ConvexShape>::iterator it=infoPolygons.begin() ; it < infoPolygons.end(); it++, i++) {
            window.draw(infoPolygons[i]);
          }

          window.draw(selectedPolygon);
          window.draw(site);
        }

        int rn =0;
        for (auto rvr : mapgen.rivers){
          sw::Spline river;
          river.setThickness(3);
          i = 0;
          int c = rvr->size();
          for(PointList::iterator it=rvr->begin() ; it < rvr->end(); it++, i++) {
            Point p = (*rvr)[i];
            river.addVertex(i, {static_cast<float>(p->x), static_cast<float>(p->y)});
            float t = float(i)/c * 2.f;
            river.setThickness(i, t);
            if(rivers_selection_mask.size() >= mapgen.rivers.size() && rivers_selection_mask[rn]) {
              river.setColor(sf::Color( 255, 70, 0));
            } else {
              river.setColor(sf::Color( 46, 46, 76, float(i)/c * 255.f));
            }
          }
          river.setBezierInterpolation(); // enable Bezier spline
          river.setInterpolationSteps(10); // curvature resolution
          river.smoothHandles();
          river.update();
          window.draw(river);
          rn++;
        }

        if (sites) {
          for (auto v : verticies) {
            window.draw(&v, 1, sf::PrimitiveType::Points);
          }
        }


        auto op = objectsWindow(&window, &mapgen);

        i = 0;
        for(std::vector<sf::ConvexShape>::iterator it=op.begin() ; it < op.end(); it++, i++) {
          window.draw(op[i]);
        }

        ImGui::SFML::Render(window);
        window.display();
    }
 
    ImGui::SFML::Shutdown();
}
