#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics.hpp>
#include "mapgen/MapGenerator.hpp"
#include <imgui.h>

std::vector<bool> selection_mask; // Dumb representation of what may be user-side selection state. You may carry selection state inside or outside your objects in whatever format you see fit.

std::vector<sf::ConvexShape> objectsWindow(sf::RenderWindow* window, MapGenerator* mapgen) {
  std::vector<sf::ConvexShape> objectPolygons;
  ImGui::Begin("Objects");
  int n = int(mapgen->clusters.size());
  bool maskInited = true;
  if(int(selection_mask.size()) < n) {
    maskInited = false;
    selection_mask.reserve(n);
  }
  if(ImGui::TreeNode((void*)(intptr_t)n, "Clusters (%d)", n)){
    int node_clicked = -1;                // Temporary storage of what node we have clicked to process selection at the end of the loop. May be a pointer to your own node type, etc.
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize()*3); // Increase spacing to differentiate leaves from expanded contents.
    int i = 0;
    for(auto cluster : mapgen->clusters) {

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
          sf::Color col = sf::Color(200, 50, 200);
          col.a = 50;
          polygon.setFillColor(col);
          polygon.setOutlineColor(col);
          polygon.setOutlineThickness(1);
          objectPolygons.push_back(polygon);
        }
      }

      if (!maskInited) {
        selection_mask.push_back(false);
      }
      ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | (selection_mask[i] ? ImGuiTreeNodeFlags_Selected : 0);

      char t[100];
      sprintf(t, "%s (%%d)", cluster->name.c_str());
      bool node_open = ImGui::TreeNodeEx((void*)(intptr_t)i, node_flags, t, int(cluster->regions.size()));
      if (ImGui::IsItemClicked()) {
        node_clicked = i;
      }
      if (node_open)
        {
          ImGui::Text("Biom: %s", cluster->biom.name.c_str());
          ImGui::Text("Regions: %zu", cluster->regions.size());
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
  int rc = int(mapgen->rivers.size());
  if(ImGui::TreeNode((void*)(intptr_t)rc, "Rivers (%d)", rc)){
    for(auto river : mapgen->rivers) {
      auto n = int(river->size());
      if(ImGui::TreeNode((void*)(intptr_t)n, "River: %d points", n)){
        ImGui::Columns(3, "cells");
        ImGui::Separator();
        ImGui::Text("x"); ImGui::NextColumn();
        ImGui::Text("y"); ImGui::NextColumn();
        ImGui::Text("z"); ImGui::NextColumn();
        ImGui::Separator();

        for (int pi = 0; pi < int(river->size()); pi++) {
          Point p = (*river)[pi];
          ImGui::Text("%f", p->x); ImGui::NextColumn();
          ImGui::Text("%f", p->y); ImGui::NextColumn();
          ImGui::Text("%f", 0.f); ImGui::NextColumn();
        }

        ImGui::TreePop();
      }
    }

    ImGui::TreePop();
  }
  ImGui::End();
  return objectPolygons;
}
