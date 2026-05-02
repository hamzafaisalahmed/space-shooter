#pragma once

#include <SFML/Graphics.hpp>
#include "Utils.h"

class Player
{
private:
    sf::Vector2f pos;
    float hp;
    float maxHp;
    float speed;
    int lives;
    int score;
    float shootTimer;
    float shootInterval;
    float iframeTimer;
    float shieldTimer;
    float tilt;
    bool alive;

public:
    Player()
        : pos(240.f, 600.f), hp(100.f), maxHp(100.f), speed(310.f),
          lives(1), score(0),
          shootTimer(0.f), shootInterval(0.10f),
          iframeTimer(0.f), shieldTimer(0.f), tilt(0.f), alive(true) {}

    sf::Vector2f getPos() const { return pos; }
    float getHp() const { return hp; }
    float getMaxHp() const { return maxHp; }
    float getSpeed() const { return speed; }
    int getLives() const { return lives; }
    int getScore() const { return score; }
    float getShootTimer() const { return shootTimer; }
    float getShootInterval() const { return shootInterval; }
    float getIframeTimer() const { return iframeTimer; }
    float getShieldTimer() const { return shieldTimer; }
    float getTilt() const { return tilt; }
    bool isAlive() const { return alive; }

    void setPos(sf::Vector2f p) { pos = p; }
    void setTilt(float t) { tilt = t; }
    void setShootTimer(float t) { shootTimer = t; }
    void setIframeTimer(float t) { iframeTimer = t; }
    void setShieldTimer(float t) { shieldTimer = t; }
    void setAlive(bool a) { alive = a; }

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

    void loseLife() { lives--; }

    void tickShootTimer(float dt) { shootTimer -= dt; }
    void tickIframe(float dt) { iframeTimer -= dt; }
    void tickShield(float dt) { shieldTimer -= dt; }

    void resetForLevel()
    {
        pos = sf::Vector2f(240.f, 600.f);
        hp = maxHp;
        speed = 310.f;
        shootTimer = 0.f;
        iframeTimer = 0.f;
        shieldTimer = 0.f;
        tilt = 0.f;
        alive = true;
        score = 0;
        lives = 1;
    }

    Player &operator+=(int scoreBonus)
    {
        score += scoreBonus;
        return *this;
    }
};
