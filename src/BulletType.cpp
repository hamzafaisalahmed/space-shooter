#include "BulletType.h"

BulletType *BulletType::PlayerNorm = nullptr;
BulletType *BulletType::EnemyNorm = nullptr;
BulletType *BulletType::EnemyBurst = nullptr;
BulletType *BulletType::BossBeam = nullptr;

static BulletTypeRegistry registryBullet;
