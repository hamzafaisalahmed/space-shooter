#include "BulletType.h"

BulletType *BulletType::PlayerNorm = new PlayerNormBullet();
BulletType *BulletType::PlayerWide = new PlayerWideBullet();
BulletType *BulletType::PlayerTriple = new PlayerTripleBullet();
BulletType *BulletType::EnemyNorm = new EnemyNormBullet();
BulletType *BulletType::EnemyBurst = new EnemyBurstBullet();
BulletType *BulletType::BossBeam = new BossBeamBullet();
