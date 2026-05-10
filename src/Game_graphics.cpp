#include "Game.h"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>
#include <stdexcept>

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