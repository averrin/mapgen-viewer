#include "mapgen/SimulationWindow.hpp"
#include <imgui.h>

SimulationWindow::SimulationWindow(std::shared_ptr<MapGenerator> m)
    : mapgen(m) {}

void SimulationWindow::draw() {

  ImGui::Text("Total cities count: %zu", mapgen->map->cities.size());
  ImGui::Text("Ports count: %zu",
              std::count_if(mapgen->map->cities.begin(),
                            mapgen->map->cities.end(),
                            [](std::shared_ptr<City>c) { return c->type == PORT; }));
  ImGui::Text("Mines count: %zu",
              std::count_if(mapgen->map->cities.begin(),
                            mapgen->map->cities.end(),
                            [](std::shared_ptr<City>c) { return c->type == MINE; }));
  ImGui::Text("Agro count: %zu",
              std::count_if(mapgen->map->cities.begin(),
                            mapgen->map->cities.end(),
                            [](std::shared_ptr<City>c) { return c->type == AGRO; }));
  ImGui::Text("Forts count: %zu",
              std::count_if(mapgen->map->cities.begin(),
                            mapgen->map->cities.end(),
                            [](std::shared_ptr<City>c) { return c->type == FORT; }));
  ImGui::Text("\n");

  if (ImGui::TreeNode("Economy variables")) {
    ImGui::Columns(2, "vars");

    ImGui::Text("POPULATION_GROWS");
    ImGui::NextColumn();
    ImGui::DragFloat("##POPULATION_GROWS",
                     &mapgen->simulator->vars->POPULATION_GROWS, 0.01, 0.01,
                     0.5);
    ImGui::NextColumn();

    ImGui::Text("POPULATION_GROWS_WEALTH_MODIFIER");
    ImGui::NextColumn();
    ImGui::DragFloat("##POPULATION_GROWS_WEALTH_MODIFIER",
                     &mapgen->simulator->vars->POPULATION_GROWS_WEALTH_MODIFIER,
                     0.01, 0.01, 1.f);
    ImGui::NextColumn();

    ImGui::Text("PACKAGES_PER_NICE");
    ImGui::NextColumn();
    ImGui::DragFloat("##PACKAGES_PER_NICE",
                     &mapgen->simulator->vars->PACKAGES_PER_NICE, 1.f, 1.f,
                     1000.f);
    ImGui::NextColumn();

    ImGui::Text("PACKAGES_AGRO_POPULATION_MODIFIER");
    ImGui::NextColumn();
    ImGui::DragFloat(
        "##PACKAGES_AGRO_POPULATION_MODIFIER",
        &mapgen->simulator->vars->PACKAGES_AGRO_POPULATION_MODIFIER, 0.01, 0.01,
        1.f);
    ImGui::NextColumn();

    ImGui::Text("PACKAGES_PER_MINERALS");
    ImGui::NextColumn();
    ImGui::DragFloat("##PACKAGES_PER_MINERALS",
                     &mapgen->simulator->vars->PACKAGES_PER_MINERALS, 1.f, 1.f,
                     1000.f);
    ImGui::NextColumn();

    ImGui::Text("PACKAGES_MINERALS_POPULATION_MODIFIER");
    ImGui::NextColumn();
    ImGui::DragFloat(
        "##PACKAGES_MINERALS_POPULATION_MODIFIER",
        &mapgen->simulator->vars->PACKAGES_MINERALS_POPULATION_MODIFIER, 0.01,
        0.01, 1.f);
    ImGui::NextColumn();

    ImGui::Text("CONSUME_AGRO_POPULATION_MODIFIER");
    ImGui::NextColumn();
    ImGui::DragFloat("##CONSUME_AGRO_POPULATION_MODIFIER",
                     &mapgen->simulator->vars->CONSUME_AGRO_POPULATION_MODIFIER,
                     0.01, 0.01, 1.f);
    ImGui::NextColumn();

    ImGui::Text("CONSUME_MINERALS_POPULATION_MODIFIER");
    ImGui::NextColumn();
    ImGui::DragFloat(
        "##CONSUME_MINERALS_POPULATION_MODIFIER",
        &mapgen->simulator->vars->CONSUME_MINERALS_POPULATION_MODIFIER, 0.01,
        0.01, 1.f);
    ImGui::NextColumn();

    ImGui::Text("CONSUME_AGRO_WEALTH_MODIFIER");
    ImGui::NextColumn();
    ImGui::DragFloat("##CONSUME_AGRO_WEALTH_MODIFIER",
                     &mapgen->simulator->vars->CONSUME_AGRO_WEALTH_MODIFIER,
                     0.01, 0.01, 1.f);
    ImGui::NextColumn();

    ImGui::Text("CONSUME_MINERALS_WEALTH_MODIFIER");
    ImGui::NextColumn();
    ImGui::DragFloat("##CONSUME_MINERALS_WEALTH_MODIFIER",
                     &mapgen->simulator->vars->CONSUME_MINERALS_WEALTH_MODIFIER,
                     0.01, 0.01, 1.f);
    ImGui::NextColumn();

    ImGui::Text("CANT_BUY_AGRO");
    ImGui::NextColumn();
    ImGui::DragFloat("##CANT_BUY_AGRO", &mapgen->simulator->vars->CANT_BUY_AGRO,
                     0.01, 0.01, 1.f);
    ImGui::NextColumn();

    ImGui::Text("CANT_BUY_MINERALS");
    ImGui::NextColumn();
    ImGui::DragFloat("##CANT_BUY_MINERALS",
                     &mapgen->simulator->vars->CANT_BUY_MINERALS, 0.01, 0.01,
                     1.f);
    ImGui::NextColumn();

    ImGui::Text("MINERALS_POPULATION_PRODUCE");
    ImGui::NextColumn();
    ImGui::DragFloat("##MINERALS_POPULATION_PRODUCE",
                     &mapgen->simulator->vars->MINERALS_POPULATION_PRODUCE,
                     0.01, 0.01, 1.f);
    ImGui::NextColumn();

    ImGui::Text("AGRO_POPULATION_PRODUCE");
    ImGui::NextColumn();
    ImGui::DragFloat("##AGRO_POPULATION_PRODUCE",
                     &mapgen->simulator->vars->AGRO_POPULATION_PRODUCE, 0.01,
                     0.01, 1.f);
    ImGui::NextColumn();

    ImGui::Text("PORT_FEE");
    ImGui::NextColumn();
    ImGui::DragFloat("##PORT_FEE", &mapgen->simulator->vars->PORT_FEE, 0.01,
                     0.01, 1.f);
    ImGui::NextColumn();

    ImGui::Text("PRICE_CORRECTION");
    ImGui::NextColumn();
    ImGui::DragFloat("##PRICE_CORRECTION", &mapgen->simulator->vars->PRICE_CORRECTION, 0.01,
                     0.01, 1.f);
    ImGui::NextColumn();

    ImGui::Columns(1);
    ImGui::TreePop();
  }
  ImGui::Text("\n");

  if (mapgen->simulator->report != nullptr) {
    ImGui::Text("Last simulation report");
    auto report = mapgen->simulator->report;
    auto pop = &report->population;
    if (pop->size() > 0) {
      char t[1000];
      sprintf(t, "Total: %f\nAvg: %f\nMax: %d\nMin: %d", pop->back(),
              pop->back() / mapgen->map->cities.size(), report->maxPopulation,
              report->minPopulation);
      ImGui::PlotLines(t,
                       [](void *data, int idx) {
                         return ((std::vector<float> *)data)->at(idx) / 1000.f;
                       },
                       (void *)pop, pop->size(), 0.f, "Total Population (k)",
                       pop->front() / 1000.f, pop->back() / 1000.f,
                       ImVec2(0, 100));
    }

    auto w = &mapgen->simulator->report->wealth;
    if (pop->size() > 0) {
      char tw[1000];
      sprintf(tw, "Total: %f\nAvg: %f\nMax: %f\nMin: %f", w->back(), w->back() / mapgen->map->cities.size(), report->maxWealth, report->minWealth);
      ImGui::PlotLines(tw,
                       [](void *data, int idx) {
                         return ((std::vector<float> *)data)->at(idx);
                       },
                       (void *)w, pop->size(), 0.f, "Total Wealth", w->front(),
                       w->back(), ImVec2(0, 100));
    }
    ImGui::Text("\n");
  }
}
