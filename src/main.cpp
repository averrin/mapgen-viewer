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
            }
        }
 
        ImGui::SFML::Update(window, deltaClock.restart());
 
        ImGui::Begin("Mapgen"); // begin window
 
        if (ImGui::InputInt("Seed", &seed)) {
          mapgen.setSeed(seed);
          mapgen.update();
        }
        if (ImGui::Button("Random")) {
          mapgen.seed();
          mapgen.update();
        }

        if (ImGui::InputInt("Height octaves", &octaves)) {
          if (octaves < 1) {
            octaves = 1;
          }
          mapgen.setOctaveCount(octaves);
          mapgen.update();
        }

        if (ImGui::InputFloat("Height freq", &freq)) {
          if (freq < 0) {
            freq = 0;
          }
          mapgen.setFrequency(freq);
          mapgen.update();
        }

        if (ImGui::InputInt("Points", &nPoints)) {
          if (nPoints < 1) {
            nPoints = 1;
          }
          mapgen.setPointCount(nPoints);
          mapgen.update();
        }

        if (ImGui::Button("Relax")) {
          mapgen.relax();
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
        }

        sf::Vector2<float> pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        ImGui::Text("Mouse: x:%f y:%f",
                    pos.x,
                    pos.y);
        sf::Vertex v;
        sf::ConvexShape selectedPolygon;


        int n = 0;
        // for (auto c : mapgen.diagram->cells)
        //   {
        //     //red point for each cell site
        //     if(c->pointIntersection(pos.x, pos.y) != -1) {
        //       sf::Vector2<double>& p = c->site.p;
        //       v = sf::Vertex({{static_cast<float>(p.x), static_cast<float>(p.y)}, sf::Color::Green});
        //       ImGui::Text("Cell: x:%f y:%f", p.x, p.y);

        //       ImGui::Columns(3, "cells");
        //       ImGui::Separator();
        //       ImGui::Text("x"); ImGui::NextColumn();
        //       ImGui::Text("y"); ImGui::NextColumn();
        //       ImGui::Text("z"); ImGui::NextColumn();
        //       ImGui::Separator();
        //       static int selected = -1;

        //       ImGui::Text("%f", p.x); ImGui::NextColumn();
        //       ImGui::Text("%f", p.y); ImGui::NextColumn();
        //       ImGui::Text("%f", heights[&p]); ImGui::NextColumn();


        //       sf::ConvexShape poly= polygons[n];
        //       polygon.setPointCount(poly.getPointCount());

        //       for (int pi = 0; pi < int(poly.getPointCount()); pi++)
        //         {
        //           polygon.setPoint(pi, poly.getPoint(pi));
        //         }
        //       polygon.setFillColor(sf::Color::Transparent);
        //       polygon.setOutlineColor(sf::Color::Red);
        //       polygon.setOutlineThickness(1);

        //       break;
        //     }
        //     n++;
        //   }
        ImGui::End(); // end window
 
        window.clear(bgColor); // fill background with color

        int i = 0;
        // std::vector<sf::ConvexShape> polygons = mapgen.getPolygons();
        std::vector<Region> regions = mapgen.getRegions();
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
          window.draw(polygon);
        }

        window.draw(selectedPolygon);

        ImGui::SFML::Render(window);
        window.display();
    }
 
    ImGui::SFML::Shutdown();
}
