#include "mapgen/MapGenerator.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <functional>

template <typename T> using selectedFunc = std::function<void(std::shared_ptr<T>)>;
template <typename T> using openedFunc = std::function<void(std::shared_ptr<T>)>;
template <typename T> using titleFunc = std::function<std::string(std::shared_ptr<T>)>;

class ObjectsWindow {
private:
  std::shared_ptr<MapGenerator> mapgen;

  template <typename T>
  void listObjects(std::vector<std::shared_ptr<T>> objects, std::vector<bool> *mask,
                   std::string title, selectedFunc<T> selected,
                   openedFunc<T> opened, titleFunc<T> getTitle);

  void higlightCluster(std::shared_ptr<Cluster>cluster);

  void higlightLocation(std::shared_ptr<Location>location);

public:
  ObjectsWindow(std::shared_ptr<MapGenerator> m);

  std::vector<sf::ConvexShape> objectPolygons;
  std::vector<bool> selection_mask;
  std::vector<bool> mega_selection_mask;
  std::vector<bool> rivers_selection_mask;
  std::vector<bool> cities_selection_mask;
  std::vector<bool> location_selection_mask;

  void draw();
};
