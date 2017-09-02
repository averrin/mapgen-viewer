#include <memory>
#include <map>
#include <imgui.h>
#include <imgui-SFML.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics.hpp>

#include "mapgen/MapGenerator.hpp"
#include "logger.cpp"
#include "../MapgenConfig.h"
#include "SelbaWard/SelbaWard.hpp"

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

int main()
{
  seed = std::clock();
  sf::ContextSettings settings;
  settings.antialiasingLevel = 8;

  sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "", sf::Style::Default, settings);

  //the generator
  MapGenerator mapgen = MapGenerator(window.getSize().x, window.getSize().y);
  octaves = mapgen.getOctaveCount();
  freq = mapgen.getFrequency();
  nPoints = mapgen.getPointCount();
  relax = mapgen.getRelax();

  //used to measure generation time
  sf::Clock timer;

  window.setVerticalSyncEnabled(true);
  ImGui::SFML::Init(window);

  static AppLog log;

  // Demo: add random items (unless Ctrl is held)
  log.AddLog("Welcome to Mapgen\n");

  std::vector<sf::ConvexShape> polygons;
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
      int a = 255 * (region->getHeight(region->site)+1.6)/3;
      if (a > 255) {
        a = 255;
      }
      col.a = a;
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

    mapgen.setSeed(140731122 /*37946940*/);
    mapgen.update();
    seed = mapgen.getSeed();
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
        ImGui::Checkbox("Info",&info);

        if (ImGui::InputInt("Seed", &seed)) {
          mapgen.setSeed(seed);
          mapgen.update();
          updateVisuals();
          relax = mapgen.getRelax();
        }
        if (ImGui::Button("Random")) {
          mapgen.seed();
          seed = mapgen.getSeed();
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
          mapgen.update();
          updateVisuals();
          relax = mapgen.getRelax();
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
          if (nPoints < 5) {
            nPoints = 5;
          }
          mapgen.setPointCount(nPoints);
          mapgen.update();
          updateVisuals();
          relax = mapgen.getRelax();
        }

        if (ImGui::Button("Relax")) {
          mapgen.relax();
          updateVisuals();
          relax = mapgen.getRelax();
        }
        ImGui::SameLine(100);
        ImGui::Text("Relax iterations: %d", relax);
        ImGui::Text("Polygons: %zu", polygons.size());

        ImGui::Text("Window size: w:%d h:%d",
                    window.getSize().x,
                    window.getSize().y
                    );
        if (ImGui::Button("+1000")) {
          nPoints+=1000;
          mapgen.setPointCount(nPoints);
          mapgen.update();
          updateVisuals();
          relax = mapgen.getRelax();
        }

        sf::ConvexShape selectedPolygon;
        sf::CircleShape site(2.f);
        if (info) {
        sf::Vector2<float> pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        ImGui::Text("Mouse: x:%f y:%f",
                    pos.x,
                    pos.y);

        Region* currentRegion =  mapgen.getRegion(pos);

        PointList points = currentRegion->getPoints();
        selectedPolygon.setPointCount(int(points.size()));

        ImGui::Text("Region: %p", currentRegion);
        ImGui::Text("Site: x:%f y:%f z:%f", currentRegion->site->x, currentRegion->site->y, currentRegion->getHeight(currentRegion->site));
        ImGui::Text("Biom: %s", currentRegion->biom.name.c_str());
        ImGui::Text("Has river: %s", currentRegion->hasRiver ? "true" : "false");
        ImGui::Text("Is border: %s", currentRegion->border ? "true" : "false");

        ImGui::Columns(3, "cells");
        ImGui::Separator();
        ImGui::Text("x"); ImGui::NextColumn();
        ImGui::Text("y"); ImGui::NextColumn();
        ImGui::Text("z"); ImGui::NextColumn();
        ImGui::Separator();

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
        site.setFillColor(sf::Color::Red);
        site.setPosition(static_cast<float>(currentRegion->site->x-1),static_cast<float>(currentRegion->site->y-1));
}

        ImGui::End(); // end window

        bool b(true);
        log.Draw("Mapgen: Log", &b);

        window.clear(bgColor); // fill background with color

        int i = 0;
        for(std::vector<sf::ConvexShape>::iterator it=polygons.begin() ; it < polygons.end(); it++, i++) {
          window.draw(polygons[i]);
        }

        sw::Spline river;
        river.setColor(sf::Color( 51,  51,  91));
        river.setThickness(3);
        i = 0;
        for(PointList::iterator it=mapgen.river.begin() ; it < mapgen.river.end(); it++, i++) {
          Point p = mapgen.river[i];
          river.addVertex(i, {static_cast<float>(p->x), static_cast<float>(p->y)});
        }
        river.setBezierInterpolation(); // enable Bezier spline
        river.setInterpolationSteps(10); // curvature resolution
        river.smoothHandles();
        river.update();
        window.draw(river);

        if(info) {
          window.draw(selectedPolygon);
          window.draw(site);
        }
        if (sites) {
          for (auto v : verticies) {
            window.draw(&v, 1, sf::PrimitiveType::Points);
          }
        }


        ImGui::SFML::Render(window);
        window.display();
    }
 
    ImGui::SFML::Shutdown();
}
