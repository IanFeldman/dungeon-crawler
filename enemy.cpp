#include "enemy.h"
#include "game.h"
#include "collisioncomponent.h"
#include "math.h"
#include "enemymove.h"

Enemy::Enemy(Game* game)
	:Actor(game)
{
	SetScale(2.0f);
	mSpriteComponent = new SpriteComponent(this);
	mSpriteComponent->SetTexture(mGame->GetTexture("assets/player.png"));
	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(16, 16);
	mEnemyMove = new EnemyMove(this);
	mType = EnemyType::Normal;
}

void Enemy::OnUpdate(float deltaTime)
{

}

void Enemy::TakeAction()
{
	// move or attack based on enemy type and distance to player
	mEnemyMove->Move();
}