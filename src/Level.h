#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "GameData.h"
#include "EnemyType.h"

class Level
{
protected:
    std::string name;
    std::string desc;
    std::vector<SpawnEvent> events;
    sf::Texture planetTexture;
    bool planetLoaded;

public:
    Level() : planetLoaded(false) {}
    virtual ~Level() {}

    std::string getName() const { return name; }
    std::string getDesc() const { return desc; }
    std::vector<SpawnEvent> &getEvents() { return events; }
    virtual std::string getPlanetTexturePath() const = 0;
    virtual void buildWaves() = 0;
    virtual void renderPlanet(sf::RenderWindow &window);
    void loadPlanet();
};

class LevelOne : public Level
{
public:
    LevelOne();
    std::string getPlanetTexturePath() const override { return "assets/textures/planet1.png"; }
    void buildWaves() override;
};

class LevelTwo : public Level
{
public:
    LevelTwo();
    std::string getPlanetTexturePath() const override { return "assets/textures/planet2.png"; }
    void buildWaves() override;
};

class LevelThree : public Level
{
public:
    LevelThree();
    std::string getPlanetTexturePath() const override { return "assets/textures/planet3.png"; }
    void buildWaves() override;
};
