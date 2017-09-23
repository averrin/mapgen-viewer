#include <SFML/Graphics/RenderWindow.hpp>
#include "mapgen/Map.hpp"
#include <functional>

template <typename T> using selectedFunc = std::function<void(T *)>;
template <typename T> using openedFunc = std::function<void(T *)>;
template <typename T> using titleFunc = std::function<std::string(T *)>;

class ObjectsWindow {
public:
  ObjectsWindow(sf::RenderWindow * w, Map* m);
  sf::RenderWindow * window;
  Map *map;

  std::vector<bool> selection_mask;
  std::vector<bool> mega_selection_mask;
  std::vector<bool> rivers_selection_mask;
  std::vector<bool> cities_selection_mask;
  std::vector<bool> location_selection_mask;

  template <typename T>
  void listObjects(std::vector<T *> objects, std::vector<bool> *mask,
                   std::string title, selectedFunc<T> selected,
                   openedFunc<T> opened, titleFunc<T> getTitle);

  void higlightCluster(std::vector<sf::ConvexShape> *objectPolygons,
                       Cluster *cluster);

  void higlightLocation(std::vector<sf::ConvexShape> *objectPolygons,
                        Location *location);

  std::vector<sf::ConvexShape> draw();
};
