#pragma once
#include "actor.h"

class Wall : public Actor
{
public:
	Wall(class Game* game, Vector2 relativePos, class Room* parent);
	void SetCollisionComponent(Vector2 size);
	Vector2 GetRelativePos() { return mRelativePosition; }

protected:
	class CollisionComponent* mCollisionComponent;
	class SpriteComponent* mSC;
	Vector2 mRelativePosition; // in world units
	class Room* mParent;
};