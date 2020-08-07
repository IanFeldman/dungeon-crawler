#pragma once
#include "actor.h"

class Projectile : public Actor
{
public:
	Projectile(class Game* game);
	void OnUpdate(float deltaTime) override;
	void Initialize(Vector2 position, Vector2 velocity) { SetPosition(position); mVelocity = velocity; }

private:
	void SetToDestroy();

protected:
	class AnimatedSprite* mASprite;
	class CollisionComponent* mCollisionComponent;

	Vector2 mVelocity;

	bool mHit;
	float mTime;
	float mLifeLength;
	float mHitTime;
	float mHitAnimLength;
};