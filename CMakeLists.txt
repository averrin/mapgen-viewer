#Change this if you need to target a specific CMake version
cmake_minimum_required(VERSION 2.6)

# Enable debug symbols by default
# must be done before project() statement
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()
# (you can also set it on the command line: -D CMAKE_BUILD_TYPE=Release)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reorder")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable")
 
file(GLOB VLIB_SOURCE "Voronoi/src/*.cpp" "Voronoi/include/*.h")
# Enable warnings
IF (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)
  # use -Wall for gcc
  IF(NOT CMAKE_CXX_FLAGS MATCHES "-Wall$")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  ENDIF(NOT CMAKE_CXX_FLAGS MATCHES "-Wall$")

  IF(NOT CMAKE_C_FLAGS MATCHES "-Wall$")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
  ENDIF(NOT CMAKE_C_FLAGS MATCHES "-Wall$")
ENDIF (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUC)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-D_CRT_SECURE_NO_WARNINGS
)

project(mapgen)

add_definitions("-Wall")
add_definitions("-Werror")

# Define sources and executable
set(EXECUTABLE_NAME "mapgen")
add_executable(${EXECUTABLE_NAME} 
  imgui/imgui.cpp
  imgui/imgui-SFML.cpp
  imgui/imgui_draw.cpp
  main.cpp
  noiseutils.cpp
)

# Without this, #include <imgui> won't work.
include_directories("imgui" "Voronoi/include")

add_library(voronoi ${VLIB_SOURCE})


# Detect and add SFML
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

target_link_libraries(${EXECUTABLE_NAME} voronoi noise)

target_compile_features(mapgen PRIVATE cxx_delegating_constructors)

# Install target
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)
