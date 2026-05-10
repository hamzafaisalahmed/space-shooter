#include "Game.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <stdexcept>

class AssetLoadException : public std::runtime_error
{
public:
    AssetLoadException() : std::runtime_error("Failed to load asset") {}
};

class InvalidLevelException : public std::runtime_error
{
public:
    InvalidLevelException() : std::runtime_error("Invalid level") {}
};

Game::Game()
    : currentLevel(0), selectedLevel(0), levelTimer(0.f),
      bgY(0.f), bossActive(false)
{
}

Game::~Game()
{
    for (int i = 0; i < (int)levels.size(); i++)
        delete levels[i];
    levels.clear();
}

void Game::buildLevels()
{
    for (int i = 0; i < (int)levels.size(); i++)
        delete levels[i];
    levels.clear();

    levels.push_back(new LevelOne());
    levels.push_back(new LevelTwo());
    levels.push_back(new LevelThree());

    for (int i = 0; i < (int)levels.size(); i++)
    {
        levels[i]->buildWaves();
        levels[i]->loadPlanet();
    }
}

void Game::loadFontOrThrow(const std::string &path)
{
    if (!font.loadFromFile(path))
    {
        if (!font.loadFromFile("ProFontWindows.ttf"))
        {
            throw AssetLoadException();
        }
    }
}

void Game::loadTextureOrThrow(sf::Texture &tex, const std::string &path)
{
    if (!tex.loadFromFile(path))
    {
        throw AssetLoadException();
    }
}

void Game::init()
{
    std::srand((unsigned)std::time(0));

    sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
    window.create(desktop, "Space Shooter", sf::Style::Default);
    window.setFramerateLimit(60);

    float scaleY = (float)desktop.height / 720.f;
    float scaledW = 480.f * scaleY;
    float xOffset = ((float)desktop.width - scaledW) / 2.f;

    gameView = sf::View(sf::FloatRect(0.f, 0.f, 480.f, 720.f));
    gameView.setViewport(sf::FloatRect(
        xOffset / (float)desktop.width,
        0.f,
        scaledW / (float)desktop.width,
        1.f));
    window.setView(gameView);

    try
    {
        loadFontOrThrow("assets/fonts/ProFontWindows.ttf");
        loadTextureOrThrow(shipTexture, "assets/textures/spaceship.png");
        loadTextureOrThrow(smallEnemyTexture, "assets/textures/alien1.png");
        loadTextureOrThrow(mediumEnemyTexture, "assets/textures/alien2.png");
        loadTextureOrThrow(bossEnemyTexture, "assets/textures/boss1.png");
        loadTextureOrThrow(homeplanet1, "assets/textures/planet1.png");
        loadTextureOrThrow(homeplanet2, "assets/textures/planet2.png");
        loadTextureOrThrow(homeplanet3, "assets/textures/planet3.png");
        loadTextureOrThrow(pauseIcon, "assets/textures/pause.png");
    }
    catch (const AssetLoadException &ex)
    {
        std::cerr << "Asset warning: " << ex.what() << "\n";
    }
    shipSprite.setScale(0.05f, 0.05f);
    shipSprite.setTexture(shipTexture);
    sf::FloatRect bounds = shipSprite.getLocalBounds();
    shipSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

    pauseSprite.setTexture(pauseIcon);
    pauseSprite.setPosition(432.f, 14.f);
    pauseSprite.setScale(0.08f, 0.08f);

    bullets.clearAll();
    enemies.clearAll();
    particles.clearAll();

    buildLevels();
    initStars();

    stateStack.push(GameState::Home);
}

void Game::run()
{
    sf::Clock clock;
    while (window.isOpen())
    {
        float dt = clock.restart().asSeconds();
        if (dt > 0.05f)
            dt = 0.05f;
        handleEvents();
        GameState current = stateStack.top();
        if (current == GameState::Playing)
        {
            update(dt);
        }
        render();
        window.display();
    }
}

