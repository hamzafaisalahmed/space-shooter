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
    sf::Color planetColor;
    bool locked;
    std::vector<SpawnEvent> events;
    sf::Texture planetTexture;
    bool planetLoaded;

public:
    Level() : planetColor(sf::Color::White), locked(true), planetLoaded(false) {}
    virtual ~Level() {}

    std::string getName() const { return name; }
    std::string getDesc() const { return desc; }
    bool isLocked() const { return locked; }
    void unlock() { locked = false; }
    std::vector<SpawnEvent> &getEvents() { return events; }

    virtual float getDuration() const { return 60.f; }
    virtual sf::Color getPlanetColor() const { return planetColor; }
    virtual std::string getPlanetTexturePath() const { return ""; }
    virtual void buildWaves() = 0;
    virtual bool isEndless() const { return false; }
    virtual void updateEndless(float /*dt*/, float /*levelTimer*/,
                               std::queue<SpawnJob> & /*q*/, int /*activeEnemies*/,
                               bool /*bossOnScreen*/) {}
    virtual void renderPlanet(sf::RenderWindow &window);
    void loadPlanet();
};

class LevelAries : public Level
{
public:
    LevelAries();
    float getDuration() const { return 55.f; }
    std::string getPlanetTexturePath() const { return "assets/textures/planet1.png"; }
    void buildWaves();
};

class LevelTaurus : public Level
{
public:
    LevelTaurus();
    float getDuration() const { return 65.f; }
    std::string getPlanetTexturePath() const { return "assets/textures/planet2.png"; }
    void buildWaves();
};

class LevelGemini : public Level
{
public:
    LevelGemini();
    float getDuration() const { return 80.f; }
    std::string getPlanetTexturePath() const { return "assets/textures/planet3.png"; }
    void buildWaves();
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
    float getDuration() const { return 99999.f; }
    std::string getPlanetTexturePath() const { return planetTexturePool[chosenPlanet]; }
    sf::Color getPlanetColor() const { return planetColorPool[chosenPlanet]; }
    bool isEndless() const { return true; }
    void buildWaves();
    void updateEndless(float dt, float levelTimer,
                       std::queue<SpawnJob> &q, int activeEnemies,
                       bool bossOnScreen);
    void rollNextCycle();
};
