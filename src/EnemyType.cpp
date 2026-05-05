#include "EnemyType.h"

static SmallEnemyType _small;
static MediumEnemyType _medium;
static BossEnemyType _boss;

EnemyType *EnemyType::Small = &_small;
EnemyType *EnemyType::Medium = &_medium;
EnemyType *EnemyType::Boss = &_boss;
