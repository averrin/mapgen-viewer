#include <SFML/Graphics/RenderWindow.hpp>
#include "mapgen/MapGenerator.hpp"
#include <imgui.h>

void infoWindow(sf::RenderWindow* window, Region* currentRegion) {
  ImGui::Begin("Region info");

  PointList points = currentRegion->getPoints();
  Cluster* cluster = currentRegion->cluster;
  if (cluster == nullptr) {
    return;
  }

  ImGui::Text("Region: %p", currentRegion);
  ImGui::Text("Minerals: %f", currentRegion->minerals);
  ImGui::Text("Goodness: %f", currentRegion->nice);
  ImGui::Text("Cluster: %p", cluster);
  ImGui::Text("Cluster size: %zu", cluster->regions.size());
  ImGui::Text("Mega Cluster: %s", currentRegion->megaCluster->name.c_str());
  ImGui::Text("Site: x:%f y:%f z:%f",
              currentRegion->site->x,
              currentRegion->site->y,
              currentRegion->getHeight(currentRegion->site)
  );
  ImGui::Text("Humidity: %f", currentRegion->humidity);
  ImGui::Text("Temperature: %f", currentRegion->temperature);
  ImGui::Text("Biom: %s", currentRegion->biom.name.c_str());
  ImGui::Text("Has river: %s", currentRegion->hasRiver ? "true" : "false");
  ImGui::Text("Is border: %s", currentRegion->border ? "true" : "false");

  ImGui::Columns(3, "cells");
  ImGui::Separator();
  ImGui::Text("x"); ImGui::NextColumn();
  ImGui::Text("y"); ImGui::NextColumn();
  ImGui::Text("z"); ImGui::NextColumn();
  ImGui::Separator();

  for (int pi = 0; pi < int(points.size()); pi++) {
    Point p = points[pi];
    ImGui::Text("%f", p->x); ImGui::NextColumn();
    ImGui::Text("%f", p->y); ImGui::NextColumn();
    ImGui::Text("%f", currentRegion->getHeight(p)); ImGui::NextColumn();
  }

  ImGui::Columns(1);
  ImGui::End();
}
