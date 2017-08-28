#include <memory>
#include <map>
#include "imgui.h"
#include "imgui-SFML.h"
 
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics.hpp>
#include <VoronoiDiagramGenerator.h>

#include <libnoise/noise.h>
#include "noiseutils.h"

int windowSize;

int relax = 0;
bool startOver = true;
bool relaxForever = false;
int seed;
int octaves = 3;
float freq = 1.0;
std::map<sf::Vector2<double>*,float> heights;

double normalize(double in, int dimension) 
{
	return in / (float)dimension*1.8 - 0.9;
}

bool sitesOrdered(const sf::Vector2<double>& s1, const sf::Vector2<double>& s2) {
	if (s1.y < s2.y)
		return true;
	if (s1.y == s2.y && s1.x < s2.x)
		return true;

	return false;
}

void genRandomSites(std::vector<sf::Vector2<double>>& sites, sf::Rect<double>& bbox, unsigned int dimension, unsigned int numSites) {
	std::vector<sf::Vector2<double>> tmpSites;

	tmpSites.reserve(numSites);
	sites.reserve(numSites);

  sf::Vector2<double> s;

	srand(seed);
	for (unsigned int i = 0; i < numSites; ++i) {
		s.x = 1 + (rand() / (double)RAND_MAX)*(dimension - 2);
		s.y = 1 + (rand() / (double)RAND_MAX)*(dimension - 2);
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
  windowSize = 800;
	int nPoints = 4000;
  seed = std::clock();

  //the generator
	VoronoiDiagramGenerator vdg = VoronoiDiagramGenerator();

  //the generated diagram
	std::unique_ptr<Diagram> diagram;

  //sites used for generation
	std::vector<sf::Vector2<double>>* sites;

  //maximum bounds for the diagram
	sf::Rect<double> bbox(0,0,windowSize,windowSize);

  //used to measure generation time
  sf::Clock timer;

  //used to draw the diagram
  std::vector<sf::Vertex> vertices;

  module::Perlin myModule;
  utils::NoiseMap heightMap;

  utils::RendererImage renderer;
  utils::Image image;



    auto updateVisuals = [&]()
    {
        //clear first
        vertices.clear();

        //then reserve the correct amount of verts
        // one point for each cell, and one line (2 verts) for each edge
        vertices.reserve(diagram->cells.size() + (diagram->edges.size() * 2));
        for (auto c : diagram->cells)
        {
            //red point for each cell site
            sf::Vector2<double>& p = c->site.p;
            vertices.push_back({{ static_cast<float>(p.x),static_cast<float>(p.y)}, sf::Color::Red});
        }

        for (Edge* e : diagram->edges)
        {
            if (e->vertA && e->vertB)
            {
                sf::Vector2<double>& p1 = *e->vertA;
                sf::Vector2<double>& p2 = *e->vertB;

                //white line for each edge
                vertices.push_back({ { static_cast<float>(p1.x),static_cast<float>(p1.y) },sf::Color(150,150,150) });
                vertices.push_back({ { static_cast<float>(p2.x),static_cast<float>(p2.y) },sf::Color(150,150,150) });
            }
        }
    };


    auto generateNewDiagram = [&]()
      {

        bbox = sf::Rect<double>(0,0,windowSize,windowSize);
        startOver = false;
        relaxForever = false;
        relax = 0;
        sites = new std::vector<sf::Vector2<double>>();
        genRandomSites(*sites, bbox, windowSize, nPoints);
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
        heightMapBuilder.SetDestSize (windowSize, windowSize);
        heightMapBuilder.SetBounds (0.0, 10.0, 0.0, 10.0);
        heightMapBuilder.Build ();

        // renderer.SetSourceNoiseMap (heightMap);
        // renderer.SetDestImage (image);
        // renderer.ClearGradient ();
        // renderer.AddGradientPoint (-1.0000, utils::Color (  0,   0, 128, 255)); // deeps
        // renderer.AddGradientPoint (-0.2500, utils::Color (  0,   0, 255, 255)); // shallow
        // renderer.AddGradientPoint ( 0.0000, utils::Color (  0, 128, 255, 255)); // shore
        // renderer.AddGradientPoint ( 0.0625, utils::Color (240, 240,  64, 255)); // sand
        // renderer.AddGradientPoint ( 0.1250, utils::Color ( 32, 160,   0, 255)); // grass
        // renderer.AddGradientPoint ( 0.3750, utils::Color (224, 224,   0, 255)); // dirt
        // renderer.AddGradientPoint ( 0.7500, utils::Color (128, 128, 128, 255)); // rock
        // renderer.AddGradientPoint ( 1.0000, utils::Color (255, 255, 255, 255)); // snow
        // renderer.EnableLight ();
        // renderer.Render ();

        // utils::WriterBMP writer;
        // writer.SetSourceImage(image);
        // writer.SetDestFilename("height.bmp");
        // writer.WriteDestFile ();

      };


    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "", sf::Style::Default, settings);
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
    windowSize = window.getSize().x;

    generateNewDiagram();
    generateHeight();
    bool no_edges = false;
    bool no_dots = false;
    bool no_height = false;
    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
 
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::Resized) {
              windowSize = window.getSize().x;
              updateVisuals();
            }
        }
 
        ImGui::SFML::Update(window, deltaClock.restart());
 
        ImGui::Begin("Mapgen"); // begin window
 
                                       // Background color edit
        // if (ImGui::ColorEdit3("Background color", color)) {
        //     // this code gets called if color value changes, so
        //     // the background color is upgraded automatically!
        //     bgColor.r = static_cast<sf::Uint8>(color[0] * 255.f);
        //     bgColor.g = static_cast<sf::Uint8>(color[1] * 255.f);
        //     bgColor.b = static_cast<sf::Uint8>(color[2] * 255.f);
        // }
        ImGui::Checkbox("No edges", &no_edges); ImGui::SameLine(150);
        ImGui::Checkbox("No dots", &no_dots);
        // ImGui::Checkbox("No height", &no_height);
 
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
        }
        ImGui::SameLine(100);
        ImGui::Text("Relax iterations: %d", relax);

        ImGui::Text("Window size: w:%d h:%d",
                    window.getSize().x,
                    window.getSize().y
                    );

        sf::Vector2<float> pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        ImGui::Text("Mouse: x:%f y:%f",
                    pos.x,
                    pos.y);
        sf::Vertex v;
        sf::ConvexShape polygon;

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

              polygon.setPointCount(int(c->getEdges().size()));

              heights.insert(std::make_pair(&p, heightMap.GetValue(p.x, p.y)));
              ImGui::Text("%f", p.x); ImGui::NextColumn();
              ImGui::Text("%f", p.y); ImGui::NextColumn();
              ImGui::Text("%f", heights[&p]); ImGui::NextColumn();

              for (int i = 0; i < int(c->getEdges().size()); i++)
                {
                  sf::Vector2<double>* p0;
                  p0 = c->getEdges()[i]->startPoint();

                  heights.insert(std::make_pair(p0, heightMap.GetValue(p0->x, p0->y)));
                  // if (ImGui::Selectable("", selected == i, ImGuiSelectableFlags_SpanAllColumns))
                  //   selected = i;
                  // ImGui::NextColumn();
                  ImGui::Text("%f", p0->x); ImGui::NextColumn();
                  ImGui::Text("%f", p0->y); ImGui::NextColumn();
                  ImGui::Text("%f", heights[p0]); ImGui::NextColumn();

                  polygon.setPoint(i, sf::Vector2f(p0->x, p0->y));
                }

              sf::Color color;
              std::array<float, 8> b = {
                -1.0000,
                -0.2500,
                  0.0000,
                  0.0625,
                  0.1250,
                  0.3750,
                  0.7500,
                  1.0000,
              };

              std::array<sf::Color, 8> c = {
                sf::Color(  0,   0, 128),
                sf::Color(  0,   0, 255),
                sf::Color(  0, 128, 255),
                sf::Color(240, 240,  64),
                sf::Color( 32, 160,   0),
                sf::Color(224, 224,   0),
                sf::Color(128, 128, 128),
                sf::Color(255, 255, 255),
              };
              for (int i = 0; i < 8; i++)
                {
                  if (heights[&p]>b[i]) {
                    color = c[i];
                  }
                }

              polygon.setFillColor(color);
              // polygon.setOutlineColor(color);
              // polygon.setOutlineThickness(1);

              break;
            }
          }
        ImGui::End(); // end window
        updateVisuals();
 
        window.clear(bgColor); // fill background with color


        // if (!no_height) {
        //   sf::Texture texture;
        //   texture.loadFromFile("height.bmp");
        //   sf::Sprite sprite;
        //   sprite.setTexture(texture, true);
        //   window.draw(sprite);
        // }

        auto pointCount = diagram->cells.size();
        if (!no_dots)
          window.draw(vertices.data(), pointCount,sf::PrimitiveType::Points);

        //then lines, starting from the vert after the last point
        if (!no_edges)
          window.draw(vertices.data() + pointCount, vertices.size() - pointCount, sf::PrimitiveType::Lines);


        window.draw(&v, 1, sf::PrimitiveType::Points);
        window.draw(polygon);

        ImGui::SFML::Render(window);
        window.display();
    }
 
    ImGui::SFML::Shutdown();
}
