#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics.hpp>
#include "mapgen/MapGenerator.hpp"
#include <imgui.h>

std::vector<bool> selection_mask;
std::vector<bool> mega_selection_mask;
std::vector<bool> rivers_selection_mask;
std::vector<bool> cities_selection_mask;

void listMegaClusters(sf::RenderWindow* window, MapGenerator* mapgen, std::vector<sf::ConvexShape>* objectPolygons) {
  int n = int(mapgen->map->megaClusters.size());
  bool megaInited = true;
  if(int(mega_selection_mask.size()) < n) {
    megaInited = false;
    mega_selection_mask.reserve(n);
  }

  if(ImGui::TreeNode((void*)(intptr_t)n, "Mega Clusters (%d)", n)){
    int node_clicked = -1;
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize()*3);
    int i = 0;
    for(auto cluster : mapgen->map->megaClusters) {

      if(mega_selection_mask[i]) {
        int ii = 0;
        for(std::vector<Region*>::iterator it=cluster->regions.begin() ; it < cluster->regions.end(); it++, ii++) {

          Region* region = cluster->regions[ii];
          sf::ConvexShape polygon;
          PointList points = region->getPoints();
          polygon.setPointCount(points.size());
          int n = 0;
          for(PointList::iterator it2=points.begin() ; it2 < points.end(); it2++, n++) {
            sf::Vector2<double>* p = points[n];
            polygon.setPoint(n, sf::Vector2f(p->x, p->y));
          }
          sf::Color col = sf::Color(255, 70, 100);
          col.a = 100;
          polygon.setFillColor(col);
          polygon.setOutlineColor(col);
          polygon.setOutlineThickness(1);
          objectPolygons->push_back(polygon);
        }
      }

      if (!megaInited) {
        mega_selection_mask.push_back(false);
      }
      ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (mega_selection_mask[i] ? ImGuiTreeNodeFlags_Selected : 0);

      // char t[100];
      // sprintf(t, "%s [%s] (%%d)",cluster->isLand ? "Land" : "Water", cluster->name.c_str());
      bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, cluster->name.c_str(), int(cluster->regions.size()));
      if (ImGui::IsItemClicked()) {
        node_clicked = i;
      }
      if (node_open)
        {
          ImGui::Text("Regions: %zu", cluster->regions.size());
          // ImGui::Text("Clusters: %zu", cluster->clusters.size());
          ImGui::Text("Is land: %s", cluster->isLand ? "true" : "false");
          ImGui::TreePop();
        }
      i++;
    }

    if (node_clicked != -1)
      {
        mega_selection_mask[node_clicked] = !mega_selection_mask[node_clicked];
      }
    ImGui::PopStyleVar();
    ImGui::TreePop();
  }
}