void Game::handleEvents()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            window.close();
            return;
        }
        GameState current = stateStack.top();

        if (event.type == sf::Event::KeyPressed)
        {
            if (current == GameState::Playing)
            {
                if (event.key.code == sf::Keyboard::P || event.key.code == sf::Keyboard::Escape)
                    stateStack.push(GameState::Paused);
            }
            else if (current == GameState::Paused)
            {
                if (event.key.code == sf::Keyboard::P || event.key.code == sf::Keyboard::Escape)
                    stateStack.pop();
            }
            else if (current == GameState::Home)
            {
                if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space)
                {
                    stateStack.push(GameState::LevelSelect);
                }
            }
        }

        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2f mp = window.mapPixelToCoords(
                sf::Vector2i(event.mouseButton.x, event.mouseButton.y));

            if (current == GameState::Home)
            {
                sf::FloatRect playRect(170.f, 460.f, 140.f, 50.f);
                if (playRect.contains(mp))
                    stateStack.push(GameState::LevelSelect);
            }
            else if (current == GameState::LevelSelect)
            {
                // Planet layout constants (must match renderLevelSelect)
                float planetY = 280.f;
                float spacing = 120.f;
                float hitRadius = 60.f; // Increased to match larger planets

                for (int i = 0; i < 3; i++)
                {
                    float px = spacing * (i + 1);
                    // Calculate distance from mouse to planet center
                    float dist = std::hypot(mp.x - px, mp.y - planetY);

                    if (dist < hitRadius)
                    {
                        selectedLevel = i;
                        break; // Found the planet, stop checking
                    }
                }

                // Play Button
                sf::FloatRect playRect(336.f, 659.f, 100.f, 36.f);
                if (playRect.contains(mp) && selectedLevel >= 0)
                {
                    startLevel(selectedLevel);
                }

                // Back Button
                sf::FloatRect backRect(44.f, 659.f, 100.f, 36.f);
                if (backRect.contains(mp))
                {
                    stateStack.pop();
                    if (stateStack.empty())
                        stateStack.push(GameState::Home);
                }
            }
            else if (current == GameState::Paused)
            {
                sf::FloatRect exitBtn(160.f, 358.f, 160.f, 34.f);
                if (exitBtn.contains(mp))
                {
                    while (!stateStack.empty())
                        stateStack.pop();
                    stateStack.push(GameState::Home);
                    stateStack.push(GameState::LevelSelect);
                }
                else
                {
                    stateStack.pop();
                }
            }
            else if (current == GameState::GameOver)
            {
                try
                {
                    startLevel(currentLevel);
                }
                catch (const InvalidLevelException &ex)
                {
                    std::cerr << "Restart failed: " << ex.what() << "\n";
                }
            }
            else if (current == GameState::LevelComplete)
            {
                while (!stateStack.empty())
                    stateStack.pop();
                stateStack.push(GameState::Home);
                stateStack.push(GameState::LevelSelect);
            }
            else if (current == GameState::Playing)
            {
                sf::FloatRect pauseBtn(430.f, 12.f, 36.f, 36.f);
                if (pauseBtn.contains(mp))
                    stateStack.push(GameState::Paused);
            }
        }
    }
}

void Game::startLevel(int index)
{
    if (index < 0 || index >= (int)levels.size())
        throw InvalidLevelException();

    currentLevel = index;
    levelTimer = 0.f;
    bossActive = false;

    bullets.clearAll();
    enemies.clearAll();
    particles.clearAll();
    while (!spawnQueue.empty())
        spawnQueue.pop();

    levels[currentLevel]->buildWaves();
    std::vector<SpawnEvent> &evs = levels[currentLevel]->getEvents();
    for (int i = 0; i < (int)evs.size(); i++)
        evs[i].fired = false;

    resetPlayer();

    while (!stateStack.empty())
        stateStack.pop();
    stateStack.push(GameState::Playing);
}

void Game::resetPlayer()
{
    player.resetForLevel();
}

