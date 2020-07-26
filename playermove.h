#pragma once
#include "MoveComponent.h"
#include <Windows.h>
#include <SDL_stdinc.h>

enum class MoveDirection
{
    Idle,
    Left,
    Right,
    Up,
    Down
};

class PlayerMove : public MoveComponent
{
public:
    PlayerMove(class Actor* owner);
    void Update(float deltaTime) override;
    void ProcessInput(const Uint8* keyboardState);

    void SetDirection(MoveDirection dir) { mDir = dir; }

private:
    class AnimatedSprite* mASprite;
    MoveDirection mDir;
    float mSpeed;
};