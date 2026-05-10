#pragma once

class BulletType
{
    int id;
    bool playerBullet;

public:
    BulletType(int id, bool playerBullet) : id(id), playerBullet(playerBullet) {}
    virtual ~BulletType() {}
    int getId() const { return id; }
    bool isPlayerBullet() const { return playerBullet; }
    static BulletType *PlayerNorm;
    static BulletType *EnemyNorm;
    static BulletType *EnemyBurst;
    static BulletType *BossBeam;
};

class PlayerNormBullet : public BulletType
{
public:
    PlayerNormBullet() : BulletType(0, true) {}
};

class EnemyNormBullet : public BulletType
{
public:
    EnemyNormBullet() : BulletType(3, false) {}
};

class EnemyBurstBullet : public BulletType
{
public:
    EnemyBurstBullet() : BulletType(4, false) {}
};

class BossBeamBullet : public BulletType
{
public:
    BossBeamBullet() : BulletType(5, false) {}
};

class BulletTypeRegistry
{
private:
    PlayerNormBullet playerNorm;
    EnemyNormBullet enemyNorm;
    EnemyBurstBullet enemyBurst;
    BossBeamBullet bossBeam;

public:
    BulletTypeRegistry()
    {
        BulletType::PlayerNorm = &playerNorm;
        BulletType::EnemyNorm = &enemyNorm;
        BulletType::EnemyBurst = &enemyBurst;
        BulletType::BossBeam = &bossBeam;
    }

    ~BulletTypeRegistry()
    {
        BulletType::PlayerNorm = nullptr;
        BulletType::EnemyNorm = nullptr;
        BulletType::EnemyBurst = nullptr;
        BulletType::BossBeam = nullptr;
    }

    // preventing copying
    BulletTypeRegistry(const BulletTypeRegistry &) = delete;
    BulletTypeRegistry &operator=(const BulletTypeRegistry &) = delete;
};