#include <memory>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

#include <VoronoiDiagramGenerator.h>

// Window dimensions
constexpr int windowSize(900);

double normalize(double in, int dimension) 
{
	return in / (float)dimension*1.8 - 0.9;
}
//globals for use in giving relaxation commands
int relax = 0;
bool startOver = true;
bool relaxForever = false;

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
    //number of points to generate
	unsigned int nPoints;
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

    //update the visuals
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

    //generate a new diagram
    auto generateNewDiagram = [&]()
    {
        std::cout << "\tPress 'R' to perform Lloyd's relaxation once.\n"
            "\tPress 'T' to perform Lloyd's relaxation ten times.\n"
            "\tPress 'Y' to toggle continuous Lloyd's relaxation.\n"
            "\tPress 'X' to generate a new diagram with a different number of sites.\n"
            "\tPress 'Esc' to exit.\n\n";
        startOver = false;
        relaxForever = false;
        relax = 0;
        sites = new std::vector<sf::Vector2<double>>();
        std::cout << "How many points? ";
        std::cin >> nPoints;
        genRandomSites(*sites, bbox, dimension, nPoints);
        timer.restart();
        diagram.reset(vdg.compute(*sites, bbox));
        auto duration = timer.getElapsedTime().asMilliseconds();
        std::cout << "Computing a diagram of " << nPoints << " points took " << duration << "ms.\n";
        delete sites;
        updateVisuals();
    };

    generateNewDiagram();

    //now open the window
    sf::RenderWindow window;
    window.create(sf::VideoMode(windowSize, windowSize), "Voronoi");
    window.setVerticalSyncEnabled(true);

	while (window.isOpen()) 
    {
        sf::Event evt;
        while (window.pollEvent(evt))
        {
            switch (evt.type)
            {
            case sf::Event::Closed:
                window.close();
                break;

            case sf::Event::KeyPressed:
                switch (evt.key.code)
                {
                case sf::Keyboard::Escape:
                    window.close();
                    break;
                    
                case sf::Keyboard::R:
                    ++relax;
                    break;

                case sf::Keyboard::T:
                    relax += 10;
                    break;

                case sf::Keyboard::X:
                    generateNewDiagram();
                    break;

                case sf::Keyboard::Y:
                    if (relaxForever)
                        relaxForever = false;
                    else
                        relaxForever = true;
                    break;
                }
                break;
            }
        }

		if (relax || relaxForever) 
        {
            timer.restart();
			diagram.reset(vdg.relax());
            auto duration = timer.getElapsedTime().asMilliseconds();

			std::cout << "Computing a diagram of " << nPoints << " points took " << duration << "ms.\n";
			--relax;
			if (relax < 0) 
                relax = 0;

            updateVisuals();
		}

        window.clear();
        
        //draw points first
        auto pointCount = diagram->cells.size();
        window.draw(vertices.data(), pointCount,sf::PrimitiveType::Points);

        //then lines, starting from the vert after the last point
        window.draw(vertices.data() + pointCount, vertices.size() - pointCount, sf::PrimitiveType::Lines);

        window.display();
	}
}