void Game::update(float dt)
{
    if (!player.isAlive())
        return;
    levelTimer += dt;
    sf::Vector2f moveDir(0.f, 0.f);
    if (window.hasFocus())
    {

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            moveDir.x -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            moveDir.x += 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            moveDir.y -= 1.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
            moveDir.y += 1.f;

        if ((sf::Keyboard::isKeyPressed(sf::Keyboard::Space) || sf::Mouse::isButtonPressed(sf::Mouse::Left)) && player.getShootTimer() <= 0.f)
        {
            spawnPlayerBullet();
            player.setShootTimer(player.getShootInterval());
        }
    }
    // player movement
    float ml = vlen(moveDir);
    if (ml > 0.f)
    {
        moveDir = vnorm(moveDir);
        player.move(moveDir * player.getSpeed() * dt);
    }
    player.clampToArena();

    float targetTilt = moveDir.x * 18.f;
    player.setTilt(lerp(player.getTilt(), targetTilt, dt * 8.f));

    // player shield and shoot timer
    if (player.getIframeTimer() > 0.f)
        player.tickIframe(dt);
    if (player.getShieldTimer() > 0.f)
        player.tickShield(dt);

    player.tickShootTimer(dt);

    // spawning enemies
    checkSpawns();

    while (!spawnQueue.empty())
    {
        SpawnJob &front = spawnQueue.front();
        front.delayRemaining -= dt;
        if (front.delayRemaining <= 0.f)
        {
            spawnEnemyFromJob(front);
            spawnQueue.pop();
        }
        else
        {
            break;
        }
    }

    bossActive = false;
    for (int i = 0; i < enemies.capacity(); i++)
    {
        Enemy &e = enemies.at(i);
        if (!e.on)
            continue;
        if (e.type == EnemyType::Boss)
            bossActive = true;
        e.pulse += dt;
        if (e.pathIndex < (int)e.path.size())
        {
            sf::Vector2f target = e.path[e.pathIndex];
            sf::Vector2f toTarget = target - e.pos;
            float dist = vlen(toTarget);
            if (dist < 8.f)
            {
                e.pathIndex++;
            }
            else
            {
                sf::Vector2f dir = vnorm(toTarget);
                e.pos += dir * e.moveSpeed * dt;
                if (e.type != EnemyType::Boss)
                {
                    float targetAngle = std::atan2(dir.x, -dir.y) * 180.f / 3.14159f;
                    e.angle = lerp(e.angle, targetAngle, dt * 4.f);
                }
            }
        }
        else if (e.type == EnemyType::Boss)
        {
            e.pos.x += std::sin(e.pulse * 0.7f) * 60.f * dt;
            e.pos.x = clampf(e.pos.x, 80.f, 400.f);
            e.pos.y = clampf(e.pos.y, 60.f, 340.f);
        }
        else
        {
            e.pos.y += 100.f * dt;
            if (e.pos.y > 800.f)
            {
                e.on = false;
                continue;
            }
        }
        e.shootTimer -= dt;
        if (e.shootTimer <= 0.f)
        {
            spawnEnemyBullet(e);
            if (e.type == EnemyType::Boss)
                e.shootTimer = 0.6f;
            else
                e.shootTimer = e.shootInterval;
        }
    }

    // bullets
    for (int i = 0; i < bullets.capacity(); i++)
    {
        Bullet &b = bullets.at(i);
        if (!b.on)
            continue;
        b.pos += b.vel * dt;
        if (b.pos.y < -20.f || b.pos.y > 740.f || b.pos.x < -20.f || b.pos.x > 500.f)
            b.on = false;
    }

    // particles
    for (int i = 0; i < particles.capacity(); i++)
    {
        Particle &p = particles.at(i);
        if (!p.on)
            continue;
        p.life -= dt;
        if (p.life <= 0.f)
        {
            p.on = false;
            continue;
        }
        p.pos += p.vel * dt;
        p.vel *= 0.97f;
    }

    // background
    bgY += 60.f * dt;
    if (bgY >= 1440.f)
        bgY -= 1440.f;
    for (int i = 0; i < MAX_STARS; i++)
    {
        Star &s = stars[i];
        s.pos.y += s.speed * dt;
        if (s.pos.y > 730.f)
        {
            s.pos.y = -5.f;
            s.pos.x = randFloat(0.f, 480.f);
        }
    }

    checkCollisions();
    if (isLevelClear())
    {
        stateStack.push(GameState::LevelComplete);
    }
}

void Game::checkCollisions()
{
    for (int bi = 0; bi < bullets.capacity(); bi++)
    {
        Bullet &b = bullets.at(bi);
        if (!b.on)
            continue;
        if (!b.type || !b.type->isPlayerBullet())
            continue;

        for (int ei = 0; ei < enemies.capacity(); ei++)
        {
            Enemy &e = enemies.at(ei);
            if (!e.on)
                continue;
            float eRadius = e.type ? e.type->getRadius() : 16.f;
            float dist = vlen(b.pos - e.pos);
            if (dist < 4.f + eRadius)
            {
                e.hp -= b.dmg;
                b.on = false;
                spawnExplosion(b.pos, sf::Color(255, 200, 80), sf::Color(255, 80, 20, 0), 3);
                if (e.hp <= 0.f)
                {
                    e.on = false;
                    sf::Color cStart, cEnd;
                    int pCount = 12;
                    if (e.type == EnemyType::Small)
                    {
                        cStart = sf::Color(255, 160, 30);
                        cEnd = sf::Color(200, 40, 10, 0);
                        spawnExplosion(e.pos, cStart, cEnd, pCount, 1);
                    }
                    else if (e.type == EnemyType::Medium)
                    {
                        cStart = sf::Color(80, 255, 120);
                        cEnd = sf::Color(20, 150, 50, 0);
                        pCount = 18;
                        spawnExplosion(e.pos, cStart, cEnd, pCount, 2);
                    }
                    else
                    {
                        cStart = sf::Color(255, 80, 80);
                        cEnd = sf::Color(80, 20, 20, 0);
                        pCount = 40;
                        spawnExplosion(e.pos, cStart, cEnd, pCount, 5);
                    }
                    player += e.scoreValue;
                }
                break;
            }
        }
    }

    if (player.getIframeTimer() <= 0.f)
    {
        for (int bi = 0; bi < bullets.capacity(); bi++)
        {
            Bullet &b = bullets.at(bi);
            if (!b.on)
                continue;
            if (!b.type || b.type->isPlayerBullet())
                continue;
            float dist = vlen(b.pos - player.getPos());
            if (dist < 5.f + 18.f)
            {
                b.on = false;
                if (player.getShieldTimer() > 0.f)
                {
                    player.setShieldTimer(0.f);
                    spawnExplosion(player.getPos(), sf::Color(60, 160, 255), sf::Color(20, 80, 180, 0), 10);
                }
                else
                {
                    player.takeDamage(12.f);
                    player.setIframeTimer(1.4f);
                    spawnExplosion(player.getPos(), sf::Color(255, 80, 80), sf::Color(255, 200, 50, 0), 6);
                    if (player.getHp() <= 0.f)
                    {
                        player.loseLife();
                        if (player.getLives() <= 0)
                        {
                            player.setAlive(false);
                            stateStack.push(GameState::GameOver);
                        }
                        else
                        {
                            player.heal(player.getMaxHp());
                            player.setIframeTimer(2.0f);
                        }
                    }
                }
            }
        }
    }

    if (player.getIframeTimer() <= 0.f)
    {
        for (int ei = 0; ei < enemies.capacity(); ei++)
        {
            Enemy &e = enemies.at(ei);
            if (!e.on)
                continue;
            float eRadius = e.type ? e.type->getRadius() : 16.f;
            float dist = vlen(e.pos - player.getPos());
            if (dist < eRadius + 18.f)
            {
                if (player.getShieldTimer() > 0.f)
                {
                    player.setShieldTimer(0.f);
                    spawnExplosion(player.getPos(), sf::Color(60, 160, 255), sf::Color(20, 80, 180, 0), 10);
                }
                else
                {
                    player.takeDamage(20.f);
                    player.setIframeTimer(1.4f);
                    spawnExplosion(player.getPos(), sf::Color(255, 80, 80), sf::Color(255, 200, 50, 0), 8);
                    if (player.getHp() <= 0.f)
                    {
                        player.loseLife();
                        if (player.getLives() <= 0)
                        {
                            player.setAlive(false);
                            stateStack.push(GameState::GameOver);
                        }
                        else
                        {
                            player.heal(player.getMaxHp());
                            player.setIframeTimer(2.0f);
                        }
                    }
                }
            }
        }
    }
}

