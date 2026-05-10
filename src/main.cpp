#include "Game.h"
#include <iostream>
#include "BulletType.h"
#include "EnemyType.h"
#include "GameState.h"

const GameState GameState::Home(0);
const GameState GameState::LevelSelect(1);
const GameState GameState::Playing(2);
const GameState GameState::Paused(3);
const GameState GameState::GameOver(4);
const GameState GameState::LevelComplete(5);

BulletType *BulletType::PlayerNorm = nullptr;
BulletType *BulletType::EnemyNorm = nullptr;
BulletType *BulletType::EnemyBurst = nullptr;
BulletType *BulletType::BossBeam = nullptr;

EnemyType *EnemyType::Small = nullptr;
EnemyType *EnemyType::Medium = nullptr;
EnemyType *EnemyType::Boss = nullptr;

int main()
{
    BulletTypeRegistry registryBullet;
    EnemyTypeRegistry registry;

    try
    {
        Game game;
        game.init();
        game.run();
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Fatal error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
