#ifndef OBJW_CPP_
#define OBJW_CPP_
#include "mapgen/MapGenerator.hpp"
#include "mapgen/Region.hpp"
#include "mapgen/ObjectsWindow.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <imgui.h>

ObjectsWindow::ObjectsWindow(sf::RenderWindow * w, Map* m) : window(w), map(m){}

template <typename T>
void ObjectsWindow::listObjects(std::vector<T *> objects, std::vector<bool> *mask,
                 std::string title, selectedFunc<T> selected,
                 openedFunc<T> opened, titleFunc<T> getTitle) {
  int n = int(objects.size());
  bool maskInited = true;
  if (int(mask->size()) < n) {
    maskInited = false;
    mask->reserve(n);
  }

  char t[100];
  sprintf(t, "%s (%%d)", title.c_str());
  if (ImGui::TreeNode((void *)(intptr_t)n, t, n)) {
    int node_clicked = -1;
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 3);
    int i = 0;
    for (auto obj : objects) {
      if (!maskInited) {
        mask->push_back(false);
      }

      if (mask->at(i)) {
        selected(obj);
      }
      ImGuiTreeNodeFlags node_flags =
          ImGuiTreeNodeFlags_OpenOnArrow |
          ImGuiTreeNodeFlags_OpenOnDoubleClick |
          (mask->at(i) ? ImGuiTreeNodeFlags_Selected : 0);

      bool node_open = ImGui::TreeNodeEx((void *)(intptr_t)i, node_flags,
                                         getTitle(obj).c_str(), i);
      if (ImGui::IsItemClicked()) {
        node_clicked = i;
      }
      if (node_open) {
        opened(obj);
        ImGui::TreePop();
      }
      i++;
    }

    if (node_clicked != -1) {
      mask->at(node_clicked) = !mask->at(node_clicked);
    }
    ImGui::PopStyleVar();
    ImGui::TreePop();
  }
}

void ObjectsWindow::higlightCluster(std::vector<sf::ConvexShape> *objectPolygons,
                     Cluster *cluster) {
  for (auto region : cluster->regions) {
    sf::ConvexShape polygon;
    PointList points = region->getPoints();
    polygon.setPointCount(points.size());
    int n = 0;
    for (PointList::iterator it2 = points.begin(); it2 < points.end();
         it2++, n++) {
      sf::Vector2<double> *p = points[n];
      polygon.setPoint(n, sf::Vector2f(p->x, p->y));
    }
    sf::Color col = sf::Color(255, 70, 100);
    col.a = 150;
    polygon.setFillColor(col);
    polygon.setOutlineColor(col);
    polygon.setOutlineThickness(1);
    objectPolygons->push_back(polygon);
  }
}

void ObjectsWindow::higlightLocation(std::vector<sf::ConvexShape> *objectPolygons,
                      Location *location) {
  auto regions = location->region->neighbors;
  for (auto region : regions) {
    sf::ConvexShape polygon;
    PointList points = region->getPoints();
    polygon.setPointCount(points.size());
    int n = 0;
    for (PointList::iterator it2 = points.begin(); it2 < points.end();
         it2++, n++) {
      sf::Vector2<double> *p = points[n];
      polygon.setPoint(n, sf::Vector2f(p->x, p->y));
    }
    sf::Color col = sf::Color(255, 70, 0);
    col.a = 100;
    polygon.setFillColor(col);
    polygon.setOutlineColor(col);
    polygon.setOutlineThickness(1);
    objectPolygons->push_back(polygon);
  }
}

