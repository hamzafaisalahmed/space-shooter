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

LevelAries::LevelAries()
{
    name = "ARIES";
    desc = "The Beginning";
    planetColor = sf::Color(180, 120, 60);
}

void LevelAries::buildWaves()
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

LevelTaurus::LevelTaurus()
{
    name = "TAURUS";
    desc = "The Challenge";
    planetColor = sf::Color(80, 60, 180);
}

void LevelTaurus::buildWaves()
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

LevelGemini::LevelGemini()
{
    name = "GEMINI";
    desc = "The Final Stand";
    planetColor = sf::Color(30, 120, 50);
}

void LevelGemini::buildWaves()
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

LevelEndless::LevelEndless()
    : chosenPlanet(0), normalsBeforeBoss(0), normalsSpawned(0),
      spawnedBoss(false), spawnTimer(0.f), currentInterval(1.5f)
{
    name = "ENDLESS";
    desc = "Pure Chaos";

    normalPathPool.push_back(pathSweepCenter());
    normalPathPool.push_back(pathSweepLeft());
    normalPathPool.push_back(pathSweepRight());
    normalPathPool.push_back(pathLoopLeft());
    normalPathPool.push_back(pathLoopRight());
    normalPathPool.push_back(pathZPattern());
    normalPathPool.push_back(pathZPatternRight());
    normalPathPool.push_back(pathDiveBomb());

    bossPathPool.push_back(pathBossEnter());
    bossPathPool.push_back(pathBossEnterLeft());
    bossPathPool.push_back(pathBossEnterRight());

    normalEnemyPool.push_back(EnemyType::Small);
    normalEnemyPool.push_back(EnemyType::Medium);

    speedPool.push_back(120.f);
    speedPool.push_back(160.f);
    speedPool.push_back(200.f);
    speedPool.push_back(240.f);
    speedPool.push_back(280.f);

    intervalPool.push_back(0.6f);
    intervalPool.push_back(0.9f);
    intervalPool.push_back(1.2f);
    intervalPool.push_back(1.5f);
    intervalPool.push_back(1.8f);
    intervalPool.push_back(2.2f);
}

void LevelEndless::buildWaves()
{
    events.clear();
    rollNextCycle();
    spawnTimer = 0.f;
}

void LevelEndless::rollNextCycle()
{
    normalsBeforeBoss = randInt(5, 20);
    normalsSpawned = 0;
    spawnedBoss = false;
    currentInterval = intervalPool[randInt(0, (int)intervalPool.size() - 1)];
}

void LevelEndless::updateEndless(float dt, float /*levelTimer*/,
                                 std::queue<SpawnJob> &q, int activeEnemies,
                                 bool bossOnScreen)
{
    spawnTimer -= dt;

    if (!spawnedBoss && normalsSpawned < normalsBeforeBoss)
    {
        if (spawnTimer <= 0.f)
        {
            int pIdx = randInt(0, (int)normalPathPool.size() - 1);
            int eIdx = randInt(0, (int)normalEnemyPool.size() - 1);
            int sIdx = randInt(0, (int)speedPool.size() - 1);
            int iIdx = randInt(0, (int)intervalPool.size() - 1);

            EnemyType *et = normalEnemyPool[eIdx];
            std::vector<sf::Vector2f> path = normalPathPool[pIdx];

            SpawnJob job;
            job.etype = et;
            job.startPos = path.empty() ? sf::Vector2f(240.f, -40.f) : path[0];
            job.delayRemaining = 0.f;
            job.path = path;
            job.hp = job.maxHp = et->getMaxHp();
            job.shootInterval = intervalPool[iIdx];
            job.scoreValue = et->getScoreValue();
            job.moveSpeed = speedPool[sIdx];
            q.push(job);

            normalsSpawned++;
            spawnTimer = currentInterval * randFloat(0.7f, 1.3f);
        }
    }
    else if (!spawnedBoss && normalsSpawned >= normalsBeforeBoss)
    {
        if (activeEnemies == 0 && q.empty())
        {
            int pIdx = randInt(0, (int)bossPathPool.size() - 1);
            std::vector<sf::Vector2f> path = bossPathPool[pIdx];

            SpawnJob job;
            job.etype = EnemyType::Boss;
            job.startPos = path.empty() ? sf::Vector2f(240.f, -80.f) : path[0];
            job.delayRemaining = 0.f;
            job.path = path;
            job.hp = job.maxHp = EnemyType::Boss->getMaxHp();
            job.shootInterval = EnemyType::Boss->getShootInterval();
            job.scoreValue = EnemyType::Boss->getScoreValue();
            job.moveSpeed = EnemyType::Boss->getMoveSpeed();
            q.push(job);

            spawnedBoss = true;
        }
    }
    else if (spawnedBoss && !bossOnScreen && activeEnemies == 0 && q.empty())
    {
        rollNextCycle();
    }
}
