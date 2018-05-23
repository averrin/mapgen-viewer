#include <cmath>
#include "mapgen/utils.hpp"
#include "rang.hpp"

namespace mg {
	double getDistance(Point p, Point p2) {
		double distancex = (p2->x - p->x);
		double distancey = (p2->y - p->y);

		return std::sqrt(distancex * distancex + distancey * distancey);
  }

  template <typename T>
  std::vector<T *> filterObjects(std::vector<T *> regions,
                                      filterFunc<T> filter, sortFunc<T> sort) {
    std::vector<T *> places;

    std::copy_if(regions.begin(), regions.end(), std::back_inserter(places),
                 filter);
    std::sort(places.begin(), places.end(), sort);
    return places;
  }
  void before(std::string method) {
    std::cout << rang::fg::green << rang::style::bold << "[ -> ]\t" << rang::style::reset << method << std::endl << std::flush;

  };
  void after(std::string method) {
    std::cout << rang::fg::red << rang::style::bold << "[ <- ]\t" << rang::style::reset << method << std::endl << std::flush;
  };
  void info(std::string prefix, std::string value) {
    std::cout << rang::style::bold << rang::fg::black << "[info]\t" << rang::style::reset << prefix << " " << rang::fg::blue << value << rang::style::reset << std::endl << std::flush;
  };
  void info(std::string prefix, int value) {
    std::cout << rang::style::bold << rang::fg::black << "[info]\t" << rang::style::reset << prefix << " " << rang::fg::blue << value << rang::style::reset << std::endl << std::flush;
  };

  void info(std::string prefix, City value) {
    std::cout << rang::style::bold << rang::fg::black << "[info]\t" << rang::style::reset << prefix << " " << rang::fg::blue << value << rang::style::reset << std::endl << std::flush;
  };

  void info(std::string prefix, fs::path value) {
    std::cout << rang::style::bold << rang::fg::black << "[info]\t" << rang::style::reset << prefix << " " << rang::fg::blue << value << rang::style::reset << std::endl << std::flush;
  };


  void warn(std::string prefix, std::string value) {
    std::cout << rang::style::bold << rang::fg::yellow << "[warn]\t" << rang::style::reset << prefix << " " << rang::fg::blue << value << rang::style::reset << std::endl << std::flush;
  };
  void warn(std::string prefix, int value) {
    std::cout << rang::style::bold << rang::fg::yellow << "[warn]\t" << rang::style::reset << prefix << " " << rang::fg::blue << value << rang::style::reset << std::endl << std::flush;
  };
  void warn(std::string prefix, City value) {
    std::cout << rang::style::bold << rang::fg::yellow << "[warn]\t" << rang::style::reset << prefix << " " << rang::fg::blue << value << rang::style::reset << std::endl << std::flush;
  };
};
