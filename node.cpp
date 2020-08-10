#include "node.h"
#include "game.h"
#include "collisioncomponent.h"
#include "spritecomponent.h"

Node::Node(Game* game)
	:Actor(game)
	,mPlaced(false)
	,mParentRoom(nullptr)
{
	mSpriteComponent = new SpriteComponent(this, 100);
	mSpriteComponent->SetTexture(mGame->GetTexture("assets/debug/red.png"));
}