#pragma once
#include "MoveComponent.h"
#include <Windows.h>
#include <SDL_stdinc.h>

class EnemyMove : public MoveComponent
{
public:
	EnemyMove(class Actor* owner);
	void Update(float deltaTime) override;
	void Move(float deltaTime);

private:
	float mSpeed;
	float mTime;
	float mMoveTime;
	float mShootTime;
};