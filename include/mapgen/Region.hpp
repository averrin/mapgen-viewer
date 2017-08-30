#include <SFML/Graphics.hpp>
#include <vector>

enum Biom {
};

class Region {
public:
  Region(std::vector<sf::Vector2<double> >* v, Biom biom);
  Biom biom;
private:
	std::vector<sf::Vector2<double> >* _verticies;
};
