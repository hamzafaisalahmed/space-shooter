#pragma once
#include <SFML/Graphics.hpp> // SFML: window creation, shapes, sprites, text, render textures
#include <array>             // std::array: fixed-size pools for bullets, enemies, particles, pickups
#include <vector>            // std::vector: dynamic lists for level data and waypoint paths
#include <queue>             // std::queue: FIFO queue for staggered enemy spawning
#include <stack>             // std::stack: LIFO stack for game state management (push/pop states)
#include <list>              // std::list: linked list for the score event log (fast front insertion)
#include <cmath>             // std::sqrt, std::sin, std::cos, std::atan2, std::hypot
#include <cstdlib>           // std::rand, std::srand for random number generation
#include <ctime>             // std::time for seeding the random number generator
#include <sstream>           // std::ostringstream for building formatted strings (not heavily used)
#include <iostream>          // std::cerr for error messages (e.g. font load failure)
#include <algorithm>         // std::min, std::max for clamping values
#include <string>            // std::string for text labels and descriptions

// ============================================================================
// SECTION 1: ENUMS
// ============================================================================
// Enums define named constants grouped by category. Using "enum class" instead
// of plain "enum" prevents name collisions (e.g. you can't accidentally compare
// a GameState to an EnemyType). Each value is scoped: GameState::Playing, not
// just Playing.
// ============================================================================

// GameState: controls which screen is active. Only one state is on top of the
// stack at any time. The game loop checks this every frame to decide whether
// to run update() or just render a menu/overlay.
enum class GameState
{
    LevelSelect,  // Main menu: planet picker, play/back buttons
    Playing,      // Active gameplay: all systems update every frame
    Paused,       // Gameplay frozen, semi-transparent overlay drawn on top
    GameOver,     // Player lost all 3 lives, score log shown
    LevelComplete // Boss defeated, level cleared, click to return to menu
};

// EnemyType: determines an enemy's stats, visual design, and shooting pattern.
// Small = weak cannon fodder, Medium = tougher with spread shots, Boss = level-ender.
enum class EnemyType
{
    Small,  // 30 HP, single aimed shot, fast movement (200 px/s)
    Medium, // 80 HP, 3-way spread shot, slower movement (160 px/s)
    Boss    // 1200 HP, multi-way fan shot, two phases, oscillates side-to-side
};

// BulletType: identifies who fired the bullet and what visual/damage it uses.
// Player bullets travel upward, enemy bullets travel toward the player.
enum class BulletType
{
    PlayerNorm,   // Single shot, power level 1
    PlayerWide,   // Dual shot spread, power level 2
    PlayerTriple, // Triple shot with slight angle spread, power level 3
    EnemyNorm,    // Single red orb aimed at player (fired by Small enemies)
    EnemyBurst,   // Part of a 3-way fan (fired by Medium enemies and Boss)
    BossBeam      // Centre bullet in boss fan shot, wider and blue/white
};

// PickupType: the kind of orb dropped by dead enemies. Player collects by contact.
enum class PickupType
{
    Score,  // Blue orb: +25 points instantly, most common drop
    Health, // Green orb: restores 25 HP (capped at max)
    Power   // Gold orb: raises bullet power level by 1, grants shield, lasts 8 seconds
};

// ============================================================================
// SECTION 2: MATH UTILITY FUNCTIONS
// ============================================================================
// Small helper functions used throughout the game for vector math, interpolation,
// color blending, clamping, and random numbers. All are "inline" so the compiler
// can paste their code directly at call sites to avoid function-call overhead.
// ============================================================================

// vlen: returns the length (magnitude) of a 2D vector.
// Formula: sqrt(x^2 + y^2), the Pythagorean theorem.
// Used to measure distances between objects for collision detection and
// to check if an enemy has reached its waypoint.
inline float vlen(sf::Vector2f v)
{
    return std::sqrt(v.x * v.x + v.y * v.y); // standard distance formula
}

