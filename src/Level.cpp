#include "Level.h"
#include <iostream>

static std::vector<sf::Vector2f> pathSweepCenter()
{
    std::vector<sf::Vector2f> v;
    v.push_back(sf::Vector2f(240, -40));
    v.push_back(sf::Vector2f(240, 200));
    v.push_back(sf::Vector2f(240, 800));
    return v;
}

static std::vector<sf::Vector2f> pathSweepLeft()
{
    std::vector<sf::Vector2f> v;
    v.push_back(sf::Vector2f(240, -40));
    v.push_back(sf::Vector2f(96, 200));
    v.push_back(sf::Vector2f(96, 500));
    v.push_back(sf::Vector2f(-80, 700));
    return v;
}

static std::vector<sf::Vector2f> pathSweepRight()
{
    std::vector<sf::Vector2f> v;
    v.push_back(sf::Vector2f(240, -40));
    v.push_back(sf::Vector2f(384, 200));
    v.push_back(sf::Vector2f(384, 500));
    v.push_back(sf::Vector2f(560, 700));
    return v;
}

static std::vector<sf::Vector2f> pathLoopLeft()
{
    std::vector<sf::Vector2f> v;
    v.push_back(sf::Vector2f(120, -40));
    v.push_back(sf::Vector2f(60, 200));
    v.push_back(sf::Vector2f(200, 380));
    v.push_back(sf::Vector2f(120, 520));
    v.push_back(sf::Vector2f(-60, 700));
    return v;
}

static std::vector<sf::Vector2f> pathLoopRight()
{
    std::vector<sf::Vector2f> v;
    v.push_back(sf::Vector2f(360, -40));
    v.push_back(sf::Vector2f(420, 200));
    v.push_back(sf::Vector2f(280, 380));
    v.push_back(sf::Vector2f(360, 520));
    v.push_back(sf::Vector2f(540, 700));
    return v;
}

static std::vector<sf::Vector2f> pathZPattern()
{
    std::vector<sf::Vector2f> v;
    v.push_back(sf::Vector2f(60, -40));
    v.push_back(sf::Vector2f(420, 180));
    v.push_back(sf::Vector2f(60, 380));
    v.push_back(sf::Vector2f(420, 600));
    v.push_back(sf::Vector2f(60, 800));
    return v;
}

static std::vector<sf::Vector2f> pathZPatternRight()
{
    std::vector<sf::Vector2f> v;
    v.push_back(sf::Vector2f(420, -40));
    v.push_back(sf::Vector2f(60, 180));
    v.push_back(sf::Vector2f(420, 380));
    v.push_back(sf::Vector2f(60, 600));
    v.push_back(sf::Vector2f(420, 800));
    return v;
}

static std::vector<sf::Vector2f> pathDiveBomb()
{
    std::vector<sf::Vector2f> v;
    v.push_back(sf::Vector2f(240, -40));
    v.push_back(sf::Vector2f(240, 800));
    return v;
}

static std::vector<sf::Vector2f> pathBossEnter()
{
    std::vector<sf::Vector2f> v;
    v.push_back(sf::Vector2f(240, -300));
    v.push_back(sf::Vector2f(240, 120));
    return v;
}

static std::vector<sf::Vector2f> pathBossEnterLeft()
{
    std::vector<sf::Vector2f> v;
    v.push_back(sf::Vector2f(160, -300));
    v.push_back(sf::Vector2f(160, 120));
    return v;
}

static std::vector<sf::Vector2f> pathBossEnterRight()
{
    std::vector<sf::Vector2f> v;
    v.push_back(sf::Vector2f(320, -300));
    v.push_back(sf::Vector2f(320, 120));
    return v;
}

static SpawnEvent makeEvent(float t, EnemyType *et, sf::Vector2f start,
                            std::vector<sf::Vector2f> path, int count,
                            float xSpace, float delay)
{
    SpawnEvent ev;
    ev.time = t;
    ev.etype = et;
    ev.startPos = start;
    ev.path = path;
    ev.count = count;
    ev.xSpacing = xSpace;
    ev.delay = delay;
    ev.fired = false;
    return ev;
}

