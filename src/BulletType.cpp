#include "BulletType.h"

BulletType *BulletType::PlayerNorm = new PlayerNormBullet();
BulletType *BulletType::EnemyNorm = new EnemyNormBullet();
BulletType *BulletType::EnemyBurst = new EnemyBurstBullet();
BulletType *BulletType::BossBeam = new BossBeamBullet();
