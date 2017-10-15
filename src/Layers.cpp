#include <SFML/Graphics.hpp>
#include "mapgen/Layers.hpp"
#include "mapgen/utils.hpp"

Layer::Layer(std::string n) : name(n) {
  cache = new sf::RenderTexture();
  cache->setSmooth(true);
};
void Layer::draw(sf::RenderTarget& target, sf::RenderStates states) const {
  if (!enabled) {
    return;
  }
    sf::Sprite sprite;
    sprite.setTexture(cache->getTexture());
    target.draw(sprite, states);
};

void Layer::update(sf::RenderWindow* window) {
  mg::info("Draw to cache:", name);
  cache->create(window->getSize().x, window->getSize().y);
  cache->clear(sf::Color::Transparent);
  for (auto shape : shapes) {
    cache->draw(*shape);
  }
  if (shader != nullptr) {
    sf::RenderTexture temp;
    temp.create(window->getSize().x, window->getSize().y);
    sf::Sprite sprite;
    cache->display();
    sprite.setTexture(cache->getTexture());
    temp.draw(sprite, shader);
    temp.display();
    sprite.setTexture(temp.getTexture());
    cache->draw(sprite);
  }
  cache->display();
}

void Layer::clear() {
  shapes.clear();
}

void Layer::add(sf::Drawable* shape) {
  shapes.push_back(shape);
}

LayersManager::LayersManager(sf::RenderWindow* w) : window(w){};

void LayersManager::draw(sf::RenderTarget& target, sf::RenderStates states) const {
  for (auto l: layers) {
    target.draw(*l, states);
  }
}

Layer* LayersManager::addLayer(std::string name) {
  auto l = new Layer(name);
  layers.push_back(l);

  return l;
}

Layer* LayersManager::getLayer(std::string name) {
  auto l = std::find_if(layers.begin(), layers.end(), [&](Layer* l) {
      return l->name == name;
    });
  if (l == layers.end()) {
    return addLayer(name);
  }
  return *l;
}

void LayersManager::setLayerEnabled(std::string name, bool enabled) {
  auto l = getLayer(name);
  l->enabled = enabled;
}

void LayersManager::invalidateLayer(std::string name) {
  auto l = getLayer(name);
  l->update(window);
}

void LayersManager::setShader(std::string name, sf::Shader* shader) {
  auto l = getLayer(name);
  l->shader = shader;
}
