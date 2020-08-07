#include "tile.h"
#include "game.h"
#include "collisioncomponent.h"

Tile::Tile(Game* game)
	:Actor(game)
{
	SetScale(2.0f);
	mSpriteComponent = new SpriteComponent(this);
}

void Tile::SetCollisionComponent(Vector2 size)
{
	mCollisionComponent = new CollisionComponent(this);
	mCollisionComponent->SetSize(size.x, size.y);
}