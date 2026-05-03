#pragma once

#include <SFML/Graphics.hpp>
#include <queue>
#include <string>
#include <vector>
#include "GameData.h"
#include "EnemyType.h"
#include "Utils.h"

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

    virtual float getDuration() const = 0;
    virtual std::string getPlanetTexturePath() const = 0;
    virtual void buildWaves() = 0;
    virtual bool isEndless() const { return false; }
    virtual void updateEndless(float, float, std::queue<SpawnJob> &, int, bool) {} // only used by endless level, but kept here so can be accessed by parent class pointer
    virtual void renderPlanet(sf::RenderWindow &window);
    void loadPlanet();
};

class LevelOne : public Level
{
public:
    LevelOne();
    float getDuration() const override { return 55.f; }
    std::string getPlanetTexturePath() const override { return "assets/textures/planet1.png"; }
    void buildWaves() override;
};

class LevelTwo : public Level
{
public:
    LevelTwo();
    float getDuration() const override { return 65.f; }
    std::string getPlanetTexturePath() const override { return "assets/textures/planet2.png"; }
    void buildWaves() override;
};

class LevelThree : public Level
{
public:
    LevelThree();
    float getDuration() const override { return 80.f; }
    std::string getPlanetTexturePath() const override { return "assets/textures/planet3.png"; }
    void buildWaves() override;
};

class LevelEndless : public Level
{
private:
    std::vector<std::vector<sf::Vector2f>> normalPathPool;
    std::vector<std::vector<sf::Vector2f>> bossPathPool;
    std::vector<EnemyType *> normalEnemyPool;
    std::vector<float> speedPool;
    std::vector<float> intervalPool;
    std::vector<sf::Color> planetColorPool;
    std::vector<std::string> planetTexturePool;

    int chosenPlanet;
    int normalsBeforeBoss;
    int normalsSpawned;
    bool spawnedBoss;
    float spawnTimer;
    float currentInterval;

public:
    LevelEndless();
    std::string getPlanetTexturePath() const override { return ""; }
    float getDuration() const override { return 99999.f; }
    bool isEndless() const override { return true; }
    void buildWaves() override;
    void updateEndless(float dt, float levelTimer,
                       std::queue<SpawnJob> &q, int activeEnemies,
                       bool bossOnScreen) override;
    void rollNextCycle();
};
