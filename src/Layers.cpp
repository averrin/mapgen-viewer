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
	if (direct) {
	  for (auto shape : shapes) {
		target.draw(*shape);
	  }
	  return;
  }
    sf::Sprite sprite;
    sprite.setTexture(cache->getTexture());
    target.draw(sprite, states);
};

void Layer::update(sf::RenderWindow* window) {
  mg::info("Draw to cache:", name);
  cache->create(window->getSize().x, window->getSize().y, sf::ContextSettings(0, 0, 8));
  cache->clear(sf::Color::Transparent);
  for (auto shape : shapes) {
    cache->draw(*shape);
  }
  if (shader != nullptr) {
    sf::RenderTexture temp;
	temp.setSmooth(true);
    temp.create(window->getSize().x, window->getSize().y, sf::ContextSettings(0, 0, 8));
    temp.clear(sf::Color::Transparent);
    sf::Sprite sprite;
    cache->display();
    sprite.setTexture(cache->getTexture());
    temp.draw(sprite, shader);
    temp.display();
    sprite.setTexture(temp.getTexture());
    cache->draw(sprite);
  }

  if (mask != nullptr) {
    mg::info("Draw masked:", mask->name);
    // shader_mask->setUniform("texture", cache->getTexture());
    shader_mask->setUniform("mask", mask->cache->getTexture());
    sf::RenderTexture temp;
	temp.setSmooth(true);
    temp.create(window->getSize().x, window->getSize().y, sf::ContextSettings(0, 0, 8));
    temp.clear(sf::Color::Transparent);
    sf::Sprite sprite;
    cache->display();
    sprite.setTexture(cache->getTexture());
    temp.draw(sprite, shader_mask);
    temp.display();
    sprite.setTexture(temp.getTexture());
    cache->clear(sf::Color::Transparent);
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

LayersManager::LayersManager(sf::RenderWindow* w, sf::Shader* m) : window(w), shader_mask(m){};

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

void LayersManager::setMask(std::string name, Layer* mask) {
  auto l = getLayer(name);
  l->mask = mask;
  l->shader_mask = shader_mask;
}
