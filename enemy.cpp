#include "enemy.h"
#include "game.h"
#include "collisioncomponent.h"
#include "math.h"
#include "enemymove.h"
#include "projectile.h"

Enemy::Enemy(Game* game)
	:Actor(game)
	,mShootRadius(500.0f)
	,mShootInterval(1.0f)
	,mShootTimer(0.0f)
	,mProjectileSpeed(200.0f)
{
	SetScale(2.0f);
	mSpriteComponent = new SpriteComponent(this);
	mSpriteComponent->SetTexture(mGame->GetTexture("assets/enemy.png"));
	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(16, 16);
	mEnemyMove = new EnemyMove(this);
	mType = EnemyType::Normal;
}

void Enemy::OnUpdate(float deltaTime)
{
	mShootTimer += deltaTime;

	Vector2 playerPos = mGame->GetPlayer()->GetPosition();

	if (mShootTimer < mShootInterval)
		return;

	if ((playerPos - mPosition).Length() <= mShootRadius)
	{
		Shoot(playerPos);
		mShootTimer = 0.0f;
	}
}

void Enemy::Shoot(Vector2 target)
{
	Vector2 toTarget = Vector2::Normalize(target - mPosition);
	class Projectile* projectile = new Projectile(mGame);
	projectile->Initialize(mPosition, toTarget * mProjectileSpeed);
}