void Game::checkSpawns()
{
    std::vector<SpawnEvent> &evs = levels[currentLevel]->getEvents();
    for (int i = 0; i < (int)evs.size(); i++)
    {
        SpawnEvent &ev = evs[i];
        if (!ev.fired && levelTimer >= ev.time)
        {
            if (ev.etype == EnemyType::Boss && bossActive)
                continue;
            ev.fired = true;
            EnemyType *et = ev.etype;
            float hp = et->getMaxHp();
            float maxHp = hp;
            float shootInt = et->getShootInterval();
            int scoreVal = et->getScoreValue();
            float moveSpd = et->getMoveSpeed();
            for (int k = 0; k < ev.count; k++)
            {
                SpawnJob job;
                job.etype = ev.etype;
                job.startPos = ev.startPos;
                job.startPos.x += (k - (ev.count - 1) / 2.f) * ev.xSpacing;
                job.delayRemaining = k * ev.delay;
                job.path = ev.path;
                if (!job.path.empty())
                    job.path[0] = job.startPos;
                job.hp = hp;
                job.maxHp = maxHp;
                job.shootInterval = shootInt;
                job.scoreValue = scoreVal;
                job.moveSpeed = moveSpd;
                spawnQueue.push(job);
            }
        }
    }
}

void Game::spawnEnemyFromJob(const SpawnJob &job)
{
    Enemy *e = enemies.alloc();
    if (!e)
        return;
    e->on = true;
    e->type = job.etype;
    e->pos = job.startPos;
    e->vel = sf::Vector2f(0.f, 0.f);
    e->hp = job.hp;
    e->maxHp = job.maxHp;
    e->shootTimer = job.shootInterval * randFloat(0.3f, 1.0f);
    e->shootInterval = job.shootInterval;
    e->path = job.path;
    e->pathIndex = 0;
    e->angle = 0.f;
    e->pulse = 0.f;
    e->scoreValue = job.scoreValue;
    e->moveSpeed = job.moveSpeed;
}

void Game::spawnPlayerBullet()
{

    Bullet *b = bullets.alloc();
    if (b)
    {
        b->on = true;
        b->pos = player.getPos() + sf::Vector2f(0.f, -20.f);
        b->vel = sf::Vector2f(0.f, -600.f);
        b->dmg = 20.f;
        b->type = BulletType::PlayerNorm;
    }
}

void Game::spawnEnemyBullet(Enemy &e)
{
    sf::Vector2f toPlayer = player.getPos() - e.pos;
    float baseAngle = std::atan2(toPlayer.y, toPlayer.x);
    if (e.type == EnemyType::Small)
    {
        sf::Vector2f dir = vnorm(toPlayer);
        Bullet *b = bullets.alloc();
        if (b)
        {
            b->on = true;
            b->pos = e.pos;
            b->vel = dir * 260.f;
            b->dmg = 12.f;
            b->type = BulletType::EnemyNorm;
        }
    }
    else if (e.type == EnemyType::Medium)
    {
        for (int i = -1; i <= 1; i++)
        {
            float a = baseAngle + i * 0.2f;
            sf::Vector2f dir(std::cos(a), std::sin(a));
            Bullet *b = bullets.alloc();
            if (b)
            {
                b->on = true;
                b->pos = e.pos;
                b->vel = dir * 240.f;
                b->dmg = 12.f;
                b->type = BulletType::EnemyBurst;
            }
        }
    }
    else if (e.type == EnemyType::Boss)
    {
        // Start 0.7 radians to the left of the base angle
        sf::Vector2f diff = player.getPos() - e.pos;
        float aimAngle = std::atan2(diff.y, diff.x);

        for (int i = 0; i < 8; i++)
        {
            // Simply add 0.2 radians for each bullet in the fan
            float a = aimAngle + (i * 0.2f);

            if (Bullet *b = bullets.alloc())
            {
                b->on = true;
                b->pos = e.pos;
                b->vel = sf::Vector2f(std::cos(a), std::sin(a)) * 220.f;
                b->dmg = 12.f;
                b->type = (i % 2) ? BulletType::BossBeam : BulletType::EnemyBurst;
            }
        }
    }
}

