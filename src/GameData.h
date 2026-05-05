#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include <vector>
#include "BulletType.h"
#include "EnemyType.h"
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
    SpawnEvent()
        : time(0.f), etype(0), startPos(0.f, 0.f),
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
    SpawnJob()
        : etype(0), startPos(0.f, 0.f), delayRemaining(0.f),
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

const int MAX_BULLETS = 700;
const int MAX_ENEMIES = 80;
const int MAX_PARTICLES = 1000;
const int MAX_STARS = 200;

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
