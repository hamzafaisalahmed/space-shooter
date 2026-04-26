#include "headers.h"
#include <windows.h>
static std::vector<sf::Vector2f> pathSweepCenter()
{
    return {{240, -40}, {240, 200}, {240, 800}};
}
static std::vector<sf::Vector2f> pathSweepLeft()
{
    return {{240, -40}, {96, 200}, {96, 500}, {-80, 700}};
}
static std::vector<sf::Vector2f> pathSweepRight()
{
    return {{240, -40}, {384, 200}, {384, 500}, {560, 700}};
}
static std::vector<sf::Vector2f> pathLoopLeft()
{
    return {{120, -40}, {60, 200}, {200, 380}, {120, 520}, {-60, 700}};
}
static std::vector<sf::Vector2f> pathLoopRight()
{
    return {{360, -40}, {420, 200}, {280, 380}, {360, 520}, {540, 700}};
}
static std::vector<sf::Vector2f> pathZPattern()
{
    return {{60, -40}, {420, 180}, {60, 380}, {420, 600}, {60, 800}};
}
static std::vector<sf::Vector2f> pathZPatternRight()
{
    return {{420, -40}, {60, 180}, {420, 380}, {60, 600}, {420, 800}};
}
static std::vector<sf::Vector2f> pathDiveBomb()
{
    return {{240, -40}, {240, 800}};
}
static std::vector<sf::Vector2f> pathBossEnter()
{
    return {{240, -300}, {240, 120}};
}
static std::vector<sf::Vector2f> pathBossEnterLeft()
{
    return {{160, -300}, {160, 120}};
}
static std::vector<sf::Vector2f> pathBossEnterRight()
{
    return {{320, -300}, {320, 120}};
}
static SpawnEvent makeEvent(float t, EnemyType et, sf::Vector2f start,
                            std::vector<sf::Vector2f> path, int count = 1,
                            float xSpace = 55.f, float delay = 0.25f)
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
void Game::buildLevels()
{
    levels.clear();
    {
        LevelData L;
        L.name = "ARIES";
        L.desc = "The Beginning";
        L.planetColor = sf::Color(180, 120, 60);
        L.duration = 55.f;
        L.locked = false;
        auto &e = L.events;
        e.push_back(makeEvent(1.0f, EnemyType::Small, {240, -40}, pathSweepCenter()));
        e.push_back(makeEvent(2.5f, EnemyType::Small, {120, -40}, pathSweepLeft()));
        e.push_back(makeEvent(2.5f, EnemyType::Small, {360, -40}, pathSweepRight()));
        e.push_back(makeEvent(5.0f, EnemyType::Small, {240, -40}, pathSweepCenter(), 3));
        e.push_back(makeEvent(8.0f, EnemyType::Small, {120, -40}, pathLoopLeft(), 2));
        e.push_back(makeEvent(8.0f, EnemyType::Small, {360, -40}, pathLoopRight(), 2));
        e.push_back(makeEvent(11.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        e.push_back(makeEvent(14.0f, EnemyType::Small, {240, -40}, pathSweepCenter(), 4));
        e.push_back(makeEvent(14.5f, EnemyType::Medium, {120, -40}, pathSweepLeft()));
        e.push_back(makeEvent(18.0f, EnemyType::Small, {60, -40}, pathZPattern()));
        e.push_back(makeEvent(18.2f, EnemyType::Small, {420, -40}, pathZPatternRight()));
        e.push_back(makeEvent(21.0f, EnemyType::Small, {240, -40}, pathSweepCenter(), 5, 50.f));
        e.push_back(makeEvent(24.0f, EnemyType::Medium, {200, -40}, pathSweepCenter(), 2, 80.f));
        e.push_back(makeEvent(27.0f, EnemyType::Small, {120, -40}, pathSweepLeft(), 3));
        e.push_back(makeEvent(27.0f, EnemyType::Small, {360, -40}, pathSweepRight(), 3));
        e.push_back(makeEvent(31.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        e.push_back(makeEvent(31.3f, EnemyType::Small, {120, -40}, pathSweepLeft(), 2));
        e.push_back(makeEvent(36.0f, EnemyType::Small, {240, -40}, pathSweepCenter(), 5, 45.f));
        e.push_back(makeEvent(38.0f, EnemyType::Medium, {240, -40}, pathSweepCenter(), 2, 80.f));
        e.push_back(makeEvent(43.0f, EnemyType::Boss, {240, -80}, pathBossEnter()));
        levels.push_back(L);
    }
    {
        LevelData L;
        L.name = "TAURUS";
        L.desc = "The Challenge";
        L.planetColor = sf::Color(80, 60, 180);
        L.duration = 65.f;
        L.locked = false;
        auto &e = L.events;
        e.push_back(makeEvent(1.0f, EnemyType::Small, {240, -40}, pathSweepCenter(), 3));
        e.push_back(makeEvent(3.0f, EnemyType::Small, {120, -40}, pathSweepLeft(), 2));
        e.push_back(makeEvent(3.0f, EnemyType::Small, {360, -40}, pathSweepRight(), 2));
        e.push_back(makeEvent(6.0f, EnemyType::Medium, {240, -40}, pathSweepCenter()));
        e.push_back(makeEvent(6.0f, EnemyType::Small, {60, -40}, pathZPattern(), 2));
        e.push_back(makeEvent(9.0f, EnemyType::Small, {240, -40}, pathSweepCenter(), 5, 45.f));
        e.push_back(makeEvent(12.0f, EnemyType::Medium, {160, -40}, pathSweepLeft()));
        e.push_back(makeEvent(12.0f, EnemyType::Medium, {320, -40}, pathSweepRight()));
        e.push_back(makeEvent(15.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        e.push_back(makeEvent(15.0f, EnemyType::Small, {120, -40}, pathLoopLeft(), 3));
        e.push_back(makeEvent(18.0f, EnemyType::Small, {240, -40}, pathSweepCenter(), 4));
        e.push_back(makeEvent(18.0f, EnemyType::Medium, {360, -40}, pathSweepRight()));
        e.push_back(makeEvent(22.0f, EnemyType::Medium, {160, -40}, pathSweepCenter()));
        e.push_back(makeEvent(22.0f, EnemyType::Medium, {320, -40}, pathSweepCenter()));
        e.push_back(makeEvent(26.0f, EnemyType::Medium, {60, -40}, pathZPattern()));
        e.push_back(makeEvent(26.0f, EnemyType::Medium, {420, -40}, pathZPatternRight()));
        e.push_back(makeEvent(30.0f, EnemyType::Small, {240, -40}, pathSweepCenter(), 5, 40.f));
        e.push_back(makeEvent(30.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        e.push_back(makeEvent(34.0f, EnemyType::Small, {120, -40}, pathSweepLeft(), 3));
        e.push_back(makeEvent(34.0f, EnemyType::Small, {360, -40}, pathSweepRight(), 3));
        e.push_back(makeEvent(38.0f, EnemyType::Medium, {200, -40}, pathSweepCenter(), 3, 60.f));
        e.push_back(makeEvent(42.0f, EnemyType::Small, {240, -40}, pathSweepCenter(), 4));
        e.push_back(makeEvent(42.0f, EnemyType::Medium, {120, -40}, pathLoopLeft()));
        e.push_back(makeEvent(46.0f, EnemyType::Small, {240, -40}, pathSweepCenter(), 5, 45.f));
        e.push_back(makeEvent(46.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        e.push_back(makeEvent(54.0f, EnemyType::Boss, {240, -80}, pathBossEnter()));
        levels.push_back(L);
    }
    {
        LevelData L;
        L.name = "GEMINI";
        L.desc = "The Final Stand";
        L.planetColor = sf::Color(30, 120, 50);
        L.duration = 80.f;
        L.locked = true;
        auto &e = L.events;
        e.push_back(makeEvent(1.0f, EnemyType::Medium, {240, -40}, pathSweepCenter(), 2, 80.f));
        e.push_back(makeEvent(3.0f, EnemyType::Small, {120, -40}, pathSweepLeft(), 3));
        e.push_back(makeEvent(3.0f, EnemyType::Small, {360, -40}, pathSweepRight(), 3));
        e.push_back(makeEvent(6.0f, EnemyType::Medium, {60, -40}, pathZPattern()));
        e.push_back(makeEvent(6.0f, EnemyType::Small, {240, -40}, pathSweepCenter(), 4));
        e.push_back(makeEvent(10.0f, EnemyType::Medium, {160, -40}, pathDiveBomb()));
        e.push_back(makeEvent(10.0f, EnemyType::Medium, {320, -40}, pathDiveBomb()));
        e.push_back(makeEvent(10.0f, EnemyType::Small, {120, -40}, pathLoopLeft(), 2));
        e.push_back(makeEvent(14.0f, EnemyType::Small, {240, -40}, pathSweepCenter(), 5, 42.f));
        e.push_back(makeEvent(14.0f, EnemyType::Medium, {120, -40}, pathSweepLeft()));
        e.push_back(makeEvent(14.0f, EnemyType::Medium, {360, -40}, pathSweepRight()));
        e.push_back(makeEvent(18.0f, EnemyType::Small, {60, -40}, pathZPattern(), 2));
        e.push_back(makeEvent(18.0f, EnemyType::Small, {420, -40}, pathZPatternRight(), 2));
        e.push_back(makeEvent(20.0f, EnemyType::Medium, {160, -40}, pathSweepCenter()));
        e.push_back(makeEvent(20.0f, EnemyType::Medium, {320, -40}, pathSweepCenter()));
        e.push_back(makeEvent(20.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        e.push_back(makeEvent(24.0f, EnemyType::Small, {240, -40}, pathSweepCenter(), 5, 45.f));
        e.push_back(makeEvent(24.0f, EnemyType::Medium, {120, -40}, pathLoopLeft()));
        e.push_back(makeEvent(28.0f, EnemyType::Medium, {200, -40}, pathSweepCenter(), 3, 60.f));
        e.push_back(makeEvent(28.0f, EnemyType::Small, {60, -40}, pathZPattern(), 3));
        e.push_back(makeEvent(32.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        e.push_back(makeEvent(32.0f, EnemyType::Medium, {120, -40}, pathSweepLeft()));
        e.push_back(makeEvent(32.0f, EnemyType::Medium, {360, -40}, pathSweepRight()));
        e.push_back(makeEvent(36.0f, EnemyType::Small, {240, -40}, pathSweepCenter(), 5, 40.f));
        e.push_back(makeEvent(36.0f, EnemyType::Medium, {160, -40}, pathLoopLeft(), 2));
        e.push_back(makeEvent(40.0f, EnemyType::Medium, {200, -40}, pathSweepCenter(), 3, 70.f));
        e.push_back(makeEvent(40.0f, EnemyType::Small, {420, -40}, pathZPatternRight(), 3));
        e.push_back(makeEvent(44.0f, EnemyType::Small, {240, -40}, pathSweepCenter(), 5, 42.f));
        e.push_back(makeEvent(44.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        e.push_back(makeEvent(48.0f, EnemyType::Medium, {160, -40}, pathSweepCenter(), 2, 80.f));
        e.push_back(makeEvent(48.0f, EnemyType::Small, {120, -40}, pathSweepLeft(), 3));
        e.push_back(makeEvent(52.0f, EnemyType::Small, {240, -40}, pathSweepCenter(), 4));
        e.push_back(makeEvent(52.0f, EnemyType::Medium, {240, -40}, pathDiveBomb()));
        e.push_back(makeEvent(57.0f, EnemyType::Boss, {160, -80}, pathBossEnterLeft()));
        e.push_back(makeEvent(70.0f, EnemyType::Boss, {320, -80}, pathBossEnterRight()));
        levels.push_back(L);
    }
}
void Game::init()
{
    std::srand((unsigned)std::time(nullptr));

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

    if (!font.loadFromFile("assets/fonts/ProFontWindows.ttf"))
    {
        if (!font.loadFromFile("ProFontWindows.ttf"))
        {
            std::cerr << "Warning: Could not load font. Text may not render.\n";
        }
    }
    if (!shipTexture.loadFromFile("assets/textures/spaceship.png"))
    {
        MessageBoxA(NULL, "Failed to load assets/textures/spaceship.png! Check build folder.", "File Error", MB_ICONERROR);
        window.close(); // Stop the game if assets are missing
    }
    else
    {
        std::cout << "Loaded texture" << std::endl;
        sf::Vector2u sz = shipTexture.getSize();
        std::cerr << "Texture loaded OK: " << sz.x << "x" << sz.y << "\n";
        shipSprite.setScale(0.05f, 0.05f);
    }
    shipSprite.setTexture(shipTexture);
    sf::FloatRect bounds = shipSprite.getLocalBounds();
    shipSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);
    for (auto &b : bullets)
        b.on = false;
    for (auto &e : enemies)
        e.on = false;
    for (auto &p : particles)
        p.on = false;
    for (auto &pk : pickups)
        pk.on = false;
    buildLevels();
    initStars();
    buildStationTile();
    stateStack.push(GameState::LevelSelect);
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
                {
                    stateStack.push(GameState::Paused);
                }
            }
            else if (current == GameState::Paused)
            {
                if (event.key.code == sf::Keyboard::P || event.key.code == sf::Keyboard::Escape)
                {
                    stateStack.pop();
                }
            }
        }
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
        {
            sf::Vector2f mp = window.mapPixelToCoords({event.mouseButton.x, event.mouseButton.y});
            if (current == GameState::LevelSelect)
            {
                float planetY = 320.f;
                for (int i = 0; i < 3; i++)
                {
                    float px = 120.f + i * 120.f;
                    float dist = std::hypot(mp.x - px, mp.y - planetY);
                    if (dist < 50.f)
                    {
                        selectedLevel = i;
                    }
                }
                if (mp.x < 50.f && mp.y > 274.f && mp.y < 366.f)
                    selectedLevel = std::max(0, selectedLevel - 1);
                if (mp.x > 430.f && mp.y > 274.f && mp.y < 366.f)
                    selectedLevel = std::min(2, selectedLevel + 1);
                sf::FloatRect playRect(330.f, 657.f, 110.f, 40.f);
                if (playRect.contains(mp) && !levels[selectedLevel].locked)
                {
                    startLevel(selectedLevel);
                }
                sf::FloatRect backRect(40.f, 657.f, 110.f, 40.f);
                if (backRect.contains(mp))
                {
                    window.close();
                }
            }
            else if (current == GameState::Paused)
            {
                // Check if exit to menu button was clicked
                sf::FloatRect exitBtn(160.f, 358.f, 160.f, 34.f);
                if (exitBtn.contains(mp))
                {
                    while (!stateStack.empty())
                        stateStack.pop();
                    stateStack.push(GameState::LevelSelect);
                }
                else
                {
                    stateStack.pop(); // resume
                }
            }
            else if (current == GameState::GameOver)
            {
                startLevel(currentLevel);
            }
            else if (current == GameState::LevelComplete)
            {
                if (currentLevel + 1 < (int)levels.size())
                {
                    levels[currentLevel + 1].locked = false;
                }
                while (stateStack.size() > 1)
                    stateStack.pop();
                if (stateStack.top() != GameState::LevelSelect)
                {
                    stateStack.pop();
                    stateStack.push(GameState::LevelSelect);
                }
            }
            else if (current == GameState::Playing)
            {
                sf::FloatRect pauseBtn(430.f, 12.f, 36.f, 36.f);
                if (pauseBtn.contains(mp))
                {
                    stateStack.push(GameState::Paused);
                }
                sf::FloatRect exitBtn(430.f, 48.f, 36.f, 36.f);
                if (exitBtn.contains(mp))
                {
                    while (!stateStack.empty())
                        stateStack.pop();
                    stateStack.push(GameState::LevelSelect);
                }
            }
        }
    }
}
void Game::startLevel(int index)
{
    currentLevel = index;
    levelTimer = 0.f;
    bossActive = false;
    for (auto &b : bullets)
        b.on = false;
    for (auto &e : enemies)
        e.on = false;
    for (auto &p : particles)
        p.on = false;
    for (auto &pk : pickups)
        pk.on = false;
    while (!spawnQueue.empty())
        spawnQueue.pop();
    for (auto &ev : levels[currentLevel].events)
    {
        ev.fired = false;
    }
    scoreLog.clear();
    resetPlayer();
    while (!stateStack.empty())
        stateStack.pop();
    stateStack.push(GameState::Playing);
}
void Game::resetPlayer()
{
    player.pos = {240.f, 600.f};
    player.hp = player.maxHp;
    player.speed = 310.f;
    player.power = 1;
    player.powerTimer = 0.f;
    player.shootTimer = 0.f;
    player.iframeTimer = 0.f;
    player.shieldTimer = 0.f;
    player.tilt = 0.f;
    player.alive = true;
    player.score = 0;
    player.lives = 3;
}
void Game::update(float dt)
{
    if (!player.alive)
        return;
    levelTimer += dt;
    sf::Vector2f moveDir(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        moveDir.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        moveDir.x += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        moveDir.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
        moveDir.y += 1.f;
    float ml = vlen(moveDir);
    if (ml > 0.f)
    {
        moveDir = vnorm(moveDir);
        player.pos += moveDir * player.speed * dt;
    }
    player.pos.x = clampf(player.pos.x, 24.f, 456.f);
    player.pos.y = clampf(player.pos.y, 24.f, 696.f);
    float targetTilt = moveDir.x * 18.f;
    player.tilt = lerp(player.tilt, targetTilt, dt * 8.f);
    if (player.iframeTimer > 0.f)
    {
        player.iframeTimer -= dt;
    }
    if (player.shieldTimer > 0.f)
    {
        player.shieldTimer -= dt;
    }
    if (player.power > 1)
    {
        player.powerTimer -= dt;
        if (player.powerTimer <= 0.f)
        {
            player.power -= 1;
            if (player.power > 1)
            {
                player.powerTimer = 8.0f;
            }
        }
    }
    player.shootTimer -= dt;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && player.shootTimer <= 0.f)
    {
        spawnPlayerBullet();
        player.shootTimer = player.shootInterval;
    }
    checkSpawns(dt);
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
    for (auto &e : enemies)
    {
        if (!e.on)
            continue;
        if (e.etype == EnemyType::Boss)
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
                if (e.etype != EnemyType::Boss)
                {
                    float targetAngle = std::atan2(dir.x, -dir.y) * 180.f / 3.14159f;
                    e.angle = lerp(e.angle, targetAngle, dt * 4.f);
                }
            }
        }
        else if (e.etype == EnemyType::Boss)
        {
            // Path done: oscillate side-to-side, stay in upper third
            e.pos.x += std::sin(e.pulse * 0.7f) * 60.f * dt;
            e.pos.x = clampf(e.pos.x, 80.f, 400.f);
            e.pos.y = clampf(e.pos.y, 60.f, 340.f); // never past halfway (360)
        }
        else
        {
            // Non-boss: drift off bottom
            e.pos.y += 100.f * dt;
            if (e.pos.y > 800.f)
            {
                e.on = false;
                continue;
            }
        }
        if (e.etype == EnemyType::Boss && e.phase == 0 && e.hp < e.maxHp * 0.4f)
        {
            e.phase = 1;
            spawnExplosion(e.pos, sf::Color(80, 100, 255), sf::Color(20, 40, 120, 0), 16);
        }
        e.shootTimer -= dt;
        if (e.shootTimer <= 0.f)
        {
            spawnEnemyBullet(e);
            if (e.etype == EnemyType::Boss)
            {
                e.shootTimer = (e.phase == 0) ? 0.8f : 0.6f;
            }
            else
            {
                e.shootTimer = e.shootInterval;
            }
        }
    }
    for (auto &b : bullets)
    {
        if (!b.on)
            continue;
        b.pos += b.vel * dt;
        if (b.pos.y < -20.f || b.pos.y > 740.f || b.pos.x < -20.f || b.pos.x > 500.f)
        {
            b.on = false;
        }
    }
    for (auto &p : particles)
    {
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
    for (auto &pk : pickups)
    {
        if (!pk.on)
            continue;
        pk.life -= dt;
        pk.pulse += dt;
        if (pk.life <= 0.f)
        {
            pk.on = false;
            continue;
        }
        pk.pos += pk.vel * dt;
        pk.vel.y += 20.f * dt;
    }
    bgY += 60.f * dt;
    if (bgY >= 1440.f)
        bgY -= 1440.f;
    for (auto &s : stars)
    {
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
    for (auto &b : bullets)
    {
        if (!b.on)
            continue;
        if (b.btype != BulletType::PlayerNorm && b.btype != BulletType::PlayerWide &&
            b.btype != BulletType::PlayerTriple)
            continue;
        for (auto &e : enemies)
        {
            if (!e.on)
                continue;
            float eRadius = 16.f;
            if (e.etype == EnemyType::Medium)
                eRadius = 22.f;
            if (e.etype == EnemyType::Boss)
                eRadius = 60.f;
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
                    if (e.etype == EnemyType::Small)
                    {
                        cStart = sf::Color(255, 160, 30);
                        cEnd = sf::Color(200, 40, 10, 0);
                    }
                    else if (e.etype == EnemyType::Medium)
                    {
                        cStart = sf::Color(80, 255, 120);
                        cEnd = sf::Color(20, 150, 50, 0);
                        pCount = 18;
                    }
                    else
                    {
                        cStart = sf::Color(255, 80, 80);
                        cEnd = sf::Color(80, 20, 20, 0);
                        pCount = 40;
                    }
                    spawnExplosion(e.pos, cStart, cEnd, pCount);
                    player.score += e.scoreValue;
                    scoreLog.push_front({(e.etype == EnemyType::Small ? "Small enemy" : e.etype == EnemyType::Medium ? "Medium enemy"
                                                                                                                     : "BOSS"),
                                         e.scoreValue, levelTimer});
                    spawnPickup(e.pos, PickupType::Score);
                    if (e.etype == EnemyType::Medium && e.hp < 40.f)
                    {
                        spawnPickup(e.pos + sf::Vector2f(15.f, 0.f), PickupType::Health);
                    }
                    if (e.etype == EnemyType::Boss)
                    {
                        for (int i = 0; i < 8; i++)
                        {
                            sf::Vector2f offset(randFloat(-40.f, 40.f), randFloat(-40.f, 40.f));
                            PickupType pt = (i % 3 == 0) ? PickupType::Health : PickupType::Score;
                            spawnPickup(e.pos + offset, pt);
                        }
                    }
                }
                break;
            }
        }
    }
    if (player.iframeTimer <= 0.f)
    {
        for (auto &b : bullets)
        {
            if (!b.on)
                continue;
            if (b.btype == BulletType::PlayerNorm || b.btype == BulletType::PlayerWide ||
                b.btype == BulletType::PlayerTriple)
                continue;
            float dist = vlen(b.pos - player.pos);
            if (dist < 5.f + 18.f)
            {
                b.on = false;
                if (player.shieldTimer > 0.f)
                {
                    player.shieldTimer = 0.f;
                    spawnExplosion(player.pos, sf::Color(60, 160, 255), sf::Color(20, 80, 180, 0), 10);
                }
                else
                {
                    player.hp -= 12.f;
                    player.iframeTimer = 1.4f;
                    spawnExplosion(player.pos, sf::Color(255, 80, 80), sf::Color(255, 200, 50, 0), 6);
                    if (player.hp <= 0.f)
                    {
                        player.lives--;
                        if (player.lives <= 0)
                        {
                            player.alive = false;
                            stateStack.push(GameState::GameOver);
                        }
                        else
                        {
                            player.hp = player.maxHp;
                            player.iframeTimer = 2.0f;
                        }
                    }
                }
            }
        }
    }
    if (player.iframeTimer <= 0.f)
    {
        for (auto &e : enemies)
        {
            if (!e.on)
                continue;
            float eRadius = 16.f;
            if (e.etype == EnemyType::Medium)
                eRadius = 22.f;
            if (e.etype == EnemyType::Boss)
                eRadius = 60.f;
            float dist = vlen(e.pos - player.pos);
            if (dist < eRadius + 18.f)
            {
                if (player.shieldTimer > 0.f)
                {
                    player.shieldTimer = 0.f;
                    spawnExplosion(player.pos, sf::Color(60, 160, 255), sf::Color(20, 80, 180, 0), 10);
                }
                else
                {
                    player.hp -= 20.f;
                    player.iframeTimer = 1.4f;
                    spawnExplosion(player.pos, sf::Color(255, 80, 80), sf::Color(255, 200, 50, 0), 8);
                    if (player.hp <= 0.f)
                    {
                        player.lives--;
                        if (player.lives <= 0)
                        {
                            player.alive = false;
                            stateStack.push(GameState::GameOver);
                        }
                        else
                        {
                            player.hp = player.maxHp;
                            player.iframeTimer = 2.0f;
                        }
                    }
                }
            }
        }
    }
    for (auto &pk : pickups)
    {
        if (!pk.on)
            continue;
        float dist = vlen(pk.pos - player.pos);
        if (dist < 32.f + 18.f)
        {
            pk.on = false;
            if (pk.ptype == PickupType::Score)
            {
                player.score += 25;
                scoreLog.push_front({"Score orb", 25, levelTimer});
            }
            else if (pk.ptype == PickupType::Health)
            {
                player.hp = std::min(player.hp + 25.f, player.maxHp);
            }
            else if (pk.ptype == PickupType::Power)
            {
                if (player.power < 3)
                    player.power++;
                player.powerTimer = 8.0f;
                player.shieldTimer = 8.0f;
            }
        }
    }
}
void Game::checkSpawns(float dt)
{
    for (auto &ev : levels[currentLevel].events)
    {
        if (!ev.fired && levelTimer >= ev.time)
        {
            ev.fired = true;
            float hp = 0.f, maxHp = 0.f, shootInt = 0.f, moveSpd = 0.f;
            int scoreVal = 0;
            switch (ev.etype)
            {
            case EnemyType::Small:
                hp = maxHp = 30.f;
                shootInt = 2.2f;
                scoreVal = 50;
                moveSpd = 200.f;
                break;
            case EnemyType::Medium:
                hp = maxHp = 80.f;
                shootInt = 1.8f;
                scoreVal = 120;
                moveSpd = 160.f;
                break;
            case EnemyType::Boss:
                hp = maxHp = 1200.f;
                shootInt = 0.8f;
                scoreVal = 2000;
                moveSpd = 80.f;
                break;
            }
            for (int i = 0; i < ev.count; i++)
            {
                SpawnJob job;
                job.etype = ev.etype;
                job.startPos = ev.startPos;
                job.startPos.x += (i - (ev.count - 1) / 2.f) * ev.xSpacing;
                job.delayRemaining = i * ev.delay;
                job.path = ev.path;
                if (!job.path.empty())
                {
                    job.path[0] = job.startPos;
                }
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
    Enemy *e = allocEnemy();
    if (!e)
        return;
    e->on = true;
    e->etype = job.etype;
    e->pos = job.startPos;
    e->vel = {0.f, 0.f};
    e->hp = job.hp;
    e->maxHp = job.maxHp;
    e->shootTimer = job.shootInterval * randFloat(0.3f, 1.0f);
    e->shootInterval = job.shootInterval;
    e->path = job.path;
    e->pathIndex = 0;
    e->angle = 0.f;
    e->pulse = 0.f;
    e->phase = 0;
    e->scoreValue = job.scoreValue;
    e->moveSpeed = job.moveSpeed;
}
void Game::spawnPlayerBullet()
{
    if (player.power == 1)
    {
        Bullet *b = allocBullet();
        if (b)
        {
            b->on = true;
            b->pos = player.pos + sf::Vector2f(0.f, -20.f);
            b->vel = {0.f, -600.f};
            b->dmg = 20.f;
            b->btype = BulletType::PlayerNorm;
        }
    }
    else if (player.power == 2)
    {
        for (int i = -1; i <= 1; i += 2)
        {
            Bullet *b = allocBullet();
            if (b)
            {
                b->on = true;
                b->pos = player.pos + sf::Vector2f(i * 7.f, -20.f);
                b->vel = {0.f, -600.f};
                b->dmg = 20.f;
                b->btype = BulletType::PlayerWide;
            }
        }
    }
    else
    {
        for (int i = -1; i <= 1; i++)
        {
            Bullet *b = allocBullet();
            if (b)
            {
                b->on = true;
                b->pos = player.pos + sf::Vector2f(i * 12.f, -20.f);
                b->vel = {i * 40.f, -600.f};
                b->dmg = 20.f;
                b->btype = BulletType::PlayerTriple;
            }
        }
    }
}
void Game::spawnEnemyBullet(Enemy &e)
{
    sf::Vector2f toPlayer = player.pos - e.pos;
    float baseAngle = std::atan2(toPlayer.y, toPlayer.x);
    if (e.etype == EnemyType::Small)
    {
        sf::Vector2f dir = vnorm(toPlayer);
        Bullet *b = allocBullet();
        if (b)
        {
            b->on = true;
            b->pos = e.pos;
            b->vel = dir * 260.f;
            b->dmg = 12.f;
            b->btype = BulletType::EnemyNorm;
        }
    }
    else if (e.etype == EnemyType::Medium)
    {
        for (int i = -1; i <= 1; i++)
        {
            float a = baseAngle + i * 0.2f;
            sf::Vector2f dir = {std::cos(a), std::sin(a)};
            Bullet *b = allocBullet();
            if (b)
            {
                b->on = true;
                b->pos = e.pos;
                b->vel = dir * 240.f;
                b->dmg = 12.f;
                b->btype = BulletType::EnemyBurst;
            }
        }
    }
    else if (e.etype == EnemyType::Boss)
    {
        int numShots = (e.phase == 0) ? 5 : 8;
        float spread = 0.22f;
        float bulletSpeed = (e.phase == 0) ? 220.f : 280.f;
        for (int i = 0; i < numShots; i++)
        {
            float a = baseAngle + (i - (numShots - 1) / 2.f) * spread;
            sf::Vector2f dir = {std::cos(a), std::sin(a)};
            Bullet *b = allocBullet();
            if (b)
            {
                b->on = true;
                b->pos = e.pos;
                b->vel = dir * bulletSpeed;
                b->dmg = 12.f;
                b->btype = (i == numShots / 2) ? BulletType::BossBeam : BulletType::EnemyBurst;
            }
        }
    }
}
Bullet *Game::allocBullet()
{
    for (auto &b : bullets)
        if (!b.on)
            return &b;
    return nullptr;
}
Enemy *Game::allocEnemy()
{
    for (auto &e : enemies)
        if (!e.on)
            return &e;
    return nullptr;
}
Particle *Game::allocParticle()
{
    for (auto &p : particles)
        if (!p.on)
            return &p;
    return nullptr;
}
Pickup *Game::allocPickup()
{
    for (auto &pk : pickups)
        if (!pk.on)
            return &pk;
    return nullptr;
}
void Game::spawnExplosion(sf::Vector2f pos, sf::Color cStart, sf::Color cEnd, int count)
{
    for (int i = 0; i < count; i++)
    {
        Particle *p = allocParticle();
        if (!p)
            break;
        p->on = true;
        p->pos = pos + sf::Vector2f(randFloat(-6.f, 6.f), randFloat(-6.f, 6.f));
        float angle = randFloat(0.f, 6.2832f);
        float speed = randFloat(40.f, 180.f);
        p->vel = {std::cos(angle) * speed, std::sin(angle) * speed};
        p->colorStart = cStart;
        p->colorEnd = cEnd;
        p->maxLife = randFloat(0.3f, 0.8f);
        p->life = p->maxLife;
        p->size = randFloat(1.5f, 4.f);
    }
}
void Game::spawnPickup(sf::Vector2f pos, PickupType ptype)
{
    Pickup *pk = allocPickup();
    if (!pk)
        return;
    pk->on = true;
    pk->pos = pos;
    pk->vel = {randFloat(-30.f, 30.f), randFloat(-50.f, -10.f)};
    pk->ptype = ptype;
    pk->life = 8.f;
    pk->pulse = 0.f;
}
bool Game::isLevelClear()
{
    for (auto &ev : levels[currentLevel].events)
    {
        if (!ev.fired)
            return false;
    }
    if (!spawnQueue.empty())
        return false;
    for (auto &e : enemies)
    {
        if (e.on)
            return false;
    }
    return true;
}
int Game::countActiveEnemies()
{
    int count = 0;
    for (auto &e : enemies)
    {
        if (e.on)
            count++;
    }
    return count;
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
void Game::buildStationTile()
{
    stationTile.create(480, 1440);
    stationTile.clear(sf::Color(6, 8, 14));
    sf::RectangleShape panel;
    panel.setSize({60.f, 1440.f});
    panel.setPosition(210.f, 0.f);
    panel.setFillColor(sf::Color(14, 18, 28));
    stationTile.draw(panel);
    for (int y = 0; y < 1440; y += 120)
    {
        panel.setSize({sf::Vector2f(180.f, 80.f)});
        panel.setPosition(10.f, (float)y);
        panel.setFillColor(sf::Color(10, 14, 22));
        panel.setOutlineColor(sf::Color(20, 28, 45));
        panel.setOutlineThickness(1.f);
        stationTile.draw(panel);
        panel.setPosition(290.f, (float)y);
        stationTile.draw(panel);
        sf::RectangleShape bar({460.f, 3.f});
        bar.setPosition(10.f, y + 90.f);
        bar.setFillColor(sf::Color(18, 24, 40));
        stationTile.draw(bar);
        for (int x = 0; x < 4; x++)
        {
            sf::CircleShape light(2.f);
            light.setOrigin(2.f, 2.f);
            light.setPosition(50.f + x * 110.f, y + 40.f);
            light.setFillColor(sf::Color(30, 50, 80, 120));
            stationTile.draw(light);
        }
        sf::RectangleShape pipe({4.f, 80.f});
        pipe.setFillColor(sf::Color(15, 20, 35));
        pipe.setPosition(195.f, (float)y);
        stationTile.draw(pipe);
        pipe.setPosition(281.f, (float)y);
        stationTile.draw(pipe);
    }
    for (int i = 0; i < 40; i++)
    {
        float bx = randFloat(10.f, 460.f);
        float by = randFloat(0.f, 1430.f);
        float bw = randFloat(8.f, 30.f);
        float bh = randFloat(6.f, 20.f);
        sf::RectangleShape greeble({bw, bh});
        greeble.setPosition(bx, by);
        greeble.setFillColor(sf::Color(12, 16, 26));
        greeble.setOutlineColor(sf::Color(16, 22, 38));
        greeble.setOutlineThickness(0.5f);
        stationTile.draw(greeble);
    }
    stationTile.display();
}
void Game::render()
{
    window.setView(window.getDefaultView());
    window.clear(sf::Color::Black);
    window.setView(gameView);
    GameState current = stateStack.top();
    if (current == GameState::LevelSelect)
    {
        renderLevelSelect();
        return;
    }
    renderBackground();
    renderStars();
    renderPickups();
    renderEnemies();
    renderBullets();
    renderParticles();
    renderPlayer();
    renderHUD();
    if (bossActive)
        renderBossHP();
    sf::RectangleShape vig;
    vig.setFillColor(sf::Color(4, 6, 12, 180));
    vig.setSize({480.f, 12.f});
    vig.setPosition(0.f, 0.f);
    window.draw(vig);
    vig.setPosition(0.f, 708.f);
    window.draw(vig);
    vig.setSize({8.f, 720.f});
    vig.setPosition(0.f, 0.f);
    window.draw(vig);
    vig.setPosition(472.f, 0.f);
    window.draw(vig);
    if (current == GameState::Paused)
        renderPauseOverlay();
    if (current == GameState::GameOver)
        renderGameOverOverlay();
    if (current == GameState::LevelComplete)
        renderLevelCompleteOverlay();
}
void Game::renderBackground()
{
    sf::Color planetCol = levels[currentLevel].planetColor;
    sf::CircleShape planet(120.f);
    planet.setOrigin(120.f, 120.f);
    planet.setPosition(360.f, 150.f);
    planet.setFillColor(sf::Color(planetCol.r / 3, planetCol.g / 3, planetCol.b / 3, 60));
    window.draw(planet);
    sf::CircleShape atmo(140.f);
    atmo.setOrigin(140.f, 140.f);
    atmo.setPosition(360.f, 150.f);
    atmo.setFillColor(sf::Color(planetCol.r / 2, planetCol.g / 2, planetCol.b / 2, 25));
    sf::RenderStates glowState;
    glowState.blendMode = sf::BlendAdd;
    window.draw(atmo, glowState);
    sf::Sprite tile(stationTile.getTexture());
    tile.setColor(sf::Color(255, 255, 255, 80));
    tile.setPosition(0.f, bgY - 1440.f);
    window.draw(tile);
    tile.setPosition(0.f, bgY);
    window.draw(tile);
}
void Game::renderStars()
{
    for (auto &s : stars)
    {
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
    if (!player.alive)
        return;
    if (player.iframeTimer > 0.f)
    {
        if ((int)(player.iframeTimer * 10.f) % 2 == 0)
            return;
    }
    drawPlayerShip(player.pos, player.tilt, 0.3f, sf::Color::White);
    if (player.shieldTimer > 0.f)
    {
        sf::CircleShape shield(24.f);
        shield.setOrigin(24.f, 24.f);
        shield.setPosition(player.pos);
        shield.setFillColor(sf::Color::Transparent);
        shield.setOutlineColor(sf::Color(60, 160, 255, 140));
        shield.setOutlineThickness(2.f);
        window.draw(shield);
        sf::CircleShape glow(28.f);
        glow.setOrigin(28.f, 28.f);
        glow.setPosition(player.pos);
        glow.setFillColor(sf::Color(60, 160, 255, 25));
        sf::RenderStates gs;
        gs.blendMode = sf::BlendAdd;
        window.draw(glow, gs);
    }
}
void Game::drawPlayerShip(sf::Vector2f pos, float tilt, float scale, sf::Color tint)
{
    sf::Transform T;
    T.translate(pos);
    T.rotate(tilt);
    T.scale(scale, scale);
    sf::RenderStates states;
    states.transform = T;
    shipSprite.setPosition(pos);
    shipSprite.setRotation(tilt);
    shipSprite.setScale(scale, scale);
    window.draw(shipSprite);
    sf::CircleShape engineGlow(8.f);
    engineGlow.setOrigin(8.f, 8.f);
    engineGlow.setPosition(0.f, 18.f);
    engineGlow.setFillColor(sf::Color(60, 180, 255, 80));
    sf::RenderStates glowState;
    glowState.transform = T;
    glowState.blendMode = sf::BlendAdd;
    window.draw(engineGlow, glowState);
    sf::CircleShape engineCore(4.f);
    engineCore.setOrigin(4.f, 4.f);
    engineCore.setPosition(0.f, 16.f);
    engineCore.setFillColor(sf::Color(140, 220, 255, 200));
    window.draw(engineCore, glowState);
    sf::CircleShape tipL(2.f);
    tipL.setOrigin(2.f, 2.f);
    tipL.setPosition(-23.f, 12.f);
    tipL.setFillColor(sf::Color(255, 80, 60, 180));
    window.draw(tipL, states);
    sf::CircleShape tipR(2.f);
    tipR.setOrigin(2.f, 2.f);
    tipR.setPosition(23.f, 12.f);
    tipR.setFillColor(sf::Color(80, 255, 80, 180));
    window.draw(tipR, states);
}
void Game::renderEnemies()
{
    for (auto &e : enemies)
    {
        if (!e.on)
            continue;
        if (e.etype == EnemyType::Small)
            drawSmallEnemy(e.pos, e.angle, e.pulse);
        else if (e.etype == EnemyType::Medium)
            drawMediumEnemy(e.pos, e.angle, e.pulse);
        else if (e.etype == EnemyType::Boss)
            drawBossEnemy(e.pos, e.angle, e.pulse, e.phase);
    }
}
void Game::drawSmallEnemy(sf::Vector2f pos, float angle, float pulse)
{
    sf::Transform T;
    T.translate(pos);
    T.rotate(angle + 180.f);
    sf::RenderStates states;
    states.transform = T;
    sf::CircleShape glow(6.f);
    glow.setOrigin(6.f, 6.f);
    glow.setPosition(0.f, -14.f);
    float flicker = 0.6f + 0.4f * std::sin(pulse * 12.f);
    glow.setFillColor(sf::Color(255, 120, 40, (sf::Uint8)(60 * flicker)));
    sf::RenderStates gs;
    gs.transform = T;
    gs.blendMode = sf::BlendAdd;
    window.draw(glow, gs);
    sf::ConvexShape body(4);
    body.setPoint(0, {0.f, 14.f});
    body.setPoint(1, {7.f, -4.f});
    body.setPoint(2, {0.f, -10.f});
    body.setPoint(3, {-7.f, -4.f});
    body.setFillColor(sf::Color(140, 50, 50));
    window.draw(body, states);
    sf::ConvexShape wL(3);
    wL.setPoint(0, {-5.f, -2.f});
    wL.setPoint(1, {-16.f, 6.f});
    wL.setPoint(2, {-4.f, 8.f});
    wL.setFillColor(sf::Color(120, 35, 35));
    window.draw(wL, states);
    sf::ConvexShape wR(3);
    wR.setPoint(0, {5.f, -2.f});
    wR.setPoint(1, {16.f, 6.f});
    wR.setPoint(2, {4.f, 8.f});
    wR.setFillColor(sf::Color(120, 35, 35));
    window.draw(wR, states);
    sf::CircleShape cockpit(2.5f);
    cockpit.setOrigin(2.5f, 2.5f);
    cockpit.setPosition(0.f, 4.f);
    cockpit.setFillColor(sf::Color(255, 60, 60, 200));
    window.draw(cockpit, states);
}
void Game::drawMediumEnemy(sf::Vector2f pos, float angle, float pulse)
{
    sf::Transform T;
    T.translate(pos);
    T.rotate(angle + 180.f);
    sf::RenderStates states;
    states.transform = T;
    sf::CircleShape glow(10.f);
    glow.setOrigin(10.f, 10.f);
    glow.setPosition(0.f, -18.f);
    float flicker = 0.5f + 0.5f * std::sin(pulse * 10.f);
    glow.setFillColor(sf::Color(60, 120, 255, (sf::Uint8)(70 * flicker)));
    sf::RenderStates gs;
    gs.transform = T;
    gs.blendMode = sf::BlendAdd;
    window.draw(glow, gs);
    sf::ConvexShape body(5);
    body.setPoint(0, {0.f, 20.f});
    body.setPoint(1, {12.f, -2.f});
    body.setPoint(2, {8.f, -16.f});
    body.setPoint(3, {-8.f, -16.f});
    body.setPoint(4, {-12.f, -2.f});
    body.setFillColor(sf::Color(50, 60, 100));
    window.draw(body, states);
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
    sf::CircleShape cockpit(4.f);
    cockpit.setOrigin(4.f, 4.f);
    cockpit.setPosition(0.f, 6.f);
    cockpit.setFillColor(sf::Color(80, 140, 255, 200));
    window.draw(cockpit, states);
}
void Game::drawBossEnemy(sf::Vector2f pos, float angle, float pulse, int phase)
{
    sf::Transform T;
    T.translate(pos);
    sf::RenderStates states;
    states.transform = T;
    sf::CircleShape bigGlow(70.f);
    bigGlow.setOrigin(70.f, 70.f);
    bigGlow.setPosition(0.f, 0.f);
    sf::Color glowCol = (phase == 0) ? sf::Color(30, 50, 120, 20) : sf::Color(120, 40, 20, 25);
    bigGlow.setFillColor(glowCol);
    sf::RenderStates gs;
    gs.transform = T;
    gs.blendMode = sf::BlendAdd;
    window.draw(bigGlow, gs);
    sf::ConvexShape hull(6);
    hull.setPoint(0, {0.f, -45.f});
    hull.setPoint(1, {35.f, -15.f});
    hull.setPoint(2, {40.f, 25.f});
    hull.setPoint(3, {0.f, 40.f});
    hull.setPoint(4, {-40.f, 25.f});
    hull.setPoint(5, {-35.f, -15.f});
    hull.setFillColor(sf::Color(55, 50, 65));
    window.draw(hull, states);
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
    sf::CircleShape dome(12.f);
    dome.setOrigin(12.f, 12.f);
    dome.setPosition(0.f, -15.f);
    if (phase == 0)
        dome.setFillColor(sf::Color(60, 100, 200, 200));
    else
        dome.setFillColor(sf::Color(200, 80, 30, 220));
    window.draw(dome, states);
    sf::CircleShape domeGlow(8.f);
    domeGlow.setOrigin(8.f, 8.f);
    domeGlow.setPosition(0.f, -15.f);
    if (phase == 0)
        domeGlow.setFillColor(sf::Color(100, 160, 255, 100));
    else
        domeGlow.setFillColor(sf::Color(255, 120, 40, 120));
    window.draw(domeGlow, gs);
    for (int i = -1; i <= 1; i++)
    {
        float ex = i * 18.f;
        float ey = 36.f;
        float flicker = 0.5f + 0.5f * std::sin(pulse * 8.f + i * 1.5f);
        sf::CircleShape eng(7.f);
        eng.setOrigin(7.f, 7.f);
        eng.setPosition(ex, ey);
        eng.setFillColor(sf::Color(100, 140, 255, (sf::Uint8)(80 * flicker)));
        window.draw(eng, gs);
        sf::CircleShape engCore(3.f);
        engCore.setOrigin(3.f, 3.f);
        engCore.setPosition(ex, ey);
        engCore.setFillColor(sf::Color(180, 220, 255, (sf::Uint8)(180 * flicker)));
        window.draw(engCore, gs);
    }
}
void Game::renderBullets()
{
    sf::RenderStates glowState;
    glowState.blendMode = sf::BlendAdd;
    for (auto &b : bullets)
    {
        if (!b.on)
            continue;
        if (b.btype == BulletType::PlayerNorm || b.btype == BulletType::PlayerWide ||
            b.btype == BulletType::PlayerTriple)
        {
            sf::RectangleShape streak({2.f, 12.f});
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
        else if (b.btype == BulletType::EnemyNorm || b.btype == BulletType::EnemyBurst)
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
        else if (b.btype == BulletType::BossBeam)
        {
            sf::RectangleShape beam({5.f, 14.f});
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
    for (auto &p : particles)
    {
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
void Game::renderPickups()
{
    sf::RenderStates glowState;
    glowState.blendMode = sf::BlendAdd;
    for (auto &pk : pickups)
    {
        if (!pk.on)
            continue;
        sf::Color col;
        if (pk.ptype == PickupType::Score)
            col = sf::Color(60, 120, 255);
        else if (pk.ptype == PickupType::Health)
            col = sf::Color(60, 220, 80);
        else
            col = sf::Color(255, 200, 40);
        float bobble = std::sin(pk.pulse * 5.f) * 0.2f + 1.0f;
        sf::CircleShape glow(12.f * bobble);
        glow.setOrigin(12.f * bobble, 12.f * bobble);
        glow.setPosition(pk.pos);
        glow.setFillColor(sf::Color(col.r, col.g, col.b, 30));
        window.draw(glow, glowState);
        sf::CircleShape orb(6.f * bobble);
        orb.setOrigin(6.f * bobble, 6.f * bobble);
        orb.setPosition(pk.pos);
        orb.setFillColor(sf::Color(col.r, col.g, col.b, 200));
        window.draw(orb);
        sf::CircleShape inner(3.f * bobble);
        inner.setOrigin(3.f * bobble, 3.f * bobble);
        inner.setPosition(pk.pos + sf::Vector2f(-1.f, -1.f));
        inner.setFillColor(sf::Color(255, 255, 255, 100));
        window.draw(inner, glowState);
    }
}
void Game::renderHUD()
{
    sf::RectangleShape hpBg({104.f, 10.f});
    hpBg.setPosition(20.f, 16.f);
    hpBg.setFillColor(sf::Color(20, 20, 30));
    hpBg.setOutlineColor(sf::Color(40, 50, 70));
    hpBg.setOutlineThickness(1.f);
    window.draw(hpBg);
    float hpPct = player.hp / player.maxHp;
    sf::Color hpCol;
    if (hpPct > 0.5f)
        hpCol = sf::Color(60, 140, 255);
    else if (hpPct > 0.25f)
        hpCol = sf::Color(255, 200, 40);
    else
        hpCol = sf::Color(255, 60, 40);
    sf::RectangleShape hpFill({100.f * hpPct, 6.f});
    hpFill.setPosition(22.f, 18.f);
    hpFill.setFillColor(hpCol);
    window.draw(hpFill);
    for (int i = 0; i < 3; i++)
    {
        float lx = 24.f + i * 22.f;
        float ly = 34.f;
        sf::ConvexShape miniShip(3);
        miniShip.setPoint(0, {0.f, -6.f});
        miniShip.setPoint(1, {5.f, 4.f});
        miniShip.setPoint(2, {-5.f, 4.f});
        miniShip.setPosition(lx, ly);
        if (i < player.lives)
            miniShip.setFillColor(sf::Color(160, 180, 210));
        else
            miniShip.setFillColor(sf::Color(40, 44, 55));
        window.draw(miniShip);
    }
    sf::Text scoreTxt;
    scoreTxt.setFont(font);
    scoreTxt.setString("SCORE: " + std::to_string(player.score));
    scoreTxt.setCharacterSize(14);
    scoreTxt.setFillColor(sf::Color(180, 200, 230));
    scoreTxt.setPosition(20.f, 48.f);
    window.draw(scoreTxt);
    // Pause button
    sf::RectangleShape pauseBg({32.f, 32.f});
    pauseBg.setPosition(432.f, 14.f);
    pauseBg.setFillColor(sf::Color(30, 40, 60, 180));
    pauseBg.setOutlineColor(sf::Color(60, 80, 120));
    pauseBg.setOutlineThickness(1.f);
    window.draw(pauseBg);
    sf::RectangleShape bar1({6.f, 16.f});
    bar1.setPosition(441.f, 22.f);
    bar1.setFillColor(sf::Color(100, 150, 220));
    window.draw(bar1);
    sf::RectangleShape bar2({6.f, 16.f});
    bar2.setPosition(451.f, 22.f);
    bar2.setFillColor(sf::Color(100, 150, 220));
    window.draw(bar2);

    // X exit button (top-right, next to pause)
    sf::RectangleShape exitBg({32.f, 32.f});
    exitBg.setPosition(432.f, 50.f);
    exitBg.setFillColor(sf::Color(60, 20, 20, 180));
    exitBg.setOutlineColor(sf::Color(120, 40, 40));
    exitBg.setOutlineThickness(1.f);
    window.draw(exitBg);
    sf::Text exitX;
    exitX.setFont(font);
    exitX.setString("X");
    exitX.setCharacterSize(18);
    exitX.setFillColor(sf::Color(220, 80, 80));
    exitX.setPosition(441.f, 53.f);
    window.draw(exitX);

    for (int i = 0; i < player.power; i++)
    {
        sf::RectangleShape pip({12.f, 6.f});
        pip.setPosition(20.f + i * 16.f, 696.f);
        pip.setFillColor(sf::Color(60, 140, 255));
        window.draw(pip);
    }
    float chargeProgress = (float)(player.score % 500) / 500.f;
    int segments = 24;
    float radius = 14.f;
    sf::Vector2f ringCenter(450.f, 694.f);
    int filledSegments = (int)(chargeProgress * segments);
    for (int i = 0; i < segments; i++)
    {
        float a1 = (float)i / segments * 6.2832f - 1.5708f;
        float a2 = (float)(i + 1) / segments * 6.2832f - 1.5708f;
        sf::ConvexShape seg(4);
        float r1 = radius - 3.f, r2 = radius;
        seg.setPoint(0, ringCenter + sf::Vector2f(std::cos(a1) * r1, std::sin(a1) * r1));
        seg.setPoint(1, ringCenter + sf::Vector2f(std::cos(a1) * r2, std::sin(a1) * r2));
        seg.setPoint(2, ringCenter + sf::Vector2f(std::cos(a2) * r2, std::sin(a2) * r2));
        seg.setPoint(3, ringCenter + sf::Vector2f(std::cos(a2) * r1, std::sin(a2) * r1));
        if (i < filledSegments)
            seg.setFillColor(sf::Color(60, 140, 255, 200));
        else
            seg.setFillColor(sf::Color(30, 40, 60, 100));
        window.draw(seg);
    }
}
void Game::renderBossHP()
{
    int bossIdx = 0;
    for (auto &e : enemies)
    {
        if (!e.on || e.etype != EnemyType::Boss)
            continue;
        float barWidth = 180.f;
        float barX = 150.f + bossIdx * 100.f;
        float barY = 8.f;
        float hpPct = e.hp / e.maxHp;
        sf::RectangleShape bg({barWidth + 4.f, 10.f});
        bg.setPosition(barX - 2.f, barY);
        bg.setFillColor(sf::Color(20, 15, 25));
        bg.setOutlineColor(sf::Color(80, 30, 30));
        bg.setOutlineThickness(1.f);
        window.draw(bg);
        sf::Color bCol = (e.phase == 0) ? sf::Color(180, 40, 40) : sf::Color(255, 100, 30);
        sf::RectangleShape fill({barWidth * hpPct, 6.f});
        fill.setPosition(barX, barY + 2.f);
        fill.setFillColor(bCol);
        window.draw(fill);
        sf::Text label;
        label.setFont(font);
        label.setString("BOSS");
        label.setCharacterSize(9);
        label.setFillColor(sf::Color(200, 100, 100));
        label.setPosition(barX + barWidth / 2.f - 14.f, barY - 2.f);
        window.draw(label);
        bossIdx++;
    }
}
void Game::renderLevelSelect()
{
    window.clear(sf::Color(4, 6, 16));
    renderStars();
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
    sf::RectangleShape header({400.f, 40.f});
    header.setPosition(40.f, 30.f);
    header.setFillColor(sf::Color(12, 16, 28));
    header.setOutlineColor(sf::Color(40, 60, 120));
    header.setOutlineThickness(1.5f);
    window.draw(header);
    drawTextCentered("LEVELS", 50.f, 22, sf::Color(140, 180, 240));
    sf::RectangleShape preview({360.f, 160.f});
    preview.setPosition(60.f, 90.f);
    preview.setFillColor(sf::Color(10, 14, 24));
    preview.setOutlineColor(sf::Color(30, 45, 80));
    preview.setOutlineThickness(1.f);
    window.draw(preview);
    LevelData &sel = levels[selectedLevel];
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
    drawBossEnemy({340.f, 190.f}, 0.f, 0.f, 0);
    const char *statNames[3] = {"HP", "ATK", "DEF"};
    int statValues[3] = {3, 2 + selectedLevel, 1 + selectedLevel};
    for (int s = 0; s < 3; s++)
    {
        sf::Text statLabel;
        statLabel.setFont(font);
        statLabel.setString(statNames[s]);
        statLabel.setCharacterSize(11);
        statLabel.setFillColor(sf::Color(100, 120, 150));
        statLabel.setPosition(80.f, 158.f + s * 22.f);
        window.draw(statLabel);
        for (int j = 0; j < 3; j++)
        {
            sf::CircleShape star(5.f, 5);
            star.setOrigin(5.f, 5.f);
            star.setPosition(130.f + j * 18.f, 165.f + s * 22.f);
            if (j < statValues[s])
                star.setFillColor(sf::Color(255, 200, 60));
            else
                star.setFillColor(sf::Color(40, 44, 55));
            window.draw(star);
        }
    }
    float planetY = 320.f;
    for (int i = 0; i < 3; i++)
    {
        float px = 120.f + i * 120.f;
        float radius = (i == selectedLevel) ? 40.f : 30.f;
        if (i == selectedLevel)
        {
            sf::CircleShape orbit(50.f);
            orbit.setOrigin(50.f, 50.f);
            orbit.setPosition(px, planetY);
            orbit.setFillColor(sf::Color::Transparent);
            orbit.setOutlineColor(sf::Color(255, 200, 40, 80));
            orbit.setOutlineThickness(1.5f);
            window.draw(orbit);
        }
        sf::Color pCol = levels[i].planetColor;
        if (levels[i].locked)
        {
            pCol = sf::Color(pCol.r / 3, pCol.g / 3, pCol.b / 3);
        }
        sf::CircleShape planet(radius);
        planet.setOrigin(radius, radius);
        planet.setPosition(px, planetY);
        planet.setFillColor(pCol);
        window.draw(planet);
        sf::CircleShape atmo(radius + 6.f);
        atmo.setOrigin(radius + 6.f, radius + 6.f);
        atmo.setPosition(px, planetY);
        atmo.setFillColor(sf::Color(pCol.r / 2, pCol.g / 2, pCol.b / 2, 40));
        window.draw(atmo, gs);
        if (levels[i].locked)
        {
            sf::RectangleShape lockBody({14.f, 12.f});
            lockBody.setOrigin(7.f, 6.f);
            lockBody.setPosition(px, planetY + 2.f);
            lockBody.setFillColor(sf::Color(80, 70, 60));
            window.draw(lockBody);
            sf::CircleShape lockArch(6.f);
            lockArch.setOrigin(6.f, 6.f);
            lockArch.setPosition(px, planetY - 8.f);
            lockArch.setFillColor(sf::Color::Transparent);
            lockArch.setOutlineColor(sf::Color(80, 70, 60));
            lockArch.setOutlineThickness(2.5f);
            window.draw(lockArch);
        }
        std::string label = levels[i].locked ? ("-- " + levels[i].name) : levels[i].name;
        sf::Text lbl;
        lbl.setFont(font);
        lbl.setString(label);
        lbl.setCharacterSize(12);
        lbl.setFillColor(levels[i].locked ? sf::Color(60, 60, 70) : sf::Color(160, 180, 210));
        sf::FloatRect lb = lbl.getLocalBounds();
        lbl.setOrigin(lb.left + lb.width / 2.f, 0.f);
        lbl.setPosition(px, planetY + radius + 12.f);
        window.draw(lbl);
    }
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
    bool canPlay = !levels[selectedLevel].locked;
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
void Game::renderPauseOverlay()
{
    sf::RectangleShape dim({480.f, 720.f});
    dim.setFillColor(sf::Color(0, 0, 0, 140));
    window.draw(dim);
    sf::RectangleShape panel({260.f, 120.f});
    panel.setOrigin(130.f, 60.f);
    panel.setPosition(240.f, 340.f);
    panel.setFillColor(sf::Color(12, 16, 28));
    panel.setOutlineColor(sf::Color(40, 60, 120));
    panel.setOutlineThickness(2.f);
    window.draw(panel);
    drawTextCentered("PAUSED", 300.f, 28, sf::Color(140, 180, 240));
    drawTextCentered("Click or press P to resume", 335.f, 13, sf::Color(100, 120, 160));

    sf::RectangleShape exitBtn({160.f, 34.f});
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
    sf::RectangleShape dim({480.f, 720.f});
    dim.setFillColor(sf::Color(0, 0, 0, 160));
    window.draw(dim);
    sf::RectangleShape panel({320.f, 320.f});
    panel.setOrigin(160.f, 160.f);
    panel.setPosition(240.f, 340.f);
    panel.setFillColor(sf::Color(14, 10, 16));
    panel.setOutlineColor(sf::Color(120, 40, 40));
    panel.setOutlineThickness(2.f);
    window.draw(panel);
    drawTextCentered("GAME OVER", 210.f, 30, sf::Color(255, 80, 60));
    drawTextCentered("Final Score: " + std::to_string(player.score), 260.f, 18, sf::Color(200, 200, 220));
    float logY = 300.f;
    int shown = 0;
    for (auto &ev : scoreLog)
    {
        if (shown >= 8)
            break;
        std::string line = ev.description + "  +" + std::to_string(ev.points);
        sf::Text t;
        t.setFont(font);
        t.setString(line);
        t.setCharacterSize(11);
        t.setFillColor(sf::Color(140, 150, 170));
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.left + b.width / 2.f, 0.f);
        t.setPosition(240.f, logY + shown * 18.f);
        window.draw(t);
        shown++;
    }
    drawTextCentered("Click to restart", 480.f, 14, sf::Color(100, 120, 160));
}
void Game::renderLevelCompleteOverlay()
{
    sf::RectangleShape dim({480.f, 720.f});
    dim.setFillColor(sf::Color(0, 0, 0, 140));
    window.draw(dim);
    sf::RectangleShape panel({280.f, 180.f});
    panel.setOrigin(140.f, 90.f);
    panel.setPosition(240.f, 340.f);
    panel.setFillColor(sf::Color(10, 16, 24));
    panel.setOutlineColor(sf::Color(40, 120, 80));
    panel.setOutlineThickness(2.f);
    window.draw(panel);
    drawTextCentered("LEVEL COMPLETE", 290.f, 26, sf::Color(80, 220, 120));
    drawTextCentered(levels[currentLevel].name, 325.f, 18, sf::Color(160, 200, 180));
    drawTextCentered("Score: " + std::to_string(player.score), 355.f, 16, sf::Color(180, 200, 220));
    drawTextCentered("Click to continue", 400.f, 13, sf::Color(100, 140, 130));
}
int main()
{
    Game game;
    game.init();
    game.run();
    return 0;
}
