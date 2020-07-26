#include "tile.h"
#include "game.h"

Tile::Tile(Game* game)
	:Actor(game)
{
	SetScale(2.0f);
	mSpriteComponent = new SpriteComponent(this);
}