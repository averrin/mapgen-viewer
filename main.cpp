#include <memory>
#include "imgui.h"
#include "imgui-SFML.h"
 
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics.hpp>
#include <VoronoiDiagramGenerator.h>

constexpr int windowSize(900);

int relax = 0;
bool startOver = true;
bool relaxForever = false;

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

	srand(std::clock());
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
	int nPoints = 2000;
	unsigned int dimension = 900;

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
                vertices.push_back({ { static_cast<float>(p1.x),static_cast<float>(p1.y) },sf::Color::White });
                vertices.push_back({ { static_cast<float>(p2.x),static_cast<float>(p2.y) },sf::Color::White });
            }
        }
    };

    auto generateNewDiagram = [&]()
      {
        startOver = false;
        relaxForever = false;
        relax = 0;
        sites = new std::vector<sf::Vector2<double>>();
        genRandomSites(*sites, bbox, dimension, nPoints);
        timer.restart();
        diagram.reset(vdg.compute(*sites, bbox));
        auto duration = timer.getElapsedTime().asMilliseconds();
        std::cout << "Computing a diagram of " << nPoints << " points took " << duration << "ms.\n";
        delete sites;
        updateVisuals();
      };

    generateNewDiagram();

    sf::RenderWindow window(sf::VideoMode(640, 480), "");
    window.setVerticalSyncEnabled(true);
    ImGui::SFML::Init(window);

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
 
    sf::Color bgColor;
 
    float color[3] = { 0.1, 0.1, 0.1 };

    bgColor.r = static_cast<sf::Uint8>(color[0] * 255.f);
    bgColor.g = static_cast<sf::Uint8>(color[1] * 255.f);
    bgColor.b = static_cast<sf::Uint8>(color[2] * 255.f);

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);
 
    // let's use char array as buffer, see next part
    // for instructions on using std::string with ImGui
    char windowTitle[255] = "ImGui + SFML = <3";
 
    window.setTitle(windowTitle);
    window.resetGLStates(); // call it if you only draw ImGui. Otherwise not needed.
    bool no_edges = false;
    bool no_dots = false;
    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
 
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }
 
        ImGui::SFML::Update(window, deltaClock.restart());
 
        ImGui::Begin("Mapgen"); // begin window
 
                                       // Background color edit
        if (ImGui::ColorEdit3("Background color", color)) {
            // this code gets called if color value changes, so
            // the background color is upgraded automatically!
            bgColor.r = static_cast<sf::Uint8>(color[0] * 255.f);
            bgColor.g = static_cast<sf::Uint8>(color[1] * 255.f);
            bgColor.b = static_cast<sf::Uint8>(color[2] * 255.f);
        }
        ImGui::Checkbox("No edges", &no_edges); ImGui::SameLine(150);
        ImGui::Checkbox("No dots", &no_dots);
 
        if (ImGui::InputInt("Points", &nPoints)) {
            generateNewDiagram();
        }

        if (ImGui::Button("Relax")) {
          diagram.reset(vdg.relax());
          relax++;
        }
        ImGui::SameLine(100);
        ImGui::Text("Relax iterations: %d", relax); // end window
        ImGui::End(); // end window
        updateVisuals();
 
        window.clear(bgColor); // fill background with color


        auto pointCount = diagram->cells.size();
        if (!no_dots)
          window.draw(vertices.data(), pointCount,sf::PrimitiveType::Points);

        //then lines, starting from the vert after the last point
        if (!no_edges)
          window.draw(vertices.data() + pointCount, vertices.size() - pointCount, sf::PrimitiveType::Lines);

        ImGui::SFML::Render(window);
        window.display();
    }
 
    ImGui::SFML::Shutdown();
}
