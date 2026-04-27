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
#include <fstream>
#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <string>

// ============================================================================
// SECTION 1: CUSTOM EXCEPTIONS
// (Exception Handling topic — thrown by asset loading and level lookups)
// ============================================================================
class AssetLoadException : public std::runtime_error
{
public:
    AssetLoadException(const std::string &msg) : std::runtime_error(msg) {}
};

class InvalidLevelException : public std::runtime_error
{
public:
    InvalidLevelException(const std::string &msg) : std::runtime_error(msg) {}
};

// ============================================================================
// SECTION 2: MATH UTILITY FUNCTIONS
// ============================================================================
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
    if (t < 0.f) t = 0.f;
    if (t > 1.f) t = 1.f;
    return sf::Color(
        (sf::Uint8)(a.r + (b.r - a.r) * t),
        (sf::Uint8)(a.g + (b.g - a.g) * t),
        (sf::Uint8)(a.b + (b.b - a.b) * t),
        (sf::Uint8)(a.a + (b.a - a.a) * t));
}
inline float clampf(float val, float lo, float hi)
{
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}
inline float randFloat(float lo, float hi)
{
    float r = (float)rand() / (float)RAND_MAX;
    return lo + r * (hi - lo);
}
inline int randInt(int lo, int hi)
{
    if (hi <= lo) return lo;
    return lo + rand() % (hi - lo + 1);
}

// ============================================================================
// SECTION 3: GAMESTATE
// (Class with private id, operator overloading for == and !=)
// ============================================================================
class GameState
{
private:
    int id;
public:
    GameState(int i = 0) : id(i) {}
    int getId() const { return id; }
    bool operator==(const GameState &o) const { return id == o.id; }
    bool operator!=(const GameState &o) const { return id != o.id; }
    static const GameState Home;
    static const GameState LevelSelect;
    static const GameState Playing;
    static const GameState Paused;
    static const GameState GameOver;
    static const GameState LevelComplete;
};

// ============================================================================
// SECTION 4: ENEMYTYPE (Abstract base + 3 subclasses — Inheritance / Abstract)
// ============================================================================
class EnemyType
{
public:
    virtual ~EnemyType() {}
    virtual float getMaxHp() const = 0;
    virtual float getShootInterval() const = 0;
    virtual int getScoreValue() const = 0;
    virtual float getMoveSpeed() const = 0;
    virtual float getRadius() const = 0;
    virtual int getKind() const = 0;
    static EnemyType *Small;
    static EnemyType *Medium;
    static EnemyType *Boss;
};

class SmallEnemyType : public EnemyType
{
public:
    float getMaxHp() const { return 30.f; }
    float getShootInterval() const { return 2.2f; }
    int getScoreValue() const { return 50; }
    float getMoveSpeed() const { return 200.f; }
    float getRadius() const { return 16.f; }
    int getKind() const { return 0; }
};
class MediumEnemyType : public EnemyType
{
public:
    float getMaxHp() const { return 80.f; }
    float getShootInterval() const { return 1.8f; }
    int getScoreValue() const { return 120; }
    float getMoveSpeed() const { return 160.f; }
    float getRadius() const { return 22.f; }
    int getKind() const { return 1; }
};
class BossEnemyType : public EnemyType
{
public:
    float getMaxHp() const { return 1200.f; }
    float getShootInterval() const { return 0.8f; }
    int getScoreValue() const { return 2000; }
    float getMoveSpeed() const { return 80.f; }
    float getRadius() const { return 60.f; }
    int getKind() const { return 2; }
};

// ============================================================================
// SECTION 5: BULLETTYPE (Abstract base + subclasses)
// ============================================================================
class BulletType
{
public:
    virtual ~BulletType() {}
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
    int getId() const { return 0; }
    bool isPlayerBullet() const { return true; }
};
class PlayerWideBullet : public BulletType
{
public:
    int getId() const { return 1; }
    bool isPlayerBullet() const { return true; }
};
class PlayerTripleBullet : public BulletType
{
public:
    int getId() const { return 2; }
    bool isPlayerBullet() const { return true; }
};
class EnemyNormBullet : public BulletType
{
public:
    int getId() const { return 3; }
    bool isPlayerBullet() const { return false; }
};
class EnemyBurstBullet : public BulletType
{
public:
    int getId() const { return 4; }
    bool isPlayerBullet() const { return false; }
};
class BossBeamBullet : public BulletType
{
public:
    int getId() const { return 5; }
    bool isPlayerBullet() const { return false; }
};