void Game::spawnExplosion(sf::Vector2f pos, sf::Color cStart, sf::Color cEnd, int count, int depth)
{
    if (depth <= 0 || count <= 0)
        return;

    for (int i = 0; i < count; i++)
    {
        Particle *p = particles.alloc();
        if (!p)
            break;
        p->on = true;
        p->pos = pos + sf::Vector2f(randFloat(-6.f, 6.f), randFloat(-6.f, 6.f));
        float angle = randFloat(0.f, 6.2832f);
        float speed = randFloat(40.f, 180.f);
        p->vel = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
        p->colorStart = cStart;
        p->colorEnd = cEnd;
        p->maxLife = randFloat(0.3f, 0.8f);
        p->life = p->maxLife;
        p->size = randFloat(1.5f, 4.f);
    }

    for (int i = 0; i < 3; i++)
    {
        sf::Vector2f offset(randFloat(-25.f, 25.f), randFloat(-25.f, 25.f));
        spawnExplosion(pos + offset, cStart, cEnd, count / 2, depth - 1);
    }
}

bool Game::isLevelClear()
{
    std::vector<SpawnEvent> &evs = levels[currentLevel]->getEvents();
    for (int i = 0; i < (int)evs.size(); i++)
        if (!evs[i].fired)
            return false;
    if (!spawnQueue.empty())
        return false;
    for (int i = 0; i < enemies.capacity(); i++)
        if (enemies.at(i).on)
            return false;
    return true;
}

int Game::countActiveEnemies()
{
    return enemies.countActive();
}

void Game::drawTextCentered(const std::string &str, float y, int size, sf::Color col)
{
    sf::Text text;
    text.setFont(font);
    text.setString(str);
    text.setCharacterSize(size);
    text.setFillColor(col);
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
    text.setPosition(240.f, y);
    window.draw(text);
}

void Game::initStars()
{
    for (int i = 0; i < MAX_STARS; i++)
    {
        stars[i].pos.x = randFloat(0.f, 480.f);
        stars[i].pos.y = randFloat(0.f, 720.f);
        int layer = i % 3;
        if (layer == 0)
        {
            stars[i].speed = 20.f;
            stars[i].brightness = 0.3f;
            stars[i].size = 1.0f;
        }
        else if (layer == 1)
        {
            stars[i].speed = 45.f;
            stars[i].brightness = 0.5f;
            stars[i].size = 1.3f;
        }
        else
        {
            stars[i].speed = 80.f;
            stars[i].brightness = 0.8f;
            stars[i].size = 1.8f;
        }
    }
}

void Game::render()
{
    window.setView(window.getDefaultView());
    window.clear(sf::Color::Black);
    window.setView(gameView);

    GameState current = stateStack.top();

    if (current == GameState::Home)
    {
        renderHomeScreen();
        return;
    }
    if (current == GameState::LevelSelect)
    {
        renderLevelSelect();
        return;
    }

    renderBackground();
    renderStars();
    renderEnemies();
    renderBullets();
    renderParticles();
    renderPlayer();
    renderHUD();
    if (bossActive)
        renderBossHP();

    if (current == GameState::Paused)
        renderPauseOverlay();
    if (current == GameState::GameOver)
        renderGameOverOverlay();
    if (current == GameState::LevelComplete)
        renderLevelCompleteOverlay();
}

void Game::renderBackground()
{
    levels[currentLevel]->renderPlanet(window);
}

void Game::renderStars()
{
    for (int i = 0; i < MAX_STARS; i++)
    {
        Star &s = stars[i];
        sf::CircleShape dot(s.size);
        dot.setOrigin(s.size, s.size);
        dot.setPosition(s.pos);
        sf::Uint8 a = (sf::Uint8)(s.brightness * 255);
        dot.setFillColor(sf::Color(200, 210, 240, a));
        window.draw(dot);
    }
}

void Game::renderPlayer()
{
    if (!player.isAlive())
        return;
    if (player.getIframeTimer() > 0.f)
    {
        if ((int)(player.getIframeTimer() * 10.f) % 2 == 0)
            return;
    }
    drawPlayerShip(player.getPos(), player.getTilt(), 0.4f);
}

void Game::drawPlayerShip(sf::Vector2f pos, float tilt, float scale)
{
    shipSprite.setPosition(pos);
    shipSprite.setRotation(tilt);
    shipSprite.setScale(scale, scale);
    window.draw(shipSprite);
}

