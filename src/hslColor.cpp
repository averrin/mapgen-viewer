#include "mapgen/hslColor.hpp"
const double D_EPSILON = 0.00000000000001; 
///Feel free to move this to your constants .h file or use it as a static constant if you don't like it here.

HSL::HSL() :Hue(0) ,Saturation(0) ,Luminance(0) {}

HSL::HSL(int H, int S, int L)
{
    ///Range control for Hue.
    if (H <= 360 && H >= 0) {Hue = H;}
    else
    {
    if(H > 360) { Hue = H%360; }
    else if(H < 0 && H > -360) { Hue = -H; }
    else if(H < 0 && H < -360) { Hue = -(H%360); }
    }

    ///Range control for Saturation.
    if (S <= 100 && S >= 0) {Saturation = S;}
    else
    {
    if(S > 100) { Saturation = S%100;}
    else if(S < 0 && S > -100) { Saturation = -S; }
    else if(S < 0 && S < -100) { Saturation = -(S%100); }
    }

    ///Range control for Luminance
    if (L <= 100 && L >= 0) {Luminance = L;}
    else
    {
    if(L > 100) { Luminance = L%100;}
    if(L < 0 && L > -100) { Luminance = -L; }
    if(L < 0 && L < -100) { Luminance = -(L%100); }
    }

}

double HSL::HueToRGB(double arg1, double arg2, double H)
{
   if ( H < 0 ) H += 1;
   if ( H > 1 ) H -= 1;
   if ( ( 6 * H ) < 1 ) { return (arg1 + ( arg2 - arg1 ) * 6 * H); }
   if ( ( 2 * H ) < 1 ) { return arg2; }
   if ( ( 3 * H ) < 2 ) { return ( arg1 + ( arg2 - arg1 ) * ( ( 2.0 / 3.0 ) - H ) * 6 ); }
   return arg1;
}

sf::Color HSL::TurnToRGB()
{
    ///Reconvert to range [0,1]
    double H = Hue/360.0;
    double S = Saturation/100.0;
    double L = Luminance/100.0;

    float arg1, arg2;

    if (S <= D_EPSILON)
    {
        sf::Color C(L*255, L*255, L*255);
        return C;
    }
    else
    {
        if ( L < 0.5 ) { arg2 = L * ( 1 + S ); }
        else { arg2 = ( L + S ) - ( S * L ); }
        arg1 = 2 * L - arg2;

        sf::Uint8 r =( 255 * HueToRGB( arg1, arg2, (H + 1.0/3.0 ) ) );
        sf::Uint8 g =( 255 * HueToRGB( arg1, arg2, H ) );
        sf::Uint8 b =( 255 * HueToRGB( arg1, arg2, (H - 1.0/3.0 ) ) );
        sf::Color C(r,g,b);
        return C;
    }

}

HSL TurnToHSL(const sf::Color& C)
{
    ///Trivial cases.
    if(C == sf::Color::White)
    { return HSL(0, 0, 100); }

    if(C == sf::Color::Black)
    { return HSL(0, 0, 0); }

    if(C == sf::Color::Red)
    { return HSL(0, 100, 50); }

    if(C == sf::Color::Yellow)
    { return HSL(60, 100, 50); }

    if(C == sf::Color::Green)
    { return HSL(120, 100, 50); }

    if(C == sf::Color::Cyan)
    { return HSL(180, 100, 50); }

    if(C == sf::Color::Blue)
    { return HSL(240, 100, 50); }

    if(C == sf::Color::Cyan)
    { return HSL(300, 100, 50); }

    double R, G, B;
    R = C.r/255.0;
    G = C.g/255.0;
    B = C.b/255.0;
    ///Casos no triviales.
    double max, min, l, s;

    ///Maximos
    max = std::max(std::max(R,G),B);

    ///Minimos
    min = std::min(std::min(R,G),B);


    HSL A;
    l = ((max + min)/2.0);

    if (max - min <= D_EPSILON )
    {
        A.Hue = 0;
        s = 0;
    }
    else
    {
        double diff = max - min;

        if(A.Luminance < 0.5)
        { s = diff/(max + min); }
    else
    { s = diff/(2 - max - min); }

	double diffR = ( (( max - R ) * 60) + (diff/2.0) ) / diff;
	double diffG = ( (( max - G ) * 60) + (diff/2.0) ) / diff;
	double diffB = ( (( max - B ) * 60) + (diff/2.0) ) / diff;


    if (max - R <= D_EPSILON) { A.Hue = diffB - diffG; }
    else if ( max - G <= D_EPSILON ) { A.Hue = (1*360)/3.0 + (diffR - diffB); }
    else if ( max - B <= D_EPSILON ) { A.Hue = (2*360)/3.0 + (diffG - diffR); }

    if (A.Hue <= 0 || A.Hue >= 360) { fmod(A.Hue, 360); }

    s *= 100;
    }

    l *= 100;
    A.Saturation = s;
    A.Luminance = l;
    return A;
}
