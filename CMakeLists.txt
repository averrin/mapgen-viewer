cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()
# (you can also set it on the command line: -D CMAKE_BUILD_TYPE=Release)

project(mapgen)
set (mapgen_VERSION_MAJOR 0)
set (mapgen_VERSION_MINOR 1)

file(GLOB SOURCE "src/*.cpp" "include/mapgen/*.h")

configure_file (
  "${PROJECT_SOURCE_DIR}/MapgenConfig.h.in"
  "${PROJECT_BINARY_DIR}/MapgenConfig.h"
  )

include_directories(
  ${PROJECT_BINARY_DIR}
  "${PROJECT_BINARY_DIR}/include"
  "${PROJECT_BINARY_DIR}/include/imgui"
  "${PROJECT_BINARY_DIR}/include/Voronoi/include")

file(GLOB VLIB_SOURCE "${PROJECT_BINARY_DIR}/include/Voronoi/src/*.cpp" "${PROJECT_BINARY_DIR}/include/Voronoi/include/*.h")
add_library(voronoi ${VLIB_SOURCE})
file(GLOB IMLIB_SOURCE "${PROJECT_BINARY_DIR}/include/imgui/*.cpp" "${PROJECT_BINARY_DIR}/include/imgui/*.h")
add_library(imgui ${IMLIB_SOURCE})

file(GLOB SWLIB_SOURCE "${PROJECT_BINARY_DIR}/include/SelbaWard/SelbaWard/*.cpp" "${PROJECT_BINARY_DIR}/include/SelbaWard/SelbaWard/*.hpp" "${PROJECT_BINARY_DIR}/include/SelbaWard/*.hpp")
add_library(sw ${SWLIB_SOURCE})

set(EXECUTABLE_NAME "mapgen")
add_executable(${EXECUTABLE_NAME}
  include/imgui/imgui.cpp
  include/imgui/imgui_draw.cpp
  include/imgui/imgui-SFML.cpp
  src/main.cpp
  src/noiseutils.cpp
  src/MapGenerator.cpp
  src/Region.cpp
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reorder")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-but-set-variable")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_definitions("-Wall")
add_definitions("-Werror")

# Detect and add SFML
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
#Find any version 2.X of SFML
#See the FindSFML.cmake file for additional details and instructions
find_package(SFML 2 REQUIRED system window graphics network audio)
if(SFML_FOUND)
  include_directories(${SFML_INCLUDE_DIR})
  target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES})
endif()

# OpenGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
if (OPENGL_FOUND)
  target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES})
endif()

target_link_libraries(${EXECUTABLE_NAME} voronoi noise imgui sw Threads::Threads)

target_compile_features(mapgen PRIVATE cxx_delegating_constructors)

# Install target
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)
