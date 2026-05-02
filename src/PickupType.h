#pragma once

#include <SFML/Graphics.hpp>

class PickupType
{
public:
    virtual ~PickupType() {}
    virtual int getId() const = 0;
    virtual sf::Color getColor() const = 0;
    static PickupType *Score;
    static PickupType *Health;
};

class ScorePickupType : public PickupType
{
public:
    int getId() const { return 0; }
    sf::Color getColor() const { return sf::Color(60, 120, 255); }
};

class HealthPickupType : public PickupType
{
public:
    int getId() const { return 1; }
    sf::Color getColor() const { return sf::Color(60, 220, 80); }
};
