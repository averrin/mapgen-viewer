#include <imgui.h>

#include "mapgen/WeatherManager.hpp"
#include "mapgen/WeatherWindow.hpp"

WeatherWindow::WeatherWindow(sf::RenderWindow *w, MapGenerator* m) : window(w), mapgen(m) {}


//TODO: apply weather change to mapgen
void WeatherWindow::draw(WeatherManager* weather, Painter* painter) {
    if (ImGui::SliderFloat("Wind angle", &weather->windAngle, 0.f, 360.f)) {
        weather->calcHumidity(mapgen->map->regions);
        weather->calcTemp(mapgen->map->regions);
        painter->invalidate(true);
    }
    if (ImGui::SliderFloat("Wind force", &weather->windForce, 0.f, 1.f)) {
        weather->calcHumidity(mapgen->map->regions);
        weather->calcTemp(mapgen->map->regions);
        painter->invalidate(true);
    }

    if (ImGui::Checkbox("Wind", &painter->wind)) {
        painter->invalidate();
    }
    ImGui::SameLine(120);
    if (ImGui::Checkbox("Humidity", &painter->hum)) {
        painter->invalidate();
    }
    ImGui::SameLine(220);
    if (ImGui::Checkbox("Temp", &painter->temp)) {
        painter->invalidate();
    }
}
