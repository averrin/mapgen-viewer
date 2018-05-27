#include "mapgen/MapGenerator.hpp"
#include "mapgen/Region.hpp"
#include "mapgen/InfoWindow.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <imgui.h>
#include <functional>

InfoWindow::InfoWindow(){
  
}

void InfoWindow::draw(Region currentRegion) {
  ImGui::Begin("Region info");

  PointList points = currentRegion.getPoints();
  auto cluster = currentRegion.cluster;
  if (cluster == nullptr) {
    return;
  }

  if (ImGui::TreeNode("Region")) {
    ImGui::Text("Is Land: %s", currentRegion.megaCluster->isLand ? "true" : "false");
    ImGui::Text("Biom: %s", currentRegion.biom.name.c_str());
    ImGui::Text("Humidity: %f", currentRegion.humidity);
    ImGui::Text("Temperature: %f", currentRegion.temperature);
    ImGui::Text("\n");
    ImGui::Text("Cluster size: %zu", cluster->regions.size());
    ImGui::Text("Mega Cluster: %s", currentRegion.megaCluster->name.c_str());
    ImGui::Text("State Cluster: %p", currentRegion.stateCluster.get());
    if (currentRegion.stateCluster != nullptr) {
      auto sc = currentRegion.stateCluster;
      ImGui::Text("State Cluster regions: %zu", sc->regions.size());
      ImGui::Text("Mega Cluster: %s", sc->megaCluster->name.c_str());
      ImGui::Text("State megaCluster regions: %zu", sc->megaCluster->regions.size());
      ImGui::Text("State 4x regions: %zu", 4*sc->regions.size());
      // sc->regions[0]->megaCluster->regions.size() > 4 * sc->regions.size()
    }
    ImGui::Text("State border: %s", currentRegion.stateBorder ? "true" : "false");
    ImGui::Text("Has river: %s", currentRegion.hasRiver ? "true" : "false");
    ImGui::Text("Is border: %s", currentRegion.border ? "true" : "false");
    ImGui::Text("\n");

    ImGui::Text("Site: x:%f y:%f z:%f", currentRegion.site->x,
                currentRegion.site->y,
                currentRegion.getHeight(currentRegion.site));

    ImGui::Columns(3, "cells");
    ImGui::Separator();
    ImGui::Text("x");
    ImGui::NextColumn();
    ImGui::Text("y");
    ImGui::NextColumn();
    ImGui::Text("z");
    ImGui::NextColumn();
    ImGui::Separator();

    for (int pi = 0; pi < int(points.size()); pi++) {
      Point p = points[pi];
      ImGui::Text("%f", p->x);
      ImGui::NextColumn();
      ImGui::Text("%f", p->y);
      ImGui::NextColumn();
      ImGui::Text("%f", currentRegion.getHeight(p));
      ImGui::NextColumn();
    }

    ImGui::Columns(1);
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Economy")) {
    ImGui::Text("Traffic: %d", currentRegion.traffic);
    ImGui::Text("Minerals: %f", currentRegion.minerals);
    ImGui::Text("Goodness: %f", currentRegion.nice);

    if (currentRegion.city != nullptr) {
      ImGui::Text("Population: %d", currentRegion.city->population);
      ImGui::Text("Wealth: %f", currentRegion.city->wealth);
    }
    if (currentRegion.state != nullptr) {
      ImGui::Text("State: %s", currentRegion.state->name.c_str());
      ImGui::Text("State border: %s", currentRegion.stateBorder ? "true" : "false");
    }
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Location")) {
    if (currentRegion.location != nullptr) {

      ImGui::Text("Name: %s", currentRegion.location->name.c_str());
      ImGui::Text("Type: %s", currentRegion.location->typeName.c_str());

      if (currentRegion.city != nullptr) {
        ImGui::Text("Trade: %d", currentRegion.city->region->traffic);
        ImGui::Text("Population: %d", currentRegion.city->population);
        ImGui::Text("Wealth: %f", currentRegion.city->wealth);
      }
    } else {
      ImGui::Text("No location");
    }

    ImGui::TreePop();
  }

  ImGui::End();
}
