#pragma once
#include "actor.h"

class Boomerang : public Actor
{
public:
	Boomerang(class Game* game);
	void OnUpdate(float deltaTime) override;
	void Initialize(Vector2 position, Vector2 velocity) { SetPosition(position); mVelocity = velocity; }
	void Destroy();

protected:
	class AnimatedSprite* mASprite;
	class CollisionComponent* mCollisionComponent;

	Vector2 mVelocity;
	float mReturnSpeed;

	float mWaitTime;
};