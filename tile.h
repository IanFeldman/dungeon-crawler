#pragma once
#include "actor.h"

class Tile : public Actor
{
public:
	Tile(class Game* game);

private:
	class SpriteComponent* mSpriteComponent;
};