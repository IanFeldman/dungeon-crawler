#include "boomerang.h"
#include "game.h"
#include "math.h"
#include "collisioncomponent.h"
#include "AnimatedSprite.h"
#include "enemy.h"
#include "player.h"

Boomerang::Boomerang(Game* game)
	:Actor(game)
	,mVelocity(Vector2::Zero)
	,mReturnSpeed(5.0f)
	,mTime(0.0f)
	,mBroken(false)
	,mImpactTime(0.0f)
{
	mASprite = new AnimatedSprite(this, 200);
	std::vector<SDL_Texture*> spinAnim{
		mGame->GetTexture("assets/boomerang/spin/boomerang-spin1.png"),
		mGame->GetTexture("assets/boomerang/spin/boomerang-spin2.png"),
		mGame->GetTexture("assets/boomerang/spin/boomerang-spin3.png"),
		mGame->GetTexture("assets/boomerang/spin/boomerang-spin4.png"),
		mGame->GetTexture("assets/boomerang/spin/boomerang-spin5.png"),
		mGame->GetTexture("assets/boomerang/spin/boomerang-spin6.png"),
		mGame->GetTexture("assets/boomerang/spin/boomerang-spin7.png"),
		mGame->GetTexture("assets/boomerang/spin/boomerang-spin8.png")
	};
	std::vector<SDL_Texture*> breakAnim{
		mGame->GetTexture("assets/boomerang/break/boomerang-break1.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break2.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break3.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break4.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break5.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break6.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break7.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break8.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break9.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break10.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break11.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break12.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break12.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break12.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break12.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break12.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break12.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break12.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break12.png"),
		mGame->GetTexture("assets/boomerang/break/boomerang-break12.png")
	};
	mASprite->AddAnimation("spin", spinAnim);
	mASprite->AddAnimation("break", breakAnim);
	mASprite->SetAnimation("spin");

	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(16, 16);

	mPlayer = mGame->GetPlayer();
}

void Boomerang::OnUpdate(float deltaTime)
{
	// update position
	mPosition += mVelocity * deltaTime;

	for (class Enemy* e : mGame->GetEnemies())
	{
		class CollisionComponent* otherCC = e->GetComponent<CollisionComponent>();
		if (mCollisionComponent->Intersect(otherCC))
		{
			// kill enemy
			// break boomerang
			SetToDestroy();
		}
	}

	mTime += deltaTime;
	if (mTime < 0.2f)
		return;

	if (mBroken)
	{
		// wait until the break animation is over
		if (mTime >= mImpactTime + 0.15f)
		{
			SetState(ActorState::Destroy);
			mPlayer->LowerBoomerangCount();
			return;
		}
	}

	// update velocity
	Vector2 toPlayer = mPlayer->GetPosition() - mPosition;
	Vector2 dirToPlayer = Vector2::Normalize(toPlayer);

	Vector2 suppressor = -1.0f * mVelocity;
	float suppressorScalar = 0.03f;

	// if we are already going pretty much towards player, don't suppress as much
	Vector2 dir = Vector2::Normalize(mVelocity);
	if (abs(dir.x - dirToPlayer.x) <= 0.25f && abs(dir.y - dirToPlayer.y) <= 0.25f)
		suppressorScalar = 0.005f;

	mVelocity += (dirToPlayer * mReturnSpeed) + (suppressor * suppressorScalar);

	// normal
	//mVelocity += (dirToPlayer * mReturnSpeed);

	// destroy
	if (toPlayer.Length() < 16.0f)
		SetToDestroy();
}

void Boomerang::SetToDestroy()
{
	mBroken = true;
	mImpactTime = mTime;
	mASprite->SetAnimation("break");
}