void Level::loadPlanet()
{
    if (planetLoaded)
        return;
    std::string path = getPlanetTexturePath();
    if (path.empty())
        return;
    if (planetTexture.loadFromFile(path))
    {
        planetLoaded = true;
    }
    else
    {
        std::cerr << "Warning: could not load planet texture: " << path << "\n";
    }
}

void Level::renderPlanet(sf::RenderWindow &window)
{
    if (planetLoaded)
    {
        sf::Sprite sprite(planetTexture);
        sf::FloatRect b = sprite.getLocalBounds();
        sprite.setOrigin(b.width / 2.f, b.height / 2.f);
        float targetSize = 240.f;
        float largest = (b.width > b.height) ? b.width : b.height;
        float scale = targetSize / largest;
        sprite.setScale(scale, scale);
        sprite.setPosition(360.f, 150.f);
        window.draw(sprite);
    }
}

LevelOne::LevelOne()
{
    name = "HOME INVASION";
    desc = "Fight off the invaders and protect Earth!";
}

void LevelOne::buildWaves()
{
    events.clear();
    events.push_back(makeEvent(1.0f, EnemyType::Small, sf::Vector2f(240, -40), pathSweepCenter(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(2.5f, EnemyType::Small, sf::Vector2f(120, -40), pathSweepLeft(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(2.5f, EnemyType::Small, sf::Vector2f(360, -40), pathSweepRight(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(5.0f, EnemyType::Small, sf::Vector2f(240, -40), pathSweepCenter(), 3, 55.f, 0.25f));
    events.push_back(makeEvent(8.0f, EnemyType::Small, sf::Vector2f(120, -40), pathLoopLeft(), 2, 55.f, 0.25f));
    events.push_back(makeEvent(8.0f, EnemyType::Small, sf::Vector2f(360, -40), pathLoopRight(), 2, 55.f, 0.25f));
    events.push_back(makeEvent(11.0f, EnemyType::Medium, sf::Vector2f(240, -40), pathDiveBomb(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(14.0f, EnemyType::Small, sf::Vector2f(240, -40), pathSweepCenter(), 4, 55.f, 0.25f));
    events.push_back(makeEvent(14.5f, EnemyType::Medium, sf::Vector2f(120, -40), pathSweepLeft(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(18.0f, EnemyType::Small, sf::Vector2f(60, -40), pathZPattern(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(18.2f, EnemyType::Small, sf::Vector2f(420, -40), pathZPatternRight(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(21.0f, EnemyType::Small, sf::Vector2f(240, -40), pathSweepCenter(), 5, 50.f, 0.25f));
    events.push_back(makeEvent(24.0f, EnemyType::Medium, sf::Vector2f(200, -40), pathSweepCenter(), 2, 80.f, 0.25f));
    events.push_back(makeEvent(27.0f, EnemyType::Small, sf::Vector2f(120, -40), pathSweepLeft(), 3, 55.f, 0.25f));
    events.push_back(makeEvent(27.0f, EnemyType::Small, sf::Vector2f(360, -40), pathSweepRight(), 3, 55.f, 0.25f));
    events.push_back(makeEvent(31.0f, EnemyType::Medium, sf::Vector2f(240, -40), pathDiveBomb(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(31.3f, EnemyType::Small, sf::Vector2f(120, -40), pathSweepLeft(), 2, 55.f, 0.25f));
    events.push_back(makeEvent(36.0f, EnemyType::Small, sf::Vector2f(240, -40), pathSweepCenter(), 5, 45.f, 0.25f));
    events.push_back(makeEvent(38.0f, EnemyType::Medium, sf::Vector2f(240, -40), pathSweepCenter(), 2, 80.f, 0.25f));
    events.push_back(makeEvent(43.0f, EnemyType::Boss, sf::Vector2f(240, -80), pathBossEnter(), 1, 55.f, 0.25f));
}

LevelTwo::LevelTwo()
{
    name = "SURPRISE ASSAULT";
    desc = "Keep pushing forward and capture their base!";
}

void LevelTwo::buildWaves()
{
    events.clear();
    events.push_back(makeEvent(1.0f, EnemyType::Small, sf::Vector2f(240, -40), pathSweepCenter(), 3, 55.f, 0.25f));
    events.push_back(makeEvent(3.0f, EnemyType::Small, sf::Vector2f(120, -40), pathSweepLeft(), 2, 55.f, 0.25f));
    events.push_back(makeEvent(3.0f, EnemyType::Small, sf::Vector2f(360, -40), pathSweepRight(), 2, 55.f, 0.25f));
    events.push_back(makeEvent(6.0f, EnemyType::Medium, sf::Vector2f(240, -40), pathSweepCenter(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(6.0f, EnemyType::Small, sf::Vector2f(60, -40), pathZPattern(), 2, 55.f, 0.25f));
    events.push_back(makeEvent(9.0f, EnemyType::Small, sf::Vector2f(240, -40), pathSweepCenter(), 5, 45.f, 0.25f));
    events.push_back(makeEvent(12.0f, EnemyType::Medium, sf::Vector2f(160, -40), pathSweepLeft(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(12.0f, EnemyType::Medium, sf::Vector2f(320, -40), pathSweepRight(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(15.0f, EnemyType::Medium, sf::Vector2f(240, -40), pathDiveBomb(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(15.0f, EnemyType::Small, sf::Vector2f(120, -40), pathLoopLeft(), 3, 55.f, 0.25f));
    events.push_back(makeEvent(18.0f, EnemyType::Small, sf::Vector2f(240, -40), pathSweepCenter(), 4, 55.f, 0.25f));
    events.push_back(makeEvent(18.0f, EnemyType::Medium, sf::Vector2f(360, -40), pathSweepRight(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(22.0f, EnemyType::Medium, sf::Vector2f(160, -40), pathSweepCenter(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(22.0f, EnemyType::Medium, sf::Vector2f(320, -40), pathSweepCenter(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(26.0f, EnemyType::Medium, sf::Vector2f(60, -40), pathZPattern(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(26.0f, EnemyType::Medium, sf::Vector2f(420, -40), pathZPatternRight(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(30.0f, EnemyType::Small, sf::Vector2f(240, -40), pathSweepCenter(), 5, 40.f, 0.25f));
    events.push_back(makeEvent(30.0f, EnemyType::Medium, sf::Vector2f(240, -40), pathDiveBomb(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(34.0f, EnemyType::Small, sf::Vector2f(120, -40), pathSweepLeft(), 3, 55.f, 0.25f));
    events.push_back(makeEvent(34.0f, EnemyType::Small, sf::Vector2f(360, -40), pathSweepRight(), 3, 55.f, 0.25f));
    events.push_back(makeEvent(38.0f, EnemyType::Medium, sf::Vector2f(200, -40), pathSweepCenter(), 3, 60.f, 0.25f));
    events.push_back(makeEvent(42.0f, EnemyType::Small, sf::Vector2f(240, -40), pathSweepCenter(), 4, 55.f, 0.25f));
    events.push_back(makeEvent(42.0f, EnemyType::Medium, sf::Vector2f(120, -40), pathLoopLeft(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(46.0f, EnemyType::Small, sf::Vector2f(240, -40), pathSweepCenter(), 5, 45.f, 0.25f));
    events.push_back(makeEvent(46.0f, EnemyType::Medium, sf::Vector2f(240, -40), pathDiveBomb(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(54.0f, EnemyType::Boss, sf::Vector2f(240, -80), pathBossEnter(), 1, 55.f, 0.25f));
}

LevelThree::LevelThree()
{
    name = "THE FINAL BATTLE";
    desc = "We found their homeworld! Let's end this war!";
}

void LevelThree::buildWaves()
{
    events.clear();
    events.push_back(makeEvent(1.0f, EnemyType::Medium, sf::Vector2f(240, -40), pathSweepCenter(), 2, 80.f, 0.25f));
    events.push_back(makeEvent(3.0f, EnemyType::Small, sf::Vector2f(120, -40), pathSweepLeft(), 3, 55.f, 0.25f));
    events.push_back(makeEvent(3.0f, EnemyType::Small, sf::Vector2f(360, -40), pathSweepRight(), 3, 55.f, 0.25f));
    events.push_back(makeEvent(6.0f, EnemyType::Medium, sf::Vector2f(60, -40), pathZPattern(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(6.0f, EnemyType::Small, sf::Vector2f(240, -40), pathSweepCenter(), 4, 55.f, 0.25f));
    events.push_back(makeEvent(10.0f, EnemyType::Medium, sf::Vector2f(160, -40), pathDiveBomb(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(10.0f, EnemyType::Medium, sf::Vector2f(320, -40), pathDiveBomb(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(10.0f, EnemyType::Small, sf::Vector2f(120, -40), pathLoopLeft(), 2, 55.f, 0.25f));
    events.push_back(makeEvent(14.0f, EnemyType::Small, sf::Vector2f(240, -40), pathSweepCenter(), 5, 42.f, 0.25f));
    events.push_back(makeEvent(14.0f, EnemyType::Medium, sf::Vector2f(120, -40), pathSweepLeft(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(14.0f, EnemyType::Medium, sf::Vector2f(360, -40), pathSweepRight(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(18.0f, EnemyType::Small, sf::Vector2f(60, -40), pathZPattern(), 2, 55.f, 0.25f));
    events.push_back(makeEvent(18.0f, EnemyType::Small, sf::Vector2f(420, -40), pathZPatternRight(), 2, 55.f, 0.25f));
    events.push_back(makeEvent(20.0f, EnemyType::Medium, sf::Vector2f(160, -40), pathSweepCenter(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(20.0f, EnemyType::Medium, sf::Vector2f(320, -40), pathSweepCenter(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(20.0f, EnemyType::Medium, sf::Vector2f(240, -40), pathDiveBomb(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(24.0f, EnemyType::Small, sf::Vector2f(240, -40), pathSweepCenter(), 5, 45.f, 0.25f));
    events.push_back(makeEvent(24.0f, EnemyType::Medium, sf::Vector2f(120, -40), pathLoopLeft(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(28.0f, EnemyType::Medium, sf::Vector2f(200, -40), pathSweepCenter(), 3, 60.f, 0.25f));
    events.push_back(makeEvent(28.0f, EnemyType::Small, sf::Vector2f(60, -40), pathZPattern(), 3, 55.f, 0.25f));
    events.push_back(makeEvent(32.0f, EnemyType::Medium, sf::Vector2f(240, -40), pathDiveBomb(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(32.0f, EnemyType::Medium, sf::Vector2f(120, -40), pathSweepLeft(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(32.0f, EnemyType::Medium, sf::Vector2f(360, -40), pathSweepRight(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(36.0f, EnemyType::Small, sf::Vector2f(240, -40), pathSweepCenter(), 5, 40.f, 0.25f));
    events.push_back(makeEvent(36.0f, EnemyType::Medium, sf::Vector2f(160, -40), pathLoopLeft(), 2, 55.f, 0.25f));
    events.push_back(makeEvent(40.0f, EnemyType::Medium, sf::Vector2f(200, -40), pathSweepCenter(), 3, 70.f, 0.25f));
    events.push_back(makeEvent(40.0f, EnemyType::Small, sf::Vector2f(420, -40), pathZPatternRight(), 3, 55.f, 0.25f));
    events.push_back(makeEvent(44.0f, EnemyType::Small, sf::Vector2f(240, -40), pathSweepCenter(), 5, 42.f, 0.25f));
    events.push_back(makeEvent(44.0f, EnemyType::Medium, sf::Vector2f(240, -40), pathDiveBomb(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(48.0f, EnemyType::Medium, sf::Vector2f(160, -40), pathSweepCenter(), 2, 80.f, 0.25f));
    events.push_back(makeEvent(48.0f, EnemyType::Small, sf::Vector2f(120, -40), pathSweepLeft(), 3, 55.f, 0.25f));
    events.push_back(makeEvent(52.0f, EnemyType::Small, sf::Vector2f(240, -40), pathSweepCenter(), 4, 55.f, 0.25f));
    events.push_back(makeEvent(52.0f, EnemyType::Medium, sf::Vector2f(240, -40), pathDiveBomb(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(57.0f, EnemyType::Boss, sf::Vector2f(160, -80), pathBossEnterLeft(), 1, 55.f, 0.25f));
    events.push_back(makeEvent(70.0f, EnemyType::Boss, sf::Vector2f(320, -80), pathBossEnterRight(), 1, 55.f, 0.25f));
}