// ============================================================================
// SECTION 6: PICKUPTYPE (Abstract base + subclasses)
// ============================================================================
class PickupType
{
public:
    virtual ~PickupType() {}
    virtual int getId() const = 0;
    virtual sf::Color getColor() const = 0;
    static PickupType *Score;
    static PickupType *Health;
    static PickupType *Power;
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
class PowerPickupType : public PickupType
{
public:
    int getId() const { return 2; }
    sf::Color getColor() const { return sf::Color(255, 200, 40); }
};

// ============================================================================
// SECTION 7: SCOREEVENT (with operator<< for output streams)
// ============================================================================
class ScoreEvent
{
public:
    std::string description;
    int points;
    float timestamp;
    ScoreEvent() : description(""), points(0), timestamp(0.f) {}
    ScoreEvent(const std::string &d, int p, float t)
        : description(d), points(p), timestamp(t) {}
    friend std::ostream &operator<<(std::ostream &os, const ScoreEvent &ev)
    {
        os << ev.description << " +" << ev.points;
        return os;
    }
};

// ============================================================================
// SECTION 8: HIGHSCORE (Encapsulation + Operator Overloading + Sorting)
// Demonstrates: private members, getters, operator< (used for sorting),
// operator<< for stream output.
// ============================================================================
class HighScore
{
private:
    std::string playerName;
    int score;
    int levelIndex;
public:
    HighScore() : playerName("---"), score(0), levelIndex(0) {}
    HighScore(const std::string &n, int s, int l)
        : playerName(n), score(s), levelIndex(l) {}

    std::string getName() const { return playerName; }
    int getScore() const { return score; }
    int getLevelIndex() const { return levelIndex; }

    void setName(const std::string &n) { playerName = n; }
    void setScore(int s) { score = s; }
    void setLevelIndex(int l) { levelIndex = l; }

    // Sort descending: a < b means a comes BEFORE b in a "highest first" list.
    bool operator<(const HighScore &o) const { return score > o.score; }
    bool operator>(const HighScore &o) const { return score < o.score; }
    bool operator==(const HighScore &o) const
    {
        return score == o.score && playerName == o.playerName;
    }

    friend std::ostream &operator<<(std::ostream &os, const HighScore &hs)
    {
        os << hs.playerName << " - " << hs.score;
        return os;
    }
};

// ============================================================================
// SECTION 9: PLAYER (Full Encapsulation — private fields + getters/setters
// + operator overloading for += score)
// ============================================================================
class Player
{
private:
    sf::Vector2f pos;
    float hp;
    float maxHp;
    float speed;
    int lives;
    int score;
    int power;
    float powerTimer;
    float shootTimer;
    float shootInterval;
    float iframeTimer;
    float shieldTimer;
    float tilt;
    bool alive;

public:
    Player()
        : pos(240.f, 600.f), hp(100.f), maxHp(100.f), speed(310.f),
          lives(3), score(0), power(1), powerTimer(0.f),
          shootTimer(0.f), shootInterval(0.10f),
          iframeTimer(0.f), shieldTimer(0.f), tilt(0.f), alive(true) {}

    // ---- Getters ----
    sf::Vector2f getPos() const { return pos; }
    float getHp() const { return hp; }
    float getMaxHp() const { return maxHp; }
    float getSpeed() const { return speed; }
    int getLives() const { return lives; }
    int getScore() const { return score; }
    int getPower() const { return power; }
    float getPowerTimer() const { return powerTimer; }
    float getShootTimer() const { return shootTimer; }
    float getShootInterval() const { return shootInterval; }
    float getIframeTimer() const { return iframeTimer; }
    float getShieldTimer() const { return shieldTimer; }
    float getTilt() const { return tilt; }
    bool isAlive() const { return alive; }

    // ---- Setters / mutators ----
    void setPos(sf::Vector2f p) { pos = p; }
    void setTilt(float t) { tilt = t; }
    void setShootTimer(float t) { shootTimer = t; }
    void setIframeTimer(float t) { iframeTimer = t; }
    void setShieldTimer(float t) { shieldTimer = t; }
    void setPowerTimer(float t) { powerTimer = t; }
    void setAlive(bool a) { alive = a; }

