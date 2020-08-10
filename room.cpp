#include "room.h"
#include "game.h"
#include "spritecomponent.h"
#include "collisioncomponent.h"

Room::Room(Game* game, Vector2 size, const char* fileName)
	:Actor(game)
	,mSize(size)
	,mFileName(fileName)
{
	mSpriteComponent = new SpriteComponent(this);

	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(mSize.x * mGame->GetCsvSize().x, mSize.y * mGame->GetCsvSize().y);
}