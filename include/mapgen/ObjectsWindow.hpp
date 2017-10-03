#include "mapgen/MapGenerator.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <functional>

template <typename T> using selectedFunc = std::function<void(T *)>;
template <typename T> using openedFunc = std::function<void(T *)>;
template <typename T> using titleFunc = std::function<std::string(T *)>;

class ObjectsWindow {
private:
  sf::RenderWindow *window;
  MapGenerator *mapgen;

  template <typename T>
  void listObjects(std::vector<T *> objects, std::vector<bool> *mask,
                   std::string title, selectedFunc<T> selected,
                   openedFunc<T> opened, titleFunc<T> getTitle);

  void higlightCluster(Cluster *cluster);

  void higlightLocation(Location *location);

public:
  ObjectsWindow(sf::RenderWindow *w, MapGenerator *m);

  std::vector<sf::ConvexShape> objectPolygons;
  std::vector<bool> selection_mask;
  std::vector<bool> mega_selection_mask;
  std::vector<bool> rivers_selection_mask;
  std::vector<bool> cities_selection_mask;
  std::vector<bool> location_selection_mask;

  void draw();
};
