#include "PlayerMove.h"
#include "Actor.h"
#include "Game.h"
#include "player.h"
#include "collisioncomponent.h"
#include "animatedsprite.h"
#include <Windows.h>
#include <iostream>

PlayerMove::PlayerMove(Actor* owner)
    :MoveComponent(owner)
    ,mDir(MoveDirection::Idle)
    ,mSpeed(500.0f)
{
    mASprite = mOwner->GetComponent<AnimatedSprite>();
}

void PlayerMove::Update(float deltaTime)
{
    mOwner->SetRotation((deltaTime * GetAngularSpeed()) + mOwner->GetRotation());
    SetVelocity(deltaTime * GetAccel() + GetVelocity());
    mOwner->SetPosition((deltaTime * GetVelocity()) + mOwner->GetPosition());

    // set animation
    switch (mDir)
    {
    case MoveDirection::Idle:
        mASprite->SetAnimation("idle");
        break;
    case MoveDirection::Left:
        mASprite->SetAnimation("runLeft");
        break;
    case MoveDirection::Right:
        mASprite->SetAnimation("runRight");
        break;
    case MoveDirection::Up:
        mASprite->SetAnimation("runUp");
        break;
    case MoveDirection::Down:
        mASprite->SetAnimation("runDown");
        break;
    default:
        break;
    }
}

void PlayerMove::ProcessInput(const Uint8* keyboardState)
{
    Vector2 direction = Vector2::Zero;

    bool left = keyboardState[SDL_SCANCODE_A];
    bool right = keyboardState[SDL_SCANCODE_D];
    bool up = keyboardState[SDL_SCANCODE_W];
    bool down = keyboardState[SDL_SCANCODE_S];
    bool leftright = !(left && right) && !(!left && !right);
    bool updown = !(up && down) && !(!up && !down);
    if (leftright || updown) {
        if (left) {
            direction.x--;
            SetDirection(MoveDirection::Left);
        }
        if (right) {
            direction.x++;
            SetDirection(MoveDirection::Right);
        }
        if (up) {
            direction.y--;
            SetDirection(MoveDirection::Up);
        }
        if (down) {
            direction.y++;
            SetDirection(MoveDirection::Down);
        }

        SetVelocity(Vector2::Normalize(direction) * mSpeed);
    }
    else {
        SetVelocity(Vector2::Zero);
        SetDirection(MoveDirection::Idle);
    }
}