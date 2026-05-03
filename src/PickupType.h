#pragma once

#include <SFML/Graphics.hpp>

class PickupType
{
    int id;
    sf::Color color;

public:
    PickupType(int id, sf::Color color) : id(id), color(color) {}
    virtual ~PickupType() {}
    int getId() const { return id; }
    sf::Color getColor() const { return color; }
    static PickupType *Score;
    static PickupType *Health;
};

class ScorePickupType : public PickupType
{
public:
    ScorePickupType() : PickupType(0, sf::Color(60, 120, 255)) {}
};

class HealthPickupType : public PickupType
{
public:
    HealthPickupType() : PickupType(1, sf::Color(60, 220, 80)) {}
};