void listClusters(sf::RenderWindow* window, MapGenerator* mapgen, std::vector<sf::ConvexShape>* objectPolygons) {
  int n = int(mapgen->map->clusters.size());
  bool maskInited = true;
  if(int(selection_mask.size()) < n) {
    maskInited = false;
    selection_mask.reserve(n);
  }

  if(ImGui::TreeNode((void*)(intptr_t)n, "Clusters (%d)", n)){
    int node_clicked = -1;
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize()*3);
    int i = 0;
    for(auto cluster : mapgen->map->clusters) {

      if(selection_mask[i]) {
        int ii = 0;
        for(std::vector<Region*>::iterator it=cluster->regions.begin() ; it < cluster->regions.end(); it++, ii++) {

          Region* region = cluster->regions[ii];
          sf::ConvexShape polygon;
          PointList points = region->getPoints();
          polygon.setPointCount(points.size());
          int n = 0;
          for(PointList::iterator it2=points.begin() ; it2 < points.end(); it2++, n++) {
            sf::Vector2<double>* p = points[n];
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

      if (!maskInited) {
        selection_mask.push_back(false);
      }
      ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (selection_mask[i] ? ImGuiTreeNodeFlags_Selected : 0);

      char t[100];
      sprintf(t, "%s [%s] (%%d)", cluster->biom.name.c_str(),
              cluster->name.c_str());
      bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, t, int(cluster->regions.size()));
      if (ImGui::IsItemClicked()) {
        node_clicked = i;
      }
      if (node_open)
        {
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
          ImGui::TreePop();
        }
      i++;
    }

    if (node_clicked != -1)
      {
          selection_mask[node_clicked] = !selection_mask[node_clicked];
      }
    ImGui::PopStyleVar();
    ImGui::TreePop();
  }
  
}

void listRivers(sf::RenderWindow* window, MapGenerator* mapgen, std::vector<sf::ConvexShape>* objectPolygons) {
  int rc = int(mapgen->map->rivers.size());
  bool riversMaskInited = true;
  if(int(rivers_selection_mask.size()) < rc) {
    riversMaskInited = false;
    rivers_selection_mask.reserve(rc);
  }
  int in = 0;
  if(ImGui::TreeNode((void*)(intptr_t)rc, "Rivers (%d)", rc)){
    int node_clicked = -1;
    for(auto river : mapgen->map->rivers) {
      if (!riversMaskInited) {
        rivers_selection_mask.push_back(false);
      }

      auto n = int(river->points->size());
      char rn[30];
      sprintf(rn,"%s [%p]: %%d points", river->name.c_str(), river);
      ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (rivers_selection_mask[in] ? ImGuiTreeNodeFlags_Selected : 0);
      bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)n, node_flags, rn, n);
      if (ImGui::IsItemClicked()) {
        node_clicked = in;
      }

      if(node_open){
        ImGui::Text("Name: %s", river->name.c_str());
        ImGui::Columns(3, "cells");
        ImGui::Separator();
        ImGui::Text("x"); ImGui::NextColumn();
        ImGui::Text("y"); ImGui::NextColumn();
        ImGui::Text(" "); ImGui::NextColumn();
        ImGui::Separator();

        for (int pi = 0; pi < int(river->points->size()); pi++) {
          Point p = (*river->points)[pi];
          ImGui::Text("%f", p->x); ImGui::NextColumn();
          ImGui::Text("%f", p->y); ImGui::NextColumn();
          // ImGui::Text("%f", 0.f); ImGui::NextColumn();
          char bn[30];
          sprintf(bn,"del %p", p);
          if(ImGui::Button(bn)) {
            river->points->erase(river->points->begin() + pi);
          }
          ImGui::NextColumn();
        }
        ImGui::Columns(1);
        ImGui::TreePop();
      }

      in++;
    }

    if (node_clicked != -1)
      {
        rivers_selection_mask[node_clicked] = !rivers_selection_mask[node_clicked];
      }

    ImGui::TreePop();
  }

}


void listCities(sf::RenderWindow* window, MapGenerator* mapgen, std::vector<sf::ConvexShape>* objectPolygons) {
  int rc = int(mapgen->map->cities.size());
  bool maskInited = true;
  if(int(cities_selection_mask.size()) < rc) {
    maskInited = false;
    cities_selection_mask.reserve(rc);
  }
  int in = 0;
  if(ImGui::TreeNode((void*)(intptr_t)rc, "Cities (%d)", rc)){
    int node_clicked = -1;
    for(auto city : mapgen->map->cities) {
      auto regions = city->region->neighbors;
      if(cities_selection_mask[in]) {
        int ii = 0;
        for(std::vector<Region*>::iterator it=regions.begin() ; it < regions.end(); it++, ii++) {

          Region* region = regions[ii];
          sf::ConvexShape polygon;
          PointList points = region->getPoints();
          polygon.setPointCount(points.size());
          int n = 0;
          for(PointList::iterator it2=points.begin() ; it2 < points.end(); it2++, n++) {
            sf::Vector2<double>* p = points[n];
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

      if (!maskInited) {
        cities_selection_mask.push_back(false);
      }

      char rn[60];
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
      }

      sprintf(rn,"%s [%s]", city->name.c_str(), type.c_str());
      ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (cities_selection_mask[in] ? ImGuiTreeNodeFlags_Selected : 0);
      bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)in, node_flags, rn, in);
      if (ImGui::IsItemClicked()) {
        node_clicked = in;
      }

      if(node_open){
        ImGui::Text("Name: %s", city->name.c_str());
        ImGui::Text("Type: %s", type.c_str());
        ImGui::TreePop();
      }

      in++;
    }

    if (node_clicked != -1)
      {
        cities_selection_mask[node_clicked] = !cities_selection_mask[node_clicked];
      }

    ImGui::TreePop();
  }

}

std::vector<sf::ConvexShape> objectsWindow(sf::RenderWindow* window, MapGenerator* mapgen) {
  std::vector<sf::ConvexShape> objectPolygons;

  ImGui::Begin("Objects");
  listMegaClusters(window, mapgen, &objectPolygons);
  listClusters(window, mapgen, &objectPolygons);
  listRivers(window, mapgen, &objectPolygons);
  listCities(window, mapgen, &objectPolygons);
  ImGui::End();

  return objectPolygons;
}
