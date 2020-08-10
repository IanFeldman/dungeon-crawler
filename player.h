#pragma once
#include "actor.h"

class Player : public Actor
{
public:
	Player(class Game* game);
	void OnUpdate(float deltaTime) override;
	void Attack(Vector2 direction);
	void LowerBoomerangCount() { if (mBoomerangCount > 0) mBoomerangCount--; }

private:
	class AnimatedSprite* mASprite; // anim must come before player move, bc playermove gets it on init
	class CollisionComponent* mCollisionComponent;
	class PlayerMove* mPlayerMove;

	float mBoomerangSpeed;
	int mTotalBoomerangCount;
	int mBoomerangCount;
};