void Game::renderEnemies()
{
    for (int i = 0; i < enemies.capacity(); i++)
    {
        Enemy &e = enemies.at(i);
        if (!e.on)
            continue;
        if (e.type == EnemyType::Small)
            drawSmallEnemy(e.pos, e.angle);
        else if (e.type == EnemyType::Medium)
            drawMediumEnemy(e.pos, e.angle);
        else if (e.type == EnemyType::Boss)
            drawBossEnemy(e.pos, e.angle);
    }
}

void Game::drawSmallEnemy(sf::Vector2f pos, float angle)
{
    sf::Sprite sprite;
    sprite.setTexture(smallEnemyTexture);
    sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    sprite.setScale(0.1f, 0.1f);
    sprite.setPosition(pos);
    sprite.setRotation(angle + 180.f);
    window.draw(sprite);
}

void Game::drawMediumEnemy(sf::Vector2f pos, float angle)
{
    sf::Sprite sprite;
    sprite.setTexture(mediumEnemyTexture);
    sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    sprite.setScale(0.4f, 0.4f);
    sprite.setPosition(pos);
    sprite.setRotation(angle + 180.f);
    window.draw(sprite);
}

void Game::drawBossEnemy(sf::Vector2f pos, float angle)
{
    sf::Sprite sprite;
    sprite.setTexture(bossEnemyTexture);
    sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    sprite.setScale(0.4f, 0.4f);
    sprite.setPosition(pos);
    sprite.setRotation(angle);
    window.draw(sprite);
}

void Game::renderBullets()
{
    sf::RenderStates glowState;
    glowState.blendMode = sf::BlendAdd;
    for (int i = 0; i < bullets.capacity(); i++)
    {
        Bullet &b = bullets.at(i);
        if (!b.on || !b.type)
            continue;
        if (b.type->isPlayerBullet())
        {
            sf::RectangleShape streak(sf::Vector2f(2.f, 12.f));
            streak.setOrigin(1.f, 6.f);
            streak.setPosition(b.pos);
            streak.setFillColor(sf::Color(150, 230, 255));
            window.draw(streak);
            sf::CircleShape glow(4.f);
            glow.setOrigin(4.f, 4.f);
            glow.setPosition(b.pos);
            glow.setFillColor(sf::Color(100, 180, 255, 60));
            window.draw(glow, glowState);
        }
        else if (b.type == BulletType::EnemyNorm || b.type == BulletType::EnemyBurst)
        {
            sf::CircleShape orb(4.f);
            orb.setOrigin(4.f, 4.f);
            orb.setPosition(b.pos);
            orb.setFillColor(sf::Color(255, 80, 60));
            window.draw(orb);
            sf::CircleShape glow(7.f);
            glow.setOrigin(7.f, 7.f);
            glow.setPosition(b.pos);
            glow.setFillColor(sf::Color(255, 40, 20, 40));
            window.draw(glow, glowState);
        }
        else if (b.type == BulletType::BossBeam)
        {
            sf::RectangleShape beam(sf::Vector2f(5.f, 14.f));
            beam.setOrigin(2.5f, 7.f);
            beam.setPosition(b.pos);
            float bangle = std::atan2(b.vel.y, b.vel.x) * 180.f / 3.14159f - 90.f;
            beam.setRotation(bangle);
            beam.setFillColor(sf::Color(140, 180, 255));
            window.draw(beam);
            sf::CircleShape glow(8.f);
            glow.setOrigin(8.f, 8.f);
            glow.setPosition(b.pos);
            glow.setFillColor(sf::Color(80, 120, 255, 50));
            window.draw(glow, glowState);
        }
    }
}

void Game::renderParticles()
{
    sf::RenderStates glowState;
    glowState.blendMode = sf::BlendAdd;
    for (int i = 0; i < particles.capacity(); i++)
    {
        Particle &p = particles.at(i);
        if (!p.on)
            continue;
        float t = 1.f - (p.life / p.maxLife);
        sf::Color c = colorLerp(p.colorStart, p.colorEnd, t);
        float s = p.size * (1.f - t * 0.5f);
        sf::CircleShape dot(s);
        dot.setOrigin(s, s);
        dot.setPosition(p.pos);
        dot.setFillColor(c);
        window.draw(dot, glowState);
    }
}

void Game::renderHUD()
{
    // 1. Health Bar
    sf::RectangleShape hpBg(sf::Vector2f(104.f, 10.f));
    hpBg.setPosition(20.f, 16.f);
    hpBg.setFillColor(sf::Color(20, 20, 30));
    window.draw(hpBg);

    float hpPct = std::max(0.f, player.getHp() / player.getMaxHp());
    sf::RectangleShape hpFill(sf::Vector2f(100.f * hpPct, 6.f));
    hpFill.setPosition(22.f, 18.f);
    hpFill.setFillColor(sf::Color(60, 140, 255));
    window.draw(hpFill);

    // 2. Lives (Simplified: between HP and Score)
    shipSprite.setRotation(0.f);
    shipSprite.setScale(0.1f, 0.1f);
    for (int i = 0; i < player.getLives(); i++)
    {
        shipSprite.setPosition(25.f + (i * 22.f), 38.f);
        window.draw(shipSprite);
    }

    // 3. Score
    sf::Text scoreTxt;
    scoreTxt.setFont(font);
    scoreTxt.setString("SCORE: " + std::to_string(player.getScore()));
    scoreTxt.setCharacterSize(14);
    scoreTxt.setPosition(20.f, 58.f);
    window.draw(scoreTxt);

    window.draw(pauseSprite);
}

