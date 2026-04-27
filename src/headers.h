#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <queue>
#include <stack>
#include <list>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <string>
#include <memory>

// ============================================================================
// SECTION 1: MATH UTILITY FUNCTIONS
// ============================================================================
inline float vlen(sf::Vector2f v)
{
    return std::sqrt(v.x * v.x + v.y * v.y);
}
inline sf::Vector2f vnorm(sf::Vector2f v)
{
    float l = vlen(v);
    if (l < 0.0001f)
        return {0.f, 0.f};
    return {v.x / l, v.y / l};
}
inline float lerp(float a, float b, float t) { return a + (b - a) * t; }
inline sf::Color colorLerp(sf::Color a, sf::Color b, float t)
{
    t = std::max(0.f, std::min(1.f, t));
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

// ============================================================================
// SECTION 2: GAMESTATE (was enum class — now a normal class)
// ============================================================================
class GameState
{
public:
    int id;
    GameState(int i = 0) : id(i) {}
    bool operator==(const GameState &o) const { return id == o.id; }
    bool operator!=(const GameState &o) const { return id != o.id; }
    static const GameState LevelSelect;
    static const GameState Playing;
    static const GameState Paused;
    static const GameState GameOver;
    static const GameState LevelComplete;
};

// ============================================================================
// SECTION 3: ENEMYTYPE (polymorphic — base + 3 subclasses)
// Was enum class { Small, Medium, Boss }. Now an abstract base class with
// virtual stat accessors, and three subclasses with static instances exposed
// via EnemyType::Small / Medium / Boss for drop-in replacement of enum syntax.
// ============================================================================
class EnemyType
{
public:
    virtual ~EnemyType() = default;
    virtual float getMaxHp() const = 0;
    virtual float getShootInterval() const = 0;
    virtual int getScoreValue() const = 0;
    virtual float getMoveSpeed() const = 0;
    virtual float getRadius() const = 0;
    virtual int getKind() const = 0; // 0 = small, 1 = medium, 2 = boss
    static EnemyType *Small;
    static EnemyType *Medium;
    static EnemyType *Boss;
};

class SmallEnemyType : public EnemyType
{
public:
    float getMaxHp() const override { return 30.f; }
    float getShootInterval() const override { return 2.2f; }
    int getScoreValue() const override { return 50; }
    float getMoveSpeed() const override { return 200.f; }
    float getRadius() const override { return 16.f; }
    int getKind() const override { return 0; }
};
class MediumEnemyType : public EnemyType
{
public:
    float getMaxHp() const override { return 80.f; }
    float getShootInterval() const override { return 1.8f; }
    int getScoreValue() const override { return 120; }
    float getMoveSpeed() const override { return 160.f; }
    float getRadius() const override { return 22.f; }
    int getKind() const override { return 1; }
};
class BossEnemyType : public EnemyType
{
public:
    float getMaxHp() const override { return 1200.f; }
    float getShootInterval() const override { return 0.8f; }
    int getScoreValue() const override { return 2000; }
    float getMoveSpeed() const override { return 80.f; }
    float getRadius() const override { return 60.f; }
    int getKind() const override { return 2; }
};

// ============================================================================
// SECTION 4: BULLETTYPE (polymorphic — base + 6 subclasses)
// ============================================================================
class BulletType
{
public:
    virtual ~BulletType() = default;
    virtual int getId() const = 0;
    virtual bool isPlayerBullet() const = 0;
    static BulletType *PlayerNorm;
    static BulletType *PlayerWide;
    static BulletType *PlayerTriple;
    static BulletType *EnemyNorm;
    static BulletType *EnemyBurst;
    static BulletType *BossBeam;
};
class PlayerNormBullet : public BulletType
{
public:
    int getId() const override { return 0; }
    bool isPlayerBullet() const override { return true; }
};
class PlayerWideBullet : public BulletType
{
public:
    int getId() const override { return 1; }
    bool isPlayerBullet() const override { return true; }
};
class PlayerTripleBullet : public BulletType
{
public:
    int getId() const override { return 2; }
    bool isPlayerBullet() const override { return true; }
};
class EnemyNormBullet : public BulletType
{
public:
    int getId() const override { return 3; }
    bool isPlayerBullet() const override { return false; }
};
class EnemyBurstBullet : public BulletType
{
public:
    int getId() const override { return 4; }
    bool isPlayerBullet() const override { return false; }
};
class BossBeamBullet : public BulletType
{
public:
    int getId() const override { return 5; }
    bool isPlayerBullet() const override { return false; }
};

// ============================================================================
// SECTION 5: PICKUPTYPE (polymorphic — base + 3 subclasses)
// ============================================================================
class PickupType
{
public:
    virtual ~PickupType() = default;
    virtual int getId() const = 0;
    virtual sf::Color getColor() const = 0;
    static PickupType *Score;
    static PickupType *Health;
    static PickupType *Power;
};
class ScorePickupType : public PickupType
{
public:
    int getId() const override { return 0; }
    sf::Color getColor() const override { return sf::Color(60, 120, 255); }
};
class HealthPickupType : public PickupType
{
public:
    int getId() const override { return 1; }
    sf::Color getColor() const override { return sf::Color(60, 220, 80); }
};
class PowerPickupType : public PickupType
{
public:
    int getId() const override { return 2; }
    sf::Color getColor() const override { return sf::Color(255, 200, 40); }
};

// ============================================================================
// SECTION 6: GAMEPLAY DATA STRUCTS
// ============================================================================
struct Player
{
    sf::Vector2f pos{240.f, 600.f};
    float hp = 100.f;
    float maxHp = 100.f;
    float speed = 310.f;
    int lives = 3;
    int score = 0;
    int power = 1;
    float powerTimer = 0.f;
    float shootTimer = 0.f;
    float shootInterval = 0.10f;
    float iframeTimer = 0.f;
    float shieldTimer = 0.f;
    float tilt = 0.f;
    bool alive = true;
};

struct Bullet
{
    sf::Vector2f pos;
    sf::Vector2f vel;
    float dmg = 20.f;
    BulletType *type = nullptr;
    bool on = false;
};

struct Enemy
{
    sf::Vector2f pos;
    sf::Vector2f vel;
    float hp = 30.f;
    float maxHp = 30.f;
    EnemyType *type = nullptr;
    bool on = false;
    float shootTimer = 0.f;
    float shootInterval = 2.2f;
    std::vector<sf::Vector2f> path;
    int pathIndex = 0;
    float angle = 0.f;
    float pulse = 0.f;
    int phase = 0;
    int scoreValue = 50;
    float moveSpeed = 200.f;
};

struct Particle
{
    sf::Vector2f pos;
    sf::Vector2f vel;
    sf::Color colorStart;
    sf::Color colorEnd;
    float life = 0.f;
    float maxLife = 0.f;
    float size = 3.f;
    bool on = false;
};

struct Pickup
{
    sf::Vector2f pos;
    sf::Vector2f vel;
    PickupType *type = nullptr;
    float life = 8.f;
    float pulse = 0.f;
    bool on = false;
};

struct SpawnEvent
{
    float time;
    EnemyType *etype;
    sf::Vector2f startPos;
    std::vector<sf::Vector2f> path;
    int count = 1;
    float xSpacing = 55.f;
    float delay = 0.25f;
    bool fired = false;
};

struct SpawnJob
{
    EnemyType *etype;
    sf::Vector2f startPos;
    float delayRemaining;
    std::vector<sf::Vector2f> path;
    float hp = 30.f;
    float maxHp = 30.f;
    float shootInterval = 2.2f;
    int scoreValue = 50;
    float moveSpeed = 200.f;
};

struct ScoreEvent
{
    std::string description;
    int points;
    float timestamp;
};

struct Star
{
    sf::Vector2f pos;
    float speed;
    float brightness;
    float size;
};

// ============================================================================
// SECTION 7: LEVEL CLASS HIERARCHY (replaces LevelData struct)
// Polymorphic: parent Level + LevelAries / LevelTaurus / LevelGemini /
// LevelEndless. Each level overrides:
//   - getDuration()           → how long the level lasts
//   - getPlanetTexturePath()  → which planet texture to render in background
//   - getPlanetColor()        → fallback tint
//   - buildWaves()            → fills the events vector with the wave script
//   - renderPlanet(window)    → draws its own background planet
//   - isEndless()             → true only for LevelEndless
//   - updateEndless(...)      → only LevelEndless overrides; runs random spawns
// ============================================================================
class Level
{
public:
    std::string name;
    std::string desc;
    sf::Color planetColor = sf::Color::White;
    bool locked = true;
    std::vector<SpawnEvent> events;
    sf::Texture planetTexture;
    bool planetLoaded = false;

    virtual ~Level() = default;
    virtual float getDuration() const { return 60.f; }
    virtual std::string getName() const { return name; }
    virtual std::string getDesc() const { return desc; }
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
    float getDuration() const override { return 55.f; }
    std::string getPlanetTexturePath() const override { return "assets/textures/planet1.png"; }
    void buildWaves() override;
};

class LevelTaurus : public Level
{
public:
    LevelTaurus();
    float getDuration() const override { return 65.f; }
    std::string getPlanetTexturePath() const override { return "assets/textures/planet2.png"; }
    void buildWaves() override;
};

class LevelGemini : public Level
{
public:
    LevelGemini();
    float getDuration() const override { return 80.f; }
    std::string getPlanetTexturePath() const override { return "assets/textures/planet3.png"; }
    void buildWaves() override;
};

class LevelEndless : public Level
{
public:
    // pools the random generator picks indices from
    std::vector<std::vector<sf::Vector2f>> normalPathPool;
    std::vector<std::vector<sf::Vector2f>> bossPathPool;
    std::vector<EnemyType *> normalEnemyPool;
    std::vector<float> speedPool;
    std::vector<float> intervalPool;
    std::vector<sf::Color> planetColorPool;
    std::vector<std::string> planetTexturePool;

    int chosenPlanet = 0;
    int normalsBeforeBoss = 0;
    int normalsSpawned = 0;
    bool spawnedBoss = false;
    float spawnTimer = 0.f;
    float currentInterval = 1.5f;

    LevelEndless();
    float getDuration() const override { return 99999.f; }
    std::string getPlanetTexturePath() const override { return planetTexturePool[chosenPlanet]; }
    sf::Color getPlanetColor() const override { return planetColorPool[chosenPlanet]; }
    bool isEndless() const override { return true; }
    void buildWaves() override; // initializes pools, no fixed events
    void updateEndless(float dt, float levelTimer,
                       std::queue<SpawnJob> &q, int activeEnemies,
                       bool bossOnScreen) override;
    void rollNextCycle();
};

// ============================================================================
// SECTION 8: POOL SIZE CONSTANTS
// ============================================================================
const int MAX_BULLETS = 700;
const int MAX_ENEMIES = 80;
const int MAX_PARTICLES = 1000;
const int MAX_PICKUPS = 64;
const int MAX_STARS = 200;

// ============================================================================
// SECTION 9: GAME CLASS
// ============================================================================
class Game
{
public:
    void init();
    void run();

private:
    sf::RenderWindow window;
    sf::Font font;
    sf::Texture shipTexture;
    sf::Sprite shipSprite;
    sf::View gameView;
    sf::Texture smallEnemyTexture;
    sf::Texture mediumEnemyTexture;
    sf::Texture bossEnemyTexture;

    std::stack<GameState> stateStack;

    Player player;
    std::array<Bullet, MAX_BULLETS> bullets;
    std::array<Enemy, MAX_ENEMIES> enemies;
    std::array<Particle, MAX_PARTICLES> particles;
    std::array<Pickup, MAX_PICKUPS> pickups;

    std::array<Star, MAX_STARS> stars;

    std::vector<std::unique_ptr<Level>> levels;
    int currentLevel = 0;
    int selectedLevel = 0;
    float levelTimer = 0.f;

    std::queue<SpawnJob> spawnQueue;
    std::list<ScoreEvent> scoreLog;

    sf::RenderTexture stationTile;
    float bgY = 0.f;

    bool bossActive = false;

    void handleEvents();
    void update(float dt);
    void render();

    void startLevel(int index);
    void resetPlayer();

    void checkCollisions();
    void checkSpawns(float dt);
    void spawnExplosion(sf::Vector2f pos, sf::Color cStart, sf::Color cEnd, int count);
    void spawnPickup(sf::Vector2f pos, PickupType *ptype);

    Bullet *allocBullet();
    Enemy *allocEnemy();
    Particle *allocParticle();
    Pickup *allocPickup();

    void spawnPlayerBullet();
    void spawnEnemyBullet(Enemy &e);
    void spawnEnemyFromJob(const SpawnJob &job);

    void renderBackground();
    void renderStars();
    void renderPlayer();
    void renderEnemies();
    void renderBullets();
    void renderParticles();
    void renderPickups();
    void renderHUD();
    void renderBossHP();
    void renderLevelSelect();
    void renderPauseOverlay();
    void renderGameOverOverlay();
    void renderLevelCompleteOverlay();

    void drawPlayerShip(sf::Vector2f pos, float tilt, float scale, sf::Color tint);
    void drawSmallEnemy(sf::Vector2f pos, float angle);
    void drawMediumEnemy(sf::Vector2f pos, float angle);
    void drawBossEnemy(sf::Vector2f pos, float angle);

    void initStars();
    void buildLevels();

    void drawTextCentered(const std::string &str, float y, int size, sf::Color col);
    bool isLevelClear();
    int countActiveEnemies();
};
