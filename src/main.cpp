#include <memory>
#include <map>
#include <imgui.h>
#include <imgui-SFML.h>
 
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics.hpp>
#include <VoronoiDiagramGenerator.h>

#include <libnoise/noise.h>
#include "noise/noiseutils.h"

int relax = 0;
int seed;
int octaves = 3;
float freq = 0.3;
std::map<sf::Vector2<double>*,float> heights;

const std::array<float, 8> borders = {
  -1.0000,
  -0.2500,
  0.0000,
  0.0625,
  0.1250,
  0.3750,
  0.7500,
  1.0000,
};

const std::array<sf::Color, 8> colors = {
  sf::Color( 39,  39,  70),
  sf::Color( 51,  51,  91),
  sf::Color( 91, 132, 173),
  sf::Color(210, 185, 139),
  sf::Color(136, 170,  85),
  sf::Color( 51, 119,  85),
  sf::Color(128, 128, 128),
  sf::Color(240, 240, 240),
};

double normalize(double in, int dimension) {
	return in / (float)dimension*1.8 - 0.9;
}

bool sitesOrdered(const sf::Vector2<double>& s1, const sf::Vector2<double>& s2) {
	if (s1.y < s2.y)
		return true;
	if (s1.y == s2.y && s1.x < s2.x)
		return true;

	return false;
}

void genRandomSites(std::vector<sf::Vector2<double>>& sites, sf::Rect<double>& bbox, unsigned int dx, unsigned int dy, unsigned int numSites) {
	std::vector<sf::Vector2<double>> tmpSites;

	tmpSites.reserve(numSites);
	sites.reserve(numSites);

  sf::Vector2<double> s;

	srand(seed);
	for (unsigned int i = 0; i < numSites; ++i) {
		s.x = 1 + (rand() / (double)RAND_MAX)*(dx - 2);
		s.y = 1 + (rand() / (double)RAND_MAX)*(dy - 2);
		tmpSites.push_back(s);
	}

	//remove any duplicates that exist
	std::sort(tmpSites.begin(), tmpSites.end(), sitesOrdered);
	sites.push_back(tmpSites[0]);
	for (sf::Vector2<double>& s : tmpSites) {
		if (s != sites.back()) sites.push_back(s);
	}
}
 
