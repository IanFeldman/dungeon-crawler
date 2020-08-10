#include "enemy.h"
#include "game.h"
#include "collisioncomponent.h"
#include "math.h"
#include "enemymove.h"
#include "projectile.h"
#include "spritecomponent.h"
#include "player.h"

Enemy::Enemy(Game* game)
	:Actor(game)
	,mTime(0.0f)
	,mMoving(true)
	,mStartedShooting(0.0f)
	,mShot(false)
	,mStartedMoving(0.0f)
{
	mSpriteComponent = new SpriteComponent(this);
	mSpriteComponent->SetTexture(mGame->GetTexture("assets/enemy.png"));
	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(16, 16);
	mEnemyMove = new EnemyMove(this);
	mType = EnemyType::Normal;
}

void Enemy::Initialize(Vector2 position, EnemyType type)
{
	SetPosition(position);
	mType = type;

	switch (mType)
	{
	case EnemyType::Normal:
		mProjectileSpeed = 150.0f;
		mMoveSpeed = 50.0f;
		mMoveTime = 2.0f;
		mShootTime = 1.0f;
		break;
	default:
		mProjectileSpeed = 100.0f;
		mMoveSpeed = 50.0f;
		mMoveTime = 1.0f;
		mShootTime = 1.0f;
		break;
	}
}

void Enemy::OnUpdate(float deltaTime)
{
	mTime += deltaTime;

	Vector2 playerPos = mGame->GetPlayer()->GetPosition();

	// moving
	if (mMoving)
	{
		// update pos
		Vector2 toPlayer = Vector2::Normalize(playerPos - mPosition);
		Vector2 vel = toPlayer * mMoveSpeed;
		SetPosition(mPosition + vel * deltaTime);

		// check if we have been moving for long enough
		if (mTime - mStartedMoving >= mMoveTime)
		{
			// stop moving
			mMoving = false;
			mStartedShooting = mTime;
			mShot = false; // so we can shoot again
		}
	}
	// shooting
	else
	{
		// if we haven't shot during this shooting session:
		if (!mShot)
		{
			// wait for half the shooting time
			if (mTime - mStartedShooting >= mShootTime / 2.0f)
			{
				Shoot(playerPos);
				mShot = true;
			}
		}
		else
		{
			// wait until shooting time is over to start moving again
			if (mTime - mStartedShooting >= mShootTime)
			{
				mMoving = true;
				mStartedMoving = mTime;
			}
		}
	}
}

void Enemy::Shoot(Vector2 target)
{
	Vector2 toTarget = Vector2::Normalize(target - mPosition);
	class Projectile* projectile = new Projectile(mGame);
	projectile->Initialize(mPosition, toTarget * mProjectileSpeed);
}