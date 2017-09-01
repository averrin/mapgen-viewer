#include <memory>
#include <map>
#include <imgui.h>
#include <imgui-SFML.h>
 
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics.hpp>

#include "mapgen/MapGenerator.hpp"

int relax = 0;
int seed;
int octaves;
float freq;
int nPoints;

int main()
{
  seed = std::clock();
  sf::ContextSettings settings;
  settings.antialiasingLevel = 8;

  sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "", sf::Style::Default, settings);

  //the generator
  MapGenerator mapgen = MapGenerator(window.getSize().x, window.getSize().y);
  seed = mapgen.getSeed();
  octaves = mapgen.getOctaveCount();
  freq = mapgen.getFrequency();
  nPoints = mapgen.getPointCount();

  //used to measure generation time
  sf::Clock timer;

  window.setVerticalSyncEnabled(true);
  ImGui::SFML::Init(window);

  std::vector<sf::ConvexShape> polygons;
  auto  updateVisuals = [&](){
    polygons.clear();
    int i = 0;
    std::vector<Region> regions = mapgen.getRegions();
    polygons.reserve(regions.size());
    for(std::vector<Region>::iterator it=regions.begin() ; it < regions.end(); it++, i++) {

      Region region = regions[i];
      sf::ConvexShape polygon;
      PointList points = region.getPoints();
      polygon.setPointCount(points.size());
      int n = 0;
      for(PointList::iterator it2=points.begin() ; it2 < points.end(); it2++, n++) {
        sf::Vector2<double>* p = points[n];
        polygon.setPoint(n, sf::Vector2f(p->x, p->y));
      }

      polygon.setFillColor(region.biom.color);
      polygon.setOutlineColor(sf::Color(100,100,100));
      polygon.setOutlineThickness(1);
      polygons.push_back(polygon);
    }
  };



    sf::Color bgColor;
    float color[3] = { 0.1, 0.1, 0.1 };

    bgColor.r = static_cast<sf::Uint8>(color[0] * 255.f);
    bgColor.g = static_cast<sf::Uint8>(color[1] * 255.f);
    bgColor.b = static_cast<sf::Uint8>(color[2] * 255.f);
 
    // let's use char array as buffer, see next part
    // for instructions on using std::string with ImGui
    char windowTitle[255] = "MapGen";
 
    window.setTitle(windowTitle);
    window.resetGLStates(); // call it if you only draw ImGui. Otherwise not needed.

    mapgen.build();
    updateVisuals();

    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
 
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::Resized) {
              mapgen.setSize(window.getSize().x, window.getSize().y);
              mapgen.update();
              updateVisuals();
            }
        }
 
        ImGui::SFML::Update(window, deltaClock.restart());
 
        ImGui::Begin("Mapgen"); // begin window
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
 
        if (ImGui::InputInt("Seed", &seed)) {
          mapgen.setSeed(seed);
          mapgen.update();
          updateVisuals();
        }
        if (ImGui::Button("Random")) {
          mapgen.seed();
          seed = mapgen.getSeed();
          mapgen.update();
          updateVisuals();
        }

        if (ImGui::InputInt("Height octaves", &octaves)) {
          if (octaves < 1) {
            octaves = 1;
          }
          mapgen.setOctaveCount(octaves);
          mapgen.update();
          updateVisuals();
        }

        if (ImGui::InputFloat("Height freq", &freq)) {
          if (freq < 0) {
            freq = 0;
          }
          mapgen.setFrequency(freq);
          mapgen.update();
          updateVisuals();
        }

        if (ImGui::InputInt("Points", &nPoints)) {
          if (nPoints < 1) {
            nPoints = 1;
          }
          mapgen.setPointCount(nPoints);
          mapgen.update();
          updateVisuals();
        }

        if (ImGui::Button("Relax")) {
          mapgen.relax();
          updateVisuals();
          relax++;
        }
        ImGui::SameLine(100);
        ImGui::Text("Relax iterations: %d", relax);

        ImGui::Text("Window size: w:%d h:%d",
                    window.getSize().x,
                    window.getSize().y
                    );
        if (ImGui::Button("+1000")) {
          nPoints+=1000;
          mapgen.setPointCount(nPoints);
          mapgen.update();
          updateVisuals();
        }

        sf::Vector2<float> pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        ImGui::Text("Mouse: x:%f y:%f",
                    pos.x,
                    pos.y);
        sf::ConvexShape selectedPolygon;


        int n = 0;
        Region* currentRegion =  mapgen.getRegion(pos);

        PointList points = currentRegion->getPoints();
        selectedPolygon.setPointCount(int(points.size()));

        ImGui::Text("Site: x:%f y:%f z:%f", currentRegion->site->x, currentRegion->site->y, currentRegion->getHeight(currentRegion->site));
        ImGui::Text("Biom: %s", currentRegion->biom.name.c_str());

        ImGui::Columns(3, "cells");
        ImGui::Separator();
        ImGui::Text("x"); ImGui::NextColumn();
        ImGui::Text("y"); ImGui::NextColumn();
        ImGui::Text("z"); ImGui::NextColumn();
        ImGui::Separator();
        static int selected = -1;

        for (int pi = 0; pi < int(points.size()); pi++)
          {
            Point p = points[pi];
            selectedPolygon.setPoint(pi, sf::Vector2f(static_cast<float>(p->x), static_cast<float>(p->y)));

                  ImGui::Text("%f", p->x); ImGui::NextColumn();
                  ImGui::Text("%f", p->y); ImGui::NextColumn();
                  ImGui::Text("%f", currentRegion->getHeight(p)); ImGui::NextColumn();
          }

        selectedPolygon.setFillColor(sf::Color::Transparent);
        selectedPolygon.setOutlineColor(sf::Color::Red);
        selectedPolygon.setOutlineThickness(2);
        // sf::Vertex site;
        // site = sf::Vertex({{static_cast<float>(currentRegion->site->x),
        //         static_cast<float>(currentRegion->site->y)}, sf::Color::Green});
        sf::CircleShape site(2.f);
        site.setFillColor(sf::Color::Red);
        site.setPosition(static_cast<float>(currentRegion->site->x),static_cast<float>(currentRegion->site->y));
        //       sf::Vector2<double>& p = c->site.p;


        ImGui::End(); // end window

        window.clear(bgColor); // fill background with color

        int i = 0;
        for(std::vector<sf::ConvexShape>::iterator it=polygons.begin() ; it < polygons.end(); it++, i++) {
          window.draw(polygons[i]);
        }
        window.draw(selectedPolygon);
        // window.draw(&site, 1, sf::PrimitiveType::Points);
        window.draw(site);


        ImGui::SFML::Render(window);
        window.display();
    }
 
    ImGui::SFML::Shutdown();
}
