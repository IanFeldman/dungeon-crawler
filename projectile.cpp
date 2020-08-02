#include "projectile.h"
#include "game.h"
#include "math.h"
#include "collisioncomponent.h"

Projectile::Projectile(Game* game)
	:Actor(game)
	,mTravelDistance(1000.0f)
	,mInitPos(Vector2::Zero)
	,mVelocity(Vector2::Zero)
{
	SetScale(1.0f);
	mSpriteComponent = new SpriteComponent(this, 150);
	mSpriteComponent->SetTexture(mGame->GetTexture("assets/projectile.png"));

	mCollisionComponent = new CollisionComponent(this);
}

void Projectile::OnUpdate(float deltaTime)
{
	// update position
	mPosition += mVelocity * deltaTime;

	// destroy if too far away
	float distance = (mPosition - mInitPos).Length();
	if (distance > mTravelDistance)
		Destroy();
}

void Projectile::Destroy()
{
	// hit animation
	SetState(ActorState::Destroy);
}