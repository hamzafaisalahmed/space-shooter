#include "BulletType.h"

// BulletType *BulletType::PlayerNorm = new PlayerNormBullet();
// BulletType *BulletType::EnemyNorm = new EnemyNormBullet();
// BulletType *BulletType::EnemyBurst = new EnemyBurstBullet();
// BulletType *BulletType::BossBeam = new BossBeamBullet();

static PlayerNormBullet _playerNorm;
static EnemyNormBullet _enemyNorm;
static EnemyBurstBullet _enemyBurst;
static BossBeamBullet _bossBeam;

BulletType *BulletType::PlayerNorm = &_playerNorm;
BulletType *BulletType::EnemyNorm = &_enemyNorm;
BulletType *BulletType::EnemyBurst = &_enemyBurst;
BulletType *BulletType::BossBeam = &_bossBeam;