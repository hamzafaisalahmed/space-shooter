#pragma once

class EnemyType
{
    float maxHp;
    float shootInterval;
    int scoreValue;
    float moveSpeed;
    float radius;
    int kind;

public:
    EnemyType(float maxHp, float shootInterval, int scoreValue, float moveSpeed, float radius, int kind)
        : maxHp(maxHp), shootInterval(shootInterval), scoreValue(scoreValue), moveSpeed(moveSpeed), radius(radius), kind(kind) {}

    virtual ~EnemyType() {}

    float getMaxHp() const { return maxHp; }
    float getShootInterval() const { return shootInterval; }
    int getScoreValue() const { return scoreValue; }
    float getMoveSpeed() const { return moveSpeed; }
    float getRadius() const { return radius; }
    int getKind() const { return kind; }

    static EnemyType *Small;
    static EnemyType *Medium;
    static EnemyType *Boss;
};

class SmallEnemyType : public EnemyType
{
public:
    SmallEnemyType()
        : EnemyType(30.f, 2.2f, 50, 200.f, 16.f, 0) {}
};

class MediumEnemyType : public EnemyType
{
public:
    MediumEnemyType()
        : EnemyType(80.f, 1.8f, 120, 160.f, 22.f, 1) {}
};

class BossEnemyType : public EnemyType
{
public:
    BossEnemyType()
        : EnemyType(2500.f, 0.8f, 2000, 80.f, 60.f, 2) {}
};

class EnemyTypeRegistry
{
private:
    SmallEnemyType small;
    MediumEnemyType medium;
    BossEnemyType boss;

public:
    EnemyTypeRegistry()
    {
        EnemyType::Small = &small;
        EnemyType::Medium = &medium;
        EnemyType::Boss = &boss;
    }

    ~EnemyTypeRegistry()
    {
        EnemyType::Small = nullptr;
        EnemyType::Medium = nullptr;
        EnemyType::Boss = nullptr;
    }

    // Prevent copying
    EnemyTypeRegistry(const EnemyTypeRegistry &) = delete;
    EnemyTypeRegistry &operator=(const EnemyTypeRegistry &) = delete;
};