#ifndef HSL_COLOR
#define HSL_COLOR

#include <SFML/Graphics/Color.hpp>
#include <algorithm>
#include <cmath>

struct HSL
{
    double Hue;
    double Saturation;
    double Luminance;

    HSL();
    HSL(int H, int S, int L);

    sf::Color TurnToRGB();

    private:

    double HueToRGB(double arg1, double arg2, double H);

};

HSL TurnToHSL(const sf::Color& C);

#endif // HSL_COLOR
