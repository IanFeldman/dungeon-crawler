#include "wall.h"
#include "game.h"
#include "collisioncomponent.h"
#include "spritecomponent.h"
#include "room.h"

Wall::Wall(Game* game, Vector2 relativePos, Room* parent)
	:Actor(game)
	,mRelativePosition(relativePos)
	,mParent(parent)
{
	mSC = new SpriteComponent(this);
	mSC->SetTexture(mGame->GetTexture("assets/debug/white.png"));
	SetPosition(mRelativePosition + mParent->GetPosition());

	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(32, 32);
}

void Wall::SetCollisionComponent(Vector2 size)
{
	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(size.x, size.y);
}