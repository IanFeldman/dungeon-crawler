#include "EnemyMove.h"
#include "Actor.h"
#include "player.h"
#include "Game.h"
#include "collisioncomponent.h"
#include <Windows.h>
#include <iostream>

EnemyMove::EnemyMove(Actor* owner)
    :MoveComponent(owner)
{
}

void EnemyMove::Update(float deltaTime)
{
}

void EnemyMove::Move()
{
    Vector2 _playerPos = mOwner->GetGame()->GetPlayer()->GetPosition();
    Vector2 _dirToPlayer = _playerPos - mOwner->GetPosition();
    Vector2 _move;
    if (Math::Abs(_dirToPlayer.x) >= Math::Abs(_dirToPlayer.y)) {
        if (_dirToPlayer.x >= 0)
            _move = Vector2(32, 0);
        else
            _move = Vector2(-32, 0);
    }
    else {
        if (_dirToPlayer.y >= 0)
            _move = Vector2(0, 32);
        else
            _move = Vector2(0, -32);
    }

    Vector2 _newPos(mOwner->GetPosition() + _move);
    mOwner->SetPosition(_newPos);
}