// vnorm: returns a unit vector (length = 1) pointing in the same direction as v.
// Divides each component by the vector's length.
// Guard: if the length is near zero, return {0,0} to avoid division by zero.
// Used to get a direction vector for movement and bullet aiming.
inline sf::Vector2f vnorm(sf::Vector2f v)
{
    float l = vlen(v); // get the length first
    if (l < 0.0001f)
        return {0.f, 0.f};     // near-zero vector has no direction
    return {v.x / l, v.y / l}; // divide each component by length
}

// lerp: linear interpolation between two floats.
// When t=0 returns a, when t=1 returns b, when t=0.5 returns the midpoint.
// Used for smooth visual transitions like the player ship tilt angle.
inline float lerp(float a, float b, float t)
{
    return a + (b - a) * t; // standard lerp formula
}

// colorLerp: linear interpolation between two SFML colors.
// Blends each channel (R, G, B, A) independently using the same lerp formula.
// t is clamped to [0,1] to prevent invalid color values.
// Used by the particle system to fade from startColor to endColor over lifetime.
inline sf::Color colorLerp(sf::Color a, sf::Color b, float t)
{
    t = std::max(0.f, std::min(1.f, t)); // clamp t to [0, 1] range
    return sf::Color(
        (sf::Uint8)(a.r + (b.r - a.r) * t), // red channel
        (sf::Uint8)(a.g + (b.g - a.g) * t), // green channel
        (sf::Uint8)(a.b + (b.b - a.b) * t), // blue channel
        (sf::Uint8)(a.a + (b.a - a.a) * t)  // alpha (transparency) channel
    );
}

// clampf: restricts a float value to the range [lo, hi].
// If val < lo, returns lo. If val > hi, returns hi. Otherwise returns val.
// Used to keep the player ship inside the screen boundaries.
inline float clampf(float val, float lo, float hi)
{
    if (val < lo)
        return lo; // below minimum, snap to minimum
    if (val > hi)
        return hi; // above maximum, snap to maximum
    return val;    // already in range, return unchanged
}

// randFloat: generates a random float between lo and hi (inclusive).
// Uses C's rand() which returns an int in [0, RAND_MAX].
// Dividing by RAND_MAX gives [0.0, 1.0], then we scale to [lo, hi].
// Used for particle velocities, random offsets, spawn delays, etc.
inline float randFloat(float lo, float hi)
{
    float r = (float)rand() / (float)RAND_MAX; // normalize to [0, 1]
    return lo + r * (hi - lo);                 // scale to [lo, hi]
}

// randInt: generates a random integer between lo and hi (inclusive).
// Uses modulo to limit the range. Not perfectly uniform for large ranges,
// but fine for game purposes (small ranges like 0-10).
inline int randInt(int lo, int hi)
{
    if (hi <= lo)
        return lo;                      // edge case: no range
    return lo + rand() % (hi - lo + 1); // modulo gives [0, hi-lo], then shift by lo
}

// ============================================================================
// SECTION 3: STRUCTS (Data Definitions for Game Objects)
// ============================================================================
// Each game object is a struct: a custom data type that groups related variables.
// Structs are like classes but with all members public by default.
// We use structs for data containers and a class for the main Game logic.
// ============================================================================

// Player: the player's ship and all its gameplay state.
// There is exactly one Player instance in the game, owned by the Game class.
struct Player
{
    sf::Vector2f pos{240.f, 600.f}; // position on screen (pixels). Starts at bottom-center.
    float hp = 100.f;               // current health points
    float maxHp = 100.f;            // maximum health points (HP bar uses hp/maxHp ratio)
    float speed = 310.f;            // movement speed in pixels per second
    int lives = 3;                  // lives remaining. Lose one when HP hits 0. Game over at 0 lives.
    int score = 0;                  // points earned this level from kills and pickups
    int power = 1;                  // current bullet power level: 1=single, 2=double, 3=triple
    float powerTimer = 0.f;         // seconds remaining until power level drops back down by 1
    float shootTimer = 0.f;         // countdown to next auto-fire (resets to shootInterval)
    float shootInterval = 0.10f;    // seconds between auto-fire shots (10 shots per second)
    float iframeTimer = 0.f;        // invincibility time remaining after being hit (seconds)
    float shieldTimer = 0.f;        // shield active time remaining (absorbs one hit)
    float tilt = 0.f;               // visual lean angle in degrees (-18 to +18), smoothly interpolated
    bool alive = true;              // false when all lives are lost (triggers Game Over)
};

