#pragma once
#include "actor.h"

class Projectile : public Actor
{
public:
	Projectile(class Game* game);
	void OnUpdate(float deltaTime) override;
	void Initialize(Vector2 position, Vector2 velocity) { SetPosition(position);  mInitPos = position; mVelocity = velocity; }
	void Destroy();

protected:
	class SpriteComponent* mSpriteComponent;
	class CollisionComponent* mCollisionComponent;

	float mTravelDistance;
	Vector2 mInitPos;
	Vector2 mVelocity;
};