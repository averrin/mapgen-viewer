#ifndef _EPSILON_H_
#define _EPSILON_H_

#include <cmath>

#define EPSILON 1e-9
#define INV_EPSILON (1.0 / EPSILON);

inline bool eq_withEpsilon(double a, double b) { return std::abs(a - b) < EPSILON; }
inline bool gt_withEpsilon(double a, double b) { return a - b > EPSILON; }
inline bool gteq_withEpsilon(double a, double b) { return b - a < EPSILON; }
inline bool lt_withEpsilon(double a, double b) { return b - a > EPSILON; }
inline bool lteq_withEpsilon(double a, double b) { return a - b < EPSILON; }

#endif