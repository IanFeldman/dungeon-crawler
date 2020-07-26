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
	void TakeAction();

private:
	class SpriteComponent* mSpriteComponent;
	class CollisionComponent* mCollisionComponent;
	class EnemyMove* mEnemyMove;
	EnemyType mType;
};