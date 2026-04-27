#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdlib>

inline float vlen(sf::Vector2f v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}

inline sf::Vector2f vnorm(sf::Vector2f v)
{
    float l = vlen(v);
    if (l < 0.0001f)
        return sf::Vector2f(0.f, 0.f);
    return sf::Vector2f(v.x / l, v.y / l);
}

inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

inline sf::Color colorLerp(sf::Color a, sf::Color b, float t)
{
    if (t < 0.f)
        t = 0.f;
    if (t > 1.f)
        t = 1.f;
    return sf::Color(
        (sf::Uint8)(a.r + (b.r - a.r) * t),
        (sf::Uint8)(a.g + (b.g - a.g) * t),
        (sf::Uint8)(a.b + (b.b - a.b) * t),
        (sf::Uint8)(a.a + (b.a - a.a) * t));
}

inline float clampf(float val, float lo, float hi)
{
    if (val < lo)
        return lo;
    if (val > hi)
        return hi;
    return val;
}

inline float randFloat(float lo, float hi)
{
    float r = (float)rand() / (float)RAND_MAX;
    return lo + r * (hi - lo);
}

inline int randInt(int lo, int hi)
{
    if (hi <= lo)
        return lo;
    return lo + rand() % (hi - lo + 1);
}