void Game::renderBossHP()
{
    if (!bossActive)
        return;

    for (int i = 0; i < enemies.capacity(); i++)
    {
        Enemy &e = enemies.at(i);
        if (e.on && e.type == EnemyType::Boss)
        {
            float hpPct = std::max(0.f, e.hp / e.maxHp);

            // 1. Background Bar (Centered at 240)
            sf::RectangleShape bg(sf::Vector2f(200.f, 10.f));
            bg.setOrigin(100.f, 5.f);
            bg.setPosition(240.f, 20.f);
            bg.setFillColor(sf::Color(20, 15, 25));
            bg.setOutlineColor(sf::Color(80, 30, 30));
            bg.setOutlineThickness(1.f);
            window.draw(bg);

            // 2. Health Fill
            sf::RectangleShape fill(sf::Vector2f(200.f * hpPct, 10.f));
            fill.setOrigin(100.f, 5.f);
            fill.setPosition(240.f, 20.f);
            fill.setFillColor(sf::Color(180, 40, 40));
            window.draw(fill);

            // 3. Simple Label
            drawTextCentered("BOSS", 35.f, 12, sf::Color(200, 100, 100));

            return; // Found the boss, stop searching
        }
    }
}

void Game::renderHomeScreen()
{
    window.clear(sf::Color(4, 6, 16));
    renderStars();

    sf::Sprite p1(homeplanet1);
    p1.setOrigin(homeplanet1.getSize().x / 2.f, homeplanet1.getSize().y / 2.f);
    p1.setPosition(80.f, 600.f);
    p1.setScale(280.f / homeplanet1.getSize().x, 280.f / homeplanet1.getSize().x);
    window.draw(p1);

    sf::Sprite p2(homeplanet2);
    p2.setOrigin(homeplanet2.getSize().x / 2.f, homeplanet2.getSize().y / 2.f);
    p2.setPosition(420.f, 100.f);
    p2.setScale(180.f / homeplanet2.getSize().x, 180.f / homeplanet2.getSize().x);
    window.draw(p2);

    drawTextCentered("DEEPSPACE", 230.f, 64, sf::Color(140, 200, 255));
    drawTextCentered("DEFENDERS", 300.f, 64, sf::Color(255, 220, 120));
    drawTextCentered("Clear the skies...Before the skies clear us", 360.f, 14, sf::Color(140, 160, 200));

    sf::RectangleShape playBtn(sf::Vector2f(140.f, 50.f));
    playBtn.setPosition(170.f, 460.f);
    playBtn.setFillColor(sf::Color(20, 50, 90));
    playBtn.setOutlineColor(sf::Color(80, 160, 240));
    playBtn.setOutlineThickness(2.f);
    window.draw(playBtn);
    drawTextCentered("PLAY", 485.f, 24, sf::Color(180, 220, 255));

    drawTextCentered("Click PLAY to start", 540.f, 12,
                     sf::Color(110, 130, 170));
}