std::vector<sf::ConvexShape> ObjectsWindow::draw() {
  std::vector<sf::ConvexShape> objectPolygons;

  ImGui::Begin("Objects");
  listObjects<MegaCluster>(
      map->megaClusters, &mega_selection_mask, "MegaClusters",
      (selectedFunc<MegaCluster>)[&](MegaCluster * cluster) {
        higlightCluster(&objectPolygons, cluster);
      },
      (openedFunc<MegaCluster>)[&](MegaCluster * cluster) {
        ImGui::Text("Regions: %zu", cluster->regions.size());
        ImGui::Text("States: %zu", cluster->states.size());
        // ImGui::Text("Clusters: %zu", cluster->clusters.size());
        ImGui::Text("Is land: %s", cluster->isLand ? "true" : "false");
      },
      (titleFunc<MegaCluster>)[&](MegaCluster * cluster) {
        return cluster->name.c_str();
      });

  listObjects<Cluster>(
      map->clusters, &selection_mask, "Clusters",
      (selectedFunc<Cluster>)[&](Cluster * cluster) {
        higlightCluster(&objectPolygons, cluster);
      },
      (openedFunc<Cluster>)[&](Cluster * cluster) {
        if (cluster->megaCluster != nullptr) {
          ImGui::Text("MegaCluster: %s", cluster->megaCluster->name.c_str());
        }
        ImGui::Text("Biom: %s", cluster->biom.name.c_str());
        ImGui::Text("Regions: %zu", cluster->regions.size());
        ImGui::Text("States: %zu", cluster->states.size());
        ImGui::Text("Neighbors: %zu", cluster->neighbors.size());
        ImGui::Text("Is land: %s", cluster->isLand ? "true" : "false");
        if (cluster->isLand) {
          ImGui::Text("Has river: %s", cluster->hasRiver ? "true" : "false");
        }

      },
      (titleFunc<Cluster>)[&](Cluster * cluster) {
        char t[100];
        sprintf(t, "%s [%s] (%%d)", cluster->biom.name.c_str(),
                cluster->name.c_str());

        return std::string(t);
      });

  // TODO: fix river edition
  listObjects<River>(map->rivers, &rivers_selection_mask, "Rivers",
                     (selectedFunc<River>)[&](River * river) {
                       for (auto region : river->regions) {
                         sf::ConvexShape polygon;
                         PointList points = region->getPoints();
                         polygon.setPointCount(points.size());
                         int n = 0;
                         for (PointList::iterator it2 = points.begin();
                              it2 < points.end(); it2++, n++) {
                           sf::Vector2<double> *p = points[n];
                           polygon.setPoint(n, sf::Vector2f(p->x, p->y));
                         }
                         sf::Color col = sf::Color(255, 70, 100);
                         col.a = 150;
                         polygon.setFillColor(col);
                         polygon.setOutlineColor(col);
                         polygon.setOutlineThickness(1);
                         objectPolygons.push_back(polygon);
                       }
                     },
                     (openedFunc<River>)[&](River * river) {
                       ImGui::Text("Name: %s", river->name.c_str());
                       ImGui::Columns(3, "cells");
                       ImGui::Separator();
                       ImGui::Text("x");
                       ImGui::NextColumn();
                       ImGui::Text("y");
                       ImGui::NextColumn();
                       ImGui::Text(" ");
                       ImGui::NextColumn();
                       ImGui::Separator();

                       for (int pi = 0; pi < int(river->points->size()); pi++) {
                         Point p = (*river->points)[pi];
                         ImGui::Text("%f", p->x);
                         ImGui::NextColumn();
                         ImGui::Text("%f", p->y);
                         ImGui::NextColumn();
                         // ImGui::Text("%f", 0.f); ImGui::NextColumn();
                         char bn[30];
                         sprintf(bn, "del %p", p);
                         if (ImGui::Button(bn)) {
                           river->points->erase(river->points->begin() + pi);
                         }
                         ImGui::NextColumn();
                       }
                       ImGui::Columns(1);
                     },
                     (titleFunc<River>)[&](River * river) {
                       auto n = int(river->points->size());
                       char t[100];
                       sprintf(t, "%s [%p]: %d points", river->name.c_str(),
                               river, n);
                       return std::string(t);
                     });

  listObjects<City>(map->cities, &cities_selection_mask, "Cities",
                    (selectedFunc<City>)[&](City * city) {
                      higlightLocation(&objectPolygons, city);
                    },
                    (openedFunc<City>)[&](City * city) {
                      ImGui::Text("Name: %s", city->name.c_str());
                      ImGui::Text("Type: %s", city->typeName.c_str());
                      ImGui::Text("Trade: %d", city->region->traffic);
                    },
                    (titleFunc<City>)[&](City * city) {
                      char t[60];
                      sprintf(t, "%s [%s]", city->name.c_str(),
                              city->typeName.c_str());
                      return std::string(t);
                    });

  listObjects<Location>(
      map->locations, &location_selection_mask, "Locations",
      (selectedFunc<Location>)[&](Location * city) {
        higlightLocation(&objectPolygons, city);
      },
      (openedFunc<Location>)[&](Location * city) {
        ImGui::Text("Name: %s", city->name.c_str());
        ImGui::Text("Type: %s", city->typeName.c_str());
        ImGui::Text("Trade: %d", city->region->traffic);
      },
      (titleFunc<Location>)[&](Location * city) {
        char t[60];
        sprintf(t, "%s [%s]", city->name.c_str(), city->typeName.c_str());
        return std::string(t);
      });

  ImGui::End();

  return objectPolygons;
}

#endif
