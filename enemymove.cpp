#include "EnemyMove.h"
#include "Actor.h"
#include "player.h"
#include "Game.h"
#include "collisioncomponent.h"
#include <Windows.h>
#include <iostream>

EnemyMove::EnemyMove(Actor* owner)
    :MoveComponent(owner)
    ,mSpeed(200.0f)
    ,mTime(0.0f)
    ,mMoveTime(1.0f)
    ,mShootTime(1.0f)
{
    
}

void EnemyMove::Update(float deltaTime)
{
    mTime += deltaTime;
}

void EnemyMove::Move(float deltaTime)
{
    Vector2 playerPos = mOwner->GetGame()->GetPlayer()->GetPosition();
    Vector2 dirToPlayer = Vector2::Normalize(playerPos - mOwner->GetPosition());

    Vector2 vel = dirToPlayer * mSpeed;

    mOwner->SetPosition(mOwner->GetPosition() + vel * deltaTime);
}