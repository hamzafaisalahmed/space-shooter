#pragma once

#include <SFML/Graphics.hpp>
#include <stack>
#include <vector>
#include <list>
#include <queue>
#include <string>
#include "GameState.h"
#include "Exceptions.h"
#include "Player.h"
#include "GameData.h"
#include "Level.h"
#include "ScoreEvent.h"
#include "HighScore.h"
#include "Utils.h"

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
    sf::Texture homeplanet1;
    sf::Texture homeplanet2;
    sf::Texture pauseIcon;
    sf::Sprite pauseSprite;

    std::stack<GameState> stateStack;

    Player player;

    ObjectPool<Bullet, MAX_BULLETS> bullets;
    ObjectPool<Enemy, MAX_ENEMIES> enemies;
    ObjectPool<Particle, MAX_PARTICLES> particles;
    ObjectPool<Pickup, MAX_PICKUPS> pickups;

    std::array<Star, MAX_STARS> stars;

    std::vector<Level *> levels;
    int currentLevel;
    int selectedLevel;
    float levelTimer;

    std::queue<SpawnJob> spawnQueue;
    std::list<ScoreEvent> scoreLog;
    std::vector<HighScore> highScores;

    float bgY;
    bool bossActive;

    void loadFontOrThrow(const std::string &path);
    void loadTextureOrThrow(sf::Texture &tex, const std::string &path);

    void handleEvents();
    void update(float dt);
    void render();

    void startLevel(int index);
    void resetPlayer();

    void checkCollisions();
    void checkSpawns(float dt);
    void spawnExplosion(sf::Vector2f pos, sf::Color cStart, sf::Color cEnd, int count);
    void spawnPickup(sf::Vector2f pos, PickupType *ptype);

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

    void recordHighScore(int score, int levelIndex);
    void sortHighScores();
    void loadHighScores();
    void saveHighScores();

public:
    Game();
    ~Game();
    void init();
    void run();
};
