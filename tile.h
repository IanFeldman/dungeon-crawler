#pragma once
#include "actor.h"

class Tile : public Actor
{
public:
	Tile(class Game* game);
	void SetCollisionComponent(Vector2 size);

private:
	class SpriteComponent* mSpriteComponent;
	class CollisionComponent* mCollisionComponent;
};