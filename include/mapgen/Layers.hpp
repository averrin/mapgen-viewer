#ifndef LAYERS_H_
#define LAYERS_H_

#include <SFML/Graphics.hpp>

class Layer : public sf::Drawable {
public:
  Layer(std::string name);
  bool needUpdate = true;
  bool enabled = true;
  std::string name = "";
  std::vector<sf::Drawable*> shapes;
  sf::Shader* shader = nullptr;
  void clear();
  void add(sf::Drawable* shape);
  void update(sf::RenderWindow* w);

private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
  sf::RenderTexture* cache = nullptr;
};

class LayersManager : public sf::Drawable {
private:
  sf::RenderWindow* window;
  Layer* addLayer(std::string name);
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

public:
  LayersManager(sf::RenderWindow* w);
  std::vector<Layer*> layers;

  Layer* getLayer(std::string name);
  void setLayerEnabled(std::string name, bool enabled);
  void setShader(std::string name, sf::Shader* shader);
  void invalidateLayer(std::string name);


};

#endif
