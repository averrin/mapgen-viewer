cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()
# (you can also set it on the command line: -D CMAKE_BUILD_TYPE=Release)

project(mapgen)
set (mapgen_VERSION_MAJOR 0)
set (mapgen_VERSION_MINOR 7)
set (mapgen_VERSION_PATCH 1)

set(PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

set(LIBMAPGEN_PATH "${PROJECT_PATH}/include/libmapgen/include")

file(GLOB SOURCE "src/*.cpp" "include/mapgen/*.h")
file(COPY "font.ttf" DESTINATION "${PROJECT_PATH}/bin")
file(COPY "images" DESTINATION "${PROJECT_PATH}/bin")
file(COPY "src/blur.frag" DESTINATION "${PROJECT_PATH}/bin")

configure_file (
  "${PROJECT_SOURCE_DIR}/MapgenConfig.h.in"
  "${PROJECT_PATH}/MapgenConfig.h"
  )

include_directories(
  ${PROJECT_PATH}
  ${LIBMAPGEN_PATH}
  "${PROJECT_PATH}/include"
  "${PROJECT_PATH}/include/imgui"
  "${PROJECT_PATH}/include/imgui-sfml"
  
  "${LIBMAPGEN_PATH}/Voronoi/include")

file(GLOB LIBMAPGEN_SOURCE "${LIBMAPGEN_PATH}/../src/*.cpp" "${LIBMAPGEN_PATH}/mapgen/include/*.hpp")
list(REMOVE_ITEM LIBMAPGEN_SOURCE "${LIBMAPGEN_PATH}/../src/GeneratorFacade.cpp")
add_library(libmapgen ${LIBMAPGEN_SOURCE})

file(GLOB VLIB_SOURCE "${LIBMAPGEN_PATH}/Voronoi/src/*.cpp" "${LIBMAPGEN_PATH}/Voronoi/include/*.h")
add_library(voronoi ${VLIB_SOURCE})

file(GLOB IMLIB_SOURCE "${PROJECT_PATH}/include/imgui/*.cpp" "${PROJECT_PATH}/include/imgui/*.h")
add_library(imgui ${IMLIB_SOURCE})
file(GLOB IMLIB_SFML_SOURCE "${PROJECT_PATH}/include/imgui-sfml/*.cpp" "${PROJECT_PATH}/include/imgui-sfml/*.h")
add_library(imgui-sfml ${IMLIB_SFML_SOURCE})

file(GLOB SWLIB_SOURCE "${PROJECT_PATH}/include/SelbaWard/SelbaWard/*.cpp" "${PROJECT_PATH}/include/SelbaWard/SelbaWard/*.hpp" "${PROJECT_PATH}/include/SelbaWard/*.hpp")
add_library(sw ${SWLIB_SOURCE})

add_subdirectory(backward-cpp)

set(EXECUTABLE_NAME "mapgen")
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/*.cpp)
add_executable(${EXECUTABLE_NAME}
  ${IMLIB_SOURCE}
  ${IMLIB_SFML_SOURCE}
  ${BACKWARD_ENABLE}

  ${LIBMAPGEN_PATH}/noiseutils.cpp
  ${LIBMAPGEN_PATH}/micropather.cpp

  src/Walker.cpp
  src/Painter.cpp
  src/objectsWindow.cpp
  src/infoWindow.cpp
  src/simulationWindow.cpp
  # src/logger.cpp
  src/application.cpp

  src/main.cpp
)

add_backward(mapgen)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

IF(NOT WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reorder")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-but-set-variable")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-function")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-narrowing")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-sign-compare")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-class-memaccess")
	set(CMAKE_CXX_STANDARD 14)
	set(CMAKE_CXX_STANDARD_REQUIRED ON)
	set(CMAKE_CXX_EXTENSIONS OFF)

	add_definitions("-Wall")
	add_definitions("-Werror")
else()
	SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /SAFESEH:NO")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY bin)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_PATH}/bin)


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

IF(NOT WIN32)
	target_link_libraries(${EXECUTABLE_NAME} libmapgen voronoi noise imgui imgui-sfml sw Threads::Threads stdc++fs)
else()
	target_link_libraries(${EXECUTABLE_NAME} libmapgen voronoi "${PROJECT_PATH}/include/libnoise.lib" imgui sw Threads::Threads)
endif()

target_compile_features(mapgen PRIVATE cxx_delegating_constructors)

# Install target
install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)
