#include "EnemyType.h"

EnemyType *EnemyType::Small = nullptr;
EnemyType *EnemyType::Medium = nullptr;
EnemyType *EnemyType::Boss = nullptr;

static EnemyTypeRegistry registry;