void Game::renderLevelSelect()
{
    window.clear(sf::Color(4, 6, 16));
    renderStars();
    // --- Header ---
    sf::RectangleShape header(sf::Vector2f(400.f, 40.f));
    header.setPosition(40.f, 30.f);
    header.setFillColor(sf::Color(12, 16, 28));
    header.setOutlineColor(sf::Color(40, 60, 120));
    header.setOutlineThickness(1.5f);
    window.draw(header);
    drawTextCentered("MAIN MENU - SELECT LEVEL", 50.f, 18, sf::Color(140, 180, 240));

    // --- Level Preview Box (Boss removed, text centered) ---
    sf::RectangleShape preview(sf::Vector2f(360.f, 130.f));
    preview.setPosition(60.f, 90.f);
    preview.setFillColor(sf::Color(10, 14, 24));
    preview.setOutlineColor(sf::Color(30, 45, 80));
    preview.setOutlineThickness(1.f);
    window.draw(preview);

    // Ensure we don't go out of bounds if selectedLevel is invalid
    if (selectedLevel >= 0 && selectedLevel < (int)levels.size())
    {
        Level *sel = levels[selectedLevel];
        drawTextCentered(sel->getName(), 120.f, 24, sf::Color::White);
        drawTextCentered(sel->getDesc(), 160.f, 13, sf::Color(120, 140, 170));
    }

    // --- 3 Planet Levels ---
    float planetY = 280.f;
    float spacing = 120.f;

    for (int i = 0; i < 3; i++)
    {
        float px = spacing * (i + 1);
        sf::Sprite planetSprite;

        if (i == 0)
            planetSprite.setTexture(homeplanet1);
        else if (i == 1)
            planetSprite.setTexture(homeplanet2);
        else if (i == 2)
            planetSprite.setTexture(homeplanet3);

        sf::FloatRect bounds = planetSprite.getLocalBounds();
        planetSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

        // Bigger Sizes: 100px if selected, 75px if not
        float targetSize = (i == selectedLevel) ? 100.f : 75.f;
        planetSprite.setScale(targetSize / bounds.width, targetSize / bounds.height);

        planetSprite.setPosition(px, planetY);
        window.draw(planetSprite);
    }
    // --- Navigation Buttons (Kept as requested) ---
    sf::RectangleShape backBtn(sf::Vector2f(100.f, 36.f));
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

    sf::RectangleShape playBtn(sf::Vector2f(100.f, 36.f));
    playBtn.setPosition(336.f, 659.f);
    playBtn.setFillColor(sf::Color(20, 40, 60));
    playBtn.setOutlineColor(sf::Color(40, 100, 180));
    playBtn.setOutlineThickness(1.5f);
    window.draw(playBtn);

    sf::Text playTxt;
    playTxt.setFont(font);
    playTxt.setString("PLAY");
    playTxt.setCharacterSize(15);
    playTxt.setFillColor(sf::Color(100, 180, 255));
    playTxt.setPosition(365.f, 666.f);
    window.draw(playTxt);
}
void Game::renderPauseOverlay()
{
    sf::RectangleShape dim(sf::Vector2f(480.f, 720.f));
    dim.setFillColor(sf::Color(0, 0, 0, 140));
    window.draw(dim);
    sf::RectangleShape panel(sf::Vector2f(260.f, 120.f));
    panel.setOrigin(130.f, 60.f);
    panel.setPosition(240.f, 340.f);
    panel.setFillColor(sf::Color(12, 16, 28));
    panel.setOutlineColor(sf::Color(40, 60, 120));
    panel.setOutlineThickness(2.f);
    window.draw(panel);
    drawTextCentered("PAUSED", 300.f, 28, sf::Color(140, 180, 240));
    drawTextCentered("Click or press P to resume", 335.f, 13, sf::Color(100, 120, 160));

    sf::RectangleShape exitBtn(sf::Vector2f(160.f, 34.f));
    exitBtn.setOrigin(80.f, 17.f);
    exitBtn.setPosition(240.f, 375.f);
    exitBtn.setFillColor(sf::Color(50, 20, 20));
    exitBtn.setOutlineColor(sf::Color(120, 40, 40));
    exitBtn.setOutlineThickness(1.5f);
    window.draw(exitBtn);
    drawTextCentered("EXIT TO MENU", 375.f, 13, sf::Color(220, 100, 100));
}

void Game::renderGameOverOverlay()
{
    // 1. Dim the background
    sf::RectangleShape dim(sf::Vector2f(480.f, 720.f));
    dim.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(dim);

    // 2. Draw the central panel
    sf::RectangleShape panel(sf::Vector2f(320.f, 200.f)); // Reduced height since log is gone
    panel.setOrigin(160.f, 100.f);
    panel.setPosition(240.f, 340.f);
    panel.setFillColor(sf::Color(14, 10, 16));
    panel.setOutlineColor(sf::Color(120, 40, 40));
    panel.setOutlineThickness(2.f);
    window.draw(panel);

    // 3. Draw Header and Final Score
    drawTextCentered("GAME OVER", 280.f, 30, sf::Color(255, 80, 60));
    drawTextCentered("Final Score: " + std::to_string(player.getScore()), 330.f, 18,
                     sf::Color(200, 200, 220));

    // 4. Interaction hint
    drawTextCentered("Click to restart", 400.f, 14, sf::Color(100, 120, 160));
}
void Game::renderLevelCompleteOverlay()
{
    sf::RectangleShape dim(sf::Vector2f(480.f, 720.f));
    dim.setFillColor(sf::Color(0, 0, 0, 140));
    window.draw(dim);
    sf::RectangleShape panel(sf::Vector2f(280.f, 180.f));
    panel.setOrigin(140.f, 90.f);
    panel.setPosition(240.f, 340.f);
    panel.setFillColor(sf::Color(10, 16, 24));
    panel.setOutlineColor(sf::Color(40, 120, 80));
    panel.setOutlineThickness(2.f);
    window.draw(panel);
    drawTextCentered("LEVEL COMPLETE", 290.f, 26, sf::Color(80, 220, 120));
    drawTextCentered(levels[currentLevel]->getName(), 325.f, 18, sf::Color(160, 200, 180));
    drawTextCentered("Score: " + std::to_string(player.getScore()), 355.f, 16,
                     sf::Color(180, 200, 220));
    drawTextCentered("Click to continue", 400.f, 13, sf::Color(100, 140, 130));
}