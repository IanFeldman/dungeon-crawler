#pragma once
#include "actor.h"

enum class EnemyType
{
	Normal
};

class Enemy : public Actor
{
public:
	Enemy(class Game* game);
	void OnUpdate(float deltaTime) override;
	void Initialize(Vector2 position, EnemyType type);
	void Shoot(Vector2 target);

private:
	class SpriteComponent* mSpriteComponent;
	class CollisionComponent* mCollisionComponent;
	class EnemyMove* mEnemyMove;
	EnemyType mType;

	float mTime;
	float mStartedShooting;
	float mProjectileSpeed;
	bool mShot;

	float mStartedMoving;
	float mMoveSpeed;
	float mMoveTime;
	float mShootTime;

	bool mMoving;
};