int main()
{
	int nPoints = 10000;
  seed = std::clock();
  sf::ContextSettings settings;
  settings.antialiasingLevel = 8;

  sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "", sf::Style::Default, settings);

  //the generator
	VoronoiDiagramGenerator vdg = VoronoiDiagramGenerator();

  //the generated diagram
	std::unique_ptr<Diagram> diagram;

  //sites used for generation
	std::vector<sf::Vector2<double>>* sites;

  //maximum bounds for the diagram
	sf::Rect<double> bbox(0,0,window.getSize().x, window.getSize().y);

  //used to measure generation time
  sf::Clock timer;

  //used to draw the diagram
  std::vector<sf::Vertex> vertices;

  module::Perlin myModule;
  utils::NoiseMap heightMap;

  utils::RendererImage renderer;
  utils::Image image;

  window.setVerticalSyncEnabled(true);
  ImGui::SFML::Init(window);

  std::vector<sf::ConvexShape> polygons;

  auto updateVisuals = [&]()
    {
      polygons.clear();
      polygons.reserve(diagram->cells.size());
      for (auto c : diagram->cells) {
        sf::ConvexShape polygon;
        polygon.setPointCount(int(c->getEdges().size()));

        float ht = 0;
        for (int i = 0; i < int(c->getEdges().size()); i++)
          {
            sf::Vector2<double>* p0;
            p0 = c->getEdges()[i]->startPoint();

            heights.insert(std::make_pair(p0, heightMap.GetValue(p0->x, p0->y)));
            polygon.setPoint(i, sf::Vector2f(p0->x, p0->y));
            ht += heights[p0];
          }
        ht = ht/c->getEdges().size();
        sf::Vector2<double>& p = c->site.p;
        heights.insert(std::make_pair(&p, ht));

        sf::Color color = sf::Color( 23,  23,  40);
        for (int i = 0; i < 8; i++)
          {
            if (ht>borders[i]) {
              color = colors[i];
            }
          }

        polygon.setFillColor(color);
        polygons.push_back(polygon);
      }
    };

    auto generateNewDiagram = [&]()
      {
        bbox = sf::Rect<double>(0,0,window.getSize().x, window.getSize().y);
        relax = 1;
        sites = new std::vector<sf::Vector2<double>>();
        genRandomSites(*sites, bbox, window.getSize().x, window.getSize().y, nPoints);
        timer.restart();
        diagram.reset(vdg.compute(*sites, bbox));
        auto duration = timer.getElapsedTime().asMilliseconds();
        std::cout << "Computing a diagram of " << nPoints << " points took " << duration << "ms.\n";
        delete sites;
        updateVisuals();
      };

    auto generateHeight = [&]()
      {
        heights.clear();
        myModule.SetSeed(seed);
        myModule.SetOctaveCount (octaves);
        myModule.SetFrequency (freq);
        utils::NoiseMapBuilderPlane heightMapBuilder;
        heightMapBuilder.SetSourceModule (myModule);
        heightMapBuilder.SetDestNoiseMap (heightMap);
        heightMapBuilder.SetDestSize (window.getSize().x, window.getSize().y);
        heightMapBuilder.SetBounds (0.0, 10.0, 0.0, 10.0);
        heightMapBuilder.Build ();
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

    generateHeight();
    generateNewDiagram();

    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
 
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::Resized) {
              updateVisuals();
            }
        }
 
        ImGui::SFML::Update(window, deltaClock.restart());
 
        ImGui::Begin("Mapgen"); // begin window
 
        if (ImGui::InputInt("Seed", &seed)) {
          generateNewDiagram();
          generateHeight();
        }
        if (ImGui::Button("Random")) {
          seed = std::clock();
          generateNewDiagram();
          generateHeight();
        }

        if (ImGui::InputInt("Height octaves", &octaves)) {
          if (octaves < 1) {
            octaves = 1;
          }
          generateHeight();
        }

        if (ImGui::InputFloat("Height freq", &freq)) {
          if (freq < 0) {
            freq = 0;
          }
          generateHeight();
        }

        if (ImGui::InputInt("Points", &nPoints)) {
          if (nPoints < 1) {
            nPoints = 1;
          }
          generateNewDiagram();
        }

        if (ImGui::Button("Relax")) {
          diagram.reset(vdg.relax());
          relax++;
          updateVisuals();
        }
        ImGui::SameLine(100);
        ImGui::Text("Relax iterations: %d", relax);

        ImGui::Text("Window size: w:%d h:%d",
                    window.getSize().x,
                    window.getSize().y
                    );
        if (ImGui::Button("+1000")) {
          nPoints+=1000;
          generateNewDiagram();
        }

        sf::Vector2<float> pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        ImGui::Text("Mouse: x:%f y:%f",
                    pos.x,
                    pos.y);
        sf::Vertex v;
        sf::ConvexShape polygon;


        int n = 0;
        for (auto c : diagram->cells)
          {
            //red point for each cell site
            if(c->pointIntersection(pos.x, pos.y) != -1) {
              sf::Vector2<double>& p = c->site.p;
              v = sf::Vertex({{static_cast<float>(p.x), static_cast<float>(p.y)}, sf::Color::Green});
              ImGui::Text("Cell: x:%f y:%f", p.x, p.y);

              ImGui::Columns(3, "cells");
              ImGui::Separator();
              ImGui::Text("x"); ImGui::NextColumn();
              ImGui::Text("y"); ImGui::NextColumn();
              ImGui::Text("z"); ImGui::NextColumn();
              ImGui::Separator();
              static int selected = -1;

              ImGui::Text("%f", p.x); ImGui::NextColumn();
              ImGui::Text("%f", p.y); ImGui::NextColumn();
              ImGui::Text("%f", heights[&p]); ImGui::NextColumn();


              sf::ConvexShape poly= polygons[n];
              polygon.setPointCount(poly.getPointCount());

              for (int pi = 0; pi < int(poly.getPointCount()); pi++)
                {
                  polygon.setPoint(pi, poly.getPoint(pi));
                }
              polygon.setFillColor(sf::Color::Transparent);
              polygon.setOutlineColor(sf::Color::Red);
              polygon.setOutlineThickness(1);

              break;
            }
            n++;
          }
        ImGui::End(); // end window
 
        window.clear(bgColor); // fill background with color

        int i = 0;
        for(std::vector<sf::ConvexShape>::iterator it=polygons.begin() ; it < polygons.end(); it++, i++) {
          window.draw(polygons[i]);
        }

        window.draw(polygon);

        ImGui::SFML::Render(window);
        window.display();
    }
 
    ImGui::SFML::Shutdown();
}
