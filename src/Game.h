#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <stack>
#include <vector>
#include <list>
#include <queue>
#include <string>
#include "GameState.h"
#include "Player.h"
#include "GameData.h"
#include "Level.h"
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
    sf::Texture homeplanet3;
    sf::Texture pauseIcon;
    sf::Sprite pauseSprite;

    sf::Music music;

    std::stack<GameState> stateStack;

    Player player;

    ObjectPool<Bullet, MAX_BULLETS> bullets;
    ObjectPool<Enemy, MAX_ENEMIES> enemies;
    ObjectPool<Particle, MAX_PARTICLES> particles;
    std::array<Star, MAX_STARS> stars;

    std::vector<Level *> levels;
    int currentLevel;
    int selectedLevel;
    float levelTimer;

    std::queue<SpawnJob> spawnQueue;

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
    void checkSpawns();
    void spawnExplosion(sf::Vector2f pos, sf::Color cStart, sf::Color cEnd, int count, int depth = 1);

    void spawnPlayerBullet();
    void spawnEnemyBullet(Enemy &e);
    void spawnEnemyFromJob(const SpawnJob &job);

    void buildLevels();
    bool isLevelClear();
    int countActiveEnemies();

    // RENDER METHODS
    void renderBackground();
    void renderStars();
    void renderPlayer();
    void renderEnemies();
    void renderBullets();
    void renderParticles();
    void renderHUD();
    void renderBossHP();
    void renderHomeScreen();
    void renderLevelSelect();
    void renderPauseOverlay();
    void renderGameOverOverlay();
    void renderLevelCompleteOverlay();

    // DRAW METHODS
    void drawPlayerShip(sf::Vector2f pos, float tilt, float scale);
    void drawSmallEnemy(sf::Vector2f pos, float angle);
    void drawMediumEnemy(sf::Vector2f pos, float angle);
    void drawBossEnemy(sf::Vector2f pos, float angle);
    void drawTextCentered(const std::string &str, float y, int size, sf::Color col);
    void initStars();

public:
    Game();
    ~Game();
    void init();
    void run();
};