// Bullet: a projectile fired by either the player or an enemy.
// Stored in a fixed-size pool (array of 700). The "on" flag marks if the slot is in use.
struct Bullet
{
    sf::Vector2f pos;                          // current position on screen
    sf::Vector2f vel;                          // velocity in pixels per second (direction + speed combined)
    float dmg = 20.f;                          // damage dealt on hit
    BulletType btype = BulletType::PlayerNorm; // identifies the bullet type for collision and rendering
    bool on = false;                           // false = this slot is free and can be reused
};

// Enemy: an enemy ship. Stored in a pool of 80.
// Each enemy follows a path of waypoints, shoots at intervals, and drops pickups on death.
struct Enemy
{
    sf::Vector2f pos;                   // current position
    sf::Vector2f vel;                   // velocity (not always used directly; path following sets movement)
    float hp = 30.f;                    // current health
    float maxHp = 30.f;                 // max health (for HP bar percentage calculation)
    EnemyType etype = EnemyType::Small; // determines stats, visuals, and shooting pattern
    bool on = false;                    // false = pool slot is free
    float shootTimer = 0.f;             // countdown to next shot
    float shootInterval = 2.2f;         // seconds between shots (varies by enemy type)
    std::vector<sf::Vector2f> path;     // waypoints to follow in order
    int pathIndex = 0;                  // index of the current target waypoint in the path vector
    float angle = 0.f;                  // visual rotation in degrees (non-boss enemies rotate toward movement)
    float pulse = 0.f;                  // time accumulator for animation effects (engine flicker, bobbing)
    int phase = 0;                      // boss only: 0 = normal, 1 = enraged (below 40% HP)
    int scoreValue = 50;                // points awarded to player when this enemy is destroyed
    float moveSpeed = 200.f;            // movement speed in pixels per second along the path
};

// Particle: a single spark/debris piece in an explosion effect.
// Stored in a pool of 1000. Particles fade from colorStart to colorEnd over their lifetime.
struct Particle
{
    sf::Vector2f pos;     // current position
    sf::Vector2f vel;     // velocity (decays each frame via drag multiplier)
    sf::Color colorStart; // color at birth (full opacity, bright)
    sf::Color colorEnd;   // color at death (usually transparent, darker)
    float life = 0.f;     // seconds remaining before deactivation
    float maxLife = 0.f;  // total lifetime (used to calculate fade progress: t = 1 - life/maxLife)
    float size = 3.f;     // radius of the circle shape drawn for this particle
    bool on = false;      // false = pool slot is free
};

// Pickup: a collectible orb dropped by dead enemies.
// Stored in a pool of 64. Player collects by proximity (radius 32 + 18 = 50 pixels).
struct Pickup
{
    sf::Vector2f pos;                     // current position
    sf::Vector2f vel;                     // velocity (slight upward burst on spawn, then gravity pulls down)
    PickupType ptype = PickupType::Score; // what effect collecting this pickup has
    float life = 8.f;                     // seconds before it disappears if not collected
    float pulse = 0.f;                    // time accumulator for bobbing animation
    bool on = false;                      // false = pool slot is free
};

// SpawnEvent: one entry in a level's wave script.
// Defines WHEN enemies spawn, WHAT type, WHERE they start, and WHAT path they follow.
// The level timer is checked every frame; when it passes event.time, the wave fires.
struct SpawnEvent
{
    float time;                     // seconds from level start when this wave triggers
    EnemyType etype;                // which enemy type to spawn
    sf::Vector2f startPos;          // screen position where the first enemy appears
    std::vector<sf::Vector2f> path; // waypoints the enemies will follow
    int count = 1;                  // how many enemies in this formation
    float xSpacing = 55.f;          // horizontal pixel gap between formation members
    float delay = 0.25f;            // seconds between each ship in the formation (stagger)
    bool fired = false;             // true once this event has been triggered (prevents re-firing)
};

