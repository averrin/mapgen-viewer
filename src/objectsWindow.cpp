#include "mapgen/MapGenerator.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <imgui.h>

std::vector<bool> selection_mask;
std::vector<bool> mega_selection_mask;
std::vector<bool> rivers_selection_mask;
std::vector<bool> cities_selection_mask;

template <typename T> using selectedFunc = std::function<void(T *)>;
template <typename T> using openedFunc = std::function<void(T *)>;
template <typename T> using titleFunc = std::function<std::string(T *)>;

template <typename T>
void listObjects(std::vector<T *> objects, std::vector<bool> *mask,
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

std::vector<sf::ConvexShape> objectsWindow(sf::RenderWindow *window,
                                           MapGenerator *mapgen) {
  std::vector<sf::ConvexShape> objectPolygons;

  ImGui::Begin("Objects");
  listObjects<MegaCluster>(
      mapgen->map->megaClusters, &mega_selection_mask, "MegaClusters",
      (selectedFunc<MegaCluster>)[&](MegaCluster * cluster) {
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
          objectPolygons.push_back(polygon);
        }
      },
      (openedFunc<MegaCluster>)[&](MegaCluster * cluster) {
        ImGui::Text("Regions: %zu", cluster->regions.size());
        // ImGui::Text("Clusters: %zu", cluster->clusters.size());
        ImGui::Text("Is land: %s", cluster->isLand ? "true" : "false");
      },
      (titleFunc<MegaCluster>)[&](MegaCluster * cluster) {
        return cluster->name.c_str();
      });

  listObjects<Cluster>(
      mapgen->map->clusters, &selection_mask, "Clusters",
      (selectedFunc<Cluster>)[&](Cluster * cluster) {
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
          sf::Color col = sf::Color(255, 70, 0);
          col.a = 100;
          polygon.setFillColor(col);
          polygon.setOutlineColor(col);
          polygon.setOutlineThickness(1);
          objectPolygons.push_back(polygon);
        }
      },
      (openedFunc<Cluster>)[&](Cluster * cluster) {
        if (cluster->megaCluster != nullptr) {
          ImGui::Text("MegaCluster: %s", cluster->megaCluster->name.c_str());
        }
        ImGui::Text("Biom: %s", cluster->biom.name.c_str());
        ImGui::Text("Regions: %zu", cluster->regions.size());
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


  //TODO: fix river edition
  listObjects<River>(
      mapgen->map->rivers, &rivers_selection_mask, "Rivers",
      (selectedFunc<River>)[&](River * river) {
        for (auto region : river->regions) {
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
        sprintf(t, "%s [%p]: %d points", river->name.c_str(), river, n);
        return std::string(t);
      });


  listObjects<City>(
      mapgen->map->cities, &cities_selection_mask, "Cities",
      (selectedFunc<City>)[&](City * city) {
        auto regions = city->region->neighbors;
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
          objectPolygons.push_back(polygon);
        }
      },
      (openedFunc<City>)[&](City * city) {
        //TODO: dry
        std::string type = "";
        switch (city->type) {
        case CAPITAL:
          type = "Capital";
          break;
        case PORT:
          type = "Port";
          break;
        case MINE:
          type = "Mine";
          break;
        case AGRO:
          type = "Agro";
          break;
        case TRADE:
          type = "Trade post";
          break;
        case LIGHTHOUSE:
          type = "Lighthouse";
          break;
        }
        ImGui::Text("Name: %s", city->name.c_str());
        ImGui::Text("Type: %s", type.c_str());
        ImGui::Text("Trade: %d", city->region->traffic);
      },
      (titleFunc<City>)[&](City * city) {
        char t[60];
        std::string type = "";
        switch (city->type) {
        case CAPITAL:
          type = "Capital";
          break;
        case PORT:
          type = "Port";
          break;
        case MINE:
          type = "Mine";
          break;
        case AGRO:
          type = "Agro";
          break;
        case TRADE:
          type = "Trade post";
          break;
        case LIGHTHOUSE:
          type = "Lighthouse";
          break;
        }

        sprintf(t, "%s [%s]", city->name.c_str(), type.c_str());
        return std::string(t);
      });

  ImGui::End();

  return objectPolygons;
}
