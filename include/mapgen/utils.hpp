#ifndef UTILS_HPP_
#define UTILS_HPP_
#include <SFML/Graphics.hpp>
#include <functional>

template <typename T> using filterFunc = std::function<bool(T *)>;
template <typename T> using sortFunc = std::function<bool(T *, T *)>;

typedef sf::Vector2<double>* Point;
namespace mg {
  double getDistance(Point p, Point p2);

  template <typename T>
  std::vector<T *> filterObjects(std::vector<T *> regions,
                                 filterFunc<T> filter, sortFunc<T> sort);
};
#endif
