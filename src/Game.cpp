#include "Game.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
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

    if (!music.openFromFile("assets/audio/music1.ogg"))
    {
        std::cerr << "Warning: Failed to load music.\n";
    }
    else
    {
        music.setLoop(true);
        music.setVolume(50.f);
        music.play();
    }

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

void Game::resetPlayer()
{
    player.resetForLevel();
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