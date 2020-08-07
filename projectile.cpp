#include "projectile.h"
#include "game.h"
#include "math.h"
#include "collisioncomponent.h"
#include "AnimatedSprite.h"

Projectile::Projectile(Game* game)
	:Actor(game)
	,mVelocity(Vector2::Zero)
	,mHit(false)
	,mTime(0.0f)
	,mLifeLength(5.0f)
	,mHitTime(0.0f)
	,mHitAnimLength(0.5f)
{
	SetScale(1.5f);
	mASprite = new AnimatedSprite(this, 200);
	std::vector<SDL_Texture*> travelAnim{
		mGame->GetTexture("assets/projectile/travel/projectile-travel1.png"),
		mGame->GetTexture("assets/projectile/travel/projectile-travel1.png"),
		mGame->GetTexture("assets/projectile/travel/projectile-travel1.png"),
		mGame->GetTexture("assets/projectile/travel/projectile-travel2.png"),
		mGame->GetTexture("assets/projectile/travel/projectile-travel2.png")
	};
	std::vector<SDL_Texture*> hitAnim{
		mGame->GetTexture("assets/projectile/hit/projectile-hit1.png"),
		mGame->GetTexture("assets/projectile/hit/projectile-hit2.png"),
		mGame->GetTexture("assets/projectile/hit/projectile-hit3.png"),
		mGame->GetTexture("assets/projectile/hit/projectile-hit4.png"),
		mGame->GetTexture("assets/projectile/hit/projectile-hit5.png"),
		mGame->GetTexture("assets/projectile/hit/projectile-hit6.png"),
		mGame->GetTexture("assets/projectile/hit/projectile-hit7.png"),
		mGame->GetTexture("assets/projectile/hit/projectile-hit8.png"),
		mGame->GetTexture("assets/projectile/hit/projectile-hit9.png"), // the blank frame at the end
		mGame->GetTexture("assets/projectile/hit/projectile-hit9.png"),
		mGame->GetTexture("assets/projectile/hit/projectile-hit9.png")
	};
	mASprite->AddAnimation("travel", travelAnim);
	mASprite->AddAnimation("hit", hitAnim);
	mASprite->SetAnimation("travel");

	mCollisionComponent = new CollisionComponent(this);
}

void Projectile::OnUpdate(float deltaTime)
{
	// update position
	mPosition += mVelocity * deltaTime;

	// update time
	mTime += deltaTime;

	if (mHit)
	{
		if (mTime >= mHitTime + mHitAnimLength)
			SetState(ActorState::Destroy);

		return;
	}

	// it gets destroyed on its own after a while
	if (mTime >= mLifeLength)
	{
		SetToDestroy();
		return;
	}

	if (mCollisionComponent->Intersect(mGame->GetPlayer()->GetComponent<CollisionComponent>()))
	{
		// deal damage to player
		// camera shake
		mGame->CameraShake(100, 15);
		SetToDestroy();
		return;
	}

	// destroy if hit wall
	for (class Actor* a : mGame->GetCollidables())
	{
		class CollisionComponent* otherCC = a->GetComponent<CollisionComponent>();
		if (mCollisionComponent->Intersect(otherCC))
		{
			SetToDestroy();
			return;
		}
	}

}

void Projectile::SetToDestroy()
{
	mHit = true;
	mASprite->SetAnimation("hit");
	mHitTime = mTime;
	mVelocity = Vector2::Zero;
}