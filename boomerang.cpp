#include "boomerang.h"
#include "game.h"
#include "math.h"
#include "collisioncomponent.h"
#include "AnimatedSprite.h"

Boomerang::Boomerang(Game* game)
	:Actor(game)
	,mVelocity(Vector2::Zero)
	,mReturnSpeed(5.0f)
	,mWaitTime(0.0f)
{
	SetScale(1.0f);
	mASprite = new AnimatedSprite(this, 150);
	std::vector<SDL_Texture*> spinAnim{
		mGame->GetTexture("assets/boomerang/boomerang1.png"),
		mGame->GetTexture("assets/boomerang/boomerang2.png"),
		mGame->GetTexture("assets/boomerang/boomerang3.png"),
		mGame->GetTexture("assets/boomerang/boomerang4.png"),
		mGame->GetTexture("assets/boomerang/boomerang5.png"),
		mGame->GetTexture("assets/boomerang/boomerang6.png"),
		mGame->GetTexture("assets/boomerang/boomerang7.png"),
		mGame->GetTexture("assets/boomerang/boomerang8.png"),
	};
	mASprite->AddAnimation("spin", spinAnim);
	mASprite->SetAnimation("spin");

	mCollisionComponent = new CollisionComponent(this);
}

void Boomerang::OnUpdate(float deltaTime)
{
	// update position
	mPosition += mVelocity * deltaTime;

	if (mWaitTime < 0.2f)
	{
		mWaitTime += deltaTime;
		return;
	}

	// update velocity
	Vector2 playerPos = mGame->GetPlayer()->GetPosition();
	Vector2 toPlayer = playerPos - mPosition;
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
		Destroy();
}

void Boomerang::Destroy()
{
	// hit animation
	SetState(ActorState::Destroy);
}