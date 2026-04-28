#pragma once

class EnemyType
{
public:
    virtual ~EnemyType() {}
    virtual float getMaxHp() const = 0;
    virtual float getShootInterval() const = 0;
    virtual int getScoreValue() const = 0;
    virtual float getMoveSpeed() const = 0;
    virtual float getRadius() const = 0;
    virtual int getKind() const = 0;
    static EnemyType *Small;
    static EnemyType *Medium;
    static EnemyType *Boss;
};

class SmallEnemyType : public EnemyType
{
public:
    float getMaxHp() const { return 30.f; }
    float getShootInterval() const { return 2.2f; }
    int getScoreValue() const { return 50; }
    float getMoveSpeed() const { return 200.f; }
    float getRadius() const { return 16.f; }
    int getKind() const { return 0; }
};

class MediumEnemyType : public EnemyType
{
public:
    float getMaxHp() const { return 80.f; }
    float getShootInterval() const { return 1.8f; }
    int getScoreValue() const { return 120; }
    float getMoveSpeed() const { return 160.f; }
    float getRadius() const { return 22.f; }
    int getKind() const { return 1; }
};

class BossEnemyType : public EnemyType
{
public:
    float getMaxHp() const { return 1200.f; }
    float getShootInterval() const { return 0.8f; }
    int getScoreValue() const { return 2000; }
    float getMoveSpeed() const { return 80.f; }
    float getRadius() const { return 60.f; }
    int getKind() const { return 2; }
};
