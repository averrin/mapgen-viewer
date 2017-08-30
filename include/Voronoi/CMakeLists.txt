cmake_minimum_required(VERSION 2.8)

project(voronoi)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

find_package(SFML 2.4 COMPONENTS graphics system window)

file(GLOB LIB_SOURCE "src/*.cpp" "include/*.h")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SFML_INCLUDE_DIR} include)

SET(CMAKE_DEBUG_POSTFIX -d)

add_library(voronoi ${LIB_SOURCE})

add_executable(sfvoronoi_example examples/SFML_Example.cpp)

target_link_libraries(sfvoronoi_example debug voronoi-d optimized voronoi ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