// SpawnJob: a single pending enemy spawn sitting in the spawn queue.
// When a SpawnEvent fires, it pushes one SpawnJob per enemy into the queue
// with staggered delays. The queue is drained each frame.
struct SpawnJob
{
    EnemyType etype;                // which enemy type
    sf::Vector2f startPos;          // where it will appear
    float delayRemaining;           // seconds until this enemy actually spawns
    std::vector<sf::Vector2f> path; // waypoints to follow
    float hp = 30.f;                // starting HP
    float maxHp = 30.f;             // max HP
    float shootInterval = 2.2f;     // seconds between shots
    int scoreValue = 50;            // points on kill
    float moveSpeed = 200.f;        // pixels per second
};

// LevelData: all data for one level (Aries, Taurus, or Gemini).
// Contains the wave script (vector of SpawnEvents) and metadata.
struct LevelData
{
    std::string name;               // display name ("ARIES", "TAURUS", "GEMINI")
    std::string desc;               // short subtitle
    sf::Color planetColor;          // color of the planet shown in background and level select
    float duration = 60.f;          // expected level duration in seconds (informational)
    std::vector<SpawnEvent> events; // the wave script: all spawn events in time order
    bool locked = true;             // true = player hasn't unlocked this level yet
};

// ScoreEvent: one entry in the score log (linked list).
// Records what happened (description), how many points, and when.
// Displayed on the Game Over screen.
struct ScoreEvent
{
    std::string description; // e.g. "Small enemy", "BOSS", "Score orb"
    int points;              // e.g. 50, 2000, 25
    float timestamp;         // seconds since level start when this happened
};

// Star: a single star in the parallax star field.
// 200 stars are split into 3 speed layers for depth illusion.
struct Star
{
    sf::Vector2f pos; // position on screen
    float speed;      // scroll speed in pixels per second (varies by layer)
    float brightness; // alpha multiplier (0.0 to 1.0) - distant stars are dimmer
    float size;       // circle radius in pixels - distant stars are smaller
};

// ============================================================================
// SECTION 4: POOL SIZE CONSTANTS
// ============================================================================
// These define how many objects can exist simultaneously. Using fixed pools
// (std::array) avoids dynamic memory allocation during gameplay, which
// would cause frame stutters. Each pool slot has an "on" flag; false = free.
// ============================================================================

const int MAX_BULLETS = 700;    // player + all enemy bullets combined
const int MAX_ENEMIES = 80;     // max simultaneous enemy ships on screen
const int MAX_PARTICLES = 1000; // explosion sparks, debris, smoke effects
const int MAX_PICKUPS = 64;     // power-up orbs on screen at once
const int MAX_STARS = 200;      // background star count (cosmetic, never changes)

// ============================================================================
// SECTION 5: GAME CLASS DECLARATION
// ============================================================================
// The Game class is the heart of the program. It owns ALL game data and
// contains ALL game logic. There is exactly one instance created in main().
//
// Public interface is minimal: init() and run().
//   - init() sets up the window, loads the font, builds levels, creates the background.
//   - run() is the main loop: handle events, update, render, repeat until window closes.
//
// Everything else is private: the update logic, rendering, collision checks,
// spawning, pool allocators, and drawing helpers.
// ============================================================================

class Game
{
public:
    void init(); // called once at startup to set up everything
    void run();  // main loop, runs until the window is closed

private:
    // --- Core SFML objects ---
    sf::RenderWindow window; // the OS window that displays the game
    sf::Font font;           // font used for all text rendering (loaded from .ttf file)
    sf::Texture shipTexture;
    sf::Sprite shipSprite;
    sf::View gameView;
    // --- State machine ---
    // The state stack manages which screen is active. Pushing a new state
    // puts it on top (e.g. Paused on top of Playing). Popping removes the
    // top state and returns to the one below (e.g. back to Playing).
    std::stack<GameState> stateStack;

