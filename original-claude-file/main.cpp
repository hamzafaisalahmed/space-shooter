// ============================================================================
// SPACE SHOOTER - Complete Game in One File
// ============================================================================
// A vertical-scrolling shoot-em-up (shmup) built with SFML 2.x and C++17.
// The player pilots a spaceship, fights waves of enemies across 3 levels,
// each ending with a boss fight. All visuals are drawn with SFML shapes
// (no image/sprite files needed, only a .ttf font).
//
// Architecture overview:
//   - Enums define all game categories (states, enemy types, bullet types, etc.)
//   - Structs define data for every game object (Player, Enemy, Bullet, etc.)
//   - One "Game" class owns everything and runs the main loop
//   - Object pools (std::array) are used for bullets, enemies, particles, pickups
//   - A spawn queue (std::queue) staggers enemy wave entries
//   - A state stack (std::stack) manages game states (menu, playing, paused, etc.)
//   - A score log (std::list) records point events for the Game Over screen
// ============================================================================

// --- Standard library includes ---
#include <SFML/Graphics.hpp>  // SFML: window creation, shapes, sprites, text, render textures
#include <array>              // std::array: fixed-size pools for bullets, enemies, particles, pickups
#include <vector>             // std::vector: dynamic lists for level data and waypoint paths
#include <queue>              // std::queue: FIFO queue for staggered enemy spawning
#include <stack>              // std::stack: LIFO stack for game state management (push/pop states)
#include <list>               // std::list: linked list for the score event log (fast front insertion)
#include <cmath>              // std::sqrt, std::sin, std::cos, std::atan2, std::hypot
#include <cstdlib>            // std::rand, std::srand for random number generation
#include <ctime>              // std::time for seeding the random number generator
#include <sstream>            // std::ostringstream for building formatted strings (not heavily used)
#include <iostream>           // std::cerr for error messages (e.g. font load failure)
#include <algorithm>          // std::min, std::max for clamping values
#include <string>             // std::string for text labels and descriptions


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
enum class GameState {
    LevelSelect,    // Main menu: planet picker, play/back buttons
    Playing,        // Active gameplay: all systems update every frame
    Paused,         // Gameplay frozen, semi-transparent overlay drawn on top
    GameOver,       // Player lost all 3 lives, score log shown
    LevelComplete   // Boss defeated, level cleared, click to return to menu
};

// EnemyType: determines an enemy's stats, visual design, and shooting pattern.
// Small = weak cannon fodder, Medium = tougher with spread shots, Boss = level-ender.
enum class EnemyType {
    Small,   // 30 HP, single aimed shot, fast movement (200 px/s)
    Medium,  // 80 HP, 3-way spread shot, slower movement (160 px/s)
    Boss     // 1200 HP, multi-way fan shot, two phases, oscillates side-to-side
};

// BulletType: identifies who fired the bullet and what visual/damage it uses.
// Player bullets travel upward, enemy bullets travel toward the player.
enum class BulletType {
    PlayerNorm,   // Single shot, power level 1
    PlayerWide,   // Dual shot spread, power level 2
    PlayerTriple, // Triple shot with slight angle spread, power level 3
    EnemyNorm,    // Single red orb aimed at player (fired by Small enemies)
    EnemyBurst,   // Part of a 3-way fan (fired by Medium enemies and Boss)
    BossBeam      // Centre bullet in boss fan shot, wider and blue/white
};

