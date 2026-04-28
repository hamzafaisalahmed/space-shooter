#pragma once

class BulletType
{
public:
    virtual ~BulletType() {}
    virtual int getId() const = 0;
    virtual bool isPlayerBullet() const = 0;
    static BulletType *PlayerNorm;
    static BulletType *PlayerWide;
    static BulletType *PlayerTriple;
    static BulletType *EnemyNorm;
    static BulletType *EnemyBurst;
    static BulletType *BossBeam;
};

class PlayerNormBullet : public BulletType
{
public:
    int getId() const { return 0; }
    bool isPlayerBullet() const { return true; }
};

class PlayerWideBullet : public BulletType
{
public:
    int getId() const { return 1; }
    bool isPlayerBullet() const { return true; }
};

class PlayerTripleBullet : public BulletType
{
public:
    int getId() const { return 2; }
    bool isPlayerBullet() const { return true; }
};

class EnemyNormBullet : public BulletType
{
public:
    int getId() const { return 3; }
    bool isPlayerBullet() const { return false; }
};

class EnemyBurstBullet : public BulletType
{
public:
    int getId() const { return 4; }
    bool isPlayerBullet() const { return false; }
};

class BossBeamBullet : public BulletType
{
public:
    int getId() const { return 5; }
    bool isPlayerBullet() const { return false; }
};