    // ---- Behaviour ----
    void move(sf::Vector2f delta) { pos += delta; }
    void clampToArena()
    {
        pos.x = clampf(pos.x, 24.f, 456.f);
        pos.y = clampf(pos.y, 24.f, 696.f);
    }
    void takeDamage(float d) { hp -= d; }
    void heal(float h)
    {
        hp = (hp + h > maxHp) ? maxHp : (hp + h);
    }
    void addScore(int s) { score += s; }
    void increasePower()
    {
        if (power < 3) power++;
    }
    void decreasePower()
    {
        if (power > 1) power--;
    }
    void loseLife() { lives--; }

    void tickPowerTimer(float dt) { powerTimer -= dt; }
    void tickShootTimer(float dt) { shootTimer -= dt; }
    void tickIframe(float dt) { iframeTimer -= dt; }
    void tickShield(float dt) { shieldTimer -= dt; }

    void resetForLevel()
    {
        pos = sf::Vector2f(240.f, 600.f);
        hp = maxHp;
        speed = 310.f;
        power = 1;
        powerTimer = 0.f;
        shootTimer = 0.f;
        iframeTimer = 0.f;
        shieldTimer = 0.f;
        tilt = 0.f;
        alive = true;
        score = 0;
        lives = 3;
    }

    // Operator overloading: adding score directly with += operator.
    Player &operator+=(int scoreBonus)
    {
        score += scoreBonus;
        return *this;
    }
};

// ============================================================================
// SECTION 10: SIMPLE GAMEPLAY DATA RECORDS
// (Bullet, Enemy, Particle, Pickup, Star — kept as plain structs because they
// are pure data containers managed by the templated ObjectPool.)
// ============================================================================
struct Bullet
{
    sf::Vector2f pos;
    sf::Vector2f vel;
    float dmg;
    BulletType *type;
    bool on;
    Bullet() : pos(0.f, 0.f), vel(0.f, 0.f), dmg(20.f), type(0), on(false) {}
};

struct Enemy
{
    sf::Vector2f pos;
    sf::Vector2f vel;
    float hp;
    float maxHp;
    EnemyType *type;
    bool on;
    float shootTimer;
    float shootInterval;
    std::vector<sf::Vector2f> path;
    int pathIndex;
    float angle;
    float pulse;
    int phase;
    int scoreValue;
    float moveSpeed;
    Enemy()
        : pos(0.f, 0.f), vel(0.f, 0.f), hp(30.f), maxHp(30.f), type(0),
          on(false), shootTimer(0.f), shootInterval(2.2f),
          pathIndex(0), angle(0.f), pulse(0.f), phase(0),
          scoreValue(50), moveSpeed(200.f) {}
};

struct Particle
{
    sf::Vector2f pos;
    sf::Vector2f vel;
    sf::Color colorStart;
    sf::Color colorEnd;
    float life;
    float maxLife;
    float size;
    bool on;
    Particle()
        : pos(0.f, 0.f), vel(0.f, 0.f),
          colorStart(sf::Color::White), colorEnd(sf::Color::Black),
          life(0.f), maxLife(0.f), size(3.f), on(false) {}
};

struct Pickup
{
    sf::Vector2f pos;
    sf::Vector2f vel;
    PickupType *type;
    float life;
    float pulse;
    bool on;
    Pickup() : pos(0.f, 0.f), vel(0.f, 0.f), type(0),
               life(8.f), pulse(0.f), on(false) {}
};

struct SpawnEvent
{
    float time;
    EnemyType *etype;
    sf::Vector2f startPos;
    std::vector<sf::Vector2f> path;
    int count;
    float xSpacing;
    float delay;
    bool fired;
    SpawnEvent() : time(0.f), etype(0), startPos(0.f, 0.f),
                   count(1), xSpacing(55.f), delay(0.25f), fired(false) {}
};

struct SpawnJob
{
    EnemyType *etype;
    sf::Vector2f startPos;
    float delayRemaining;
    std::vector<sf::Vector2f> path;
    float hp;
    float maxHp;
    float shootInterval;
    int scoreValue;
    float moveSpeed;
    SpawnJob() : etype(0), startPos(0.f, 0.f), delayRemaining(0.f),
                 hp(30.f), maxHp(30.f), shootInterval(2.2f),
                 scoreValue(50), moveSpeed(200.f) {}
};

struct Star
{
    sf::Vector2f pos;
    float speed;
    float brightness;
    float size;
    Star() : pos(0.f, 0.f), speed(20.f), brightness(0.5f), size(1.f) {}
};

// ============================================================================
// SECTION 11: POOL SIZE CONSTANTS
// ============================================================================
const int MAX_BULLETS = 700;
const int MAX_ENEMIES = 80;
const int MAX_PARTICLES = 1000;
const int MAX_PICKUPS = 64;
const int MAX_STARS = 200;

// ============================================================================
// SECTION 12: TEMPLATED OBJECT POOL
// (Templates topic — generic fixed-capacity pool for Bullet/Enemy/Particle/
// Pickup. Each pooled item must expose a public bool `on` flag.)
// ============================================================================
template <class T, int N>
class ObjectPool
{
private:
    std::array<T, N> items;

public:
    ObjectPool()
    {
        for (int i = 0; i < N; i++)
            items[i].on = false;
    }
    int capacity() const { return N; }
    T &at(int i) { return items[i]; }
    const T &at(int i) const { return items[i]; }