// PickupType: the kind of orb dropped by dead enemies. Player collects by contact.
enum class PickupType {
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
inline float vlen(sf::Vector2f v) {
    return std::sqrt(v.x * v.x + v.y * v.y); // standard distance formula
}

// vnorm: returns a unit vector (length = 1) pointing in the same direction as v.
// Divides each component by the vector's length.
// Guard: if the length is near zero, return {0,0} to avoid division by zero.
// Used to get a direction vector for movement and bullet aiming.
inline sf::Vector2f vnorm(sf::Vector2f v) {
    float l = vlen(v);                          // get the length first
    if (l < 0.0001f) return {0.f, 0.f};        // near-zero vector has no direction
    return {v.x / l, v.y / l};                  // divide each component by length
}

// lerp: linear interpolation between two floats.
// When t=0 returns a, when t=1 returns b, when t=0.5 returns the midpoint.
// Used for smooth visual transitions like the player ship tilt angle.
inline float lerp(float a, float b, float t) {
    return a + (b - a) * t; // standard lerp formula
}

// colorLerp: linear interpolation between two SFML colors.
// Blends each channel (R, G, B, A) independently using the same lerp formula.
// t is clamped to [0,1] to prevent invalid color values.
// Used by the particle system to fade from startColor to endColor over lifetime.
inline sf::Color colorLerp(sf::Color a, sf::Color b, float t) {
    t = std::max(0.f, std::min(1.f, t));    // clamp t to [0, 1] range
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
inline float clampf(float val, float lo, float hi) {
    if (val < lo) return lo;  // below minimum, snap to minimum
    if (val > hi) return hi;  // above maximum, snap to maximum
    return val;               // already in range, return unchanged
}

// randFloat: generates a random float between lo and hi (inclusive).
// Uses C's rand() which returns an int in [0, RAND_MAX].
// Dividing by RAND_MAX gives [0.0, 1.0], then we scale to [lo, hi].
// Used for particle velocities, random offsets, spawn delays, etc.
inline float randFloat(float lo, float hi) {
    float r = (float)rand() / (float)RAND_MAX; // normalize to [0, 1]
    return lo + r * (hi - lo);                  // scale to [lo, hi]
}

// randInt: generates a random integer between lo and hi (inclusive).
// Uses modulo to limit the range. Not perfectly uniform for large ranges,
// but fine for game purposes (small ranges like 0-10).
inline int randInt(int lo, int hi) {
    if (hi <= lo) return lo;                // edge case: no range
    return lo + rand() % (hi - lo + 1);     // modulo gives [0, hi-lo], then shift by lo
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
struct Player {
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
struct Bullet {
    sf::Vector2f pos;                             // current position on screen
    sf::Vector2f vel;                             // velocity in pixels per second (direction + speed combined)
    float dmg = 20.f;                             // damage dealt on hit
    BulletType btype = BulletType::PlayerNorm;    // identifies the bullet type for collision and rendering
    bool on = false;                              // false = this slot is free and can be reused
};

// Enemy: an enemy ship. Stored in a pool of 80.
// Each enemy follows a path of waypoints, shoots at intervals, and drops pickups on death.
struct Enemy {
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
struct Particle {
    sf::Vector2f pos;          // current position
    sf::Vector2f vel;          // velocity (decays each frame via drag multiplier)
    sf::Color colorStart;      // color at birth (full opacity, bright)
    sf::Color colorEnd;        // color at death (usually transparent, darker)
    float life = 0.f;          // seconds remaining before deactivation
    float maxLife = 0.f;       // total lifetime (used to calculate fade progress: t = 1 - life/maxLife)
    float size = 3.f;          // radius of the circle shape drawn for this particle
    bool on = false;           // false = pool slot is free
};

// Pickup: a collectible orb dropped by dead enemies.
// Stored in a pool of 64. Player collects by proximity (radius 32 + 18 = 50 pixels).
struct Pickup {
    sf::Vector2f pos;                      // current position
    sf::Vector2f vel;                      // velocity (slight upward burst on spawn, then gravity pulls down)
    PickupType ptype = PickupType::Score;  // what effect collecting this pickup has
    float life = 8.f;                      // seconds before it disappears if not collected
    float pulse = 0.f;                     // time accumulator for bobbing animation
    bool on = false;                       // false = pool slot is free
};

// SpawnEvent: one entry in a level's wave script.
// Defines WHEN enemies spawn, WHAT type, WHERE they start, and WHAT path they follow.
// The level timer is checked every frame; when it passes event.time, the wave fires.
struct SpawnEvent {
    float time;                        // seconds from level start when this wave triggers
    EnemyType etype;                   // which enemy type to spawn
    sf::Vector2f startPos;             // screen position where the first enemy appears
    std::vector<sf::Vector2f> path;    // waypoints the enemies will follow
    int count = 1;                     // how many enemies in this formation
    float xSpacing = 55.f;            // horizontal pixel gap between formation members
    float delay = 0.25f;              // seconds between each ship in the formation (stagger)
    bool fired = false;               // true once this event has been triggered (prevents re-firing)
};

// SpawnJob: a single pending enemy spawn sitting in the spawn queue.
// When a SpawnEvent fires, it pushes one SpawnJob per enemy into the queue
// with staggered delays. The queue is drained each frame.
struct SpawnJob {
    EnemyType etype;               // which enemy type
    sf::Vector2f startPos;         // where it will appear
    float delayRemaining;          // seconds until this enemy actually spawns
    std::vector<sf::Vector2f> path;// waypoints to follow
    float hp = 30.f;               // starting HP
    float maxHp = 30.f;            // max HP
    float shootInterval = 2.2f;    // seconds between shots
    int scoreValue = 50;           // points on kill
    float moveSpeed = 200.f;       // pixels per second
};

// LevelData: all data for one level (Aries, Taurus, or Gemini).
// Contains the wave script (vector of SpawnEvents) and metadata.
struct LevelData {
    std::string name;              // display name ("ARIES", "TAURUS", "GEMINI")
    std::string desc;              // short subtitle
    sf::Color planetColor;         // color of the planet shown in background and level select
    float duration = 60.f;         // expected level duration in seconds (informational)
    std::vector<SpawnEvent> events;// the wave script: all spawn events in time order
    bool locked = true;            // true = player hasn't unlocked this level yet
};

// ScoreEvent: one entry in the score log (linked list).
// Records what happened (description), how many points, and when.
// Displayed on the Game Over screen.
struct ScoreEvent {
    std::string description; // e.g. "Small enemy", "BOSS", "Score orb"
    int points;              // e.g. 50, 2000, 25
    float timestamp;         // seconds since level start when this happened
};

// Star: a single star in the parallax star field.
// 200 stars are split into 3 speed layers for depth illusion.
struct Star {
    sf::Vector2f pos;   // position on screen
    float speed;        // scroll speed in pixels per second (varies by layer)
    float brightness;   // alpha multiplier (0.0 to 1.0) - distant stars are dimmer
    float size;         // circle radius in pixels - distant stars are smaller
};


// ============================================================================
// SECTION 4: POOL SIZE CONSTANTS
// ============================================================================
// These define how many objects can exist simultaneously. Using fixed pools
// (std::array) avoids dynamic memory allocation during gameplay, which
// would cause frame stutters. Each pool slot has an "on" flag; false = free.
// ============================================================================

const int MAX_BULLETS   = 700;  // player + all enemy bullets combined
const int MAX_ENEMIES   = 80;   // max simultaneous enemy ships on screen
const int MAX_PARTICLES = 1000; // explosion sparks, debris, smoke effects
const int MAX_PICKUPS   = 64;   // power-up orbs on screen at once
const int MAX_STARS     = 200;  // background star count (cosmetic, never changes)


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

class Game {
public:
    void init(); // called once at startup to set up everything
    void run();  // main loop, runs until the window is closed

private:
    // --- Core SFML objects ---
    sf::RenderWindow window; // the OS window that displays the game
    sf::Font font;           // font used for all text rendering (loaded from .ttf file)

    // --- State machine ---
    // The state stack manages which screen is active. Pushing a new state
    // puts it on top (e.g. Paused on top of Playing). Popping removes the
    // top state and returns to the one below (e.g. back to Playing).
    std::stack<GameState> stateStack;

    // --- Entity pools ---
    // Fixed-size arrays act as object pools. Each slot has an "on" flag.
    // To "create" an object, find a slot with on=false, fill it, set on=true.
    // To "destroy" an object, set on=false. No memory allocation needed.
    Player player;                                   // the one and only player ship
    std::array<Bullet, MAX_BULLETS> bullets;          // bullet pool (player + enemy bullets mixed)
    std::array<Enemy, MAX_ENEMIES> enemies;           // enemy pool
    std::array<Particle, MAX_PARTICLES> particles;    // particle pool for explosions
    std::array<Pickup, MAX_PICKUPS> pickups;          // pickup orb pool

    // --- Star field ---
    std::array<Star, MAX_STARS> stars; // parallax background stars in 3 speed layers

    // --- Level data ---
    std::vector<LevelData> levels; // all 3 levels (Aries, Taurus, Gemini)
    int currentLevel = 0;         // index of the level currently being played
    int selectedLevel = 0;        // index of the level selected on the Level Select screen
    float levelTimer = 0.f;       // seconds elapsed since the current level started

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
    float bgY = 0.f;              // current vertical scroll offset for the background

    // --- Boss tracking ---
    bool bossActive = false; // true if any boss enemy is alive (triggers boss HP bar rendering)

    // --- Private methods (declared here, defined below) ---

    // Core loop phases
    void handleEvents();    // process OS events (close, key press, mouse click)
    void update(float dt);  // update all game logic for one frame
    void render();          // draw everything to the screen

    // Game flow
    void startLevel(int index); // reset everything and begin a level
    void resetPlayer();         // reset the player struct to starting values

    // Gameplay systems
    void checkCollisions();          // test all collision pairs and apply results
    void checkSpawns(float dt);      // check if any wave events should fire this frame
    void spawnExplosion(sf::Vector2f pos, sf::Color cStart, sf::Color cEnd, int count); // burst of particles
    void spawnPickup(sf::Vector2f pos, PickupType ptype); // create a pickup orb

    // Pool allocators: scan the pool for a free slot (on=false) and return a pointer.
    // Returns nullptr if the pool is full (all slots in use).
    Bullet*   allocBullet();
    Enemy*    allocEnemy();
    Particle* allocParticle();
    Pickup*   allocPickup();

    // Spawn helpers
    void spawnPlayerBullet();        // fire bullet(s) based on current power level
    void spawnEnemyBullet(Enemy& e); // fire bullet(s) based on enemy type
    void spawnEnemyFromJob(const SpawnJob& job); // create an enemy from a queued job

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
    void drawTextCentered(const std::string& str, float y, int size, sf::Color col); // draw centered text
    bool isLevelClear();      // check if all enemies are gone and all waves have fired
    int countActiveEnemies(); // count how many enemies have on=true
};


// ============================================================================
// SECTION 6: WAYPOINT PATH DEFINITIONS
// ============================================================================
// These functions return pre-built waypoint paths that enemies follow.
// Each path is a vector of 2D points. The enemy moves from point to point
// at its moveSpeed. When all waypoints are reached, it drifts off-screen.
//
// The screen is 480 pixels wide and 720 pixels tall.
// Y = -40 means above the screen (off-screen spawn point).
// Y = 800 means below the screen (off-screen exit point).
// ============================================================================

// pathSweepCenter: enters from top-center, goes straight down through center.
// Simplest path. Used for tutorial waves and formation attacks.
static std::vector<sf::Vector2f> pathSweepCenter() {
    return {{240, -40}, {240, 200}, {240, 800}};
}

// pathSweepLeft: enters center, curves to the left side, exits off-screen left.
static std::vector<sf::Vector2f> pathSweepLeft() {
    return {{240, -40}, {96, 200}, {96, 500}, {-80, 700}};
}

// pathSweepRight: enters center, curves to the right side, exits off-screen right.
static std::vector<sf::Vector2f> pathSweepRight() {
    return {{240, -40}, {384, 200}, {384, 500}, {560, 700}};
}

// pathLoopLeft: enters from the left, arcs in an S-curve, exits left.
// Tests the player's ability to dodge on the left side.
static std::vector<sf::Vector2f> pathLoopLeft() {
    return {{120, -40}, {60, 200}, {200, 380}, {120, 520}, {-60, 700}};
}

// pathLoopRight: mirror of pathLoopLeft, enters and exits on the right.
static std::vector<sf::Vector2f> pathLoopRight() {
    return {{360, -40}, {420, 200}, {280, 380}, {360, 520}, {540, 700}};
}

// pathZPattern: zigzags across the screen from left to right.
// Forces the player to track horizontally. Very dangerous path.
static std::vector<sf::Vector2f> pathZPattern() {
    return {{60, -40}, {420, 180}, {60, 380}, {420, 600}, {60, 800}};
}

// pathZPatternRight: zigzags from right to left (mirror of pathZPattern).
static std::vector<sf::Vector2f> pathZPatternRight() {
    return {{420, -40}, {60, 180}, {420, 380}, {60, 600}, {420, 800}};
}

// pathDiveBomb: straight vertical drop through the center.
// Fastest and most dangerous path - gives the player least time to react.
static std::vector<sf::Vector2f> pathDiveBomb() {
    return {{240, -40}, {240, 800}};
}

// pathBossEnter: boss enters from top-center and stops at Y=120 (upper third).
// After reaching this point, the boss oscillates left/right.
static std::vector<sf::Vector2f> pathBossEnter() {
    return {{240, -80}, {240, 120}};
}

// pathBossEnterLeft: Gemini level - first boss enters on the left side.
static std::vector<sf::Vector2f> pathBossEnterLeft() {
    return {{160, -80}, {160, 120}};
}

// pathBossEnterRight: Gemini level - second boss enters on the right side.
// Both bosses are alive simultaneously in the final level.
static std::vector<sf::Vector2f> pathBossEnterRight() {
    return {{320, -80}, {320, 120}};
}


// ============================================================================
// SECTION 7: LEVEL BUILDING
// ============================================================================
// buildLevels() creates all 3 levels. Each level has a name, planet color,
// and a list of SpawnEvents (the wave script). Each SpawnEvent says:
//   "At time T, spawn N enemies of type X at position P, following path Q."
//
// Helper: makeEvent() is a convenience function to build a SpawnEvent struct.
// ============================================================================

// makeEvent: builds a SpawnEvent with the given parameters.
// Avoids writing out the full struct initializer every time.
static SpawnEvent makeEvent(float t, EnemyType et, sf::Vector2f start,
                            std::vector<sf::Vector2f> path, int count = 1,
                            float xSpace = 55.f, float delay = 0.25f) {
    SpawnEvent ev;          // create a blank SpawnEvent
    ev.time = t;            // when to trigger (seconds from level start)
    ev.etype = et;          // which enemy type
    ev.startPos = start;    // where the first enemy spawns
    ev.path = path;         // waypoints to follow
    ev.count = count;       // how many enemies in this formation
    ev.xSpacing = xSpace;   // horizontal gap between formation members
    ev.delay = delay;       // stagger delay between each ship in the formation
    ev.fired = false;       // hasn't fired yet
    return ev;              // return the completed event
}

// buildLevels: creates all 3 levels with their complete wave scripts.
// Called once during init(). The wave tables follow the GDD exactly.
void Game::buildLevels() {
    levels.clear(); // remove any existing levels (safety, in case called twice)

    // ===== LEVEL 1: ARIES =====
    // The introductory level. Unlocked by default. Duration ~55 seconds.
    // Gradually introduces Small enemies, then Medium, then the Boss.
    // Planet color: brown/orange (warm, inviting).
    {
        LevelData L;                                // create a blank level
        L.name = "ARIES";                           // display name
        L.desc = "The Beginning";                   // subtitle
        L.planetColor = sf::Color(180, 120, 60);    // brown/orange planet
        L.duration = 55.f;                          // expected duration
        L.locked = false;                           // unlocked from the start

        auto& e = L.events; // shorthand reference to the events vector

        // Time 1.0s: single Small enemy, center sweep. Tutorial wave - one target.
        e.push_back(makeEvent(1.0f,  EnemyType::Small,  {240, -40}, pathSweepCenter()));
        // Time 2.5s: two Small enemies from left and right sides simultaneously.
        e.push_back(makeEvent(2.5f,  EnemyType::Small,  {120, -40}, pathSweepLeft()));
        e.push_back(makeEvent(2.5f,  EnemyType::Small,  {360, -40}, pathSweepRight()));
        // Time 5.0s: formation of 3 Smalls across center. First multi-enemy wave.
        e.push_back(makeEvent(5.0f,  EnemyType::Small,  {240, -40}, pathSweepCenter(), 3));
        // Time 8.0s: 2 Smalls looping left and 2 looping right. Tests side dodging.
        e.push_back(makeEvent(8.0f,  EnemyType::Small,  {120, -40}, pathLoopLeft(), 2));
        e.push_back(makeEvent(8.0f,  EnemyType::Small,  {360, -40}, pathLoopRight(), 2));
        // Time 11.0s: first Medium enemy, dive bomb path. Wider shot spread.
        e.push_back(makeEvent(11.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        // Time 14.0s: 4 Smalls sweep + 1 Medium sweep left. Busy screen.
        e.push_back(makeEvent(14.0f, EnemyType::Small,  {240, -40}, pathSweepCenter(), 4));
        e.push_back(makeEvent(14.5f, EnemyType::Medium, {120, -40}, pathSweepLeft()));
        // Time 18.0s: Z-pattern enemies from both sides. Tests diagonal dodging.
        e.push_back(makeEvent(18.0f, EnemyType::Small,  {60, -40},  pathZPattern()));
        e.push_back(makeEvent(18.2f, EnemyType::Small,  {420, -40}, pathZPatternRight()));
        // Time 21.0s: 5 Smalls wide formation. Biggest Small wave so far.
        e.push_back(makeEvent(21.0f, EnemyType::Small,  {240, -40}, pathSweepCenter(), 5, 50.f));
        // Time 24.0s: 2 Mediums side by side. First multi-Medium wave.
        e.push_back(makeEvent(24.0f, EnemyType::Medium, {200, -40}, pathSweepCenter(), 2, 80.f));
        // Time 27.0s: 3 Smalls from each side simultaneously.
        e.push_back(makeEvent(27.0f, EnemyType::Small,  {120, -40}, pathSweepLeft(), 3));
        e.push_back(makeEvent(27.0f, EnemyType::Small,  {360, -40}, pathSweepRight(), 3));
        // Time 31.0s: 1 Medium dive bomb + 2 Small escorts.
        e.push_back(makeEvent(31.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        e.push_back(makeEvent(31.3f, EnemyType::Small,  {120, -40}, pathSweepLeft(), 2));
        // Time 36.0s: dense 5-Small wave. Pre-boss cleanup.
        e.push_back(makeEvent(36.0f, EnemyType::Small,  {240, -40}, pathSweepCenter(), 5, 45.f));
        // Time 38.0s: 2 Mediums sweep down, clearing the screen before boss.
        e.push_back(makeEvent(38.0f, EnemyType::Medium, {240, -40}, pathSweepCenter(), 2, 80.f));
        // Time 43.0s: BOSS. Enters from top-center. Boss fight begins.
        e.push_back(makeEvent(43.0f, EnemyType::Boss,   {240, -80}, pathBossEnter()));

        levels.push_back(L); // add completed level to the levels vector
    }

    // ===== LEVEL 2: TAURUS =====
    // Medium difficulty. Unlocked by default. Duration ~65 seconds.
    // More Mediums, bigger formations, faster wave pacing.
    // Planet color: blue/purple (cooler, more intense).
    {
        LevelData L;
        L.name = "TAURUS";
        L.desc = "The Challenge";
        L.planetColor = sf::Color(80, 60, 180);     // blue/purple planet
        L.duration = 65.f;
        L.locked = false; // unlocked by default per GDD

        auto& e = L.events;
        // Taurus starts faster - 3 Smalls immediately
        e.push_back(makeEvent(1.0f,  EnemyType::Small,  {240, -40}, pathSweepCenter(), 3));
        e.push_back(makeEvent(3.0f,  EnemyType::Small,  {120, -40}, pathSweepLeft(), 2));
        e.push_back(makeEvent(3.0f,  EnemyType::Small,  {360, -40}, pathSweepRight(), 2));
        e.push_back(makeEvent(6.0f,  EnemyType::Medium, {240, -40}, pathSweepCenter()));
        e.push_back(makeEvent(6.0f,  EnemyType::Small,  {60, -40},  pathZPattern(), 2));
        e.push_back(makeEvent(9.0f,  EnemyType::Small,  {240, -40}, pathSweepCenter(), 5, 45.f));
        e.push_back(makeEvent(12.0f, EnemyType::Medium, {160, -40}, pathSweepLeft()));
        e.push_back(makeEvent(12.0f, EnemyType::Medium, {320, -40}, pathSweepRight()));
        // Elite/Medium enemies appear from here on with increasing density
        e.push_back(makeEvent(15.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        e.push_back(makeEvent(15.0f, EnemyType::Small,  {120, -40}, pathLoopLeft(), 3));
        e.push_back(makeEvent(18.0f, EnemyType::Small,  {240, -40}, pathSweepCenter(), 4));
        e.push_back(makeEvent(18.0f, EnemyType::Medium, {360, -40}, pathSweepRight()));
        // Two simultaneous Mediums at 22s
        e.push_back(makeEvent(22.0f, EnemyType::Medium, {160, -40}, pathSweepCenter()));
        e.push_back(makeEvent(22.0f, EnemyType::Medium, {320, -40}, pathSweepCenter()));
        // Two Z-pattern Mediums at 26s
        e.push_back(makeEvent(26.0f, EnemyType::Medium, {60, -40},  pathZPattern()));
        e.push_back(makeEvent(26.0f, EnemyType::Medium, {420, -40}, pathZPatternRight()));
        e.push_back(makeEvent(30.0f, EnemyType::Small,  {240, -40}, pathSweepCenter(), 5, 40.f));
        e.push_back(makeEvent(30.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        e.push_back(makeEvent(34.0f, EnemyType::Small,  {120, -40}, pathSweepLeft(), 3));
        e.push_back(makeEvent(34.0f, EnemyType::Small,  {360, -40}, pathSweepRight(), 3));
        e.push_back(makeEvent(38.0f, EnemyType::Medium, {200, -40}, pathSweepCenter(), 3, 60.f));
        e.push_back(makeEvent(42.0f, EnemyType::Small,  {240, -40}, pathSweepCenter(), 4));
        e.push_back(makeEvent(42.0f, EnemyType::Medium, {120, -40}, pathLoopLeft()));
        e.push_back(makeEvent(46.0f, EnemyType::Small,  {240, -40}, pathSweepCenter(), 5, 45.f));
        e.push_back(makeEvent(46.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        // Boss arrives at 54s
        e.push_back(makeEvent(54.0f, EnemyType::Boss,   {240, -80}, pathBossEnter()));

        levels.push_back(L);
    }

    // ===== LEVEL 3: GEMINI =====
    // Hardest level. Locked until Taurus is completed. Duration ~80 seconds.
    // All enemy types from wave 1. Two bosses spawn at different times.
    // Planet color: dark green (ominous, final).
    {
        LevelData L;
        L.name = "GEMINI";
        L.desc = "The Final Stand";
        L.planetColor = sf::Color(30, 120, 50);     // dark green planet
        L.duration = 80.f;
        L.locked = true; // locked until Taurus is beaten

        auto& e = L.events;
        // No tutorial - starts with Mediums immediately
        e.push_back(makeEvent(1.0f,  EnemyType::Medium, {240, -40}, pathSweepCenter(), 2, 80.f));
        e.push_back(makeEvent(3.0f,  EnemyType::Small,  {120, -40}, pathSweepLeft(), 3));
        e.push_back(makeEvent(3.0f,  EnemyType::Small,  {360, -40}, pathSweepRight(), 3));
        e.push_back(makeEvent(6.0f,  EnemyType::Medium, {60, -40},  pathZPattern()));
        e.push_back(makeEvent(6.0f,  EnemyType::Small,  {240, -40}, pathSweepCenter(), 4));
        // Double Medium dive bombs at 10s
        e.push_back(makeEvent(10.0f, EnemyType::Medium, {160, -40}, pathDiveBomb()));
        e.push_back(makeEvent(10.0f, EnemyType::Medium, {320, -40}, pathDiveBomb()));
        e.push_back(makeEvent(10.0f, EnemyType::Small,  {120, -40}, pathLoopLeft(), 2));
        e.push_back(makeEvent(14.0f, EnemyType::Small,  {240, -40}, pathSweepCenter(), 5, 42.f));
        e.push_back(makeEvent(14.0f, EnemyType::Medium, {120, -40}, pathSweepLeft()));
        e.push_back(makeEvent(14.0f, EnemyType::Medium, {360, -40}, pathSweepRight()));
        e.push_back(makeEvent(18.0f, EnemyType::Small,  {60, -40},  pathZPattern(), 2));
        e.push_back(makeEvent(18.0f, EnemyType::Small,  {420, -40}, pathZPatternRight(), 2));
        // Triple Medium at 20s - very intense
        e.push_back(makeEvent(20.0f, EnemyType::Medium, {160, -40}, pathSweepCenter()));
        e.push_back(makeEvent(20.0f, EnemyType::Medium, {320, -40}, pathSweepCenter()));
        e.push_back(makeEvent(20.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        e.push_back(makeEvent(24.0f, EnemyType::Small,  {240, -40}, pathSweepCenter(), 5, 45.f));
        e.push_back(makeEvent(24.0f, EnemyType::Medium, {120, -40}, pathLoopLeft()));
        e.push_back(makeEvent(28.0f, EnemyType::Medium, {200, -40}, pathSweepCenter(), 3, 60.f));
        e.push_back(makeEvent(28.0f, EnemyType::Small,  {60, -40},  pathZPattern(), 3));
        e.push_back(makeEvent(32.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        e.push_back(makeEvent(32.0f, EnemyType::Medium, {120, -40}, pathSweepLeft()));
        e.push_back(makeEvent(32.0f, EnemyType::Medium, {360, -40}, pathSweepRight()));
        e.push_back(makeEvent(36.0f, EnemyType::Small,  {240, -40}, pathSweepCenter(), 5, 40.f));
        e.push_back(makeEvent(36.0f, EnemyType::Medium, {160, -40}, pathLoopLeft(), 2));
        e.push_back(makeEvent(40.0f, EnemyType::Medium, {200, -40}, pathSweepCenter(), 3, 70.f));
        e.push_back(makeEvent(40.0f, EnemyType::Small,  {420, -40}, pathZPatternRight(), 3));
        e.push_back(makeEvent(44.0f, EnemyType::Small,  {240, -40}, pathSweepCenter(), 5, 42.f));
        e.push_back(makeEvent(44.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        e.push_back(makeEvent(48.0f, EnemyType::Medium, {160, -40}, pathSweepCenter(), 2, 80.f));
        e.push_back(makeEvent(48.0f, EnemyType::Small,  {120, -40}, pathSweepLeft(), 3));
        e.push_back(makeEvent(52.0f, EnemyType::Small,  {240, -40}, pathSweepCenter(), 4));
        e.push_back(makeEvent(52.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        // First boss at 57s - enters from the left side
        e.push_back(makeEvent(57.0f, EnemyType::Boss,   {160, -80}, pathBossEnterLeft()));
        // Second boss at 70s - enters from the right side. BOTH alive simultaneously.
        e.push_back(makeEvent(70.0f, EnemyType::Boss,   {320, -80}, pathBossEnterRight()));

        levels.push_back(L);
    }
}


// ============================================================================
// SECTION 8: GAME INITIALIZATION
// ============================================================================

// init: called once at the very start. Sets up the window, loads the font,
// initializes all object pools to empty, builds the levels, creates the
// star field, generates the station background texture, and pushes the
// initial state (LevelSelect) onto the state stack.
void Game::init() {
    std::srand((unsigned)std::time(nullptr)); // seed random number generator with current time

    // Create the game window: 480x720 pixels, titled "Space Shooter", close button only (no resize)
    window.create(sf::VideoMode(480, 720), "Space Shooter", sf::Style::Close);
    window.setFramerateLimit(60); // cap at 60 FPS to prevent excessive CPU usage

    // Try loading the font from two possible locations.
    // The CMake build copies it to build/assets/, but if running from the project root
    // it might be at assets/game_font.ttf. If both fail, text won't render but the game
    // will still run (shapes and gameplay work fine without text).
    if (!font.loadFromFile("assets/game_font.ttf")) {   // try the assets subfolder first
        if (!font.loadFromFile("game_font.ttf")) {       // try the current directory
            std::cerr << "Warning: Could not load font. Text may not render.\n";
        }
    }

    // Initialize all pool slots to "off" (available for use)
    for (auto& b : bullets)   b.on = false; // mark all bullet slots as free
    for (auto& e : enemies)   e.on = false; // mark all enemy slots as free
    for (auto& p : particles) p.on = false; // mark all particle slots as free
    for (auto& pk : pickups)  pk.on = false; // mark all pickup slots as free

    buildLevels();       // create the 3 levels with their wave scripts
    initStars();         // randomize 200 stars across 3 parallax layers
    buildStationTile();  // generate the 480x1440 station background texture

    stateStack.push(GameState::LevelSelect); // start on the level select menu
}


// ============================================================================
// SECTION 9: MAIN GAME LOOP
// ============================================================================

// run: the main loop. Executes every frame until the window is closed.
// Each iteration:
//   1. Measure time since last frame (dt = delta time)
//   2. Handle OS events (keyboard, mouse, close)
//   3. If state is Playing, update all game logic
//   4. Clear the screen, render everything, display
void Game::run() {
    sf::Clock clock; // SFML clock to measure time between frames

    while (window.isOpen()) { // keep running until the window is closed
        float dt = clock.restart().asSeconds(); // get time since last frame in seconds, and reset clock
        if (dt > 0.05f) dt = 0.05f; // cap delta time at 50ms (20fps minimum)
                                      // this prevents objects from teleporting if the game hitches

        handleEvents(); // process all pending OS events (keys, mouse, close button)

        // Only update game logic when the state is Playing.
        // In Paused/GameOver/LevelComplete/LevelSelect, the game world is frozen.
        GameState current = stateStack.top(); // peek at the top of the state stack
        if (current == GameState::Playing) {
            update(dt); // advance all game systems by dt seconds
        }

        window.clear(sf::Color(4, 6, 12)); // fill screen with very dark blue-black
        render();                            // draw everything based on current state
        window.display();                    // swap buffers, showing the rendered frame
    }
}


// ============================================================================
// SECTION 10: EVENT HANDLING
// ============================================================================

// handleEvents: processes all OS events queued since last frame.
// This includes window close, key presses (P/ESC for pause), and mouse clicks
// (menu buttons, pause button, overlay dismissal).
// SFML queues events; we poll them all in a while loop.
void Game::handleEvents() {
    sf::Event event; // struct to receive each event

    // pollEvent returns true if there's an event waiting, and fills "event" with it.
    // We loop to process ALL pending events, not just the first one.
    while (window.pollEvent(event)) {

        // Window close button (X) clicked
        if (event.type == sf::Event::Closed) {
            window.close(); // close the window, which will exit the main loop
            return;          // stop processing further events
        }

        GameState current = stateStack.top(); // what state are we in right now?

        // === Key press events (single press, not held down) ===
        if (event.type == sf::Event::KeyPressed) {
            if (current == GameState::Playing) {
                // P or ESC toggles pause during gameplay
                if (event.key.code == sf::Keyboard::P || event.key.code == sf::Keyboard::Escape) {
                    stateStack.push(GameState::Paused); // push Paused on top of Playing
                }
            }
            else if (current == GameState::Paused) {
                // P or ESC unpauses
                if (event.key.code == sf::Keyboard::P || event.key.code == sf::Keyboard::Escape) {
                    stateStack.pop(); // remove Paused, revealing Playing underneath
                }
            }
        }

        // === Mouse click events ===
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            // Convert pixel coordinates to world coordinates (accounts for any view transforms)
            sf::Vector2f mp = window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});

            if (current == GameState::LevelSelect) {
                // --- Planet selection ---
                // Three planets are positioned at x = 120, 240, 360 at y = 320.
                // Check if the click is within 50 pixels of any planet center.
                float planetY = 320.f;
                for (int i = 0; i < 3; i++) {
                    float px = 120.f + i * 120.f; // planet x positions: 120, 240, 360
                    float dist = std::hypot(mp.x - px, mp.y - planetY); // distance from click to planet center
                    if (dist < 50.f) {
                        selectedLevel = i; // select this planet/level
                    }
                }

                // --- Arrow buttons (left and right of the planet row) ---
                // Left arrow: x < 50, y between 274 and 366
                if (mp.x < 50.f && mp.y > 274.f && mp.y < 366.f)
                    selectedLevel = std::max(0, selectedLevel - 1); // move selection left, clamp to 0
                // Right arrow: x > 430, same y range
                if (mp.x > 430.f && mp.y > 274.f && mp.y < 366.f)
                    selectedLevel = std::min(2, selectedLevel + 1); // move selection right, clamp to 2

                // --- Play button (bottom right) ---
                sf::FloatRect playRect(330.f, 657.f, 110.f, 40.f); // button bounding box
                if (playRect.contains(mp) && !levels[selectedLevel].locked) {
                    startLevel(selectedLevel); // begin the selected level
                }

                // --- Back button (bottom left) ---
                sf::FloatRect backRect(40.f, 657.f, 110.f, 40.f);
                if (backRect.contains(mp)) {
                    window.close(); // exit the game
                }
            }
            else if (current == GameState::Paused) {
                stateStack.pop(); // clicking anywhere on the pause overlay resumes the game
            }
            else if (current == GameState::GameOver) {
                startLevel(currentLevel); // clicking restarts the same level from wave 1
            }
            else if (current == GameState::LevelComplete) {
                // Unlock the next level (if there is one)
                if (currentLevel + 1 < (int)levels.size()) {
                    levels[currentLevel + 1].locked = false; // unlock next level
                }
                // Return to the level select screen
                while (stateStack.size() > 1) stateStack.pop(); // pop down to one state
                if (stateStack.top() != GameState::LevelSelect) {
                    stateStack.pop();                               // remove whatever is left
                    stateStack.push(GameState::LevelSelect);        // push level select
                }
            }
            else if (current == GameState::Playing) {
                // --- In-game pause button (top-right corner) ---
                sf::FloatRect pauseBtn(430.f, 12.f, 36.f, 36.f); // button bounding box
                if (pauseBtn.contains(mp)) {
                    stateStack.push(GameState::Paused); // pause via mouse click
                }
            }
        }
    }
}


// ============================================================================
// SECTION 11: START LEVEL / RESET PLAYER
// ============================================================================

// startLevel: initializes everything for a fresh level attempt.
// Clears all pools, resets the player, resets the level timer and wave events.
void Game::startLevel(int index) {
    currentLevel = index;   // set which level we're playing
    levelTimer = 0.f;       // reset the level clock to zero
    bossActive = false;     // no boss yet

    // Clear all object pools by marking every slot as inactive
    for (auto& b : bullets)   b.on = false;
    for (auto& e : enemies)   e.on = false;
    for (auto& p : particles) p.on = false;
    for (auto& pk : pickups)  pk.on = false;

    // Empty the spawn queue (std::queue has no clear(), so we swap with an empty queue)
    while (!spawnQueue.empty()) spawnQueue.pop();

    // Reset all wave events to un-fired so they can trigger again
    for (auto& ev : levels[currentLevel].events) {
        ev.fired = false;
    }

    scoreLog.clear(); // clear the score event log
    resetPlayer();    // reset the player's stats and position

    // Set the state to Playing (clear the stack first to avoid stale states)
    while (!stateStack.empty()) stateStack.pop(); // empty the stack
    stateStack.push(GameState::Playing);          // push Playing as the only state
}

// resetPlayer: sets all player fields back to their starting values.
// Called when starting a level or restarting after game over.
void Game::resetPlayer() {
    player.pos = {240.f, 600.f}; // bottom-center of the screen
    player.hp = player.maxHp;    // full health
    player.speed = 310.f;        // default movement speed
    player.power = 1;            // single shot
    player.powerTimer = 0.f;     // no power-up active
    player.shootTimer = 0.f;     // ready to fire immediately
    player.iframeTimer = 0.f;    // not invincible
    player.shieldTimer = 0.f;    // no shield
    player.tilt = 0.f;           // facing straight up
    player.alive = true;         // alive
    player.score = 0;            // zero score
    player.lives = 3;            // three lives
}


// ============================================================================
// SECTION 12: UPDATE (Main Game Logic)
// ============================================================================

// update: advances all game systems by dt seconds. Called once per frame
// when the state is Playing. This is the core gameplay tick.
//
// Order of operations:
//   1. Player movement (keyboard input)
//   2. Player timers (invincibility, shield, power, auto-fire)
//   3. Spawn checks (trigger any due wave events)
//   4. Drain spawn queue (create enemies whose delay has expired)
//   5. Update enemies (path following, shooting, boss phase check)
//   6. Update bullets (move, off-screen check)
//   7. Update particles (move, fade, lifetime)
//   8. Update pickups (move, gravity, lifetime)
//   9. Scroll background and stars
//   10. Collision detection
//   11. Level clear check
void Game::update(float dt) {
    if (!player.alive) return; // skip everything if the player is dead (waiting for Game Over click)

    levelTimer += dt; // advance the level clock

    // ---- 1. Player movement ----
    // Build a direction vector from which keys are held down RIGHT NOW.
    // sf::Keyboard::isKeyPressed checks the real-time key state, not events.
    // This allows smooth, continuous movement as long as the key is held.
    sf::Vector2f moveDir(0.f, 0.f); // starts as no movement
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        moveDir.x -= 1.f; // move left
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        moveDir.x += 1.f; // move right
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        moveDir.y -= 1.f; // move up (negative Y is upward in screen coords)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        moveDir.y += 1.f; // move down

    // Normalize diagonal movement so moving diagonally isn't faster than straight.
    // Without normalization, diagonal vector (1,1) has length sqrt(2) = 1.414, making
    // diagonal movement 41% faster. Normalizing makes it length 1, same as straight.
    float ml = vlen(moveDir); // length of the direction vector
    if (ml > 0.f) {
        moveDir = vnorm(moveDir);                  // make it a unit vector (length 1)
        player.pos += moveDir * player.speed * dt; // move: position += direction * speed * time
    }

    // Clamp the player position to the screen with a small margin.
    // This prevents the ship from leaving the visible area.
    player.pos.x = clampf(player.pos.x, 24.f, 456.f); // left and right bounds (480 - 24 = 456)
    player.pos.y = clampf(player.pos.y, 24.f, 696.f);  // top and bottom bounds (720 - 24 = 696)

    // Visual tilt: the ship leans left/right when moving horizontally.
    // targetTilt is the desired angle based on input direction.
    // lerp smoothly interpolates from current tilt to target tilt.
    // The dt*8 factor controls how fast the tilt catches up (higher = snappier).
    float targetTilt = moveDir.x * 18.f;                // max tilt: 18 degrees left or right
    player.tilt = lerp(player.tilt, targetTilt, dt * 8.f); // smooth interpolation

    // ---- 2. Timers ----
    // Invincibility frames: countdown after being hit. While > 0, player can't take damage.
    if (player.iframeTimer > 0.f) {
        player.iframeTimer -= dt; // count down
    }

    // Shield: absorbs the next hit. Granted by Power pickup. Counts down to 0.
    if (player.shieldTimer > 0.f) {
        player.shieldTimer -= dt;
    }

    // Power-up timer: if power > 1, the timer counts down. When it expires, power drops by 1.
    // If still > 1 after dropping, the timer resets for the next level.
    if (player.power > 1) {
        player.powerTimer -= dt;
        if (player.powerTimer <= 0.f) {
            player.power -= 1;           // drop one power level
            if (player.power > 1) {
                player.powerTimer = 8.0f; // reset timer for the next level down
            }
        }
    }

    // ---- 3. Auto-fire ----
    // The player fires automatically every shootInterval seconds (0.10s = 10 shots/sec).
    // No shoot button is needed. The timer counts down each frame; when it hits 0,
    // bullets are spawned and the timer resets.
    player.shootTimer -= dt;
    if (player.shootTimer <= 0.f) {
        spawnPlayerBullet();                     // create bullet(s) based on current power level
        player.shootTimer = player.shootInterval; // reset the timer
    }

    // ---- 4. Spawn checks ----
    // Scan the current level's wave events. If enough time has passed and the event
    // hasn't fired yet, push SpawnJobs into the queue.
    checkSpawns(dt);

    // ---- 5. Drain spawn queue ----
    // Process the front of the queue. Decrement the delay; if it reaches 0,
    // create the enemy and pop the job. If the front job isn't ready yet, stop
    // (all later jobs have equal or greater delays).
    while (!spawnQueue.empty()) {
        SpawnJob& front = spawnQueue.front(); // peek at the front job
        front.delayRemaining -= dt;           // count down its delay
        if (front.delayRemaining <= 0.f) {
            spawnEnemyFromJob(front);         // create the enemy in the pool
            spawnQueue.pop();                  // remove the completed job
        } else {
            break; // front job not ready, so no later jobs are either
        }
    }

    // ---- 6. Update enemies ----
    bossActive = false; // reset each frame; set to true if any boss is found alive
    for (auto& e : enemies) {
        if (!e.on) continue; // skip inactive (dead/despawned) enemies

        if (e.etype == EnemyType::Boss) bossActive = true; // boss found, show boss HP bar

        e.pulse += dt; // advance animation timer (used for engine flicker effects)

        // --- Path following ---
        // If the enemy still has waypoints left to reach:
        if (e.pathIndex < (int)e.path.size()) {
            sf::Vector2f target = e.path[e.pathIndex];     // current target waypoint
            sf::Vector2f toTarget = target - e.pos;         // vector from enemy to target
            float dist = vlen(toTarget);                    // distance to target

            if (dist < 8.f) {
                e.pathIndex++; // close enough to waypoint, advance to next one
            } else {
                sf::Vector2f dir = vnorm(toTarget);             // direction toward target
                e.pos += dir * e.moveSpeed * dt;                 // move toward target

                // Non-boss enemies rotate to face their movement direction
                if (e.etype != EnemyType::Boss) {
                    // atan2 gives the angle in radians; convert to degrees for SFML.
                    // We use atan2(dir.x, -dir.y) because screen Y is inverted (down = positive).
                    float targetAngle = std::atan2(dir.x, -dir.y) * 180.f / 3.14159f;
                    e.angle = lerp(e.angle, targetAngle, dt * 4.f); // smooth rotation
                }
            }
        } else {
            // All waypoints exhausted: drift downward off the bottom of the screen
            e.pos.y += 100.f * dt;
            if (e.pos.y > 800.f) { // off-screen
                e.on = false;       // deactivate (free the pool slot)
                continue;            // skip to next enemy
            }
        }

        // --- Boss oscillation ---
        // After the boss reaches its final waypoint, it sways left and right using a sine wave.
        if (e.etype == EnemyType::Boss && e.pathIndex >= (int)e.path.size()) {
            e.pos.x += std::sin(e.pulse * 0.7f) * 60.f * dt; // sine wave horizontal movement
            e.pos.x = clampf(e.pos.x, 80.f, 400.f);          // keep boss on screen
        }

        // --- Boss phase change ---
        // When the boss drops below 40% HP, it enters Phase 2 (enraged):
        // dome color changes, fire rate increases, bullets move faster.
        if (e.etype == EnemyType::Boss && e.phase == 0 && e.hp < e.maxHp * 0.4f) {
            e.phase = 1; // switch to enraged phase
            // Visual flash explosion to signal the phase change
            spawnExplosion(e.pos, sf::Color(80, 100, 255), sf::Color(20, 40, 120, 0), 16);
        }

        // --- Enemy shooting ---
        // Each enemy has a shoot timer that counts down. When it hits 0, the enemy fires
        // and the timer resets to shootInterval (which varies by type and boss phase).
        e.shootTimer -= dt;
        if (e.shootTimer <= 0.f) {
            spawnEnemyBullet(e); // fire bullets based on enemy type
            if (e.etype == EnemyType::Boss) {
                e.shootTimer = (e.phase == 0) ? 0.8f : 0.6f; // boss fires faster in phase 2
            } else {
                e.shootTimer = e.shootInterval; // normal enemies use their preset interval
            }
        }
    }

    // ---- 7. Update bullets ----
    for (auto& b : bullets) {
        if (!b.on) continue;            // skip inactive bullets
        b.pos += b.vel * dt;            // move bullet: position += velocity * time

        // Deactivate bullets that have left the screen (no point tracking them)
        if (b.pos.y < -20.f || b.pos.y > 740.f || b.pos.x < -20.f || b.pos.x > 500.f) {
            b.on = false; // free the pool slot
        }
    }

    // ---- 8. Update particles ----
    for (auto& p : particles) {
        if (!p.on) continue;    // skip inactive particles
        p.life -= dt;           // count down lifetime
        if (p.life <= 0.f) {
            p.on = false;       // lifetime expired, deactivate
            continue;
        }
        p.pos += p.vel * dt;    // move the particle
        p.vel *= 0.97f;         // apply drag: velocity reduces by 3% each frame (decelerates)
    }

    // ---- 9. Update pickups ----
    for (auto& pk : pickups) {
        if (!pk.on) continue;       // skip inactive pickups
        pk.life -= dt;              // count down lifetime
        pk.pulse += dt;             // advance bobbing animation timer
        if (pk.life <= 0.f) {
            pk.on = false;          // expired without being collected
            continue;
        }
        pk.pos += pk.vel * dt;      // move the pickup
        pk.vel.y += 20.f * dt;      // apply slight gravity (pulls downward over time)
    }

    // ---- 10. Scrolling background ----
    bgY += 60.f * dt;             // scroll the station tile downward at 60 pixels/second
    if (bgY >= 1440.f) bgY -= 1440.f; // wrap around when the tile has scrolled its full height

    // Update stars (each star scrolls at its own speed for parallax depth)
    for (auto& s : stars) {
        s.pos.y += s.speed * dt;   // move star downward
        if (s.pos.y > 730.f) {     // if star has scrolled off the bottom
            s.pos.y = -5.f;        // wrap to just above the top
            s.pos.x = randFloat(0.f, 480.f); // randomize horizontal position
        }
    }

    // ---- 11. Collision detection ----
    checkCollisions(); // test all collision pairs (bullets vs enemies, etc.)

    // ---- 12. Level clear check ----
    // The level is complete when: all wave events have fired, the spawn queue is empty,
    // and no enemies are alive. This happens after the boss is destroyed.
    if (isLevelClear()) {
        stateStack.push(GameState::LevelComplete); // push the victory overlay
    }
}


// ============================================================================
// SECTION 13: COLLISION DETECTION
// ============================================================================
// All collision uses circle-circle intersection:
//   Two circles collide if distance(center1, center2) < radius1 + radius2
//
// Collision pairs are checked in this order:
//   1. Player bullets vs enemies (deal damage, kill enemies, drop pickups)
//   2. Enemy bullets vs player (deal damage, check shield, check lives)
//   3. Enemy bodies vs player (same as bullet hit but more damage)
//   4. Pickups vs player (collect pickups)
// ============================================================================

void Game::checkCollisions() {
    // ---- 1. Player bullets vs enemies ----
    for (auto& b : bullets) {
        if (!b.on) continue; // skip inactive bullets

        // Only check player bullets (skip enemy bullets)
        if (b.btype != BulletType::PlayerNorm && b.btype != BulletType::PlayerWide &&
            b.btype != BulletType::PlayerTriple) continue;

        for (auto& e : enemies) {
            if (!e.on) continue; // skip inactive enemies

            // Determine enemy collision radius based on type
            float eRadius = 16.f;                              // Small enemy radius
            if (e.etype == EnemyType::Medium) eRadius = 22.f;  // Medium is bigger
            if (e.etype == EnemyType::Boss)   eRadius = 60.f;  // Boss is much bigger

            float dist = vlen(b.pos - e.pos); // distance between bullet center and enemy center
            if (dist < 4.f + eRadius) {       // 4.f = player bullet radius
                e.hp -= b.dmg; // deal damage to the enemy
                b.on = false;  // consume the bullet (it's used up)

                // Spawn a small hit spark at the impact point
                spawnExplosion(b.pos, sf::Color(255, 200, 80), sf::Color(255, 80, 20, 0), 3);

                // Check if the enemy is dead
                if (e.hp <= 0.f) {
                    e.on = false; // deactivate the enemy (free pool slot)

                    // Choose explosion colors and intensity based on enemy type
                    sf::Color cStart, cEnd;
                    int pCount = 12; // number of particles in the death explosion
                    if (e.etype == EnemyType::Small) {
                        cStart = sf::Color(255, 160, 30);  // orange start
                        cEnd = sf::Color(200, 40, 10, 0);  // red->transparent end
                    } else if (e.etype == EnemyType::Medium) {
                        cStart = sf::Color(80, 255, 120);  // green start
                        cEnd = sf::Color(20, 150, 50, 0);  // darker green->transparent
                        pCount = 18;                        // more particles for bigger enemy
                    } else { // Boss
                        cStart = sf::Color(255, 80, 80);   // red start
                        cEnd = sf::Color(80, 20, 20, 0);   // dark red->transparent
                        pCount = 40;                        // big explosion for boss
                    }
                    spawnExplosion(e.pos, cStart, cEnd, pCount); // create the death explosion

                    // Award score points
                    player.score += e.scoreValue;
                    // Log the kill in the score log (linked list, pushed to front)
                    scoreLog.push_front({
                        (e.etype == EnemyType::Small ? "Small enemy" :
                         e.etype == EnemyType::Medium ? "Medium enemy" : "BOSS"),
                        e.scoreValue, levelTimer
                    });

                    // Drop pickups
                    spawnPickup(e.pos, PickupType::Score); // all enemies drop a Score orb
                    // Medium enemies drop a Health orb if they had low HP when killed
                    if (e.etype == EnemyType::Medium && e.hp < 40.f) {
                        spawnPickup(e.pos + sf::Vector2f(15.f, 0.f), PickupType::Health);
                    }
                    // Boss drops 8 pickups: mix of Score and Health orbs
                    if (e.etype == EnemyType::Boss) {
                        for (int i = 0; i < 8; i++) {
                            sf::Vector2f offset(randFloat(-40.f, 40.f), randFloat(-40.f, 40.f));
                            PickupType pt = (i % 3 == 0) ? PickupType::Health : PickupType::Score;
                            spawnPickup(e.pos + offset, pt);
                        }
                    }
                }
                break; // bullet is consumed, stop checking this bullet against other enemies
            }
        }
    }

    // ---- 2. Enemy bullets vs player ----
    // Only check if the player is NOT invincible (iframeTimer <= 0)
    if (player.iframeTimer <= 0.f) {
        for (auto& b : bullets) {
            if (!b.on) continue;
            // Skip player bullets (only check enemy bullets)
            if (b.btype == BulletType::PlayerNorm || b.btype == BulletType::PlayerWide ||
                b.btype == BulletType::PlayerTriple) continue;

            float dist = vlen(b.pos - player.pos); // distance from bullet to player
            if (dist < 5.f + 18.f) {               // 5 = enemy bullet radius, 18 = player radius
                b.on = false; // consume the bullet

                // Check if shield is active
                if (player.shieldTimer > 0.f) {
                    player.shieldTimer = 0.f; // shield absorbs the hit and breaks
                    // Blue shield-break explosion
                    spawnExplosion(player.pos, sf::Color(60, 160, 255), sf::Color(20, 80, 180, 0), 10);
                } else {
                    // No shield: take damage
                    player.hp -= 12.f;            // each hit deals 12 damage
                    player.iframeTimer = 1.4f;    // 1.4 seconds of invincibility after being hit
                    // Red hit explosion
                    spawnExplosion(player.pos, sf::Color(255, 80, 80), sf::Color(255, 200, 50, 0), 6);

                    // Check if player ran out of HP
                    if (player.hp <= 0.f) {
                        player.lives--;           // lose a life
                        if (player.lives <= 0) {
                            player.alive = false;  // all lives gone
                            stateStack.push(GameState::GameOver); // show game over overlay
                        } else {
                            player.hp = player.maxHp;  // restore HP for the next life
                            player.iframeTimer = 2.0f; // longer invincibility on respawn
                        }
                    }
                }
            }
        }
    }

    // ---- 3. Enemy bodies vs player ----
    // Ramming into an enemy deals 20 damage (more than a bullet).
    // The enemy is NOT destroyed by ramming (it's bigger than you).
    if (player.iframeTimer <= 0.f) {
        for (auto& e : enemies) {
            if (!e.on) continue;
            float eRadius = 16.f;
            if (e.etype == EnemyType::Medium) eRadius = 22.f;
            if (e.etype == EnemyType::Boss)   eRadius = 60.f;

            float dist = vlen(e.pos - player.pos);
            if (dist < eRadius + 18.f) { // enemy radius + player radius
                if (player.shieldTimer > 0.f) {
                    player.shieldTimer = 0.f; // shield absorbs
                    spawnExplosion(player.pos, sf::Color(60, 160, 255), sf::Color(20, 80, 180, 0), 10);
                } else {
                    player.hp -= 20.f;         // ram damage is higher than bullet damage
                    player.iframeTimer = 1.4f;
                    spawnExplosion(player.pos, sf::Color(255, 80, 80), sf::Color(255, 200, 50, 0), 8);

                    if (player.hp <= 0.f) {
                        player.lives--;
                        if (player.lives <= 0) {
                            player.alive = false;
                            stateStack.push(GameState::GameOver);
                        } else {
                            player.hp = player.maxHp;
                            player.iframeTimer = 2.0f;
                        }
                    }
                }
            }
        }
    }

    // ---- 4. Pickups vs player ----
    // Pickup collection uses a generous radius (32 + 18 = 50 pixels)
    // so the player doesn't have to be pixel-perfect.
    for (auto& pk : pickups) {
        if (!pk.on) continue;
        float dist = vlen(pk.pos - player.pos);
        if (dist < 32.f + 18.f) { // 32 = pickup radius, 18 = player radius
            pk.on = false; // consume the pickup

            if (pk.ptype == PickupType::Score) {
                player.score += 25;                            // add 25 bonus points
                scoreLog.push_front({"Score orb", 25, levelTimer}); // log it
            } else if (pk.ptype == PickupType::Health) {
                player.hp = std::min(player.hp + 25.f, player.maxHp); // heal 25 HP, cap at max
            } else if (pk.ptype == PickupType::Power) {
                if (player.power < 3) player.power++;  // increase power level (max 3)
                player.powerTimer = 8.0f;              // power-up lasts 8 seconds
                player.shieldTimer = 8.0f;             // also grants a shield
            }
        }
    }
}


// ============================================================================
// SECTION 14: SPAWNING SYSTEMS
// ============================================================================

// checkSpawns: iterates through all wave events in the current level.
// For each event whose time has been reached and hasn't fired yet,
// it pushes SpawnJobs into the queue (one per enemy in the formation).
void Game::checkSpawns(float dt) {
    for (auto& ev : levels[currentLevel].events) {
        if (!ev.fired && levelTimer >= ev.time) { // time reached and not yet fired
            ev.fired = true; // mark as fired so it won't trigger again

            // Determine enemy stats based on type
            float hp = 0.f, maxHp = 0.f, shootInt = 0.f, moveSpd = 0.f;
            int scoreVal = 0;
            switch (ev.etype) {
                case EnemyType::Small:
                    hp = maxHp = 30.f;     // low health
                    shootInt = 2.2f;       // slow fire rate
                    scoreVal = 50;         // low score
                    moveSpd = 200.f;       // fast movement
                    break;
                case EnemyType::Medium:
                    hp = maxHp = 80.f;     // medium health
                    shootInt = 1.8f;       // medium fire rate
                    scoreVal = 120;        // medium score
                    moveSpd = 160.f;       // slower movement
                    break;
                case EnemyType::Boss:
                    hp = maxHp = 1200.f;   // very high health
                    shootInt = 0.8f;       // fast fire rate
                    scoreVal = 2000;       // big score reward
                    moveSpd = 80.f;        // slow movement
                    break;
            }

            // Create one SpawnJob per enemy in the formation
            for (int i = 0; i < ev.count; i++) {
                SpawnJob job;
                job.etype = ev.etype;
                job.startPos = ev.startPos;
                // Offset each formation member horizontally from center
                // Formula: (i - (count-1)/2) centers the group around the start position
                job.startPos.x += (i - (ev.count - 1) / 2.f) * ev.xSpacing;
                job.delayRemaining = i * ev.delay; // stagger: 0s, 0.25s, 0.5s, etc.
                job.path = ev.path;
                // Adjust the first waypoint to match this enemy's offset position
                if (!job.path.empty()) {
                    job.path[0] = job.startPos;
                }
                job.hp = hp;
                job.maxHp = maxHp;
                job.shootInterval = shootInt;
                job.scoreValue = scoreVal;
                job.moveSpeed = moveSpd;
                spawnQueue.push(job); // add to the back of the queue
            }
        }
    }
}

// spawnEnemyFromJob: creates an actual enemy in the pool from a SpawnJob.
// Called when a job's delay reaches zero in the queue drain loop.
void Game::spawnEnemyFromJob(const SpawnJob& job) {
    Enemy* e = allocEnemy(); // find a free slot in the enemy pool
    if (!e) return;          // pool is full, can't spawn (rare)

    e->on = true;                    // activate this slot
    e->etype = job.etype;            // copy enemy type
    e->pos = job.startPos;           // set starting position
    e->vel = {0.f, 0.f};            // velocity starts at zero (path following handles movement)
    e->hp = job.hp;                  // set health
    e->maxHp = job.maxHp;            // set max health
    // Randomize initial shoot timer so all enemies in a formation don't fire simultaneously
    e->shootTimer = job.shootInterval * randFloat(0.3f, 1.0f);
    e->shootInterval = job.shootInterval;
    e->path = job.path;              // copy the waypoint path
    e->pathIndex = 0;                // start at the first waypoint
    e->angle = 0.f;                  // no rotation yet
    e->pulse = 0.f;                  // animation timer starts at 0
    e->phase = 0;                    // boss starts in phase 0
    e->scoreValue = job.scoreValue;
    e->moveSpeed = job.moveSpeed;
}

// spawnPlayerBullet: creates bullet(s) based on the player's current power level.
// Power 1: one bullet straight up.
// Power 2: two bullets side by side (7px apart).
// Power 3: three bullets with slight left/center/right spread.
void Game::spawnPlayerBullet() {
    if (player.power == 1) {
        // Single bullet, straight up
        Bullet* b = allocBullet();
        if (b) {
            b->on = true;
            b->pos = player.pos + sf::Vector2f(0.f, -20.f); // spawn slightly above the ship
            b->vel = {0.f, -600.f};                          // travel upward at 600 px/s
            b->dmg = 20.f;                                    // 20 damage per hit
            b->btype = BulletType::PlayerNorm;
        }
    } else if (player.power == 2) {
        // Two bullets, offset 7 pixels left and right
        for (int i = -1; i <= 1; i += 2) { // i = -1 (left) and +1 (right)
            Bullet* b = allocBullet();
            if (b) {
                b->on = true;
                b->pos = player.pos + sf::Vector2f(i * 7.f, -20.f); // offset horizontally
                b->vel = {0.f, -600.f};                              // straight up
                b->dmg = 20.f;
                b->btype = BulletType::PlayerWide;
            }
        }
    } else { // power == 3
        // Three bullets: left, center, right with slight angle spread
        for (int i = -1; i <= 1; i++) { // i = -1, 0, +1
            Bullet* b = allocBullet();
            if (b) {
                b->on = true;
                b->pos = player.pos + sf::Vector2f(i * 12.f, -20.f); // wider horizontal offset
                b->vel = {i * 40.f, -600.f}; // side bullets angle slightly outward
                b->dmg = 20.f;
                b->btype = BulletType::PlayerTriple;
            }
        }
    }
}

// spawnEnemyBullet: fires bullet(s) from an enemy based on its type.
// Small: single aimed bullet toward the player.
// Medium: 3-way spread fan aimed at the player.
// Boss: 5-way fan (phase 1) or 8-way fan (phase 2) with a center BossBeam.
void Game::spawnEnemyBullet(Enemy& e) {
    sf::Vector2f toPlayer = player.pos - e.pos;             // vector from enemy to player
    float baseAngle = std::atan2(toPlayer.y, toPlayer.x);   // angle in radians toward player

    if (e.etype == EnemyType::Small) {
        // Single aimed bullet
        sf::Vector2f dir = vnorm(toPlayer);  // unit direction toward player
        Bullet* b = allocBullet();
        if (b) {
            b->on = true;
            b->pos = e.pos;                   // start at enemy position
            b->vel = dir * 260.f;             // 260 px/s toward player
            b->dmg = 12.f;                    // 12 damage per hit
            b->btype = BulletType::EnemyNorm;
        }
    }
    else if (e.etype == EnemyType::Medium) {
        // 3-way spread: one aimed at player, one 0.2 radians left, one 0.2 radians right
        for (int i = -1; i <= 1; i++) {
            float a = baseAngle + i * 0.2f;                  // offset angle by -0.2, 0, +0.2 radians
            sf::Vector2f dir = {std::cos(a), std::sin(a)};   // direction from angle
            Bullet* b = allocBullet();
            if (b) {
                b->on = true;
                b->pos = e.pos;
                b->vel = dir * 240.f;         // 240 px/s
                b->dmg = 12.f;
                b->btype = BulletType::EnemyBurst;
            }
        }
    }
    else if (e.etype == EnemyType::Boss) {
        // Multi-way fan: 5 bullets in phase 1, 8 in phase 2
        int numShots = (e.phase == 0) ? 5 : 8;
        float spread = 0.22f; // radians between each bullet in the fan
        float bulletSpeed = (e.phase == 0) ? 220.f : 280.f; // faster in phase 2

        for (int i = 0; i < numShots; i++) {
            // Calculate the angle for this bullet in the fan.
            // (i - (numShots-1)/2) centers the fan around baseAngle.
            float a = baseAngle + (i - (numShots - 1) / 2.f) * spread;
            sf::Vector2f dir = {std::cos(a), std::sin(a)};
            Bullet* b = allocBullet();
            if (b) {
                b->on = true;
                b->pos = e.pos;
                b->vel = dir * bulletSpeed;
                b->dmg = 12.f;
                // The center bullet is a BossBeam (wider, blue); others are EnemyBurst (red)
                b->btype = (i == numShots / 2) ? BulletType::BossBeam : BulletType::EnemyBurst;
            }
        }
    }
}


// ============================================================================
// SECTION 15: POOL ALLOCATORS
// ============================================================================
// Each allocator scans its pool for a free slot (on == false) and returns
// a pointer to it. If the pool is full (all slots are active), returns nullptr.
// The caller must check for nullptr before using the pointer.
// ============================================================================

Bullet* Game::allocBullet() {
    for (auto& b : bullets) if (!b.on) return &b; // find first free slot
    return nullptr;                                 // pool full
}
Enemy* Game::allocEnemy() {
    for (auto& e : enemies) if (!e.on) return &e;
    return nullptr;
}
Particle* Game::allocParticle() {
    for (auto& p : particles) if (!p.on) return &p;
    return nullptr;
}
Pickup* Game::allocPickup() {
    for (auto& pk : pickups) if (!pk.on) return &pk;
    return nullptr;
}


// ============================================================================
// SECTION 16: EXPLOSION & PICKUP SPAWNERS
// ============================================================================

// spawnExplosion: creates a burst of particles at a position.
// Each particle gets a random velocity direction, random speed, random lifetime,
// and random size. They fade from cStart color to cEnd color over their lifetime.
void Game::spawnExplosion(sf::Vector2f pos, sf::Color cStart, sf::Color cEnd, int count) {
    for (int i = 0; i < count; i++) {
        Particle* p = allocParticle();      // get a free particle slot
        if (!p) break;                       // pool full, stop spawning

        p->on = true;
        // Start position: slightly randomized around the center point
        p->pos = pos + sf::Vector2f(randFloat(-6.f, 6.f), randFloat(-6.f, 6.f));
        // Random direction and speed
        float angle = randFloat(0.f, 6.2832f);      // random angle (0 to 2*PI radians)
        float speed = randFloat(40.f, 180.f);        // random speed
        p->vel = {std::cos(angle) * speed, std::sin(angle) * speed}; // convert angle+speed to velocity
        p->colorStart = cStart;
        p->colorEnd = cEnd;
        p->maxLife = randFloat(0.3f, 0.8f);          // random lifetime (0.3 to 0.8 seconds)
        p->life = p->maxLife;                          // start at full life
        p->size = randFloat(1.5f, 4.f);              // random visual size
    }
}

// spawnPickup: creates a pickup orb at a position with a random burst velocity.
void Game::spawnPickup(sf::Vector2f pos, PickupType ptype) {
    Pickup* pk = allocPickup();          // get a free pickup slot
    if (!pk) return;                      // pool full

    pk->on = true;
    pk->pos = pos;
    pk->vel = {randFloat(-30.f, 30.f), randFloat(-50.f, -10.f)}; // burst upward and slightly sideways
    pk->ptype = ptype;
    pk->life = 8.f;  // disappears after 8 seconds if not collected
    pk->pulse = 0.f;  // reset animation timer
}


// ============================================================================
// SECTION 17: UTILITY FUNCTIONS
// ============================================================================

// isLevelClear: returns true when the level is complete.
// Three conditions must all be true:
//   1. Every SpawnEvent has been fired (no pending waves)
//   2. The spawn queue is empty (no pending enemy spawns)
//   3. No enemies are alive (all on == false)
bool Game::isLevelClear() {
    for (auto& ev : levels[currentLevel].events) {
        if (!ev.fired) return false;     // there are still un-triggered waves
    }
    if (!spawnQueue.empty()) return false; // there are still enemies waiting to spawn
    for (auto& e : enemies) {
        if (e.on) return false;           // there are still living enemies
    }
    return true; // all clear!
}

// countActiveEnemies: counts how many enemies are currently active.
// Used for debugging and could be used for wave pacing.
int Game::countActiveEnemies() {
    int count = 0;
    for (auto& e : enemies) {
        if (e.on) count++;
    }
    return count;
}

// drawTextCentered: draws a text string centered horizontally at the given Y position.
// Calculates the text bounds and offsets the origin to center it.
void Game::drawTextCentered(const std::string& str, float y, int size, sf::Color col) {
    sf::Text text;
    text.setFont(font);
    text.setString(str);
    text.setCharacterSize(size);
    text.setFillColor(col);
    // Get the bounding box of the rendered text
    sf::FloatRect bounds = text.getLocalBounds();
    // Set the origin to the center of the text (so setPosition centers it)
    text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    text.setPosition(240.f, y); // 240 = center of the 480px wide screen
    window.draw(text);
}


// ============================================================================
// SECTION 18: INITIALIZATION HELPERS
// ============================================================================

// initStars: creates 200 stars with random positions and assigns them to
// one of 3 parallax layers. Slower/dimmer/smaller = farther away.
void Game::initStars() {
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].pos.x = randFloat(0.f, 480.f); // random x across screen width
        stars[i].pos.y = randFloat(0.f, 720.f);  // random y across screen height

        int layer = i % 3; // cycle through 3 layers: 0, 1, 2, 0, 1, 2, ...
        if (layer == 0) {
            // Far layer: slow, dim, small
            stars[i].speed = 20.f;
            stars[i].brightness = 0.3f;
            stars[i].size = 1.0f;
        } else if (layer == 1) {
            // Mid layer: medium speed, medium brightness
            stars[i].speed = 45.f;
            stars[i].brightness = 0.5f;
            stars[i].size = 1.3f;
        } else {
            // Near layer: fast, bright, large
            stars[i].speed = 80.f;
            stars[i].brightness = 0.8f;
            stars[i].size = 1.8f;
        }
    }
}

// buildStationTile: generates a 480x1440 pixel texture with space station details.
// This is drawn ONCE at startup, then reused as a scrolling sprite every frame.
// Drawing hundreds of shapes once and caching as a texture is much faster than
// re-drawing them every frame (that's the whole point of sf::RenderTexture).
void Game::buildStationTile() {
    stationTile.create(480, 1440); // create the off-screen render surface
    stationTile.clear(sf::Color(6, 8, 14)); // fill with very dark blue-black

    sf::RectangleShape panel; // reusable rectangle shape for drawing station geometry

    // Main corridor: a vertical strip down the center
    panel.setSize({60.f, 1440.f});          // full height, 60px wide
    panel.setPosition(210.f, 0.f);           // centered (210 + 60 = 270, centered on 240)
    panel.setFillColor(sf::Color(14, 18, 28)); // slightly lighter than background
    stationTile.draw(panel);

    // Side panels and cross-bars repeating every 120 pixels vertically
    for (int y = 0; y < 1440; y += 120) {
        // Left panel block
        panel.setSize({sf::Vector2f(180.f, 80.f)});
        panel.setPosition(10.f, (float)y);
        panel.setFillColor(sf::Color(10, 14, 22));       // dark panel fill
        panel.setOutlineColor(sf::Color(20, 28, 45));     // subtle border
        panel.setOutlineThickness(1.f);
        stationTile.draw(panel);

        // Right panel block (mirrored on the right side)
        panel.setPosition(290.f, (float)y);
        stationTile.draw(panel);

        // Horizontal cross-bar connecting the two sides
        sf::RectangleShape bar({460.f, 3.f});             // thin horizontal line
        bar.setPosition(10.f, y + 90.f);
        bar.setFillColor(sf::Color(18, 24, 40));
        stationTile.draw(bar);

        // Small detail lights (4 per row)
        for (int x = 0; x < 4; x++) {
            sf::CircleShape light(2.f);                    // tiny circle
            light.setOrigin(2.f, 2.f);                     // center the origin
            light.setPosition(50.f + x * 110.f, y + 40.f); // spaced across the width
            light.setFillColor(sf::Color(30, 50, 80, 120)); // dim blue-ish
            stationTile.draw(light);
        }

        // Vertical pipes on each side of the corridor
        sf::RectangleShape pipe({4.f, 80.f});              // thin vertical line
        pipe.setFillColor(sf::Color(15, 20, 35));
        pipe.setPosition(195.f, (float)y);                  // left pipe
        stationTile.draw(pipe);
        pipe.setPosition(281.f, (float)y);                  // right pipe
        stationTile.draw(pipe);
    }

    // Random greeble (small detail boxes) scattered across the tile
    // These add visual complexity that makes the station look detailed.
    for (int i = 0; i < 40; i++) {
        float bx = randFloat(10.f, 460.f);  // random x
        float by = randFloat(0.f, 1430.f);  // random y
        float bw = randFloat(8.f, 30.f);    // random width
        float bh = randFloat(6.f, 20.f);    // random height
        sf::RectangleShape greeble({bw, bh});
        greeble.setPosition(bx, by);
        greeble.setFillColor(sf::Color(12, 16, 26));
        greeble.setOutlineColor(sf::Color(16, 22, 38));
        greeble.setOutlineThickness(0.5f);
        stationTile.draw(greeble);
    }

    stationTile.display(); // finalize the texture (must be called after all drawing)
}


// ============================================================================
// SECTION 19: RENDERING (Main render dispatcher)
// ============================================================================

// render: called every frame. Decides what to draw based on the current game state.
// - LevelSelect: draws the level select menu (no gameplay objects)
// - Playing/Paused/GameOver/LevelComplete: draws the game world, then overlays on top
void Game::render() {
    GameState current = stateStack.top(); // check what state we're in

    // Level select has its own complete rendering (no gameplay objects visible)
    if (current == GameState::LevelSelect) {
        renderLevelSelect();
        return; // done, don't draw gameplay stuff
    }

    // For all gameplay states, draw the game world first
    renderBackground(); // scrolling station tile and background planet
    renderStars();      // parallax star field
    renderPickups();    // pickup orbs (drawn behind enemies for visual layering)
    renderEnemies();    // enemy ships
    renderBullets();    // all bullets (player and enemy)
    renderParticles();  // explosion particles (drawn on top with additive blending)
    renderPlayer();     // player ship (drawn on top of everything except HUD)
    renderHUD();        // HP bar, lives, score, pause button, power pips, charge ring
    if (bossActive) renderBossHP(); // boss HP bar at top-center (only when boss is alive)

    // Vignette: dark semi-transparent rectangles on all 4 screen edges.
    // This frames the play area and hides the hard screen boundary.
    sf::RectangleShape vig;
    vig.setFillColor(sf::Color(4, 6, 12, 180)); // dark, semi-transparent
    // Top edge
    vig.setSize({480.f, 12.f});
    vig.setPosition(0.f, 0.f);
    window.draw(vig);
    // Bottom edge
    vig.setPosition(0.f, 708.f);
    window.draw(vig);
    // Left edge
    vig.setSize({8.f, 720.f});
    vig.setPosition(0.f, 0.f);
    window.draw(vig);
    // Right edge
    vig.setPosition(472.f, 0.f);
    window.draw(vig);

    // Draw overlays on top of the game world if we're in a non-playing state
    if (current == GameState::Paused)        renderPauseOverlay();
    if (current == GameState::GameOver)      renderGameOverOverlay();
    if (current == GameState::LevelComplete) renderLevelCompleteOverlay();
}


// ============================================================================
// SECTION 20: BACKGROUND RENDERING
// ============================================================================

// renderBackground: draws the background planet (atmosphere glow) and the
// scrolling station tile. The station tile is 1440px tall; two copies are drawn
// back-to-back so the scroll wraps seamlessly.
void Game::renderBackground() {
    // --- Background planet ---
    // A large circle with the level's planet color, drawn faintly in the background
    sf::Color planetCol = levels[currentLevel].planetColor;

    sf::CircleShape planet(120.f);       // large radius
    planet.setOrigin(120.f, 120.f);       // center the origin
    planet.setPosition(360.f, 150.f);     // upper-right area
    // Use a dark, transparent version of the planet color
    planet.setFillColor(sf::Color(planetCol.r / 3, planetCol.g / 3, planetCol.b / 3, 60));
    window.draw(planet);

    // Atmosphere glow: a slightly larger circle drawn with additive blending
    sf::CircleShape atmo(140.f);
    atmo.setOrigin(140.f, 140.f);
    atmo.setPosition(360.f, 150.f);
    atmo.setFillColor(sf::Color(planetCol.r / 2, planetCol.g / 2, planetCol.b / 2, 25));
    sf::RenderStates glowState;
    glowState.blendMode = sf::BlendAdd; // additive blending makes colors add (creates glow)
    window.draw(atmo, glowState);

    // --- Scrolling station tile ---
    // The tile is 1440px tall. We draw it twice: once at bgY-1440 (above) and once at bgY.
    // As bgY increases, both copies scroll down. When bgY >= 1440, we wrap it back to 0.
    // This creates an infinite seamless scroll.
    sf::Sprite tile(stationTile.getTexture());
    tile.setColor(sf::Color(255, 255, 255, 80)); // semi-transparent (30% opacity)
    tile.setPosition(0.f, bgY - 1440.f);          // upper copy
    window.draw(tile);
    tile.setPosition(0.f, bgY);                    // lower copy (visible on screen)
    window.draw(tile);
}

// renderStars: draws each star as a small circle with brightness-based alpha.
void Game::renderStars() {
    for (auto& s : stars) {
        sf::CircleShape dot(s.size);          // circle with the star's size
        dot.setOrigin(s.size, s.size);         // center the origin
        dot.setPosition(s.pos);
        sf::Uint8 a = (sf::Uint8)(s.brightness * 255); // convert brightness [0-1] to alpha [0-255]
        dot.setFillColor(sf::Color(200, 210, 240, a));  // pale blue-white, variable alpha
        window.draw(dot);
    }
}


// ============================================================================
// SECTION 21: PLAYER RENDERING
// ============================================================================

// renderPlayer: draws the player ship and shield ring.
// During invincibility frames, the ship blinks on and off.
void Game::renderPlayer() {
    if (!player.alive) return; // don't draw if dead

    // Iframe blinking: rapidly toggle visibility (5 times per second)
    if (player.iframeTimer > 0.f) {
        // Multiply timer by 10, cast to int, check if even or odd
        if ((int)(player.iframeTimer * 10.f) % 2 == 0) return; // skip this frame (invisible)
    }

    drawPlayerShip(player.pos, player.tilt, 1.0f, sf::Color::White); // draw the ship

    // Shield ring: drawn when the shield is active
    if (player.shieldTimer > 0.f) {
        // Outline ring
        sf::CircleShape shield(24.f);
        shield.setOrigin(24.f, 24.f);
        shield.setPosition(player.pos);
        shield.setFillColor(sf::Color::Transparent);             // no fill
        shield.setOutlineColor(sf::Color(60, 160, 255, 140));   // semi-transparent blue
        shield.setOutlineThickness(2.f);
        window.draw(shield);

        // Soft glow behind the ring (additive blending)
        sf::CircleShape glow(28.f);
        glow.setOrigin(28.f, 28.f);
        glow.setPosition(player.pos);
        glow.setFillColor(sf::Color(60, 160, 255, 25));         // very faint blue
        sf::RenderStates gs;
        gs.blendMode = sf::BlendAdd;
        window.draw(glow, gs);
    }
}

// drawPlayerShip: draws the player ship as a collection of ConvexShapes and CircleShapes.
// Uses sf::Transform to position and rotate the entire ship as one unit.
// The tilt parameter rotates the ship left/right for visual feedback during movement.
void Game::drawPlayerShip(sf::Vector2f pos, float tilt, float scale, sf::Color tint) {
    // Build a transform: translate to position, then rotate by tilt angle, then scale
    sf::Transform T;
    T.translate(pos);      // move the origin to the ship's position
    T.rotate(tilt);        // rotate around that position
    T.scale(scale, scale); // scale (1.0 = normal size)

    sf::RenderStates states;
    states.transform = T; // apply this transform to all shapes drawn with "states"

    // --- Engine glow (drawn first, behind the ship) ---
    // Uses additive blending for a glowing effect
    sf::CircleShape engineGlow(8.f);
    engineGlow.setOrigin(8.f, 8.f);         // center origin
    engineGlow.setPosition(0.f, 18.f);       // behind the ship (below center)
    engineGlow.setFillColor(sf::Color(60, 180, 255, 80)); // faint blue
    sf::RenderStates glowState;
    glowState.transform = T;                 // same transform as the ship
    glowState.blendMode = sf::BlendAdd;      // additive blending
    window.draw(engineGlow, glowState);

    // Engine core: brighter, smaller circle at the center of the glow
    sf::CircleShape engineCore(4.f);
    engineCore.setOrigin(4.f, 4.f);
    engineCore.setPosition(0.f, 16.f);
    engineCore.setFillColor(sf::Color(140, 220, 255, 200)); // bright cyan
    window.draw(engineCore, glowState);

    // --- Fuselage (main body) ---
    // ConvexShape with 5 vertices forming an elongated diamond/arrow shape
    sf::ConvexShape body(5);
    body.setPoint(0, {0.f, -22.f});   // nose (top)
    body.setPoint(1, {8.f, -4.f});    // right shoulder
    body.setPoint(2, {6.f, 16.f});    // right rear
    body.setPoint(3, {-6.f, 16.f});   // left rear
    body.setPoint(4, {-8.f, -4.f});   // left shoulder
    body.setFillColor(sf::Color(160, 170, 190)); // light grey-blue
    window.draw(body, states);

    // --- Left wing ---
    sf::ConvexShape wingL(4);
    wingL.setPoint(0, {-6.f, 0.f});    // inner edge, top
    wingL.setPoint(1, {-24.f, 10.f});  // wing tip
    wingL.setPoint(2, {-22.f, 16.f});  // wing tip trailing edge
    wingL.setPoint(3, {-5.f, 14.f});   // inner edge, bottom
    wingL.setFillColor(sf::Color(120, 130, 155)); // slightly darker than body
    window.draw(wingL, states);

    // --- Right wing (mirror of left) ---
    sf::ConvexShape wingR(4);
    wingR.setPoint(0, {6.f, 0.f});
    wingR.setPoint(1, {24.f, 10.f});
    wingR.setPoint(2, {22.f, 16.f});
    wingR.setPoint(3, {5.f, 14.f});
    wingR.setFillColor(sf::Color(120, 130, 155));
    window.draw(wingR, states);

    // --- Cockpit (small circle on the nose) ---
    sf::CircleShape cockpit(4.f);
    cockpit.setOrigin(4.f, 4.f);
    cockpit.setPosition(0.f, -10.f);   // on the upper body
    cockpit.setFillColor(sf::Color(100, 200, 255, 200)); // glowing cyan
    window.draw(cockpit, states);

    // --- Wing tip lights ---
    // Red on left, green on right (like real aircraft navigation lights)
    sf::CircleShape tipL(2.f);
    tipL.setOrigin(2.f, 2.f);
    tipL.setPosition(-23.f, 12.f);
    tipL.setFillColor(sf::Color(255, 80, 60, 180)); // red
    window.draw(tipL, states);

    sf::CircleShape tipR(2.f);
    tipR.setOrigin(2.f, 2.f);
    tipR.setPosition(23.f, 12.f);
    tipR.setFillColor(sf::Color(80, 255, 80, 180)); // green
    window.draw(tipR, states);
}


// ============================================================================
// SECTION 22: ENEMY RENDERING
// ============================================================================

// renderEnemies: iterates through the enemy pool and draws each active enemy
// using the appropriate drawing function based on its type.
void Game::renderEnemies() {
    for (auto& e : enemies) {
        if (!e.on) continue; // skip inactive enemies
        if (e.etype == EnemyType::Small)       drawSmallEnemy(e.pos, e.angle, e.pulse);
        else if (e.etype == EnemyType::Medium)  drawMediumEnemy(e.pos, e.angle, e.pulse);
        else if (e.etype == EnemyType::Boss)    drawBossEnemy(e.pos, e.angle, e.pulse, e.phase);
    }
}

// drawSmallEnemy: draws a small fighter ship. Red-tinted, compact design.
// The +180 degree rotation makes it face downward (toward the player).
void Game::drawSmallEnemy(sf::Vector2f pos, float angle, float pulse) {
    sf::Transform T;
    T.translate(pos);
    T.rotate(angle + 180.f); // +180 to face downward (enemies fly toward the player)
    sf::RenderStates states;
    states.transform = T;

    // Engine glow (flickering based on pulse timer)
    sf::CircleShape glow(6.f);
    glow.setOrigin(6.f, 6.f);
    glow.setPosition(0.f, -14.f); // behind the ship (relative to its rotated frame)
    float flicker = 0.6f + 0.4f * std::sin(pulse * 12.f); // oscillates between 0.2 and 1.0
    glow.setFillColor(sf::Color(255, 120, 40, (sf::Uint8)(60 * flicker))); // orange, variable alpha
    sf::RenderStates gs;
    gs.transform = T;
    gs.blendMode = sf::BlendAdd;
    window.draw(glow, gs);

    // Body: small diamond shape
    sf::ConvexShape body(4);
    body.setPoint(0, {0.f, 14.f});    // nose (pointing down after rotation)
    body.setPoint(1, {7.f, -4.f});
    body.setPoint(2, {0.f, -10.f});
    body.setPoint(3, {-7.f, -4.f});
    body.setFillColor(sf::Color(140, 50, 50)); // dark red
    window.draw(body, states);

    // Left wing (small triangle)
    sf::ConvexShape wL(3);
    wL.setPoint(0, {-5.f, -2.f});
    wL.setPoint(1, {-16.f, 6.f});
    wL.setPoint(2, {-4.f, 8.f});
    wL.setFillColor(sf::Color(120, 35, 35)); // darker red
    window.draw(wL, states);

    // Right wing (mirror)
    sf::ConvexShape wR(3);
    wR.setPoint(0, {5.f, -2.f});
    wR.setPoint(1, {16.f, 6.f});
    wR.setPoint(2, {4.f, 8.f});
    wR.setFillColor(sf::Color(120, 35, 35));
    window.draw(wR, states);

    // Cockpit dot (red)
    sf::CircleShape cockpit(2.5f);
    cockpit.setOrigin(2.5f, 2.5f);
    cockpit.setPosition(0.f, 4.f);
    cockpit.setFillColor(sf::Color(255, 60, 60, 200));
    window.draw(cockpit, states);
}

// drawMediumEnemy: draws a larger fighter ship. Blue-tinted, broader design.
void Game::drawMediumEnemy(sf::Vector2f pos, float angle, float pulse) {
    sf::Transform T;
    T.translate(pos);
    T.rotate(angle + 180.f); // face downward
    sf::RenderStates states;
    states.transform = T;

    // Larger blue engine glow
    sf::CircleShape glow(10.f);
    glow.setOrigin(10.f, 10.f);
    glow.setPosition(0.f, -18.f);
    float flicker = 0.5f + 0.5f * std::sin(pulse * 10.f);
    glow.setFillColor(sf::Color(60, 120, 255, (sf::Uint8)(70 * flicker)));
    sf::RenderStates gs;
    gs.transform = T;
    gs.blendMode = sf::BlendAdd;
    window.draw(glow, gs);

    // Body: pentagonal shape, larger than Small
    sf::ConvexShape body(5);
    body.setPoint(0, {0.f, 20.f});
    body.setPoint(1, {12.f, -2.f});
    body.setPoint(2, {8.f, -16.f});
    body.setPoint(3, {-8.f, -16.f});
    body.setPoint(4, {-12.f, -2.f});
    body.setFillColor(sf::Color(50, 60, 100)); // dark blue
    window.draw(body, states);

    // Broader wings
    sf::ConvexShape wL(4);
    wL.setPoint(0, {-10.f, -4.f});
    wL.setPoint(1, {-26.f, 6.f});
    wL.setPoint(2, {-22.f, 14.f});
    wL.setPoint(3, {-8.f, 10.f});
    wL.setFillColor(sf::Color(40, 48, 85));
    window.draw(wL, states);

    sf::ConvexShape wR(4);
    wR.setPoint(0, {10.f, -4.f});
    wR.setPoint(1, {26.f, 6.f});
    wR.setPoint(2, {22.f, 14.f});
    wR.setPoint(3, {8.f, 10.f});
    wR.setFillColor(sf::Color(40, 48, 85));
    window.draw(wR, states);

    // Blue cockpit
    sf::CircleShape cockpit(4.f);
    cockpit.setOrigin(4.f, 4.f);
    cockpit.setPosition(0.f, 6.f);
    cockpit.setFillColor(sf::Color(80, 140, 255, 200));
    window.draw(cockpit, states);
}

// drawBossEnemy: draws the large boss capital ship.
// Features: wide hull, wing cannons, secondary wings, bridge dome (changes color by phase),
// three engine glows. No rotation (bosses face straight down).
void Game::drawBossEnemy(sf::Vector2f pos, float angle, float pulse, int phase) {
    sf::Transform T;
    T.translate(pos); // bosses don't rotate, just translate
    sf::RenderStates states;
    states.transform = T;

    // Large ambient glow behind the boss
    sf::CircleShape bigGlow(70.f);
    bigGlow.setOrigin(70.f, 70.f);
    bigGlow.setPosition(0.f, 0.f);
    // Blue in phase 0, orange-red in phase 1 (enraged)
    sf::Color glowCol = (phase == 0) ? sf::Color(30, 50, 120, 20) : sf::Color(120, 40, 20, 25);
    bigGlow.setFillColor(glowCol);
    sf::RenderStates gs;
    gs.transform = T;
    gs.blendMode = sf::BlendAdd;
    window.draw(bigGlow, gs);

    // Hull: hexagonal shape
    sf::ConvexShape hull(6);
    hull.setPoint(0, {0.f, -45.f});    // front
    hull.setPoint(1, {35.f, -15.f});
    hull.setPoint(2, {40.f, 25.f});
    hull.setPoint(3, {0.f, 40.f});     // rear center
    hull.setPoint(4, {-40.f, 25.f});
    hull.setPoint(5, {-35.f, -15.f});
    hull.setFillColor(sf::Color(55, 50, 65)); // dark grey-purple
    window.draw(hull, states);

    // Main wings (large, swept back)
    sf::ConvexShape wL(4);
    wL.setPoint(0, {-30.f, -10.f});
    wL.setPoint(1, {-70.f, 5.f});
    wL.setPoint(2, {-60.f, 20.f});
    wL.setPoint(3, {-25.f, 15.f});
    wL.setFillColor(sf::Color(45, 42, 58));
    window.draw(wL, states);

    sf::ConvexShape wR(4);
    wR.setPoint(0, {30.f, -10.f});
    wR.setPoint(1, {70.f, 5.f});
    wR.setPoint(2, {60.f, 20.f});
    wR.setPoint(3, {25.f, 15.f});
    wR.setFillColor(sf::Color(45, 42, 58));
    window.draw(wR, states);

    // Secondary lower wings (smaller)
    sf::ConvexShape swL(3);
    swL.setPoint(0, {-20.f, 20.f});
    swL.setPoint(1, {-50.f, 35.f});
    swL.setPoint(2, {-15.f, 35.f});
    swL.setFillColor(sf::Color(40, 38, 52));
    window.draw(swL, states);

    sf::ConvexShape swR(3);
    swR.setPoint(0, {20.f, 20.f});
    swR.setPoint(1, {50.f, 35.f});
    swR.setPoint(2, {15.f, 35.f});
    swR.setFillColor(sf::Color(40, 38, 52));
    window.draw(swR, states);

    // Wing cannons (rectangular barrels on the wing tips)
    sf::RectangleShape cannonL({6.f, 18.f});
    cannonL.setOrigin(3.f, 0.f);
    cannonL.setPosition(-55.f, 8.f);
    cannonL.setFillColor(sf::Color(70, 65, 80));
    window.draw(cannonL, states);

    sf::RectangleShape cannonR({6.f, 18.f});
    cannonR.setOrigin(3.f, 0.f);
    cannonR.setPosition(55.f, 8.f);
    cannonR.setFillColor(sf::Color(70, 65, 80));
    window.draw(cannonR, states);

    // Bridge dome: color changes with phase
    sf::CircleShape dome(12.f);
    dome.setOrigin(12.f, 12.f);
    dome.setPosition(0.f, -15.f);
    if (phase == 0)
        dome.setFillColor(sf::Color(60, 100, 200, 200));  // blue (normal)
    else
        dome.setFillColor(sf::Color(200, 80, 30, 220));   // orange-red (enraged)
    window.draw(dome, states);

    // Dome inner glow (additive)
    sf::CircleShape domeGlow(8.f);
    domeGlow.setOrigin(8.f, 8.f);
    domeGlow.setPosition(0.f, -15.f);
    if (phase == 0)
        domeGlow.setFillColor(sf::Color(100, 160, 255, 100)); // bright blue glow
    else
        domeGlow.setFillColor(sf::Color(255, 120, 40, 120));  // bright orange glow
    window.draw(domeGlow, gs);

    // Three engine glows at the rear (bottom of the ship)
    for (int i = -1; i <= 1; i++) { // left, center, right
        float ex = i * 18.f;        // horizontal offset
        float ey = 36.f;            // below the hull
        float flicker = 0.5f + 0.5f * std::sin(pulse * 8.f + i * 1.5f); // phase-shifted flicker

        // Outer glow
        sf::CircleShape eng(7.f);
        eng.setOrigin(7.f, 7.f);
        eng.setPosition(ex, ey);
        eng.setFillColor(sf::Color(100, 140, 255, (sf::Uint8)(80 * flicker)));
        window.draw(eng, gs);

        // Inner core (brighter)
        sf::CircleShape engCore(3.f);
        engCore.setOrigin(3.f, 3.f);
        engCore.setPosition(ex, ey);
        engCore.setFillColor(sf::Color(180, 220, 255, (sf::Uint8)(180 * flicker)));
        window.draw(engCore, gs);
    }
}


// ============================================================================
// SECTION 23: BULLET RENDERING
// ============================================================================

// renderBullets: draws each active bullet with appropriate colors and effects.
// Player bullets are cyan/white streaks, enemy bullets are red orbs,
// boss beams are wider blue/white streaks.
void Game::renderBullets() {
    sf::RenderStates glowState;
    glowState.blendMode = sf::BlendAdd; // additive blending for glow effects

    for (auto& b : bullets) {
        if (!b.on) continue; // skip inactive bullets

        if (b.btype == BulletType::PlayerNorm || b.btype == BulletType::PlayerWide ||
            b.btype == BulletType::PlayerTriple) {
            // Player bullet: thin cyan/white vertical streak
            sf::RectangleShape streak({2.f, 12.f});  // 2px wide, 12px tall
            streak.setOrigin(1.f, 6.f);               // center origin
            streak.setPosition(b.pos);
            streak.setFillColor(sf::Color(150, 230, 255)); // light cyan
            window.draw(streak);

            // Soft glow around the bullet
            sf::CircleShape glow(4.f);
            glow.setOrigin(4.f, 4.f);
            glow.setPosition(b.pos);
            glow.setFillColor(sf::Color(100, 180, 255, 60)); // faint blue
            window.draw(glow, glowState);
        }
        else if (b.btype == BulletType::EnemyNorm || b.btype == BulletType::EnemyBurst) {
            // Enemy bullet: red circular orb
            sf::CircleShape orb(4.f);
            orb.setOrigin(4.f, 4.f);
            orb.setPosition(b.pos);
            orb.setFillColor(sf::Color(255, 80, 60)); // bright red
            window.draw(orb);

            // Red glow
            sf::CircleShape glow(7.f);
            glow.setOrigin(7.f, 7.f);
            glow.setPosition(b.pos);
            glow.setFillColor(sf::Color(255, 40, 20, 40)); // faint red
            window.draw(glow, glowState);
        }
        else if (b.btype == BulletType::BossBeam) {
            // Boss beam: wider blue/white streak, rotated to face movement direction
            sf::RectangleShape beam({5.f, 14.f});     // wider than player bullet
            beam.setOrigin(2.5f, 7.f);
            beam.setPosition(b.pos);
            // Calculate rotation angle from velocity direction
            float bangle = std::atan2(b.vel.y, b.vel.x) * 180.f / 3.14159f - 90.f;
            beam.setRotation(bangle);
            beam.setFillColor(sf::Color(140, 180, 255)); // pale blue
            window.draw(beam);

            // Blue glow
            sf::CircleShape glow(8.f);
            glow.setOrigin(8.f, 8.f);
            glow.setPosition(b.pos);
            glow.setFillColor(sf::Color(80, 120, 255, 50));
            window.draw(glow, glowState);
        }
    }
}


// ============================================================================
// SECTION 24: PARTICLE RENDERING
// ============================================================================

// renderParticles: draws each active particle as a fading circle.
// Color interpolates from colorStart to colorEnd based on remaining life.
// Size shrinks slightly over time. Uses additive blending for glow.
void Game::renderParticles() {
    sf::RenderStates glowState;
    glowState.blendMode = sf::BlendAdd;

    for (auto& p : particles) {
        if (!p.on) continue;

        float t = 1.f - (p.life / p.maxLife);           // 0 at birth, 1 at death
        sf::Color c = colorLerp(p.colorStart, p.colorEnd, t); // blend start->end color
        float s = p.size * (1.f - t * 0.5f);            // shrink to 50% of original size by death

        sf::CircleShape dot(s);
        dot.setOrigin(s, s);
        dot.setPosition(p.pos);
        dot.setFillColor(c);
        window.draw(dot, glowState); // additive blending makes overlapping particles glow
    }
}


// ============================================================================
// SECTION 25: PICKUP RENDERING
// ============================================================================

// renderPickups: draws each active pickup as a glowing orb with a bobbing animation.
// Color depends on type: blue (Score), green (Health), gold (Power).
void Game::renderPickups() {
    sf::RenderStates glowState;
    glowState.blendMode = sf::BlendAdd;

    for (auto& pk : pickups) {
        if (!pk.on) continue;

        // Choose color based on pickup type
        sf::Color col;
        if (pk.ptype == PickupType::Score)       col = sf::Color(60, 120, 255);  // blue
        else if (pk.ptype == PickupType::Health)  col = sf::Color(60, 220, 80);   // green
        else                                      col = sf::Color(255, 200, 40);  // gold

        // Bobbing scale factor: oscillates between 0.8 and 1.2
        float bobble = std::sin(pk.pulse * 5.f) * 0.2f + 1.0f;

        // Outer glow (large, faint)
        sf::CircleShape glow(12.f * bobble);
        glow.setOrigin(12.f * bobble, 12.f * bobble);
        glow.setPosition(pk.pos);
        glow.setFillColor(sf::Color(col.r, col.g, col.b, 30)); // very transparent
        window.draw(glow, glowState);

        // Core orb (smaller, brighter)
        sf::CircleShape orb(6.f * bobble);
        orb.setOrigin(6.f * bobble, 6.f * bobble);
        orb.setPosition(pk.pos);
        orb.setFillColor(sf::Color(col.r, col.g, col.b, 200));
        window.draw(orb);

        // Inner highlight (white-ish, small, offset slightly for a 3D look)
        sf::CircleShape inner(3.f * bobble);
        inner.setOrigin(3.f * bobble, 3.f * bobble);
        inner.setPosition(pk.pos + sf::Vector2f(-1.f, -1.f));
        inner.setFillColor(sf::Color(255, 255, 255, 100));
        window.draw(inner, glowState);
    }
}


// ============================================================================
// SECTION 26: HUD (Heads-Up Display) RENDERING
// ============================================================================

// renderHUD: draws all persistent UI elements during gameplay:
// HP bar, lives icons, score text, pause button, power pips, and charge ring.
void Game::renderHUD() {
    // === HP bar (top-left) ===
    // Background: dark rectangle with a subtle border
    sf::RectangleShape hpBg({104.f, 10.f});
    hpBg.setPosition(20.f, 16.f);
    hpBg.setFillColor(sf::Color(20, 20, 30));
    hpBg.setOutlineColor(sf::Color(40, 50, 70));
    hpBg.setOutlineThickness(1.f);
    window.draw(hpBg);

    // Fill bar: width proportional to current HP. Color changes by HP percentage.
    float hpPct = player.hp / player.maxHp; // 0.0 to 1.0
    sf::Color hpCol;
    if (hpPct > 0.5f)       hpCol = sf::Color(60, 140, 255);  // blue (healthy)
    else if (hpPct > 0.25f) hpCol = sf::Color(255, 200, 40);  // yellow (warning)
    else                     hpCol = sf::Color(255, 60, 40);   // red (critical)

    sf::RectangleShape hpFill({100.f * hpPct, 6.f}); // width scales with HP
    hpFill.setPosition(22.f, 18.f);
    hpFill.setFillColor(hpCol);
    window.draw(hpFill);

    // === Lives display (below HP bar) ===
    // 3 small ship icons. Filled = alive, dark = lost.
    for (int i = 0; i < 3; i++) {
        float lx = 24.f + i * 22.f; // horizontal spacing
        float ly = 34.f;
        sf::ConvexShape miniShip(3); // simple triangle
        miniShip.setPoint(0, {0.f, -6.f});  // top
        miniShip.setPoint(1, {5.f, 4.f});   // bottom-right
        miniShip.setPoint(2, {-5.f, 4.f});  // bottom-left
        miniShip.setPosition(lx, ly);
        if (i < player.lives)
            miniShip.setFillColor(sf::Color(160, 180, 210)); // bright = alive
        else
            miniShip.setFillColor(sf::Color(40, 44, 55));    // dark = lost
        window.draw(miniShip);
    }

    // === Score text (below lives) ===
    sf::Text scoreTxt;
    scoreTxt.setFont(font);
    scoreTxt.setString("SCORE: " + std::to_string(player.score));
    scoreTxt.setCharacterSize(14);
    scoreTxt.setFillColor(sf::Color(180, 200, 230));
    scoreTxt.setPosition(20.f, 48.f);
    window.draw(scoreTxt);

    // === Pause button (top-right corner) ===
    // Dark rectangle with two vertical bars inside (standard pause icon)
    sf::RectangleShape pauseBg({32.f, 32.f});
    pauseBg.setPosition(432.f, 14.f);
    pauseBg.setFillColor(sf::Color(30, 40, 60, 180));
    pauseBg.setOutlineColor(sf::Color(60, 80, 120));
    pauseBg.setOutlineThickness(1.f);
    window.draw(pauseBg);

    sf::RectangleShape bar1({6.f, 16.f});   // left pause bar
    bar1.setPosition(441.f, 22.f);
    bar1.setFillColor(sf::Color(100, 150, 220));
    window.draw(bar1);
    sf::RectangleShape bar2({6.f, 16.f});   // right pause bar
    bar2.setPosition(451.f, 22.f);
    bar2.setFillColor(sf::Color(100, 150, 220));
    window.draw(bar2);

    // === Power pips (bottom-left) ===
    // 1-3 small rectangles indicating current power level
    for (int i = 0; i < player.power; i++) {
        sf::RectangleShape pip({12.f, 6.f});
        pip.setPosition(20.f + i * 16.f, 696.f);
        pip.setFillColor(sf::Color(60, 140, 255));
        window.draw(pip);
    }

    // === Charge ring (bottom-right) ===
    // A circular segmented ring that fills as score increases in 500-point increments.
    // When it fills completely (500 points), it resets and starts filling again.
    float chargeProgress = (float)(player.score % 500) / 500.f; // 0.0 to 1.0 within current 500
    int segments = 24;          // total segments in the ring
    float radius = 14.f;        // ring radius
    sf::Vector2f ringCenter(450.f, 694.f); // position
    int filledSegments = (int)(chargeProgress * segments); // how many segments are lit

    for (int i = 0; i < segments; i++) {
        // Calculate the arc angles for this segment (starting from top: -PI/2)
        float a1 = (float)i / segments * 6.2832f - 1.5708f;
        float a2 = (float)(i + 1) / segments * 6.2832f - 1.5708f;

        // Each segment is a thin ConvexShape (4 vertices forming a wedge)
        sf::ConvexShape seg(4);
        float r1 = radius - 3.f, r2 = radius; // inner and outer radius
        seg.setPoint(0, ringCenter + sf::Vector2f(std::cos(a1) * r1, std::sin(a1) * r1));
        seg.setPoint(1, ringCenter + sf::Vector2f(std::cos(a1) * r2, std::sin(a1) * r2));
        seg.setPoint(2, ringCenter + sf::Vector2f(std::cos(a2) * r2, std::sin(a2) * r2));
        seg.setPoint(3, ringCenter + sf::Vector2f(std::cos(a2) * r1, std::sin(a2) * r1));

        if (i < filledSegments)
            seg.setFillColor(sf::Color(60, 140, 255, 200)); // bright blue (filled)
        else
            seg.setFillColor(sf::Color(30, 40, 60, 100));   // dark (empty)
        window.draw(seg);
    }
}


// ============================================================================
// SECTION 27: BOSS HP BAR
// ============================================================================

// renderBossHP: draws the boss HP bar at the top-center of the screen.
// Only called when bossActive is true. Supports multiple bosses (Gemini level).
void Game::renderBossHP() {
    int bossIdx = 0; // tracks which boss we're drawing (for horizontal offset)
    for (auto& e : enemies) {
        if (!e.on || e.etype != EnemyType::Boss) continue; // skip non-boss enemies

        float barWidth = 180.f;
        float barX = 150.f + bossIdx * 100.f; // offset for multiple bosses
        float barY = 8.f;
        float hpPct = e.hp / e.maxHp; // HP percentage

        // Background bar
        sf::RectangleShape bg({barWidth + 4.f, 10.f});
        bg.setPosition(barX - 2.f, barY);
        bg.setFillColor(sf::Color(20, 15, 25));
        bg.setOutlineColor(sf::Color(80, 30, 30));
        bg.setOutlineThickness(1.f);
        window.draw(bg);

        // Fill bar (color changes with phase)
        sf::Color bCol = (e.phase == 0) ? sf::Color(180, 40, 40) : sf::Color(255, 100, 30);
        sf::RectangleShape fill({barWidth * hpPct, 6.f});
        fill.setPosition(barX, barY + 2.f);
        fill.setFillColor(bCol);
        window.draw(fill);

        // "BOSS" label above the bar
        sf::Text label;
        label.setFont(font);
        label.setString("BOSS");
        label.setCharacterSize(9);
        label.setFillColor(sf::Color(200, 100, 100));
        label.setPosition(barX + barWidth / 2.f - 14.f, barY - 2.f);
        window.draw(label);

        bossIdx++; // move to the next boss position
    }
}


// ============================================================================
// SECTION 28: LEVEL SELECT SCREEN
// ============================================================================

// renderLevelSelect: draws the main menu / level picker screen.
// Includes: header, level preview box, planet selector row, arrow buttons,
// Back and Play buttons, and a decorative background.
void Game::renderLevelSelect() {
    window.clear(sf::Color(4, 6, 16)); // deep space background

    renderStars(); // draw the star field for atmosphere

    // --- Large decorative planet (top-right, cosmetic only) ---
    sf::CircleShape bgPlanet(100.f);
    bgPlanet.setOrigin(100.f, 100.f);
    bgPlanet.setPosition(420.f, 80.f);
    bgPlanet.setFillColor(sf::Color(20, 25, 45, 100));
    window.draw(bgPlanet);

    sf::CircleShape bgAtmo(115.f);
    bgAtmo.setOrigin(115.f, 115.f);
    bgAtmo.setPosition(420.f, 80.f);
    bgAtmo.setFillColor(sf::Color(30, 40, 80, 30));
    sf::RenderStates gs;
    gs.blendMode = sf::BlendAdd;
    window.draw(bgAtmo, gs);

    // --- Header bar ---
    sf::RectangleShape header({400.f, 40.f});
    header.setPosition(40.f, 30.f);
    header.setFillColor(sf::Color(12, 16, 28));
    header.setOutlineColor(sf::Color(40, 60, 120));
    header.setOutlineThickness(1.5f);
    window.draw(header);
    drawTextCentered("LEVELS", 50.f, 22, sf::Color(140, 180, 240));

    // --- Level preview box ---
    sf::RectangleShape preview({360.f, 160.f});
    preview.setPosition(60.f, 90.f);
    preview.setFillColor(sf::Color(10, 14, 24));
    preview.setOutlineColor(sf::Color(30, 45, 80));
    preview.setOutlineThickness(1.f);
    window.draw(preview);

    // Level name and description
    LevelData& sel = levels[selectedLevel];
    sf::Text nameText;
    nameText.setFont(font);
    nameText.setString(sel.name);
    nameText.setCharacterSize(24);
    nameText.setFillColor(sel.locked ? sf::Color(80, 80, 90) : sf::Color(200, 220, 255));
    nameText.setPosition(80.f, 100.f);
    window.draw(nameText);

    sf::Text descText;
    descText.setFont(font);
    descText.setString(sel.desc);
    descText.setCharacterSize(13);
    descText.setFillColor(sf::Color(120, 140, 170));
    descText.setPosition(80.f, 132.f);
    window.draw(descText);

    // Mini boss ship drawn in the preview box
    drawBossEnemy({340.f, 190.f}, 0.f, 0.f, 0);

    // Star ratings (HP / ATK / DEF) - visual difficulty indicators
    const char* statNames[3] = {"HP", "ATK", "DEF"};
    int statValues[3] = {3, 2 + selectedLevel, 1 + selectedLevel}; // scale with level
    for (int s = 0; s < 3; s++) {
        sf::Text statLabel;
        statLabel.setFont(font);
        statLabel.setString(statNames[s]);
        statLabel.setCharacterSize(11);
        statLabel.setFillColor(sf::Color(100, 120, 150));
        statLabel.setPosition(80.f, 158.f + s * 22.f);
        window.draw(statLabel);

        // 3 star icons per stat (pentagon shape as a star substitute)
        for (int j = 0; j < 3; j++) {
            sf::CircleShape star(5.f, 5); // 5-sided polygon looks like a star
            star.setOrigin(5.f, 5.f);
            star.setPosition(130.f + j * 18.f, 165.f + s * 22.f);
            if (j < statValues[s])
                star.setFillColor(sf::Color(255, 200, 60));  // gold (filled)
            else
                star.setFillColor(sf::Color(40, 44, 55));    // dark (empty)
            window.draw(star);
        }
    }

    // --- Planet selector row ---
    float planetY = 320.f;
    for (int i = 0; i < 3; i++) {
        float px = 120.f + i * 120.f;                      // planet x positions: 120, 240, 360
        float radius = (i == selectedLevel) ? 40.f : 30.f;  // selected planet is larger

        // Yellow orbit ring around the selected planet
        if (i == selectedLevel) {
            sf::CircleShape orbit(50.f);
            orbit.setOrigin(50.f, 50.f);
            orbit.setPosition(px, planetY);
            orbit.setFillColor(sf::Color::Transparent);
            orbit.setOutlineColor(sf::Color(255, 200, 40, 80));
            orbit.setOutlineThickness(1.5f);
            window.draw(orbit);
        }

        // Planet circle
        sf::Color pCol = levels[i].planetColor;
        if (levels[i].locked) {
            pCol = sf::Color(pCol.r / 3, pCol.g / 3, pCol.b / 3); // darken locked planets
        }
        sf::CircleShape planet(radius);
        planet.setOrigin(radius, radius);
        planet.setPosition(px, planetY);
        planet.setFillColor(pCol);
        window.draw(planet);

        // Atmosphere glow
        sf::CircleShape atmo(radius + 6.f);
        atmo.setOrigin(radius + 6.f, radius + 6.f);
        atmo.setPosition(px, planetY);
        atmo.setFillColor(sf::Color(pCol.r / 2, pCol.g / 2, pCol.b / 2, 40));
        window.draw(atmo, gs);

        // Lock icon for locked levels
        if (levels[i].locked) {
            // Lock body (rectangle)
            sf::RectangleShape lockBody({14.f, 12.f});
            lockBody.setOrigin(7.f, 6.f);
            lockBody.setPosition(px, planetY + 2.f);
            lockBody.setFillColor(sf::Color(80, 70, 60));
            window.draw(lockBody);
            // Lock arch (circle outline)
            sf::CircleShape lockArch(6.f);
            lockArch.setOrigin(6.f, 6.f);
            lockArch.setPosition(px, planetY - 8.f);
            lockArch.setFillColor(sf::Color::Transparent);
            lockArch.setOutlineColor(sf::Color(80, 70, 60));
            lockArch.setOutlineThickness(2.5f);
            window.draw(lockArch);
        }

        // Level name label below the planet
        std::string label = levels[i].locked ? ("-- " + levels[i].name) : levels[i].name;
        sf::Text lbl;
        lbl.setFont(font);
        lbl.setString(label);
        lbl.setCharacterSize(12);
        lbl.setFillColor(levels[i].locked ? sf::Color(60, 60, 70) : sf::Color(160, 180, 210));
        sf::FloatRect lb = lbl.getLocalBounds();
        lbl.setOrigin(lb.left + lb.width / 2.f, 0.f); // center horizontally
        lbl.setPosition(px, planetY + radius + 12.f);
        window.draw(lbl);
    }

    // --- Arrow buttons (<< and >>) ---
    sf::Text arrL;
    arrL.setFont(font);
    arrL.setString("<<");
    arrL.setCharacterSize(20);
    arrL.setFillColor(sf::Color(100, 140, 200));
    arrL.setPosition(12.f, planetY - 14.f);
    window.draw(arrL);

    sf::Text arrR;
    arrR.setFont(font);
    arrR.setString(">>");
    arrR.setCharacterSize(20);
    arrR.setFillColor(sf::Color(100, 140, 200));
    arrR.setPosition(444.f, planetY - 14.f);
    window.draw(arrR);

    // --- Back button (bottom-left) ---
    sf::RectangleShape backBtn({100.f, 36.f});
    backBtn.setPosition(44.f, 659.f);
    backBtn.setFillColor(sf::Color(30, 25, 35));
    backBtn.setOutlineColor(sf::Color(80, 60, 70));
    backBtn.setOutlineThickness(1.f);
    window.draw(backBtn);
    sf::Text backTxt;
    backTxt.setFont(font);
    backTxt.setString("BACK");
    backTxt.setCharacterSize(15);
    backTxt.setFillColor(sf::Color(180, 150, 160));
    backTxt.setPosition(70.f, 666.f);
    window.draw(backTxt);

    // --- Play button (bottom-right) ---
    bool canPlay = !levels[selectedLevel].locked; // only clickable if level is unlocked
    sf::RectangleShape playBtn({100.f, 36.f});
    playBtn.setPosition(336.f, 659.f);
    playBtn.setFillColor(canPlay ? sf::Color(20, 40, 60) : sf::Color(20, 20, 25));
    playBtn.setOutlineColor(canPlay ? sf::Color(40, 100, 180) : sf::Color(40, 40, 50));
    playBtn.setOutlineThickness(1.5f);
    window.draw(playBtn);
    sf::Text playTxt;
    playTxt.setFont(font);
    playTxt.setString("PLAY");
    playTxt.setCharacterSize(15);
    playTxt.setFillColor(canPlay ? sf::Color(100, 180, 255) : sf::Color(50, 50, 60));
    playTxt.setPosition(365.f, 666.f);
    window.draw(playTxt);
}


// ============================================================================
// SECTION 29: OVERLAY SCREENS
// ============================================================================

// renderPauseOverlay: semi-transparent dark overlay with "PAUSED" text.
void Game::renderPauseOverlay() {
    // Full-screen dark tint
    sf::RectangleShape dim({480.f, 720.f});
    dim.setFillColor(sf::Color(0, 0, 0, 140)); // semi-transparent black
    window.draw(dim);

    // Central panel
    sf::RectangleShape panel({260.f, 120.f});
    panel.setOrigin(130.f, 60.f);
    panel.setPosition(240.f, 340.f);
    panel.setFillColor(sf::Color(12, 16, 28));
    panel.setOutlineColor(sf::Color(40, 60, 120));
    panel.setOutlineThickness(2.f);
    window.draw(panel);

    drawTextCentered("PAUSED", 320.f, 28, sf::Color(140, 180, 240));
    drawTextCentered("Click or press P to resume", 360.f, 13, sf::Color(100, 120, 160));
}

// renderGameOverOverlay: shows "GAME OVER", final score, and the score log.
void Game::renderGameOverOverlay() {
    sf::RectangleShape dim({480.f, 720.f});
    dim.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(dim);

    sf::RectangleShape panel({320.f, 320.f});
    panel.setOrigin(160.f, 160.f);
    panel.setPosition(240.f, 340.f);
    panel.setFillColor(sf::Color(14, 10, 16));
    panel.setOutlineColor(sf::Color(120, 40, 40)); // red border for game over
    panel.setOutlineThickness(2.f);
    window.draw(panel);

    drawTextCentered("GAME OVER", 210.f, 30, sf::Color(255, 80, 60));
    drawTextCentered("Final Score: " + std::to_string(player.score), 260.f, 18, sf::Color(200, 200, 220));

    // Score log: show the last 8 score events (most recent first)
    float logY = 300.f;
    int shown = 0;
    for (auto& ev : scoreLog) {
        if (shown >= 8) break; // limit to 8 entries
        std::string line = ev.description + "  +" + std::to_string(ev.points);
        sf::Text t;
        t.setFont(font);
        t.setString(line);
        t.setCharacterSize(11);
        t.setFillColor(sf::Color(140, 150, 170));
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.left + b.width / 2.f, 0.f); // center each line
        t.setPosition(240.f, logY + shown * 18.f);
        window.draw(t);
        shown++;
    }

    drawTextCentered("Click to restart", 480.f, 14, sf::Color(100, 120, 160));
}

// renderLevelCompleteOverlay: shows "LEVEL COMPLETE" with score.
void Game::renderLevelCompleteOverlay() {
    sf::RectangleShape dim({480.f, 720.f});
    dim.setFillColor(sf::Color(0, 0, 0, 140));
    window.draw(dim);

    sf::RectangleShape panel({280.f, 180.f});
    panel.setOrigin(140.f, 90.f);
    panel.setPosition(240.f, 340.f);
    panel.setFillColor(sf::Color(10, 16, 24));
    panel.setOutlineColor(sf::Color(40, 120, 80)); // green border for success
    panel.setOutlineThickness(2.f);
    window.draw(panel);

    drawTextCentered("LEVEL COMPLETE", 290.f, 26, sf::Color(80, 220, 120));
    drawTextCentered(levels[currentLevel].name, 325.f, 18, sf::Color(160, 200, 180));
    drawTextCentered("Score: " + std::to_string(player.score), 355.f, 16, sf::Color(180, 200, 220));
    drawTextCentered("Click to continue", 400.f, 13, sf::Color(100, 140, 130));
}


// ============================================================================
// SECTION 30: ENTRY POINT
// ============================================================================

// main: the program starts here. Creates one Game instance, initializes it,
// and runs the main loop. When run() returns (window closed), the program exits.
int main() {
    Game game;   // create the game object (all pools, vectors, etc. are allocated here)
    game.init(); // set up window, font, levels, stars, background texture
    game.run();  // enter the main loop (blocks until window is closed)
    return 0;    // program exits cleanly
}