    // --- Entity pools ---
    // Fixed-size arrays act as object pools. Each slot has an "on" flag.
    // To "create" an object, find a slot with on=false, fill it, set on=true.
    // To "destroy" an object, set on=false. No memory allocation needed.
    Player player;                                 // the one and only player ship
    std::array<Bullet, MAX_BULLETS> bullets;       // bullet pool (player + enemy bullets mixed)
    std::array<Enemy, MAX_ENEMIES> enemies;        // enemy pool
    std::array<Particle, MAX_PARTICLES> particles; // particle pool for explosions
    std::array<Pickup, MAX_PICKUPS> pickups;       // pickup orb pool

    // --- Star field ---
    std::array<Star, MAX_STARS> stars; // parallax background stars in 3 speed layers

    // --- Level data ---
    std::vector<LevelData> levels; // all 3 levels (Aries, Taurus, Gemini)
    int currentLevel = 0;          // index of the level currently being played
    int selectedLevel = 0;         // index of the level selected on the Level Select screen
    float levelTimer = 0.f;        // seconds elapsed since the current level started

    // --- Spawn queue ---
    // When a wave triggers, SpawnJobs are pushed into this FIFO queue with
    // staggered delays. Each frame, the front job's delay is decremented.
    // When it hits 0, the enemy is created and the job is popped.
    std::queue<SpawnJob> spawnQueue;

    // --- Score log ---
    // Every kill and pickup adds a ScoreEvent to the front of this linked list.
    // Displayed on the Game Over screen (most recent first).
    std::list<ScoreEvent> scoreLog;

    // --- Background ---
    sf::RenderTexture stationTile; // pre-rendered space station background (480x1440 px)
    float bgY = 0.f;               // current vertical scroll offset for the background

    // --- Boss tracking ---
    bool bossActive = false; // true if any boss enemy is alive (triggers boss HP bar rendering)

    // --- Private methods (declared here, defined below) ---

    // Core loop phases
    void handleEvents();   // process OS events (close, key press, mouse click)
    void update(float dt); // update all game logic for one frame
    void render();         // draw everything to the screen

    // Game flow
    void startLevel(int index); // reset everything and begin a level
    void resetPlayer();         // reset the player struct to starting values

    // Gameplay systems
    void checkCollisions();                                                             // test all collision pairs and apply results
    void checkSpawns(float dt);                                                         // check if any wave events should fire this frame
    void spawnExplosion(sf::Vector2f pos, sf::Color cStart, sf::Color cEnd, int count); // burst of particles
    void spawnPickup(sf::Vector2f pos, PickupType ptype);                               // create a pickup orb

    // Pool allocators: scan the pool for a free slot (on=false) and return a pointer.
    // Returns nullptr if the pool is full (all slots in use).
    Bullet *allocBullet();
    Enemy *allocEnemy();
    Particle *allocParticle();
    Pickup *allocPickup();

    // Spawn helpers
    void spawnPlayerBullet();                    // fire bullet(s) based on current power level
    void spawnEnemyBullet(Enemy &e);             // fire bullet(s) based on enemy type
    void spawnEnemyFromJob(const SpawnJob &job); // create an enemy from a queued job

    // Rendering sub-functions (each draws one category of objects)
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

    // Ship drawing helpers: each builds a ship shape from ConvexShapes/CircleShapes
    // and draws it at the given position with rotation and effects.
    void drawPlayerShip(sf::Vector2f pos, float tilt, float scale, sf::Color tint);
    void drawSmallEnemy(sf::Vector2f pos, float angle, float pulse);
    void drawMediumEnemy(sf::Vector2f pos, float angle, float pulse);
    void drawBossEnemy(sf::Vector2f pos, float angle, float pulse, int phase);

    // Initialization helpers
    void buildStationTile(); // generate the scrolling station background texture
    void initStars();        // randomize star positions, speeds, and sizes
    void buildLevels();      // create all 3 levels with their wave scripts

    // Utility
    void drawTextCentered(const std::string &str, float y, int size, sf::Color col); // draw centered text
    bool isLevelClear();                                                             // check if all enemies are gone and all waves have fired
    int countActiveEnemies();                                                        // count how many enemies have on=true
};