    T *alloc()
    {
        for (int i = 0; i < N; i++)
        {
            if (!items[i].on)
                return &items[i];
        }
        return 0;
    }
    void clearAll()
    {
        for (int i = 0; i < N; i++)
            items[i].on = false;
    }
    int countActive() const
    {
        int c = 0;
        for (int i = 0; i < N; i++)
            if (items[i].on)
                c++;
        return c;
    }
};

// ============================================================================
// SECTION 13: LEVEL CLASS HIERARCHY
// (Inheritance + Abstract Class + virtual function dispatch.)
// ============================================================================
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
    std::vector<std::vector<sf::Vector2f> > normalPathPool;
    std::vector<std::vector<sf::Vector2f> > bossPathPool;
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

// ============================================================================
// SECTION 14: GAME CLASS
// ============================================================================
class Game
{
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

    ObjectPool<Bullet, MAX_BULLETS> bullets;
    ObjectPool<Enemy, MAX_ENEMIES> enemies;
    ObjectPool<Particle, MAX_PARTICLES> particles;
    ObjectPool<Pickup, MAX_PICKUPS> pickups;

    std::array<Star, MAX_STARS> stars;

    std::vector<Level *> levels; // raw pointers — destructor deletes them
    int currentLevel;
    int selectedLevel;
    float levelTimer;

    std::queue<SpawnJob> spawnQueue;
    std::list<ScoreEvent> scoreLog;
    std::vector<HighScore> highScores;

    float bgY;
    bool bossActive;

    // ---- Asset loading helpers (throw AssetLoadException on failure) ----
    void loadFontOrThrow(const std::string &path);
    void loadTextureOrThrow(sf::Texture &tex, const std::string &path);

    // ---- Core loop ----
    void handleEvents();
    void update(float dt);
    void render();

    // ---- Level / player flow ----
    void startLevel(int index);
    void resetPlayer();

    // ---- Game logic ----
    void checkCollisions();
    void checkSpawns(float dt);
    void spawnExplosion(sf::Vector2f pos, sf::Color cStart, sf::Color cEnd, int count);
    void spawnPickup(sf::Vector2f pos, PickupType *ptype);

    void spawnPlayerBullet();
    void spawnEnemyBullet(Enemy &e);
    void spawnEnemyFromJob(const SpawnJob &job);

    // ---- Rendering ----
    void renderBackground();
    void renderStars();
    void renderPlayer();
    void renderEnemies();
    void renderBullets();
    void renderParticles();
    void renderPickups();
    void renderHUD();
    void renderBossHP();
    void renderHomeScreen();
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

    // ---- High score table (Sorting topic) ----
    void recordHighScore(int score, int levelIndex);
    void sortHighScores();   // iterative insertion sort (no recursion)
    void loadHighScores();
    void saveHighScores();

public:
    Game();
    ~Game();
    void init();
    void run();
};
